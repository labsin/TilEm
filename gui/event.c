#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>

#include <tilem.h>
#include <gui.h>


static int scan_click(int MAX, double x, double y, GLOBAL_SKIN_INFOS * gsi);

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

void quit_with_save()
{
	printf("\nThank you for using tilem...\n");
	DGLOBAL_L2_A0("**************** SAVE_STATE ****************************\n");
	SAVE_STATE=1;
	DGLOBAL_L2_A1("*  YES (%d)                                             *\n",SAVE_STATE);
	DGLOBAL_L2_A0("********************************************************\n\n");
	gtk_main_quit();
}

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

void on_reset(GLOBAL_SKIN_INFOS * gsi)
{
	tilem_calc_reset(gsi->emu->calc);
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
		
		code =scan_click(50, bevent->x, bevent->y, gsi),
		
		g_mutex_lock(emu->calc_mutex);
		tilem_keypad_press_key(emu->calc, code);
		g_mutex_unlock(emu->calc_mutex);
	}
	return FALSE;
}


gboolean mouse_release_event(GtkWidget* pWindow,GdkEvent *event,GLOBAL_SKIN_INFOS * gsi) {
	
	DCLICK_L0_A0(">>>>>>>\"RELEASE\"<<<<<<<\n");
	DCLICK_L0_A0("**************** fct : mouse_release_event *************\n");
	TilemCalcEmulator* emu = gsi->emu;
	int code=0;
	pWindow=pWindow;
	GdkEventButton *bevent = (GdkEventButton *) event;
	/* DCLICK_L0_A2("%G %G",bevent->x,bevent->y); */
	

/* #################### Right Click Menu	#################### */
	if(event->button.button==3)  {	/*detect a right click to build menu */
		DCLICK_L0_A0("*  right click !                                       *\n");
		static GtkItemFactoryEntry right_click_menu[] = {
			{"/Load skin...", "F12", SkinSelection, 1, NULL,NULL},
			{"/Send file...", "F11", send_file, 1, NULL, NULL},
			{"/Enter debugger...", "F11", launch_debugger, 1, NULL, NULL},
			{"/Switch view",NULL,switch_view,1,NULL,NULL},
			{"/Switch borderless",NULL,switch_borderless,1,NULL,NULL},
			{"/Use this skin as default for this rom model",NULL,add_or_modify_defaultskin, 1, NULL, NULL},
			{"/About", "<control>Q",show_about, 0, NULL, NULL},
			{"/---", NULL, NULL, 0, "<Separator>", NULL},
			{"/Reset", "<control>R", on_reset, 0, NULL, NULL},
			{"/Quit without saving", "<control>Q", on_destroy, 0, NULL, NULL},
			{"/Exit and save state", "<alt>X", quit_with_save, 1, NULL, NULL}
		};
		right_click_menu[0].extra_data=gsi;
		create_menus(gsi->pWindow,event,right_click_menu, sizeof(right_click_menu) / sizeof(GtkItemFactoryEntry), "<magic_right_click_menu>",(gpointer)gsi);
		

		DCLICK_L0_A0("********************************************************\n\n");
/* #################### Left Click (test wich key is it)#################### */
	} else {
		code =scan_click(50, bevent->x, bevent->y, gsi),
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

/* return the key code.This code is used to send a signal to libtilemcore */
int scan_click(int MAX, double x, double y,  GLOBAL_SKIN_INFOS * gsi)
 {
	 int i=0;
	 DCLICK_L2_A0("..............................x   y   z   t ......... \n");
		for (i = 0; i < MAX; i++)
		{
			
			DCLICK_L2_A5("%d............testing........ %d %d %d %d.........\n",i,gsi->si->keys_pos[i].left,gsi->si->keys_pos[i].top,gsi->si->keys_pos[i].right,gsi->si->keys_pos[i].bottom);
			if ((x > gsi->si->keys_pos[i].left) && (x < gsi->si->keys_pos[i].right) &&(y > gsi->si->keys_pos[i].top) && (y < gsi->si->keys_pos[i].bottom))
			break;
			
		}	
		DCLICK_L0_A1("*  Key number : %d                                     *\n",i);
		DCLICK_L0_A1("*  Key name : %s     ",x3_keylist[i].label);
		DCLICK_L0_A1("Key code : %02X                     *\n",x3_keylist[i].code);
		DCLICK_L0_A2("*  Click coordinates :     ----> x=%G    y=%G <----   *\n",x,y);
		DCLICK_L0_A0("********************************************************\n\n");
		
		return x3_keylist[i].code;
}

/* This function hide the border window, even if you load another skin, or switch view (debugger is NOT borderless because... this is useless?!) */
void switch_borderless(GLOBAL_SKIN_INFOS* gsi) {
	if(gtk_window_get_decorated(GTK_WINDOW(gsi->pWindow)))
	{
		gtk_window_set_decorated(GTK_WINDOW(gsi->pWindow) , FALSE);
	}else 
	{
		gtk_window_set_decorated(GTK_WINDOW(gsi->pWindow) , TRUE);
	}
}

static int is_ready(CalcHandle* h)
{
         int err;
 
         err = ticalcs_calc_isready(h);
         printf("Hand-held is %sready !\n", err ? "not " : "");
 
         return 0;
}

static void print_lc_error(int errnum)
{
	char *msg;

        ticables_error_get(errnum, &msg);
	fprintf(stderr, "Link cable error (code %i)...\n<<%s>>\n",
        errnum, msg);
        free(msg);
}


void send_file(GLOBAL_SKIN_INFOS *gsi) {
	CableHandle* cable;
	CalcHandle* calc;
	FileContent *filec;
	int err;

	// init libs
	ticables_library_init();
	ticalcs_library_init();
	tifiles_library_init();

	// set cable
	cable = ticables_handle_new(CABLE_ILP, PORT_0);
	if(cable==NULL) 
		printf("cable == null\n");


	// set calc
	calc = ticalcs_handle_new(CALC_TI83);

	if(calc==NULL) 
		printf("calc == null\n");

	//calc = gsi->emu->calc;
	// attach cable to calc (and open cable)
	err = ticalcs_cable_attach(calc, cable);
	ticables_handle_show(cable);

	//err = ticalcs_calc_isready(calc);
	is_ready(calc);
	if(err)
	        print_lc_error(err);
	//cable->priv = gsi->emu->calc;	
	filec = tifiles_content_create_regular(calc->model);
	err = tifiles_file_read_regular("test.83p", filec);
	ticalcs_calc_send_var(calc, MODE_NORMAL, filec);
	//err= ticalcs_calc_send_var(cable, 
	//printf("Hand-held is %sready !\n", err ? "not " : "");

	// detach cable (made by handle_del, too)
	err = ticalcs_cable_detach(calc);

	// remove calc & cable
	ticalcs_handle_del(calc);
	ticables_handle_del(cable);

}



