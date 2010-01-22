#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>

#include <tilem.h>
#include <gui.h>

void run_with_key(TilemCalc* calc, int key)
{
	tilem_z80_run_time(calc, 500000, NULL);
	tilem_keypad_press_key(calc, key);
	tilem_z80_run_time(calc, 1000000, NULL);
	tilem_keypad_release_key(calc, key);
	tilem_z80_run_time(calc, 500000, NULL);
}

/* Just close the window (freeing allocated memory maybe in the futur?)*/
void on_destroy()
{
	/*printf("\nThank you for using tilem...\n");*/
	gtk_main_quit();
}



/* This event is executed when key (on keyboard not a click with mouse) is pressed */
void keyboard_event() 
{ 
	printf("You press a key : keyboard_event\n");	/* debug */
}



/* This event is executed when click with mouse (the Calc_Key_Map is given as parameter) */
/* Guess what key was clicked... ;D */
void mouse_event(GtkWidget* pWindow,GdkEvent *event,GLOBAL_SKIN_INFOS *gsi) 	// void mouse_event(GdkEvent *event) doesn't work !!Necessite first parameter (I've lost 3hours for this).
{  	
	int i;
	pWindow=pWindow;	// just to stop warning when I compil (that is in part why I made the mistake above)
	
	/*printf("\nMOUSE_EVENT : name %s\n",gsi->si->name);*/

	/* An alternative solution (used by "tilem old generation") */
	GdkEventButton *bevent = (GdkEventButton *) event;
	printf("%G %G",bevent->x,bevent->y);
	
	
/* #################### Right Click Menu	#################### */
	if(event->button.button==3)  {	/*detect a right click to build menu */
		printf("right click !\n");
		static GtkItemFactoryEntry right_click_menu[] = {
			{"/Load skin...", "F12", SkinSelection, 1, NULL,NULL},
			{"/About", "<control>Q", on_about, 0, NULL, NULL},
			{"/---", NULL, NULL, 0, "<Separator>", NULL},
			{"/Quit without saving", "<control>Q", on_destroy, 0, NULL, NULL},
			{"/Exit and save state", "<alt>X", NULL, 1, NULL, NULL}
		};
		right_click_menu[0].extra_data=gsi;
		create_menus(pWindow,event,right_click_menu, sizeof(right_click_menu) / sizeof(GtkItemFactoryEntry), "<magic_right_click_menu>",(gpointer)gsi);
		
/* #################### Left Click (test wich key is it)#################### */
	} else {
		for (i = 0; i < 80; i++)
		{
			
			//printf("\n testing ... %d %d %d %d",si->keys_pos[i].left,si->keys_pos[i].top,si->keys_pos[i].right,si->keys_pos[i].bottom);
			if ((bevent->x > gsi->si->keys_pos[i].left) && 
			(bevent->x < gsi->si->keys_pos[i].right) &&
			(bevent->y > gsi->si->keys_pos[i].top) && 
			(bevent->y < gsi->si->keys_pos[i].bottom))
			break;
			
		}
		
		gsi->act=1; // inform the core that a button is clicked.
		tilem_keypad_press_key(gsi->emu->calc, 45);//x3_keylist[i].code); 
		
		printstate(gsi->emu); 
		
		
		screen_repaint(gsi->emu->lcdwin,event, gsi); 
		//screen_update(gsi->emu);
		gsi->act=0; // button released
		printf("\nKey number : %d\n",i);
		printf("Key name : %s\n",x3_keylist[i].label);
	
		printf("click :     x=%G    y=%G\n",event->button.x,event->button.y);	//debug
	}
};


void on_about(GtkWidget *pBtn, gpointer data)
{
    GtkWidget *pAbout;
	pBtn=pBtn;

    /* Creation de la boite de message */
    /* Type : Information -> GTK_MESSAGE_INFO */
    /* Bouton : 1 OK -> GTK_BUTTONS_OK */
    pAbout = gtk_message_dialog_new (GTK_WINDOW(data),
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_OK,
        "TilEm2");

    /* Affichage de la boite de message */
    gtk_dialog_run(GTK_DIALOG(pAbout));

    /* Destruction de la boite de message */
    gtk_widget_destroy(pAbout);
}




void btnbreak(gpointer data)
{
	GLOBAL_SKIN_INFOS *gsi = data;

	g_mutex_lock(gsi->emu->run_mutex);
	gsi->emu->forcebreak = TRUE;
	g_mutex_unlock(gsi->emu->run_mutex);
}
