#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <readline/readline.h>

#include <tilem.h>
#include <z80.h>
#include <scancodes.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <ticalcs.h>

typedef struct _TilemCalcEmulator {
	GMutex* run_mutex;
	gboolean exiting;
	gboolean forcebreak;

	GMutex* calc_mutex;
	TilemCalc* calc;

	GMutex* lcd_mutex;
	guchar* lcdlevel;
	guchar* lcdimage;
	GdkColor lcdfg, lcdbg;
	GdkPixbuf* lcdpb;
	GdkPixbuf* lcdscaledpb;

	GtkWidget* lcdwin;
} TilemCalcEmulator;

/* Memory management */

void tilem_free(void* p)
{
	g_free(p);
}

void* tilem_malloc(size_t s)
{
	return g_malloc(s);
}

void* tilem_realloc(void* p, size_t s)
{
	return g_realloc(p, s);
}

void* tilem_try_malloc(size_t s)
{
	return g_try_malloc(s);
}

void* tilem_malloc0(size_t s)
{
	return g_malloc0(s);
}

void* tilem_try_malloc0(size_t s)
{
	return g_try_malloc0(s);
}

void* tilem_malloc_atomic(size_t s)
{
	return g_malloc(s);
}

void* tilem_try_malloc_atomic(size_t s)
{
	return g_try_malloc(s);
}

/* Logging */

void tilem_message(TilemCalc* calc, const char* msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	fprintf(stderr, "x%c: ", calc->hw.model_id);
	vfprintf(stderr, msg, ap);
	fputc('\n', stderr);
	va_end(ap);
}

void tilem_warning(TilemCalc* calc, const char* msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	fprintf(stderr, "x%c: WARNING: ", calc->hw.model_id);
	vfprintf(stderr, msg, ap);
	fputc('\n', stderr);
	va_end(ap);
}

void tilem_internal(TilemCalc* calc, const char* msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	fprintf(stderr, "x%c: INTERNAL ERROR: ", calc->hw.model_id);
	vfprintf(stderr, msg, ap);
	fputc('\n', stderr);
	va_end(ap);
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
	emu->calc->z80.stop_mask = ~(TILEM_STOP_LINK_READ_BYTE
				      | TILEM_STOP_LINK_WRITE_BYTE
				      | TILEM_STOP_LINK_ERROR);

	tilem_z80_run_time(emu->calc, 1000, NULL);

	while (count > 0) {
		if (!tilem_linkport_graylink_send_byte(emu->calc, data[0])) {
			data++;
			count--;
		}

		for (i = 0; i < cbl->timeout; i++)
			if (tilem_z80_run_time(emu->calc, 100000, NULL))
				break;

		if (i == cbl->timeout
		    || (emu->calc->z80.stop_reason & TILEM_STOP_LINK_ERROR)) {
			tilem_linkport_graylink_reset(emu->calc);
			status = ERROR_WRITE_TIMEOUT;
			break;
		}
	}

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

	while (count > 0) {
		value = tilem_linkport_graylink_get_byte(emu->calc);

		if (value != -1) {
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

static CableHandle* internal_link_handle_new(TilemCalcEmulator* emu)
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

static void run_with_key(TilemCalc* calc, int key)
{
	tilem_z80_run_time(calc, 200000, NULL);
	tilem_keypad_press_key(calc, key);
	tilem_z80_run_time(calc, 1000000, NULL);
	tilem_keypad_release_key(calc, key);
	tilem_z80_run_time(calc, 200000, NULL);
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
}

static void tmr_press_key(TilemCalc* calc, void* data)
{
	tilem_keypad_press_key(calc, TILEM_PTR_TO_DWORD(data));
}

static void send_file(TilemCalcEmulator* emu, CalcHandle* ch,
		      int first, int last, const char* filename)
{
	CalcScreenCoord sc;
	uint8_t *bitmap = NULL;
	int tmr;

	if (first) {
		prepare_for_link(emu->calc);
	}

	switch (tifiles_file_get_class(filename)) {
	case TIFILE_SINGLE:
	case TIFILE_GROUP:
	case TIFILE_REGULAR:
		print_tilibs_error(ticalcs_calc_send_var2(ch, (last ? MODE_SEND_LAST_VAR : 0), filename));
		break;

	case TIFILE_BACKUP:
		/* press enter to accept backup */
		tmr = tilem_z80_add_timer(emu->calc, 1000000, 0, 1,
					  tmr_press_key,
					  TILEM_DWORD_TO_PTR(9));
		print_tilibs_error(ticalcs_calc_send_backup2(ch, filename));
		tilem_z80_remove_timer(emu->calc, tmr);
		tilem_keypad_release_key(emu->calc, 9);

		if (!last) {
			prepare_for_link(emu->calc);
		}
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
		if (1)
			print_tilibs_error(ticalcs_calc_send_key(ch, 0xA0));
		else {
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


/* LCD */

#define X_FRINGE 2
#define Y_FRINGE 1

static void update_lcdimage(TilemCalcEmulator* emu)
{
	int x, y, i, level;
	int br, bg, bb, dr, dg, db;
	guchar* p;

	br = emu->lcdbg.red;
	bg = emu->lcdbg.green;
	bb = emu->lcdbg.blue;
	dr = emu->lcdfg.red - br;
	dg = emu->lcdfg.green - bg;
	db = emu->lcdfg.blue - bb;

	p = emu->lcdimage;

	for (i = 0; i < Y_FRINGE * (emu->calc->hw.lcdwidth + 2 * X_FRINGE);
	     i++) {
		p[0] = br / 256;
		p[1] = bg / 256;
		p[2] = bb / 256;
		p += 3;
	}

	for (y = 0; y < emu->calc->hw.lcdheight; y++) {
		for (i = 0; i < X_FRINGE; i++) {
			p[0] = br / 256;
			p[1] = bg / 256;
			p[2] = bb / 256;
			p += 3;
		}

		for (x = 0; x < emu->calc->hw.lcdwidth; x++) {
			level = emu->lcdlevel[y * emu->calc->hw.lcdwidth + x];
			p[0] = (br * 128 + dr * level + 16384) / 32768;
			p[1] = (bg * 128 + dg * level + 16384) / 32768;
			p[2] = (bb * 128 + db * level + 16384) / 32768;
			p += 3;
		}

		for (i = 0; i < X_FRINGE; i++) {
			p[0] = br / 256;
			p[1] = bg / 256;
			p[2] = bb / 256;
			p += 3;
		}
	}

	for (i = 0; i < Y_FRINGE * (emu->calc->hw.lcdwidth + 2 * X_FRINGE);
	     i++) {
		p[0] = br / 256;
		p[1] = bg / 256;
		p[2] = bb / 256;
		p += 3;
	}
}

static void printstate(TilemCalcEmulator* emu)
{
	printf("  PC=%02X:%04X AF=%04X BC=%04X DE=%04X"
	       " HL=%04X IX=%04X IY=%04X SP=%04X\n",
	       emu->calc->mempagemap[emu->calc->z80.r.pc.b.h >> 6],
	       emu->calc->z80.r.pc.w.l,
	       emu->calc->z80.r.af.w.l,
	       emu->calc->z80.r.bc.w.l,
	       emu->calc->z80.r.de.w.l,
	       emu->calc->z80.r.hl.w.l,
	       emu->calc->z80.r.ix.w.l,
	       emu->calc->z80.r.iy.w.l,
	       emu->calc->z80.r.sp.w.l);
}

static volatile int sforcebreak = 0;

static void catchint(int sig G_GNUC_UNUSED)
{
	sforcebreak = 1;
}

static const char* bpstring[7] = {0, "read", "exec", "write", "in", "out", "op"};

static gpointer core_thread(gpointer data)
{
	TilemCalcEmulator* emu = data;
	int debugmode = 0;
	TilemZ80Breakpoint* bp;
	char *cmd, *prev = NULL, *end;
	dword addr;
	int width, height;
	guchar* lcddata;
	int x, y;
	int low, high, v, old;

	width = emu->calc->hw.lcdwidth;
	height = emu->calc->hw.lcdheight;
	lcddata = g_new(guchar, (width / 8) * height);

	signal(SIGINT, &catchint);

	while (1) {
		g_mutex_lock(emu->run_mutex);
		if (emu->exiting) {
			g_mutex_unlock(emu->run_mutex);
			g_free(lcddata);
			return NULL;
		}
		if (emu->forcebreak || sforcebreak) {
			printf("Interrupted at %04X\n", emu->calc->z80.clock);
			printstate(emu);
			debugmode = 1;
			emu->forcebreak = FALSE;
			sforcebreak = 0;
		}
		g_mutex_unlock(emu->run_mutex);

		if (debugmode) {
			if ((cmd = readline("> "))) {
				if (!*cmd && prev) {
					free(cmd);
					cmd = prev;
				}
				else {
					if (prev)
						free(prev);
					prev = cmd;
				}

				switch (cmd[0]) {
				case 'c':
					debugmode = 0;
					break;

				case 'r':
					g_mutex_lock(emu->calc_mutex);
					tilem_calc_reset(emu->calc);
					g_mutex_unlock(emu->calc_mutex);
					debugmode = 0;
					break;

				case 's':
					g_mutex_lock(emu->calc_mutex);
					tilem_z80_run(emu->calc, 1, NULL);
					printstate(emu);
					g_mutex_unlock(emu->calc_mutex);
					break;

				case 'b':
					addr = strtol(cmd + 1, &end, 16);
					g_mutex_lock(emu->calc_mutex);
					tilem_z80_add_breakpoint(emu->calc, TILEM_BREAK_MEM_EXEC, addr, addr, 0xFFFF, NULL, 0);
					g_mutex_unlock(emu->calc_mutex);
					printf("Added breakpoint (exec at %04X)\n", addr);
					break;

				case 'w':
					addr = strtol(cmd + 1, &end, 16);
					g_mutex_lock(emu->calc_mutex);
					tilem_z80_add_breakpoint(emu->calc, TILEM_BREAK_MEM_WRITE, addr, addr, 0xFFFF, NULL, 0);
					g_mutex_unlock(emu->calc_mutex);
					printf("Added breakpoint (write at %04X)\n", addr);
					break;

				case 't':
					g_mutex_lock(emu->calc_mutex);
					printf("*** Stack ***\n");
					for (addr = emu->calc->z80.r.sp.w.l;
					     addr < 0x10000;
					     addr += 2) {
						printf(" %04X: ", addr);
						printf("%02X", (*emu->calc->hw.z80_rdmem)(emu->calc, addr + 1));
						printf("%02X\n", (*emu->calc->hw.z80_rdmem)(emu->calc, addr));
					}
					g_mutex_unlock(emu->calc_mutex);
					break;

				default:
					printf("Continue, Reset, Step, sTack, Brkpt, Write-brkpt?\n");
					break;
				}
			}

			continue;
		}

		g_mutex_lock(emu->calc_mutex);

		/*{
			TilemZ80Timer* tmr;
			if (emu->calc->z80.timers_cpu) {
				printf("C:");
				for (tmr = emu->calc->z80.timers_cpu; tmr; tmr = tmr->next) {
					printf(" %X", tmr->count - emu->calc->z80.clock);
				}
				printf("\n");
			}
			if (emu->calc->z80.timers_rt) {
				printf("R:");
				for (tmr = emu->calc->z80.timers_rt; tmr; tmr = tmr->next) {
					printf(" %X", tmr->count - emu->calc->z80.clock);
				}
				printf("\n");
			}
		}*/

		if (tilem_z80_run_time(emu->calc, 10000, NULL)) {
			bp = &emu->calc->z80.breakpoints[emu->calc->z80.stop_breakpoint];
			printf("Breakpoint (%s at %X) at time %04X\n",
			       bpstring[bp->type], bp->start,
			       emu->calc->z80.clock);
			printstate(emu);
			debugmode = 1;
		}

		if (!emu->calc->lcd.active || !emu->calc->lcd.poweron) {
			low = high = 0;
		}
		else if (emu->calc->lcd.contrast < 32) {
			low = 0;
			high = emu->calc->lcd.contrast * 4;
		}
		else {
			low = (emu->calc->lcd.contrast - 32) * 4;
			high = 128;
		}

		(*emu->calc->hw.get_lcd)(emu->calc, lcddata);
		g_mutex_unlock(emu->calc_mutex);

		g_mutex_lock(emu->lcd_mutex);
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				if (lcddata[(y * width + x) / 8]
				    & (0x80 >> (x % 8)))
					v = high;
				else
					v = low;

				old = emu->lcdlevel[y * width + x];
				emu->lcdlevel[y * width + x]
					= v + ((old - v) * 3) / 4;
			}
		}
		g_mutex_unlock(emu->lcd_mutex);

		g_usleep(10000);
	}
}

void screenresize(GtkWidget* w G_GNUC_UNUSED,
		  GtkAllocation* alloc, gpointer data)
{
	TilemCalcEmulator* emu = data;

	g_object_unref(emu->lcdscaledpb);
	emu->lcdscaledpb = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8,
					  alloc->width, alloc->height);
}

void screenrestyle(GtkWidget* w, GtkStyle* oldstyle G_GNUC_UNUSED,
		   gpointer data)
{
	TilemCalcEmulator* emu = data;
	emu->lcdfg = w->style->fg[GTK_STATE_NORMAL];
	emu->lcdbg = w->style->bg[GTK_STATE_NORMAL];
	gtk_widget_queue_draw(emu->lcdwin);
}

gboolean screenrepaint(GtkWidget* w G_GNUC_UNUSED,
		       GdkEventExpose* ev G_GNUC_UNUSED,
		       gpointer data)
{
	TilemCalcEmulator* emu = data;
	double fx, fy;

	fx = ((double) w->allocation.width
	      / (emu->calc->hw.lcdwidth + 2 * X_FRINGE));
	fy = ((double) w->allocation.height
	      / (emu->calc->hw.lcdheight + 2 * Y_FRINGE));

	g_mutex_lock(emu->lcd_mutex);
	update_lcdimage(emu);
	gdk_pixbuf_scale(emu->lcdpb, emu->lcdscaledpb,
			 0, 0, w->allocation.width, w->allocation.height,
			 0.0, 0.0, fx, fy, GDK_INTERP_TILES);
	g_mutex_unlock(emu->lcd_mutex);

	gdk_draw_pixbuf(w->window, w->style->fg_gc[w->state],
			emu->lcdscaledpb, 0, 0, 0, 0,
			w->allocation.width, w->allocation.height,
			GDK_RGB_DITHER_NONE, 0, 0);

	return TRUE;
}

gboolean screenupdate(gpointer data)
{
	TilemCalcEmulator* emu = data;
	gtk_widget_queue_draw(emu->lcdwin);
	return TRUE;
}

void btnbreak(GtkButton* btn G_GNUC_UNUSED, gpointer data)
{
	TilemCalcEmulator* emu = data;

	g_mutex_lock(emu->run_mutex);
	emu->forcebreak = TRUE;
	g_mutex_unlock(emu->run_mutex);
}

void keypad_button_toggle(GtkToggleButton* tb, gpointer data)
{
	TilemCalcEmulator* emu = data;
	gpointer data2 = g_object_get_data(G_OBJECT(tb), "keycode");
	int keycode = GPOINTER_TO_INT(data2);

	g_mutex_lock(emu->calc_mutex);
	if (gtk_toggle_button_get_active(tb))
		tilem_keypad_press_key(emu->calc, keycode);
	else
		tilem_keypad_release_key(emu->calc, keycode);
	g_mutex_unlock(emu->calc_mutex);
}

gboolean keypad_button_press(GtkWidget* w, GdkEventButton* ev,
			     gpointer data G_GNUC_UNUSED)
{
	if (ev->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK))
		return FALSE;

	if (ev->button == 1) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), TRUE);
		return TRUE;
	}
	else {
		gtk_widget_activate(w);
		return TRUE;
	}
}

gboolean keypad_button_release(GtkWidget* w, GdkEventButton* ev,
			       gpointer data G_GNUC_UNUSED)
{
	if (ev->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK))
		return FALSE;

	if (ev->button == 1) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), FALSE);
		return TRUE;
	}
	else {
		return FALSE;
	}
}

struct kpinfo {
	int x, y, code;
	const char* label;
};

GtkWidget* make_keypad_button(TilemCalcEmulator* emu, const struct kpinfo* ki,
			      GtkWidget* lbl)
{
	GtkWidget *button;

	if (ki->label) {
		button = gtk_toggle_button_new_with_label(ki->label);
		lbl = gtk_bin_get_child(GTK_BIN(button));
		gtk_label_set_use_markup(GTK_LABEL(lbl), TRUE);
	}
	else {
		button = gtk_toggle_button_new();
		gtk_container_add(GTK_CONTAINER(button), lbl);
		gtk_widget_show(lbl);
	}

	GTK_WIDGET_UNSET_FLAGS(button, GTK_CAN_DEFAULT);

	gtk_widget_set_name(button, "tilem-key");
	gtk_widget_set_name(lbl, "tilem-key");

	g_object_set_data(G_OBJECT(button), "keycode",
			  GINT_TO_POINTER(ki->code));

	g_signal_connect(button, "toggled",
			 G_CALLBACK(keypad_button_toggle), emu);
	g_signal_connect(button, "button-press-event",
			 G_CALLBACK(keypad_button_press), emu);
	g_signal_connect(button, "button-release-event",
			 G_CALLBACK(keypad_button_release), emu);

	return button;
}

static const struct kpinfo kpinfo_main[] = {
	{ 4, 8, 0x09, "ENTER" },
	{ 4, 7, 0x0A, "+" },
	{ 4, 6, 0x0B, "\342\210\222" },
	{ 4, 5, 0x0C, "\303\227" },
	{ 4, 4, 0x0D, "\303\267" },
	{ 4, 3, 0x0E, "^" },
	{ 4, 2, 0x0F, "CLEAR" },

	{ 3, 8, 0x11, "(\342\210\222)" },
	{ 3, 7, 0x12, "3" },
	{ 3, 6, 0x13, "6" },
	{ 3, 5, 0x14, "9" },
	{ 3, 4, 0x15, ")" },
	{ 3, 3, 0x16, "TAN" },
	{ 3, 2, 0x17, "VARS" },

	{ 2, 8, 0x19, "." },
	{ 2, 7, 0x1A, "2" },
	{ 2, 6, 0x1B, "5" },
	{ 2, 5, 0x1C, "8" },
	{ 2, 4, 0x1D, "(" },
	{ 2, 3, 0x1E, "COS" },
	{ 2, 2, 0x1F, "PRGM" },
	{ 2, 1, 0x20, "STAT" },

	{ 1, 8, 0x21, "0" },
	{ 1, 7, 0x22, "1" },
	{ 1, 6, 0x23, "4" },
	{ 1, 5, 0x24, "7" },
	{ 1, 4, 0x25, "," },
	{ 1, 3, 0x26, "SIN" },
	{ 1, 2, 0x27, "APPS" },
	{ 1, 1, 0x28, "X,T,\342\200\212\316\270,\342\200\212<i>n</i>" },

	{ 0, 8, 0x29, "ON" },
	{ 0, 7, 0x2A, "STO\342\200\211\342\226\266" },
	{ 0, 6, 0x2B, "LN" },
	{ 0, 5, 0x2C, "LOG" },
	{ 0, 4, 0x2D, "X<sup>2</sup>" },
	{ 0, 3, 0x2E, "X<sup>-1</sup>" },
	{ 0, 2, 0x2F, "MATH" },
	{ 0, 1, 0x30, "ALPHA" },

	{ 0, 0, 0x36, "2nd" },
	{ 1, 0, 0x37, "MODE" },
	{ 2, 0, 0x38, "DEL" }};

static const struct kpinfo kpinfo_arrows[] = {
	{ 1, 1, 0x01, NULL },
	{ 0, 0, 0x02, NULL },
	{ 3, 0, 0x03, NULL },
	{ 1, 0, 0x04, NULL }};

static const struct kpinfo kpinfo_topkeys[] = {
	{ 0, -1, 0x35, "Y=" },
	{ 1, -1, 0x34, "WINDOW" },
	{ 2, -1, 0x33, "ZOOM" },
	{ 3, -1, 0x32, "TRACE" },
	{ 4, -1, 0x31, "GRAPH" }};

static const int awidth[4] = { 2, 1, 1, 2 };
static const int aheight[4] = { 1, 2, 2, 1 };
static const int atype[4] = { GTK_ARROW_DOWN, GTK_ARROW_LEFT,
			      GTK_ARROW_RIGHT, GTK_ARROW_UP };

static const char rcstr[] =
	"style \"tilem-key-default\" {\n"
	"  font_name = \"Sans 7\"\n"
	"  GtkButton::inner-border = { 0, 0, 0, 0 }\n"
	"  GtkButton::focus-padding = 0\n"
	"}\n"
	"widget \"*.tilem-key\" style \"tilem-key-default\"\n"
	"style \"tilem-lcd-default\" {\n"
	"  bg[NORMAL] = { 0.74, 0.74, 0.70 }\n"
	"  fg[NORMAL] = { 0.0, 0.0, 0.0 }\n"
	"}\n"
	"widget \"*.tilem-lcd\" style \"tilem-lcd-default\"\n";

int main(int argc, char** argv)
{
	const char* romname = "xp.rom";
	char* savname;
	char calc_id;
	TilemCalcEmulator emu;
	FILE *romfile, *savfile;
	GtkWidget *window, *vbox, *af, *hbox, *tbl, *button1, *button2,
		*tbl2, *arrow;
	GThread *th;
	int screenwidth, screenheight;
	gsize i;
	int j;
	char* p;

	g_thread_init(NULL);
	gtk_init(&argc, &argv);

	gtk_rc_parse_string(rcstr);

	if (argc >= 2) {
		romname = argv[1];
	}

	savname = g_malloc(strlen(romname) + 5);
	strcpy(savname, romname);
	if ((p = strrchr(savname, '.'))) {
		strcpy(p, ".sav");
	}
	else {
		strcat(savname, ".sav");
	}

	emu.run_mutex = g_mutex_new();
	emu.calc_mutex = g_mutex_new();
	emu.lcd_mutex = g_mutex_new();
	emu.exiting = FALSE;
	emu.forcebreak = FALSE;

	romfile = g_fopen(romname, "rb");
	if (!romfile) {
		perror(romname);
		return 1;
	}

	calc_id = tilem_calc_detect_rom(romfile);
	if (!calc_id) {
		fprintf(stderr, "%s: unknown calculator type\n", romname);
		fclose(romfile);
		return 1;
	}

	savfile = g_fopen(savname, "rt");

	emu.calc = tilem_calc_new(calc_id);
	if (!emu.calc) {
		fprintf(stderr, "cannot create calc\n");
		fclose(romfile);
		if (savfile)
			fclose(savfile);
		return 1;
	}
	tilem_calc_load_state(emu.calc, romfile, savfile);

	emu.calc->lcd.emuflags = TILEM_LCD_REQUIRE_DELAY;
	emu.calc->flash.emuflags = TILEM_FLASH_REQUIRE_DELAY;

	fclose(romfile);
	if (savfile)
		fclose(savfile);

	emu.lcdlevel = g_new0(guchar, (emu.calc->hw.lcdwidth
				       * emu.calc->hw.lcdheight));

	screenwidth = emu.calc->hw.lcdwidth + 2 * X_FRINGE;
	screenheight = emu.calc->hw.lcdheight + 2 * Y_FRINGE;
	emu.lcdimage = g_new0(guchar, 3 * screenwidth * screenheight);

	emu.lcdpb = gdk_pixbuf_new_from_data(emu.lcdimage,
					     GDK_COLORSPACE_RGB, FALSE, 8,
					     screenwidth, screenheight,
					     screenwidth * 3,
					     NULL, NULL);

	emu.lcdscaledpb = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8,
					 screenwidth, screenheight);

	if (argc > 2) {
		CableHandle* cbl;
		CalcHandle* ch;

		ticables_library_init();
		tifiles_library_init();
		ticalcs_library_init();

		cbl = internal_link_handle_new(&emu);
		if (!cbl) {
			fprintf(stderr, "Cannot create ilp handle\n");
			return 1;
		}

		ch = ticalcs_handle_new(get_calc_model(emu.calc));
		if (!ch) {
			fprintf(stderr, "Cannot create calc handle\n");
			return 1;
		}

		ticalcs_cable_attach(ch, cbl);

		for (j = 2; j < argc; j++) {
			send_file(&emu, ch, (j == 2), (j == argc - 1),
				  argv[j]);
		}

		ticalcs_cable_detach(ch);
		ticalcs_handle_del(ch);
		ticables_handle_del(cbl);

		ticalcs_library_exit();
		tifiles_library_exit();
		ticables_library_exit();
	}

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	{
		vbox = gtk_vbox_new(FALSE, 6);
		gtk_container_set_border_width(GTK_CONTAINER(vbox), 3);
		{
			af = gtk_aspect_frame_new(NULL, 0.5, 0.5, 1.0, TRUE);
			gtk_frame_set_shadow_type(GTK_FRAME(af),
						  GTK_SHADOW_NONE);
			{
				emu.lcdwin = gtk_drawing_area_new();
				gtk_widget_set_name(emu.lcdwin, "tilem-lcd");

				gtk_widget_set_size_request(emu.lcdwin,
							    screenwidth * 2,
							    screenheight * 2);
				gtk_container_add(GTK_CONTAINER(af),
						  emu.lcdwin);
				gtk_widget_show(emu.lcdwin);
			}
			gtk_box_pack_start(GTK_BOX(vbox), af,
					   TRUE, TRUE, 0);
			gtk_widget_show(af);

			button1 = gtk_button_new_with_label("Break");
			gtk_box_pack_start(GTK_BOX(vbox), button1,
					   FALSE, FALSE, 0);
			gtk_widget_show(button1);

			hbox = gtk_hbox_new(TRUE, 3);
			for (i = 0; i < G_N_ELEMENTS(kpinfo_topkeys); i++) {
				button2 = make_keypad_button(&emu,
							     &kpinfo_topkeys[i],
							     NULL);
				gtk_box_pack_start(GTK_BOX(hbox),
						   button2, TRUE, TRUE, 0);
				gtk_widget_show(button2);
			}
			gtk_box_pack_start(GTK_BOX(vbox), hbox,
					   TRUE, TRUE, 0);
			gtk_widget_show(hbox);

			tbl = gtk_table_new(9, 5, TRUE);
			gtk_table_set_row_spacings(GTK_TABLE(tbl), 3);
			gtk_table_set_col_spacings(GTK_TABLE(tbl), 3);

			for (i = 0; i < G_N_ELEMENTS(kpinfo_main); i++) {
				button2 = make_keypad_button(&emu,
							     &kpinfo_main[i],
							     NULL);
				gtk_table_attach(GTK_TABLE(tbl), button2,
						 kpinfo_main[i].x,
						 kpinfo_main[i].x + 1,
						 kpinfo_main[i].y,
						 kpinfo_main[i].y + 1,
						 GTK_EXPAND | GTK_FILL,
						 GTK_EXPAND | GTK_FILL, 0, 0);
				gtk_widget_show(button2);
			}

			tbl2 = gtk_table_new(2, 4, TRUE);
			gtk_container_set_border_width(GTK_CONTAINER(tbl2), 1);
			gtk_table_set_row_spacings(GTK_TABLE(tbl2), 4);
			gtk_table_set_col_spacings(GTK_TABLE(tbl2), 4);
			for (i = 0; i < G_N_ELEMENTS(kpinfo_arrows); i++) {
				arrow = gtk_arrow_new(atype[i], GTK_SHADOW_NONE);
				button2 = make_keypad_button(&emu,
							     &kpinfo_arrows[i],
							     arrow);
				gtk_table_attach(GTK_TABLE(tbl2), button2,
						 kpinfo_arrows[i].x,
						 kpinfo_arrows[i].x + awidth[i],
						 kpinfo_arrows[i].y,
						 kpinfo_arrows[i].y + aheight[i],
						 GTK_EXPAND | GTK_FILL,
						 GTK_EXPAND | GTK_FILL, 0, 0);
				gtk_widget_show(button2);
			}

			gtk_table_attach(GTK_TABLE(tbl), tbl2, 3, 5, 0, 2,
					 GTK_EXPAND | GTK_FILL,
					 GTK_EXPAND | GTK_FILL, 0, 0);
			gtk_widget_show(tbl2);

			gtk_box_pack_start(GTK_BOX(vbox), tbl,
					   TRUE, TRUE, 0);
			gtk_widget_show(tbl);
		}
		gtk_container_add(GTK_CONTAINER(window), vbox);
		gtk_widget_show(vbox);
	}

	g_signal_connect(emu.lcdwin, "size-allocate",
			 G_CALLBACK(screenresize), &emu);
	g_signal_connect(emu.lcdwin, "expose-event",
			 G_CALLBACK(screenrepaint), &emu);
	g_signal_connect(emu.lcdwin, "style-set",
			 G_CALLBACK(screenrestyle), &emu);
	g_signal_connect(button1, "clicked", G_CALLBACK(btnbreak), &emu);
	g_signal_connect(window, "delete-event", G_CALLBACK(gtk_main_quit), NULL);

	signal(SIGINT, &catchint);

	th = g_thread_create(&core_thread, &emu, TRUE, NULL);

	g_timeout_add(50, screenupdate, &emu);

	gtk_widget_show(window);
	gtk_main();

	g_mutex_lock(emu.run_mutex);
	emu.exiting = TRUE;
	g_mutex_unlock(emu.run_mutex);
	g_thread_join(th);

	romfile = NULL; /*g_fopen(romname, "wb");*/
	savfile = g_fopen(savname, "wt");
	tilem_calc_save_state(emu.calc, romfile, savfile);
	if (romfile)
		fclose(romfile);
	if (savfile)
		fclose(savfile);

	tilem_calc_free(emu.calc);
	g_mutex_free(emu.run_mutex);
	g_mutex_free(emu.calc_mutex);
	g_mutex_free(emu.lcd_mutex);
	g_free(emu.lcdlevel);
	g_free(emu.lcdimage);

	return 0;
}
