/*
 * TilEm II
 *
 * Copyright (c) 2010-2011 Thibault Duponchelle
 * Copyright (c) 2010-2011 Benjamin Moody
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>

#include <tilem.h>
#include <z80.h>
#include <scancodes.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <ticalcs.h>
#include <gui.h>



/* Test if the calc is ready */
int is_ready(CalcHandle* h)
{
         int err;
	  
         err = ticalcs_calc_isready(h);
         printf("Hand-held is %sready !\n", err ? "not " : "");
		    
         return err;
}

/* Print the error (libtis error) */
void print_lc_error(int errnum)
{
	char *msg;

        ticables_error_get(errnum, &msg);
	fprintf(stderr, "Link cable error (code %i)...\n<<%s>>\n", errnum, msg);
        free(msg);
}


/* Internal link emulation */

/* Open cable */
static int ilp_open(CableHandle* cbl)
{
	TilemCalcEmulator* emu = cbl->priv;

	g_mutex_lock(emu->calc_mutex);

	if (emu->ilp_active) {
		fprintf(stderr, "INTERNAL ERROR: cable already opened\n");
		g_mutex_unlock(emu->calc_mutex);
		return 1;
	}

	emu->calc->z80.stop_mask &= ~(TILEM_STOP_LINK_READ_BYTE
	                              | TILEM_STOP_LINK_WRITE_BYTE
	                              | TILEM_STOP_LINK_ERROR);

	emu->ilp_active = TRUE;
	emu->ilp_error = FALSE;
	emu->ilp_abort = FALSE;
	emu->ilp_timeout = 0;
	emu->ilp_write_count = 0;
	emu->ilp_read_count = 0;
	tilem_linkport_graylink_reset(emu->calc);

	g_cond_broadcast(emu->calc_wakeup_cond);
	g_mutex_unlock(emu->calc_mutex);
	return 0;
}

/* Close cable */
static int ilp_close(CableHandle* cbl)
{
	TilemCalcEmulator* emu = cbl->priv;

	g_mutex_lock(emu->calc_mutex);

	if (!emu->ilp_active) {
		fprintf(stderr, "INTERNAL ERROR: cable already closed\n");
		g_mutex_unlock(emu->calc_mutex);
		return 1;
	}

	emu->calc->linkport.linkemu = TILEM_LINK_EMULATOR_NONE;
	emu->calc->z80.stop_mask |= (TILEM_STOP_LINK_STATE
	                             | TILEM_STOP_LINK_READ_BYTE
	                             | TILEM_STOP_LINK_WRITE_BYTE
	                             | TILEM_STOP_LINK_ERROR);

	emu->ilp_active = FALSE;
	tilem_linkport_graylink_reset(emu->calc);

	g_cond_broadcast(emu->calc_wakeup_cond);
	g_mutex_unlock(emu->calc_mutex);
	return 0;
}

/* Reset cable */
static int ilp_reset(CableHandle* cbl)
{
	TilemCalcEmulator* emu = cbl->priv;

	g_mutex_lock(emu->calc_mutex);

	emu->ilp_error = FALSE;
	emu->ilp_timeout = 0;
	emu->ilp_write_count = 0;
	emu->ilp_read_count = 0;
	tilem_linkport_graylink_reset(emu->calc);

	g_cond_broadcast(emu->calc_wakeup_cond);
	g_mutex_unlock(emu->calc_mutex);
	return 0;
}

/* Send data to calc */
static int ilp_send(CableHandle* cbl, uint8_t* data, uint32_t count)
{
	TilemCalcEmulator* emu = cbl->priv;
	int status = 0;

	g_mutex_lock(emu->calc_mutex);

	g_cond_broadcast(emu->calc_wakeup_cond);

	emu->calc->linkport.linkemu = TILEM_LINK_EMULATOR_GRAY;

	emu->ilp_timeout = emu->ilp_timeout_max = cbl->timeout * 10;
	emu->ilp_write_queue = data;
	emu->ilp_write_count = count;

	while (emu->ilp_write_count != 0) {
		if (emu->ilp_abort || emu->ilp_error) {
			status = ERROR_WRITE_TIMEOUT;
			break;
		}
		g_cond_wait(emu->ilp_finished_cond, emu->calc_mutex);
	}

	emu->ilp_timeout = 0;

	g_mutex_unlock(emu->calc_mutex);
	return status;
}

/* Receive data from calc */
static int ilp_recv(CableHandle* cbl, uint8_t* data, uint32_t count)
{
	TilemCalcEmulator* emu = cbl->priv;
	int status = 0;

	g_mutex_lock(emu->calc_mutex);

	g_cond_broadcast(emu->calc_wakeup_cond);

	emu->calc->linkport.linkemu = TILEM_LINK_EMULATOR_GRAY;

	emu->ilp_timeout = emu->ilp_timeout_max = cbl->timeout * 10;
	emu->ilp_read_queue = data;
	emu->ilp_read_count = count;

	while (emu->ilp_read_count != 0) {
		if (emu->ilp_abort || emu->ilp_error) {
			status = ERROR_WRITE_TIMEOUT;
			break;
		}
		g_cond_wait(emu->ilp_finished_cond, emu->calc_mutex);
	}

	emu->ilp_timeout = 0;

	g_mutex_unlock(emu->calc_mutex);
	return status;
}

static int ilp_check(CableHandle* cbl, int* status)
{
	TilemCalcEmulator* emu = cbl->priv;

	g_mutex_lock(emu->calc_mutex);

	*status = STATUS_NONE;
	if (emu->calc->linkport.lines)
		*status |= STATUS_RX;
	if (emu->calc->linkport.extlines)
		*status |= STATUS_TX;

	g_mutex_unlock(emu->calc_mutex);
	return 0;
}

CableHandle* internal_link_handle_new(TilemCalcEmulator* emu)
{
	CableHandle* cbl;

	cbl = ticables_handle_new(CABLE_ILP, PORT_0);
	if (!cbl)
		return NULL;

	cbl->priv = emu;
	cbl->cable->open = ilp_open;
	cbl->cable->close = ilp_close;
	cbl->cable->reset = ilp_reset;
	cbl->cable->send = ilp_send;
	cbl->cable->recv = ilp_recv;
	cbl->cable->check = ilp_check;

	return cbl;
}

static int print_tilibs_error(int errcode)
{
	char* p = NULL;
	if (errcode) {
		if (ticalcs_error_get(errcode, &p)
		    && tifiles_error_get(errcode, &p)
		    && ticables_error_get(errcode, &p)) {
			fprintf(stderr, "Unknown error: %d\n", errcode);
		}
		else {
			fprintf(stderr, "%s\n", p);
			g_free(p);
		}
	}
	return errcode;
}

void run_with_key(TilemCalc* calc, int key)
{
	tilem_z80_run_time(calc, 500000, NULL);
	tilem_keypad_press_key(calc, key);
	tilem_z80_run_time(calc, 1000000, NULL);
	tilem_keypad_release_key(calc, key);
	tilem_z80_run_time(calc, 500000, NULL);
}

static void prepare_for_link(TilemCalc* calc)
{
	run_with_key(calc, TILEM_KEY_ON);
	run_with_key(calc, TILEM_KEY_ON);

	if (calc->hw.model_id == TILEM_CALC_TI82) {
		run_with_key(calc, TILEM_KEY_2ND);
		run_with_key(calc, TILEM_KEY_MODE);
		run_with_key(calc, TILEM_KEY_2ND);
		run_with_key(calc, TILEM_KEY_GRAPHVAR);
		run_with_key(calc, TILEM_KEY_RIGHT);
		run_with_key(calc, TILEM_KEY_ENTER);
	}
	else if (calc->hw.model_id == TILEM_CALC_TI85) {
		run_with_key(calc, TILEM_KEY_MODE);
		run_with_key(calc, TILEM_KEY_MODE);
		run_with_key(calc, TILEM_KEY_MODE);
		run_with_key(calc, TILEM_KEY_2ND);
		run_with_key(calc, TILEM_KEY_GRAPHVAR);
		run_with_key(calc, TILEM_KEY_WINDOW);
	}
}

static void tmr_press_key(TilemCalc* calc, void* data)
{
	tilem_keypad_press_key(calc, TILEM_PTR_TO_DWORD(data));
}

void send_file(TilemCalcEmulator* emu, CalcHandle* ch, const char* filename)
{
	CalcScreenCoord sc;
	uint8_t *bitmap = NULL;
	int tmr, k, err;
	FileContent* filec;
		
	g_mutex_lock(emu->calc_mutex);
	prepare_for_link(emu->calc);
	g_cond_broadcast(emu->calc_wakeup_cond);
	g_mutex_unlock(emu->calc_mutex);

	switch (tifiles_file_get_class(filename)) {
	case TIFILE_SINGLE:
	case TIFILE_GROUP:
	case TIFILE_REGULAR:
		filec = tifiles_content_create_regular(ch->model);
		err = tifiles_file_read_regular(filename, filec);
		if ( err )
		{
			print_tilibs_error(err);
			tifiles_content_delete_regular(filec);
		}
		//mode = MODE_SEND_LAST_VAR;
		//mode = MODE_NORMAL;

		err = ticalcs_calc_send_var(ch, MODE_NORMAL, filec);
		tifiles_content_delete_regular(filec);
		break;	
		
		//print_tilibs_error(ticalcs_calc_send_var2(ch, MODE_NORMAL , filename));
		//break;

	case TIFILE_BACKUP:
		/* press enter to accept backup */
		if (emu->calc->hw.model_id == TILEM_CALC_TI85
		    || emu->calc->hw.model_id == TILEM_CALC_TI86) {
			k = TILEM_KEY_YEQU;
		}
		else {
			k = TILEM_KEY_ENTER;
		}

		tmr = tilem_z80_add_timer(emu->calc, 100, 0, 1, tmr_press_key, TILEM_DWORD_TO_PTR(k));
		//while(is_ready(ch)){ }
		print_tilibs_error(ticalcs_calc_send_backup2(ch, filename));
		tilem_z80_remove_timer(emu->calc, tmr);
		tilem_keypad_release_key(emu->calc, k);

		break;

	case TIFILE_FLASH:
	case TIFILE_OS:
	case TIFILE_APP:
		if (tifiles_file_is_os(filename))
			print_tilibs_error(ticalcs_calc_send_os2(ch, filename));
		else if (tifiles_file_is_app(filename))
			print_tilibs_error(ticalcs_calc_send_app2(ch, filename));
		break;

	case TIFILE_TIGROUP:
		print_tilibs_error(ticalcs_calc_send_tigroup2(ch, filename, TIG_ALL));
		break;

	default:
		if (1) {
			print_tilibs_error(ticalcs_calc_send_key(ch, 0xA0));
		} else {
			print_tilibs_error(ticalcs_calc_recv_screen(ch, &sc, &bitmap));
			g_free(bitmap);
		}
		break;
	}
}

int get_calc_model(TilemCalc* calc)
{
	switch (calc->hw.model_id) {
	case TILEM_CALC_TI73:
		return CALC_TI73;

	case TILEM_CALC_TI81:
	case TILEM_CALC_TI82:
		return CALC_TI82;

	case TILEM_CALC_TI83:
		return CALC_TI83;

	case TILEM_CALC_TI83P:
	case TILEM_CALC_TI83P_SE:
		return CALC_TI83P;

	case TILEM_CALC_TI84P:
	case TILEM_CALC_TI84P_SE:
		return CALC_TI84P;

	case TILEM_CALC_TI85:
		return CALC_TI85;

	case TILEM_CALC_TI86:
		return CALC_TI86;

	default:
		return CALC_NONE;
	}
}

/* Link thread main loop */
static gpointer link_main(gpointer data)
{
	TilemCalcEmulator *emu = data;
	char *fname;
	gfloat progress = 0;
	CalcHandle *ch;
	CableHandle *cbl;

	ticables_library_init();
	tifiles_library_init();
	ticalcs_library_init();

	cbl = internal_link_handle_new(emu);

	g_mutex_lock(emu->link_queue_mutex);
	while (!emu->link_cancel) {
		if (!(fname = g_queue_pop_head(emu->link_queue))) {
			g_cond_wait(emu->link_queue_cond, emu->link_queue_mutex);
			continue;
		}
		g_mutex_unlock(emu->link_queue_mutex);

		ch = ticalcs_handle_new(get_calc_model(emu->calc));
		if (!ch) {
			fprintf(stderr, "INTERNAL ERROR: unsupported calc\n");
			g_free(fname);
			break;
		}
		progress = progress + 0.1;	
		//gtk_main_iteration();
		progress_bar_update(emu, progress) ;
		ticalcs_update_set(ch, emu->link_update);

		ticalcs_cable_attach(ch, cbl);

		send_file(emu, ch, fname);

		ticalcs_cable_detach(ch);
		ticalcs_handle_del(ch);

		g_free(fname);

		g_mutex_lock(emu->link_queue_mutex);
	}
	g_mutex_unlock(emu->link_queue_mutex);

	ticables_handle_del(cbl);

	ticalcs_library_exit();
	tifiles_library_exit();
	ticables_library_exit();
	on_destroy_progress_win(emu->progress_win);

	return NULL;
}

void tilem_calc_emulator_send_file(TilemCalcEmulator *emu,
                                   const char *filename)
{
	progress_bar_init(emu);
	g_return_if_fail(emu != NULL);
	g_return_if_fail(emu->calc != NULL);
	g_return_if_fail(filename != NULL);

	g_mutex_lock(emu->link_queue_mutex);
	g_queue_push_tail(emu->link_queue, g_strdup(filename));
	g_cond_broadcast(emu->link_queue_cond);
	g_mutex_unlock(emu->link_queue_mutex);

	if (!emu->link_thread)
		emu->link_thread = g_thread_create(&link_main, emu, TRUE, NULL);
}

void tilem_calc_emulator_cancel_link(TilemCalcEmulator *emu)
{
	char *fname;
	CalcUpdate *update;

	g_return_if_fail(emu != NULL);

	if (!emu->link_thread)
		return;

	/* remove any queued files that haven't yet been sent, and notify
	   link thread that it should exit */
	g_mutex_lock(emu->link_queue_mutex);
	emu->link_cancel = 1;
	while ((fname = g_queue_pop_head(emu->link_queue)))
		g_free(fname);
	g_cond_broadcast(emu->link_queue_cond);
	g_mutex_unlock(emu->link_queue_mutex);

	/* notify ticalcs that the operation is to be aborted */
	update = emu->link_update;
	update->cancel = 1;

	/* cancel any ongoing transfer */
	g_mutex_lock(emu->calc_mutex);
	emu->ilp_abort = TRUE;
	g_cond_broadcast(emu->ilp_finished_cond);
	g_mutex_unlock(emu->calc_mutex);

	/* wait for link thread to exit */
	g_thread_join(emu->link_thread);
	emu->link_thread = NULL;

	update->cancel = 0;
}


void progress_bar_init(TilemCalcEmulator* emu) {
	create_progress_window(emu);
}

void progress_bar_update(TilemCalcEmulator* emu, gfloat percentage) {
	gtk_progress_bar_update(GTK_PROGRESS_BAR(emu->ilp_progress_bar), percentage );


}

void on_destroy_progress_win(GtkWidget* progress_win)   {
	gtk_widget_destroy(GTK_WIDGET(progress_win));
}

void create_progress_window(TilemCalcEmulator* emu) {
	emu->ilp_progress_bar = gtk_progress_bar_new();
	emu->progress_win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(emu->progress_win), "Progress");
	gtk_window_set_default_size(GTK_WINDOW(emu->progress_win), 400,30);
	
	gtk_signal_connect(GTK_OBJECT(emu->progress_win), "delete-event", G_CALLBACK(on_destroy_progress_win), NULL);

	gtk_container_add(GTK_CONTAINER(emu->progress_win), emu->ilp_progress_bar);

		
	gtk_widget_show_all(emu->progress_win);
}
