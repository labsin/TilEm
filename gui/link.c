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

static int ilp_reset(CableHandle* cbl)
{
	TilemCalcEmulator* emu = cbl->priv;

	g_mutex_lock(emu->calc_mutex);
	tilem_linkport_graylink_reset(emu->calc);
	g_mutex_unlock(emu->calc_mutex);
	return 0;
}

static int ilp_send(CableHandle* cbl, uint8_t* data, uint32_t count)
{
	TilemCalcEmulator* emu = cbl->priv;
	int status = 0;
	unsigned int i;
	dword prevmask;

	g_mutex_lock(emu->calc_mutex);

	emu->calc->linkport.linkemu = TILEM_LINK_EMULATOR_GRAY;
	prevmask = emu->calc->z80.stop_mask;
	emu->calc->z80.stop_mask = ~(TILEM_STOP_LINK_READ_BYTE | TILEM_STOP_LINK_WRITE_BYTE | TILEM_STOP_LINK_ERROR);

	tilem_z80_run_time(emu->calc, 1000, NULL);

	printf(" >>");
	while (count > 0) {
		if (!tilem_linkport_graylink_send_byte(emu->calc, data[0])) {
			printf(" %02X", data[0]);
			data++;
			count--;
		}

		for (i = 0; i < cbl->timeout; i++)
			if (tilem_linkport_graylink_ready(emu->calc)|| tilem_z80_run_time(emu->calc, 100000, NULL))
				break;

		if (i == cbl->timeout || (emu->calc->z80.stop_reason & TILEM_STOP_LINK_ERROR)) {
			tilem_linkport_graylink_reset(emu->calc);
			status = ERROR_WRITE_TIMEOUT;
			break;
		}
	}
	printf("\n");

	emu->calc->linkport.linkemu = TILEM_LINK_EMULATOR_NONE;
	emu->calc->z80.stop_mask = prevmask;

	g_mutex_unlock(emu->calc_mutex);
	return status;
}

static int ilp_recv(CableHandle* cbl, uint8_t* data, uint32_t count)
{
	TilemCalcEmulator* emu = cbl->priv;
	int status = 0;
	int value;
	unsigned int i;
	dword prevmask;

	g_mutex_lock(emu->calc_mutex);

	emu->calc->linkport.linkemu = TILEM_LINK_EMULATOR_GRAY;
	prevmask = emu->calc->z80.stop_mask;
	emu->calc->z80.stop_mask = ~(TILEM_STOP_LINK_READ_BYTE
				      | TILEM_STOP_LINK_WRITE_BYTE
				      | TILEM_STOP_LINK_ERROR);

	tilem_z80_run_time(emu->calc, 1000, NULL);

	printf(" <<");
	while (count > 0) {
		value = tilem_linkport_graylink_get_byte(emu->calc);

		if (value != -1) {
			printf(" %02X", value);
			data[0] = value;
			data++;
			count--;
			if (!count)
				break;
		}

		for (i = 0; i < cbl->timeout; i++)
			if (tilem_z80_run_time(emu->calc, 100000, NULL))
				break;

		if (i == cbl->timeout
		    || (emu->calc->z80.stop_reason & TILEM_STOP_LINK_ERROR)) {
			tilem_linkport_graylink_reset(emu->calc);
			status = ERROR_READ_TIMEOUT;
			break;
		}
	}
	printf("\n");

	emu->calc->linkport.linkemu = TILEM_LINK_EMULATOR_NONE;
	emu->calc->z80.stop_mask = prevmask;

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

