#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>

#include <tilem.h>
#include <gui.h>


int scan_click(int MAX, double x, double y, GLOBAL_SKIN_INFOS * gsi);

/* Just close the window (freeing allocated memory maybe in the futur?)*/
void on_destroy()
{
	DEBUGGINGGLOBAL_L2_A0("**************** SAVE_STATE ****************************\n");
	SAVE_STATE=0;
	DEBUGGINGGLOBAL_L2_A1("*  NO (%d)                                              *\n",SAVE_STATE);
	DEBUGGINGGLOBAL_L2_A0("********************************************************\n\n");
	printf("\nThank you for using tilem...\n");
	gtk_main_quit();
}

void quit_with_save()
{
	printf("\nThank you for using tilem...\n");
	DEBUGGINGGLOBAL_L2_A0("**************** SAVE_STATE ****************************\n");
	SAVE_STATE=1;
	DEBUGGINGGLOBAL_L2_A1("*  YES (%d)                                             *\n",SAVE_STATE);
	DEBUGGINGGLOBAL_L2_A0("********************************************************\n\n");
	gtk_main_quit();
}



void on_about(GtkWidget *pBtn, gpointer data)
{
	GtkWidget *pAbout;
	pBtn=pBtn;

	pAbout = gtk_message_dialog_new (GTK_WINDOW(data),
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_OK,
        "TilEm2\n\nCoded by : \n* Benjamin Moody (core/debugger) *\n* Thibault Duponchelle (GTK's gui) *\n* Luc Bruant (QT's gui) *\n\nCopyleft 2010 ;D");

	/* Show the msg box */
	gtk_dialog_run(GTK_DIALOG(pAbout));
	
	/* Destroy the msg box */
	gtk_widget_destroy(pAbout);
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
	DEBUGGINGCLICK_L0_A0(">>>>>>>\"PRESS\"<<<<<<<\n");
	DEBUGGINGCLICK_L0_A0("**************** fct : mouse_press_event ***************\n");
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
	
	DEBUGGINGCLICK_L0_A0(">>>>>>>\"RELEASE\"<<<<<<<\n");
	DEBUGGINGCLICK_L0_A0("**************** fct : mouse_release_event *************\n");
	TilemCalcEmulator* emu = gsi->emu;
	int code=0;
	pWindow=pWindow;
	GdkEventButton *bevent = (GdkEventButton *) event;
	/* DEBUGGINGCLICK_L0_A2("%G %G",bevent->x,bevent->y); */
	

/* #################### Right Click Menu	#################### */
	if(event->button.button==3)  {	/*detect a right click to build menu */
		DEBUGGINGCLICK_L0_A0("*  right click !                                       *\n");
		static GtkItemFactoryEntry right_click_menu[] = {
			{"/Load skin...", "F12", SkinSelection, 1, NULL,NULL},
			{"/Switch view",NULL,switch_view,1,NULL,NULL},
			{"/About", "<control>Q",on_about, 0, NULL, NULL},
			{"/---", NULL, NULL, 0, "<Separator>", NULL},
			{"/Quit without saving", "<control>Q", on_destroy, 0, NULL, NULL},
			{"/Exit and save state", "<alt>X", quit_with_save, 1, NULL, NULL}
		};
		right_click_menu[0].extra_data=gsi;
		create_menus(gsi->pWindow,event,right_click_menu, sizeof(right_click_menu) / sizeof(GtkItemFactoryEntry), "<magic_right_click_menu>",(gpointer)gsi);
		

		DEBUGGINGCLICK_L0_A0("********************************************************\n\n");
/* #################### Left Click (test wich key is it)#################### */
	} else {
		code =scan_click(50, bevent->x, bevent->y, gsi),
		/* Send the signal to libtilemcore */
		g_mutex_lock(emu->calc_mutex);
		tilem_keypad_release_key(emu->calc, code);
		g_mutex_unlock(emu->calc_mutex);
	}
	
	
	return FALSE;
}

/* return the key code.This code is used to send a signal to libtilemcore */
int scan_click(int MAX, double x, double y,  GLOBAL_SKIN_INFOS * gsi)
 {
	 int i=0;
	 DEBUGGINGCLICK_L2_A0("..............................x   y   z   t ......... \n");
		for (i = 0; i < MAX; i++)
		{
			
			DEBUGGINGCLICK_L2_A5("%d............testing........ %d %d %d %d.........\n",i,gsi->si->keys_pos[i].left,gsi->si->keys_pos[i].top,gsi->si->keys_pos[i].right,gsi->si->keys_pos[i].bottom);
			if ((x > gsi->si->keys_pos[i].left) && (x < gsi->si->keys_pos[i].right) &&(y > gsi->si->keys_pos[i].top) && (y < gsi->si->keys_pos[i].bottom))
			break;
			
		}	
		DEBUGGINGCLICK_L0_A1("*  Key number : %d                                     *\n",i);
		DEBUGGINGCLICK_L0_A1("*  Key name : %s     ",x3_keylist[i].label);
		DEBUGGINGCLICK_L0_A1("Key code : %02X                     *\n",x3_keylist[i].code);
		DEBUGGINGCLICK_L0_A2("*  Click coordinates :     ----> x=%G    y=%G <----   *\n",x,y);
		DEBUGGINGCLICK_L0_A0("********************************************************\n\n");
		
		return x3_keylist[i].code;
}










