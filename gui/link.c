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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <ticalcs.h>
#include <ticonv.h>
#include <string.h>
#include <tilem.h>
#include <scancodes.h>

#include "gui.h"
#include "ti81prg.h"

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

	if (emu->ilp.active) {
		fprintf(stderr, "INTERNAL ERROR: cable already opened\n");
		g_mutex_unlock(emu->calc_mutex);
		return 1;
	}

	emu->calc->z80.stop_mask &= ~(TILEM_STOP_LINK_READ_BYTE
	                              | TILEM_STOP_LINK_WRITE_BYTE
	                              | TILEM_STOP_LINK_ERROR);

	emu->ilp.active = TRUE;
	emu->ilp.error = FALSE;
	emu->ilp.abort = FALSE;
	emu->ilp.timeout = 0;
	emu->ilp.write_count = 0;
	emu->ilp.read_count = 0;
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

	if (!emu->ilp.active) {
		fprintf(stderr, "INTERNAL ERROR: cable already closed\n");
		g_mutex_unlock(emu->calc_mutex);
		return 1;
	}

	emu->calc->linkport.linkemu = TILEM_LINK_EMULATOR_NONE;
	emu->calc->z80.stop_mask |= (TILEM_STOP_LINK_STATE
	                             | TILEM_STOP_LINK_READ_BYTE
	                             | TILEM_STOP_LINK_WRITE_BYTE
	                             | TILEM_STOP_LINK_ERROR);

	emu->ilp.active = FALSE;
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

	emu->ilp.error = FALSE;
	emu->ilp.timeout = 0;
	emu->ilp.write_count = 0;
	emu->ilp.read_count = 0;
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

	emu->ilp.timeout = emu->ilp.timeout_max = cbl->timeout * 10;
	emu->ilp.write_queue = data;
	emu->ilp.write_count = count;

	while (emu->ilp.write_count != 0) {
		if (emu->ilp.abort || emu->ilp.error) {
			status = ERROR_WRITE_TIMEOUT;
			break;
		}
		g_cond_wait(emu->ilp.finished_cond, emu->calc_mutex);
	}

	emu->ilp.timeout = 0;

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

	emu->ilp.timeout = emu->ilp.timeout_max = cbl->timeout * 10;
	emu->ilp.read_queue = data;
	emu->ilp.read_count = count;

	while (emu->ilp.read_count != 0) {
		if (emu->ilp.abort || emu->ilp.error) {
			status = ERROR_WRITE_TIMEOUT;
			break;
		}
		g_cond_wait(emu->ilp.finished_cond, emu->calc_mutex);
	}

	emu->ilp.timeout = 0;

	g_mutex_unlock(emu->calc_mutex);
	return status;
}

/* Check if ready */
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

/* Print the tilibs error */
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

/* Run a key (wait, press, wait; release; wait) */
void run_with_key(TilemCalc* calc, int key)
{
	tilem_z80_run_time(calc, 500000, NULL);
	tilem_keypad_press_key(calc, key);
	tilem_z80_run_time(calc, 1000000, NULL);
	tilem_keypad_release_key(calc, key);
	tilem_z80_run_time(calc, 500000, NULL);
}

/* Automatically press key to be in the receive mode (ti82 and ti85) */
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
/* Press a key */
static void tmr_press_key(TilemCalc* calc, void* data)
{
	tilem_keypad_press_key(calc, TILEM_PTR_TO_DWORD(data));
}

/* Send a file to the calc */
void send_file(TilemCalcEmulator* emu, CableHandle *cbl, const char* filename)
{
	CalcHandle* ch;
	CalcScreenCoord sc;
	uint8_t *bitmap = NULL;
	int tmr, k, err;
	FileContent* filec;
	TI81Program* prgm = NULL;
	FILE* f;

	g_mutex_lock(emu->calc_mutex);
	prepare_for_link(emu->calc);
	g_cond_broadcast(emu->calc_wakeup_cond);
	g_mutex_unlock(emu->calc_mutex);

	if (emu->calc->hw.model_id == TILEM_CALC_TI81) {
		f = g_fopen(filename, "rb");
		if (!f) {
			perror(filename);
			return;
		}

		ti81_read_prg_file(f, &prgm);
		fclose(f);

		if (prgm) {
			g_mutex_lock(emu->calc_mutex);
			ti81_load_program(emu->calc, prgm);
			g_mutex_unlock(emu->calc_mutex);
			ti81_program_free(prgm);
		}
		return;
	}

	ch = ticalcs_handle_new(get_calc_model(emu->calc));
	if (!ch) {
		fprintf(stderr, "INTERNAL ERROR: unsupported calc\n");
		return;
	}

	ticalcs_update_set(ch, emu->link_update);
	ticalcs_cable_attach(ch, cbl);
		
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

	ticalcs_cable_detach(ch);
	ticalcs_handle_del(ch);
}

/* Get the calc model (compatible for ticalcs) */
int get_calc_model(TilemCalc* calc)
{
	switch (calc->hw.model_id) {
	case TILEM_CALC_TI73:
		return CALC_TI73;

	case TILEM_CALC_TI81:
	case TILEM_CALC_TI82:
		return CALC_TI82;

	case TILEM_CALC_TI83:
	case TILEM_CALC_TI76:
		return CALC_TI83;

	case TILEM_CALC_TI83P:
	case TILEM_CALC_TI83P_SE:
		return CALC_TI83P;

	case TILEM_CALC_TI84P:
	case TILEM_CALC_TI84P_SE:
	case TILEM_CALC_TI84P_NSPIRE:
		return CALC_TI84P;

	case TILEM_CALC_TI85:
		return CALC_TI85;

	case TILEM_CALC_TI86:
		return CALC_TI86;

	default:
		return CALC_NONE;
	}
}

/* idle callback to start progress bar */
static gboolean pbar_start(gpointer data)
{
	TilemCalcEmulator *emu = data;

	if (!emu->linkpb->ilp_progress_win)
		progress_bar_init(emu);

	return FALSE;
}

/* idle callback to close progress bar */
static gboolean pbar_stop(gpointer data)
{
	TilemCalcEmulator *emu = data;

	if (emu->linkpb->ilp_progress_win) {
		gtk_widget_destroy(emu->linkpb->ilp_progress_win);
		emu->linkpb->ilp_progress_win = NULL;
		emu->linkpb->ilp_progress_bar1 = NULL;
		emu->linkpb->ilp_progress_bar2 = NULL;
		emu->linkpb->ilp_progress_label = NULL;
	}

	return FALSE;
}

/* idle callback to update progress bar */
static gboolean pbar_update(gpointer data)
{
	TilemCalcEmulator *emu = data;

	if (emu->linkpb->ilp_progress_win)
		progress_bar_update_activity(emu);

	return FALSE;
}

static GStaticPrivate current_emu_key = G_STATIC_PRIVATE_INIT;

/* ticalcs progress bar callback */
static void pbar_do_update()
{
	TilemCalcEmulator *emu = g_static_private_get(&current_emu_key);
	g_idle_add(&pbar_update, emu);
}

/* Link thread main loop */
static gpointer link_main(gpointer data)
{
	TilemCalcEmulator *emu = data;
	char *fname;
	CableHandle *cbl;

	ticables_library_init();
	tifiles_library_init();
	ticalcs_library_init();

	cbl = internal_link_handle_new(emu);

	g_static_private_set(&current_emu_key, emu, NULL);

	g_mutex_lock(emu->link_queue_mutex);
	while (!emu->link_cancel) {
		if (!(fname = g_queue_pop_head(emu->link_queue))) {
			g_cond_wait(emu->link_queue_cond, emu->link_queue_mutex);
			continue;
		}
		g_mutex_unlock(emu->link_queue_mutex);

		emu->link_update->max1 = 0;
		emu->link_update->max2 = 0;
		emu->link_update->text[0] = 0;

		g_idle_add(&pbar_start, emu);

		emu->link_update->pbar = &pbar_do_update;
		emu->link_update->label = &pbar_do_update;

		send_file(emu, cbl, fname);

		g_free(fname);

		g_mutex_lock(emu->link_queue_mutex);

		if (g_queue_is_empty(emu->link_queue))
			g_idle_add(&pbar_stop, emu);
	}
	g_mutex_unlock(emu->link_queue_mutex);

	ticables_handle_del(cbl);

	ticalcs_library_exit();
	tifiles_library_exit();
	ticables_library_exit();

	return NULL;
}

/* Send a file creating an using a separate thread */
void tilem_calc_emulator_send_file(TilemCalcEmulator *emu,
                                   const char *filename)
{
	g_return_if_fail(emu != NULL);
	g_return_if_fail(emu->calc != NULL);
	g_return_if_fail(filename != NULL);

	emu->ilp.abort = FALSE;
	emu->link_cancel = FALSE;

	g_mutex_lock(emu->link_queue_mutex);
	g_queue_push_tail(emu->link_queue, g_strdup(filename));
	g_cond_broadcast(emu->link_queue_cond);
	g_mutex_unlock(emu->link_queue_mutex);

	if (!emu->link_thread)
		emu->link_thread = g_thread_create(&link_main, emu, TRUE, NULL);
}

/* Cancel the transfert (used as signal callback) */
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
	emu->ilp.abort = TRUE;
	g_cond_broadcast(emu->ilp.finished_cond);
	g_mutex_unlock(emu->calc_mutex);

	/* wait for link thread to exit */
	g_thread_join(emu->link_thread);
	emu->link_thread = NULL;

	pbar_stop(emu);

	update->cancel = 0;
}

void print_child(G_GNUC_UNUSED GNode *vars, G_GNUC_UNUSED gpointer data) {

	printf("print child \n");

}



/* Display the list of vars/app */		
/* This is a modified version of get_dirlist (ticalc) 
 * because tiz80 never have folders (can you confirm that Benjamin...?) 
 */
void tilem_dirlist_display(GNode* tree)
{
	GNode *vars = tree;
	TreeInfo *info = (TreeInfo *)(tree->data);
	int i, j, k;
	char *utf8;
  
	if (tree == NULL)
		return;

	printf(  "+------------------+----------+----------+\n");
	printf(  "| B. name          | T. name  | Size     |\n");
	printf(  "+------------------+----------+----------+\n");
	i = 0;

	GNode *parent = g_node_nth_child(vars, i);


	for (j = 0; j < (int)g_node_n_children(parent); j++)	//parse variables
	{
		GNode *child = g_node_nth_child(parent, j);
		VarEntry *ve = (VarEntry *) (child->data);

		utf8 = ticonv_varname_to_utf8(info->model, ve->name, ve->type);

	printf("| ");
	for (k = 0; k < 8; k++) 
		printf("%02X", (uint8_t) (ve->name)[k]);
	printf(" | ");
	printf("%8s", utf8);
	printf(" | ");
	printf("%08X", ve->size);
	printf(" | ");
		printf("\n");

		g_free(utf8);
	}
	printf("+------------------+----------+----------+");
	printf("\n");
}

/* Get the list of varname. I plan to use it into a list (in a menu) */
char ** tilem_get_dirlist(GNode* tree)
{
	GNode *vars = tree;
	TreeInfo *info = (TreeInfo *)(tree->data);
	int i, j;
	char *utf8;
  
	if (tree == NULL)
		return NULL;

	i = 0;
	GNode *parent = g_node_nth_child(vars, i);
		
	char ** list = g_new(char*, g_node_n_children(parent));
	//printf("Number of children : %d\n", g_node_n_children(parent));


	for (j = 0; j < (int)g_node_n_children(parent); j++)	//parse variables
	{
		GNode *child = g_node_nth_child(parent, j);
		VarEntry *ve = (VarEntry *) (child->data);

		utf8 = ticonv_varname_to_utf8(info->model, ve->name, ve->type);

		list[j] = g_strdup(utf8);
		g_free(utf8);
	}
	printf("\n");
	return list;
}

/* Get the list of varname. I plan to use it into a list (in a menu) */
gint tilem_get_dirlist_size(GNode* tree)
{
	GNode *vars = tree;
	int i = 0;
	
	GNode *parent = g_node_nth_child(vars, i);
		
	return g_node_n_children(parent);

}


/* Print the var list and apps list */
void get_dirlist(CalcHandle *ch) {
	GNode *vars, *apps;
	ticalcs_calc_get_dirlist(ch, &vars, &apps);
	g_node_children_foreach(vars, G_TRAVERSE_ALL, print_child, vars);
        //ticalcs_dirlist_display(vars);
        tilem_dirlist_display(vars);
        char ** list = tilem_get_dirlist(vars);
	int i=0;
	for(i = 0; list[i]; i++)
		printf("#%d : %s\n", i, list[i]);
	printf("Number of children : %d\n", tilem_get_dirlist_size(vars));
        //ticalcs_dirlist_display(apps);
        //ticalcs_dirlist_destroy(&vars);
        //ticalcs_dirlist_destroy(&apps);
}

/* Get var from calc to PC */
/* This function should really use a separate thread because it freeze the calc
 * a long time. 
 */
void get_var(TilemCalcEmulator *emu)
{
	CableHandle* cbl;
	CalcHandle* ch;
	
	/* Init the libtis */
	ticables_library_init();
	tifiles_library_init();
	ticalcs_library_init();
	
	/* Create cable (here an internal an dvirtual cabla) */
	cbl = internal_link_handle_new(emu);
	if (!cbl) 
		fprintf(stderr, "Cannot create ilp handle\n");
	

	ch = ticalcs_handle_new(get_calc_model(emu->calc));
	if (!ch) {
		fprintf(stderr, "INTERNAL ERROR: unsupported calc\n");
		return;
	}

	ticalcs_cable_attach(ch, cbl);
	get_dirlist(ch);	
	//receive_var(ch);
	

	ticalcs_cable_detach(ch);
	ticalcs_handle_del(ch);

	ticables_handle_del(cbl);

	/* Exit the libtis */
	ticalcs_library_exit();
	tifiles_library_exit();
	ticables_library_exit();

}

/* Get a var from calc and save it into filename on PC */
int receive_var(CalcHandle * ch) {

	/* Currently, this code is based on romain lievins example */
	char filename[1024] = ""; 
        int ret;
	VarEntry ve ;
	FileContent* filec;

	filec = tifiles_content_create_regular(ch->model);

        printf("Enter filename: ");
        ret = scanf("%1023s", filename);
        if(ret < 1)
                return 0;

        printf("Enter variable name: ");
        ret = scanf("%1023s", ve.name);
        if(ret < 1)
                return 0;

        ticalcs_calc_recv_var(ch, MODE_NORMAL, filec, &ve);	
	tifiles_file_write_regular(filename, filec, NULL);

	return 0;
}

