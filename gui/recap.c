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

/* LCD */
#define X_FRINGE 2
#define Y_FRINGE 1

"click" -> keypad_button_press -> gtk_toggle_button_set_active(bouton,true) -> gtk_button_activate
"released" ->gtk_toggle_button_set_active(bouton,false) -> keypad_button_release
"toggled" -> keypad_button_toggle -> tilem_keypad_press_key(emu->calc, keycode) 

"expose-event" -> screenrepaint -> update_lcdimage [décale le curseur je pense]
main -> g_timeout_add -> screenupdate -> gtk_widget_queue_draw [redessine le lcd] 

tilem_z80_run_time(emu->calc, 10000, NULL); /* Absolument necessaire */
(Dans la fonction core_thread)... Demarrage du core?

Pour que l'interface arrive à discuter avec le core, il faut utiliser les click comme le release.
Avec quelques milisec d'ecart.



void show_about(GtkWidget *widget, gpointer data)
{

  GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file("tilem.png", NULL);

  GtkWidget *dialog = gtk_about_dialog_new();
  gtk_about_dialog_set_name(GTK_ABOUT_DIALOG(dialog), "TilEm");
  gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), "2.0"); 
  gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog), 
      "(c) Benjamin Moody\n(c) Thibault Duponchelle\n(c) Luc Bruant\n");
  gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), 
     "TilEm is a TI Linux Emulator.\n It emulates all current z80 models.\n TI73, TI76, TI81, TI82, TI83(+)(SE), TI84+(SE), TI85 and TI86 ;D");
  gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dialog), 
      "http://lpg.ticalc.org/prj_tilem/");
  gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(dialog), pixbuf);
  g_object_unref(pixbuf), pixbuf = NULL;
  gtk_dialog_run(GTK_DIALOG (dialog));
  gtk_widget_destroy(dialog);

}





/* Model IDs */
enum {
	TILEM_CALC_TI73 = '7',	       /* TI-73 / TI-73 Explorer */
	TILEM_CALC_TI76 = 'f',	       /* TI-76.fr */
	TILEM_CALC_TI81 = '1',	       /* TI-81 */
	TILEM_CALC_TI82 = '2',	       /* TI-82 */
	TILEM_CALC_TI83 = '3',	       /* TI-83 / TI-82 STATS [.fr] */
	TILEM_CALC_TI83P = 'p',	       /* TI-83 Plus */
	TILEM_CALC_TI83P_SE = 's',     /* TI-83 Plus Silver Edition */
	TILEM_CALC_TI84P = '4',	       /* TI-84 Plus */
	TILEM_CALC_TI84P_SE = 'z',     /* TI-84 Plus Silver Edition */
	TILEM_CALC_TI84P_NSPIRE = 'n', /* TI-Nspire 84 Plus emulator */
	TILEM_CALC_TI85 = '5',	       /* TI-85 */
	TILEM_CALC_TI86 = '6'	       /* TI-86 */
};


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



void switch_view_buggy(GLOBAL_SKIN_INFOS * gsi) 
{
	if(gsi->view==1) 
	{
		gsi->view=0;
		DLCD_L2_A0("Entering : draw_not_only_lcd...\n");
		GtkWidget *pImage;
		GtkWidget *pAf;
		GtkWidget * pLayout;

		DLCD_L2_A1("draw_not_only_lcd name : %s\n",gsi->si->name);
		skin_unload(gsi->si);
		skin_load(gsi->si,gsi->SkinFileName);
		
		/* Remove the pImage from the pWindow */ 
		gtk_container_remove(GTK_CONTAINER(gsi->pWindow),gsi->pLayout);
		
		pImage=gtk_image_new_from_pixbuf(gsi->si->image);
		pAf=create_draw_area(gsi);
			       
		pLayout=gtk_layout_new(NULL,NULL);
		
		gsi->pLayout=pLayout;
		gtk_widget_show(pAf);
		gtk_layout_put(GTK_LAYOUT(pLayout),pImage,0,0);
		gtk_layout_put(GTK_LAYOUT(pLayout),pAf,gsi->si->lcd_pos.left,gsi->si->lcd_pos.top);
		gtk_container_add(GTK_CONTAINER(gsi->pWindow),gsi->pLayout);
		gtk_window_resize(GTK_WINDOW(gsi->pWindow),gsi->si->width,gsi->si->height);
		g_signal_connect(G_OBJECT(gsi->pWindow),"destroy",G_CALLBACK(on_destroy),NULL); 
		
		/* Connection signal keyboard key press */
		g_signal_connect(gsi->emu->lcdwin, "size-allocate",G_CALLBACK(screen_resize), gsi);
		g_signal_connect(gsi->emu->lcdwin, "style-set", G_CALLBACK(screen_restyle), gsi); 
		gtk_widget_add_events(pLayout, GDK_KEY_RELEASE_MASK); 
		gtk_signal_connect(GTK_OBJECT(pLayout), "key_press_event", G_CALLBACK(keyboard_event), NULL);
		gtk_widget_add_events(pLayout,GDK_BUTTON_PRESS_MASK );
		gtk_signal_connect(GTK_OBJECT(pLayout), "button-press-event", G_CALLBACK(mouse_press_event),gsi);
		gtk_widget_add_events(pLayout, GDK_BUTTON_RELEASE_MASK);	
		gtk_signal_connect(GTK_OBJECT(pLayout), "button-release-event", G_CALLBACK(mouse_release_event), gsi); 
		g_signal_connect(GTK_OBJECT(gsi->emu->lcdwin), "expose-event",G_CALLBACK(screen_repaint), gsi);
		gtk_widget_show_all(gsi->pWindow);	/*display the window and all that it contains.*/
		//g_timeout_add(50, screen_update, gsi->emu);

		DLCD_L2_A0("Exiting : draw_not_only_lcd...\n");
	} else {
		/* Draw ONLY the lcd area */
		gsi->view=1; /* keep in memory we just print lcd */
		DLCD_L2_A0("Entering : draw_only_lcd...\n");
		GtkWidget *pAf;
		
		/* Remove the pImage from the pWindow */ 
		gtk_container_remove(GTK_CONTAINER(gsi->pWindow),gsi->pAf);
		
		int screenwidth=gsi->si->lcd_pos.right-gsi->si->lcd_pos.left;	
		int screenheight=gsi->si->lcd_pos.bottom-gsi->si->lcd_pos.top;
		printf("%d %d", screenwidth, screenheight);
		gtk_window_resize(GTK_WINDOW(gsi->pWindow),screenwidth, screenheight);
		pAf=create_draw_area(gsi);
		gsi->pAf=pAf;
		
		
		//gtk_container_add(GTK_CONTAINER(gsi->pWindow),pAf);
		gtk_container_add(GTK_CONTAINER(gsi->pWindow),pAf);
		gtk_widget_show(pAf);
		g_signal_connect(G_OBJECT(gsi->pWindow),"destroy",G_CALLBACK(on_destroy),NULL); 
		
		/* Connection signal keyboard key press */
		g_signal_connect(gsi->emu->lcdwin, "size-allocate",G_CALLBACK(screen_resize), gsi);
		g_signal_connect(gsi->emu->lcdwin, "style-set", G_CALLBACK(screen_restyle), gsi); 
		gtk_widget_add_events(gsi->pWindow, GDK_KEY_RELEASE_MASK); // Get the event on the window (leftclick, rightclick)
		gtk_signal_connect(GTK_OBJECT(gsi->pWindow), "key_press_event", G_CALLBACK(keyboard_event), NULL);
		gtk_widget_add_events(gsi->pWindow,GDK_BUTTON_PRESS_MASK );
		gtk_signal_connect(GTK_OBJECT(gsi->pWindow), "button-press-event", G_CALLBACK(mouse_press_event),gsi);
		gtk_widget_add_events(gsi->pWindow, GDK_BUTTON_RELEASE_MASK);	
		gtk_signal_connect(GTK_OBJECT(gsi->pWindow), "button-release-event", G_CALLBACK(mouse_release_event), gsi); 
		g_signal_connect(GTK_OBJECT(gsi->emu->lcdwin), "expose-event",G_CALLBACK(screen_repaint), gsi);
		gtk_widget_show_all(gsi->pWindow);	/*display the window and all that it contains.*/
		//g_timeout_add(50, screen_update, gsi->emu);
		
		DLCD_L2_A0("Exiting : draw_only_lcd...\n");
	}

}


/* Dans cette fonction, on crée l'ecran emu->lcd_image */
/* Chaque boucle correspond à une partie de l'écran */
static void update_lcdimage(TilemCalcEmulator* emu)   /* Absolument necessaire */
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

		if (!emu->calc->lcd.active
		    || (emu->calc->z80.halted && !emu->calc->poweronhalt)) {
			low = high = 0;
		}
		else {
			(*emu->calc->hw.get_lcd)(emu->calc, lcddata);
			if (emu->calc->lcd.contrast < 32) {
				low = 0;
				high = emu->calc->lcd.contrast * 4;
			}
			else {
				low = (emu->calc->lcd.contrast - 32) * 4;
				high = 128;
			}
		}

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

	fx = ((double) w->allocation.width/ (emu->calc->hw.lcdwidth + 2 * X_FRINGE));
	fy = ((double) w->allocation.height/ (emu->calc->hw.lcdheight + 2 * Y_FRINGE));

	g_mutex_lock(emu->lcd_mutex);
	update_lcdimage(emu);
	gdk_pixbuf_scale(emu->lcdpb, emu->lcdscaledpb,0, 0, w->allocation.width, w->allocation.height,0.0, 0.0, fx, fy, GDK_INTERP_TILES);
	g_mutex_unlock(emu->lcd_mutex);

	gdk_draw_pixbuf(w->window, w->style->fg_gc[w->state],emu->lcdscaledpb, 0, 0, 0, 0,w->allocation.width, w->allocation.height,GDK_RGB_DITHER_NONE, 0, 0);

	return TRUE;
}

gboolean screenupdate(gpointer data)
{
	TilemCalcEmulator* emu = data;
	gtk_widget_queue_draw(emu->lcdwin);
	return TRUE;
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

	calc_id = tilem_guess_rom_type(romfile);
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

	printf(">>> %s\n", emu.calc->hw.desc);

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

	if (emu.calc->hw.flags & TILEM_CALC_HAS_FLASH)
		romfile = g_fopen(romname, "wb");
	else
		romfile = NULL;

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


GLOBAL_SKIN_INFOS* redraw_screen(GtkWidget *pWindow,GLOBAL_SKIN_INFOS * gsi) 
{
	
	D ("Entering : redraw_screen...\n");
	GtkWidget *pImage;
	GtkWidget *pAf;
	GtkWidget * pLayout;
	/*gtk_stop();*/
	D2("REDRAW_SCREEN name : %s\n",gsi->si->name);
		skin_unload(gsi->si);
		skin_load(gsi->si,gsi->SkinFileName);
	
	/* Remove the pImage from the pWindow */ 
	gtk_container_remove(GTK_CONTAINER(gsi->pWindow),gsi->pLayout);
	
	pImage=gtk_image_new_from_pixbuf(gsi->si->image);
	pAf=create_draw_area(gsi);
		       
	
	pLayout=gtk_layout_new(NULL,NULL);
	gsi->pLayout=pLayout;
	gtk_widget_show(pAf);
	gtk_layout_put(GTK_LAYOUT(pLayout),pImage,0,0);
	gtk_layout_put(GTK_LAYOUT(pLayout),pAf,gsi->si->lcd_pos.left,gsi->si->lcd_pos.top);
	gtk_container_add(GTK_CONTAINER(pWindow),pLayout);
	gtk_window_resize(GTK_WINDOW(pWindow),gsi->si->width,gsi->si->height);
	g_signal_connect(G_OBJECT(pWindow),"destroy",G_CALLBACK(on_destroy),NULL); 
	
	/* Connection signal keyboard key press */
	gtk_widget_add_events(pWindow, GDK_KEY_RELEASE_MASK); // Get the event on the window (leftclick, rightclick)
	/*gtk_signal_connect(GTK_OBJECT(pWindow), "key_press_event", GTK_SIGNAL_FUNC(keyboard_event), NULL); */
	gtk_signal_connect(GTK_OBJECT(pWindow), "key_press_event", G_CALLBACK(keyboard_event), NULL);
	gtk_widget_add_events(pWindow, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
	gtk_signal_connect(GTK_OBJECT(pWindow), "button_press_event", G_CALLBACK(mouse_event),gsi);
	gtk_widget_show_all(pWindow);	/*display the window and all that it contains.*/
	
	D ("Exiting : redraw_screen...\n");
	return gsi;

}
