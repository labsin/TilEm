#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>

#include <tilem.h>
#include <scancodes.h>
#include <gui.h>

/* Table for translating skin-file key number (based on actual
   position, and defined by the VTI/TiEmu file formats) into a
   scancode.  Note that the TILEM_KEY_* constants are named according
   to the TI-83 keypad layout; other models use different names for
   the keys, but the same scancodes. */
static const int keycode_map[] =
	{ TILEM_KEY_YEQU,
	  TILEM_KEY_WINDOW,
	  TILEM_KEY_ZOOM,
	  TILEM_KEY_TRACE,
	  TILEM_KEY_GRAPH,

	  TILEM_KEY_2ND,
	  TILEM_KEY_MODE,
	  TILEM_KEY_DEL,
	  TILEM_KEY_LEFT,
	  TILEM_KEY_RIGHT,
	  TILEM_KEY_UP,
	  TILEM_KEY_DOWN,
	  TILEM_KEY_ALPHA,
	  TILEM_KEY_GRAPHVAR,
	  TILEM_KEY_STAT,

	  TILEM_KEY_MATH,
	  TILEM_KEY_MATRIX,
	  TILEM_KEY_PRGM,
	  TILEM_KEY_VARS,
	  TILEM_KEY_CLEAR,

	  TILEM_KEY_RECIP,
	  TILEM_KEY_SIN,
	  TILEM_KEY_COS,
	  TILEM_KEY_TAN,
	  TILEM_KEY_POWER,

	  TILEM_KEY_SQUARE,
	  TILEM_KEY_COMMA,
	  TILEM_KEY_LPAREN,
	  TILEM_KEY_RPAREN,
	  TILEM_KEY_DIV,

	  TILEM_KEY_LOG,
	  TILEM_KEY_7,
	  TILEM_KEY_8,
	  TILEM_KEY_9,
	  TILEM_KEY_MUL,

	  TILEM_KEY_LN,
	  TILEM_KEY_4,
	  TILEM_KEY_5,
	  TILEM_KEY_6,
	  TILEM_KEY_SUB,

	  TILEM_KEY_STORE,
	  TILEM_KEY_1,
	  TILEM_KEY_2,
	  TILEM_KEY_3,
	  TILEM_KEY_ADD,

	  TILEM_KEY_ON,
	  TILEM_KEY_0,
	  TILEM_KEY_DECPNT,
	  TILEM_KEY_CHS,
	  TILEM_KEY_ENTER };

/* Find the keycode for the key (if any) at the given position.  If
   the keys overlap, choose the "nearest" (according to Manhattan
   distance to the midpoint.) */
static int scan_click(const SKIN_INFOS* skin, double x, double y)
{
	guint ix, iy, nearest = 0, i;
	int dx, dy, d, best_d = G_MAXINT;

	ix = (x + 0.5);
	iy = (y + 0.5);

	for (i = 0; i < G_N_ELEMENTS(keycode_map); i++) {
		if (ix >= skin->keys_pos[i].left
		    && ix < skin->keys_pos[i].right
		    && iy >= skin->keys_pos[i].top
		    && iy < skin->keys_pos[i].bottom) {
			dx = (skin->keys_pos[i].left + skin->keys_pos[i].right
			      - 2 * ix);
			dy = (skin->keys_pos[i].top + skin->keys_pos[i].bottom
			      - 2 * iy);
			d = ABS(dx) + ABS(dy);

			if (d < best_d) {
				best_d = d;
				nearest = keycode_map[i];
			}
		}
	}

	return nearest;
}

/* Just close the window (freeing allocated memory maybe in the futur?)*/
void on_destroy()
{
	DGLOBAL_L2_A0("**************** SAVE_STATE ****************************\n");
	SAVE_STATE=0;
	DGLOBAL_L2_A1("*  NO (%d)                                              *\n",SAVE_STATE);
	DGLOBAL_L2_A0("********************************************************\n\n");
	printf("\nThank you for using tilem...\n");
	gtk_main_quit();
}

/* Save state */
void save_state(GLOBAL_SKIN_INFOS * gsi)
{
	FILE* romfile, *savfile;
	DGLOBAL_L2_A0("**************** SAVE_STATE ****************************\n");
	DGLOBAL_L2_A1("*  YES (%d)                                             *\n",SAVE_STATE);
	DGLOBAL_L2_A0("********************************************************\n\n");
	romfile = g_fopen(gsi->RomName, "wb");
	savfile = g_fopen(gsi->SavName, "wt");
	g_mutex_lock(gsi->emu->calc_mutex);
	tilem_calc_save_state(gsi->emu->calc, romfile, savfile);
	g_mutex_unlock(gsi->emu->calc_mutex);
}

/* Save tilem and save state */
void quit_with_save()
{
	printf("\nThank you for using tilem...\n");
	DGLOBAL_L2_A0("**************** SAVE_STATE ****************************\n");
	SAVE_STATE=1;
	DGLOBAL_L2_A1("*  YES (%d)                                             *\n",SAVE_STATE);
	DGLOBAL_L2_A0("********************************************************\n\n");
	gtk_main_quit();
}

/* Show a nice GtkAboutDialog */
void show_about()
{

  GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file("pix/tilem.png", NULL);

  GtkWidget *dialog = gtk_about_dialog_new();
  gtk_about_dialog_set_name(GTK_ABOUT_DIALOG(dialog), "TilEm");
  gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), "2.0"); 
  gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog), "(c) Benjamin Moody\n(c) Thibault Duponchelle\n(c) Luc Bruant\n");
  gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), "TilEm is a TI Linux Emulator.\n It emulates all current z80 models.\n TI73, TI76, TI81, TI82, TI83(+)(SE), TI84+(SE), TI85 and TI86 ;D");
  gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dialog), "http://lpg.ticalc.org/prj_tilem/");
  gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(dialog), pixbuf);
  g_object_unref(pixbuf), pixbuf = NULL;
  gtk_dialog_run(GTK_DIALOG (dialog));
  gtk_widget_destroy(dialog);

}

/* Show about dialog box */
void on_about(GtkWidget *pBtn)
{
	GtkWidget *pAbout;
	pBtn=pBtn;

	pAbout = gtk_message_dialog_new (NULL,
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_OK,
        "TilEm2\n\nCoded by : \n* Benjamin Moody (core/debugger) *\n* Thibault Duponchelle (GTK's gui) *\n* Luc Bruant (QT's gui) *\n\nCopyleft 2010 ;D");

	/* Show the msg box */
	gtk_dialog_run(GTK_DIALOG(pAbout));
	
	/* Destroy the msg box */
	gtk_widget_destroy(pAbout);
}

/* Reset the calc */
void on_reset(GLOBAL_SKIN_INFOS * gsi)
{
	tilem_calc_reset(gsi->emu->calc);
	
	/* Press and release Key "ON" */
	g_mutex_lock(gsi->emu->calc_mutex);
	run_with_key(gsi->emu->calc, TILEM_KEY_ON);			
	g_mutex_unlock(gsi->emu->calc_mutex);
	printf(">>>>>>>>>RESET \n");
}

/* This event is executed when key (on keyboard) is pressed */
void keyboard_event() 
{ 
	printf("You press a key : keyboard_event\n");	/* debug */
}



/* This event is executed when click with mouse (the Calc_Key_Map is given as parameter) */
/* Guess what key was clicked... ;D */
gboolean mouse_press_event(GtkWidget* pWindow,GdkEvent *event,GLOBAL_SKIN_INFOS *gsi) 	// void mouse_event(GdkEvent *event) doesn't work !!Necessite first parameter (I've lost 3hours for this).
{  	
	DCLICK_L0_A0(">>>>>>>\"PRESS\"<<<<<<<\n");
	DCLICK_L0_A0("**************** fct : mouse_press_event ***************\n");
	int code=0;
	TilemCalcEmulator* emu = gsi->emu;
	pWindow=pWindow;
	GdkEventButton *bevent = (GdkEventButton *) event;

	if(event->button.button==3)  
	{	
	/* Right click ?  then do ... nothing ! (just press not release !)*/
	} else {
		
		code = scan_click(gsi->si, bevent->x, bevent->y),
		
		g_mutex_lock(emu->calc_mutex);
		tilem_keypad_press_key(emu->calc, code);
		g_mutex_unlock(emu->calc_mutex);
	}
	return FALSE;
}

/* Handle release event */
gboolean mouse_release_event(GtkWidget* pWindow,GdkEvent *event,GLOBAL_SKIN_INFOS * gsi) {
	
	DCLICK_L0_A0(">>>>>>>\"RELEASE\"<<<<<<<\n");
	DCLICK_L0_A0("**************** fct : mouse_release_event *************\n");
	TilemCalcEmulator* emu = gsi->emu;
	int code=0;
	char* codechar;
	pWindow=pWindow;
	GdkEventButton *bevent = (GdkEventButton *) event;
	/* DCLICK_L0_A2("%G %G",bevent->x,bevent->y); */
	

/* #################### Right Click Menu	#################### */
	if(event->button.button==3)  {	/*detect a right click to build menu */
		DCLICK_L0_A0("*  right click !                                       *\n");
		static GtkItemFactoryEntry right_click_menu[] = {
			{"/Load skin...", "F12", skin_selection, 1, NULL,NULL},
			{"/Send file...", "F11", load_file, 1, NULL, NULL},
			{"/Enter debugger...", "F11", launch_debugger, 1, NULL, NULL},
			{"/---", NULL, NULL, 0, "<Separator>", NULL},
			{"/Screenshot !",NULL, create_screenshot_window, 1, NULL, NULL},
			{"/Animated screenshot !",NULL, screenshot_anim_create_nostatic, 1, NULL, NULL},
			{"/Animated screenshot add frame !",NULL, screenshot_anim_addframe, 1, NULL, NULL},
			{"/Display lcd into console !",NULL, display_lcdimage_into_terminal, 1, NULL, NULL},
			{"/Switch view",NULL,switch_view,1,NULL,NULL},
			{"/Switch borderless",NULL,switch_borderless,1,NULL,NULL},
			{"/Use this model as default for this rom",NULL,add_or_modify_defaultmodel, 1, NULL, NULL},
			{"/Use this skin as default for this rom ",NULL,add_or_modify_defaultskin, 1, NULL, NULL},
			{"/Save state... ",NULL,save_state, 1, NULL, NULL},
			{"/---", NULL, NULL, 0, "<Separator>", NULL},
			{"/Start recording...", "<control>Q",start_record_macro, 0, NULL, NULL},
			{"/Stop recording.", "<control>Q",stop_record_macro, 0, NULL, NULL},
			{"/Play !", "<control>Q", play_macro, 0, NULL, NULL},
			{"/Play ! (from file...)", "<control>Q", play_macro_from_file, 0, NULL, NULL},
			{"/---", NULL, NULL, 0, "<Separator>", NULL},
			{"/About", "<control>Q",show_about, 0, NULL, NULL},
			{"/Reset", "<control>R", on_reset, 0, NULL, NULL},
			{"/Quit without saving", "<control>Q", on_destroy, 0, NULL, NULL},
			{"/Exit and save state", "<alt>X", quit_with_save, 1, NULL, NULL}
		};
		right_click_menu[0].extra_data=gsi;
		create_menus(gsi->pWindow,event,right_click_menu, sizeof(right_click_menu) / sizeof(GtkItemFactoryEntry), "<magic_right_click_menu>",(gpointer)gsi);
		

		DCLICK_L0_A0("********************************************************\n\n");
/* #################### Left Click (test wich key is it)#################### */
	} else {
		code = scan_click(gsi->si, bevent->x, bevent->y);
		
		if(gsi->isMacroRecording) {     
			codechar= (char*) malloc(sizeof(int));
			sprintf(codechar, "%04d", code);
			add_event_in_macro_file(gsi, codechar);     
		}
		
		/* Send the signal to libtilemcore */
		g_mutex_lock(emu->calc_mutex);
		tilem_keypad_release_key(emu->calc, code);
		g_mutex_unlock(emu->calc_mutex);
	}
	if(gsi->isDebuggerRunning) {
		refresh_register(gsi);
		refresh_stack(gsi);
	}
	return FALSE;
}

/* This function hide the border window, even if you load another skin, or switch view (debugger is NOT borderless because... this is useless?!) */
void switch_borderless(GLOBAL_SKIN_INFOS* gsi) {
	
	if(gtk_window_get_decorated(GTK_WINDOW(gsi->pWindow)))
	{
		gtk_window_set_decorated(GTK_WINDOW(gsi->pWindow) , FALSE);
	} else {
		gtk_window_set_decorated(GTK_WINDOW(gsi->pWindow) , TRUE);
	}
}



/* Load a file */
void load_file(GLOBAL_SKIN_INFOS *gsi) {
		CableHandle* cbl;
		CalcHandle* ch;
		char* filename= NULL;
	
		/* Init the libtis */
		ticables_library_init();
		tifiles_library_init();
		ticalcs_library_init();
		
		/* Create cable (here an internal an dvirtual cabla) */
		cbl = internal_link_handle_new(gsi->emu);
		if (!cbl) 
			fprintf(stderr, "Cannot create ilp handle\n");
		
		/* Create calc */
		ch = ticalcs_handle_new(get_calc_model(gsi->emu->calc));
		if (!ch) 
			fprintf(stderr, "Cannot create calc handle\n");
		
		/* Attach cable to the emulated calc */
		ticalcs_cable_attach(ch, cbl);
	
		/* Launch and get the result of a GtkFileChooserDialog. Cancelled -> filename == NULL */
		filename = select_file(gsi, NULL);		
		
		/* Test if FileChooser cancelled ... */
		if(filename != NULL) {
			//printf("filename = %s", filename);
			send_file(gsi->emu, ch,  filename); /* See link.c for send_file function */
			if(gsi->isMacroRecording) 
				add_load_file_in_macro_file(gsi, strlen(filename), filename) ;
		}
				
			

		ticalcs_cable_detach(ch);
		ticalcs_handle_del(ch);
		ticables_handle_del(cbl);

		/* Exit the libtis */
		ticalcs_library_exit();
		tifiles_library_exit();
		ticables_library_exit();
}	

/* Load a file without file_selector */
void load_file_from_file(GLOBAL_SKIN_INFOS *gsi, char* filename) {
		CableHandle* cbl;
		CalcHandle* ch;
	
		/* Init the libtis */
		ticables_library_init();
		tifiles_library_init();
		ticalcs_library_init();
		
		/* Create cable (here an internal an dvirtual cabla) */
		cbl = internal_link_handle_new(gsi->emu);
		if (!cbl) 
			fprintf(stderr, "Cannot create ilp handle\n");
		
		/* Create calc */
		ch = ticalcs_handle_new(get_calc_model(gsi->emu->calc));
		if (!ch) 
			fprintf(stderr, "Cannot create calc handle\n");
		
		/* Attach cable to the emulated calc */
		ticalcs_cable_attach(ch, cbl);
		
		send_file(gsi->emu, ch,  filename); /* See link.c for send_file function */
		
		ticalcs_cable_detach(ch);
		ticalcs_handle_del(ch);
		ticables_handle_del(cbl);

		/* Exit the libtis */
		ticalcs_library_exit();
		tifiles_library_exit();
		ticables_library_exit();
}	




