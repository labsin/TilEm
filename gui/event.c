#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>

#include <tilem.h>
#include <gui.h>


/* Just close the window (freeing allocation maybe in the futur?)*/
void OnDestroy()
{
	printf("Thank you for using tilem...\n");
	gtk_main_quit();
}


/* This event is executed when key (on keyboard not a click with mouse) is pressed */
void keyboard_event() 
{ 
	printf("You press a key : keyboard_event\n");	/* debug */
}
void toto(GtkWidget* pWindow,GdkEvent *event,GtkWidget * widget) {
	pWindow=pWindow;
	event=event;
gtk_menu_popup(GTK_MENU(widget), NULL, NULL, NULL, NULL,event->button.button, event->button.time);
}


/* This event is executed when click with mouse (the Calc_Key_Map is given as parameter) */
/* Guess what key was clicked... ;D */
void mouse_event(GtkWidget* pWindow,GdkEvent *event,TilemKeyMap *Calc_Key_Map) 	// void mouse_event(GdkEvent *event) doesn't work !!Necessite first parameter (I've lost 3hours for this).
{  	
	int i;
	pWindow=pWindow;	// just to stop warning when I compil (that is in part why I made the mistake above)


	/* An alternative solution (used by "tilem old generation") */
	GdkEventButton *bevent = (GdkEventButton *) event;
	printf("%G %G",bevent->x,bevent->y);
	/* if((event->button.x>40)&&(event->button.y>100)) */
	/* end */
	printf("hey %d %d %d %d",Calc_Key_Map->Calc_Key_Coord[0].x,Calc_Key_Map->Calc_Key_Coord[0].y,Calc_Key_Map->Calc_Key_Coord[0].h,Calc_Key_Map->Calc_Key_Coord[0].w);
	
	
/* #################### Right Click Menu	#################### */
	if(event->button.button==3)  {	//detect a right click to build menu
		printf("right click !\n");
		static GtkItemFactoryEntry right_click_menu[] = {
			{"/Load file...", "F12", NULL, 0, NULL, NULL},
			{"/Debugger...", "F11", NULL, 0, NULL, NULL},
			{"/Hardware...", NULL, NULL, 0, NULL, NULL},
#ifdef extlink
			{"/Linking...", NULL, NULL, 0, NULL, NULL},
#endif
			{"/---", NULL, NULL, 0, "<Separator>", NULL},
			{"/Toggle autosave", NULL, NULL, 0, NULL, NULL},
			{"/Toggle window", NULL, NULL, 0, NULL, NULL},
			{"/Toggle speed", NULL, NULL, 0, NULL, NULL},
			{"/Toggle key place", NULL, NULL, 0, NULL, NULL},
			{"/Reset", NULL, NULL, 0, NULL, NULL},
			{"/Reload state", NULL, NULL, 0, NULL, NULL},
			{"/About", "<control>Q", OnAbout, 0, NULL, NULL},
			{"/---", NULL, NULL, 0, "<Separator>", NULL},
			{"/Quit without saving", "<control>Q", OnDestroy, 0, NULL, NULL},
			{"/Exit and save state", "<alt>X", NULL, 1, NULL, NULL}
		};
create_menus(pWindow,event,right_click_menu, sizeof(right_click_menu) / sizeof(GtkItemFactoryEntry), "<magic_right_click_menu>");
/* #################### Left Click (test wich key is it)#################### */
	} else {
		for (i = 0; i < 60; i++)
		{
			
			printf("\n testing ... %d %d %d %d",Calc_Key_Map->Calc_Key_Coord[0].x,Calc_Key_Map->Calc_Key_Coord[0].y,Calc_Key_Map->Calc_Key_Coord[0].h,Calc_Key_Map->Calc_Key_Coord[0].w);
			if ((bevent->x > Calc_Key_Map->Calc_Key_Coord[i].x) && 
			(bevent->x < (Calc_Key_Map->Calc_Key_Coord[i].x + Calc_Key_Map->Calc_Key_Coord[i].w)) &&
			(bevent->y > Calc_Key_Map->Calc_Key_Coord[i].y) && 
			(bevent->y < (Calc_Key_Map->Calc_Key_Coord[i].y + Calc_Key_Map->Calc_Key_Coord[i].h)))
			break;
			
		}
		printf("Key number : %d\n",i);
		printf("Key : %s \n\n",Calc_Key_Map->Calc_Key_List[i].label);
	
		printf("click :     x=%G    y=%G\n",event->button.x,event->button.y);	//debug
	}
};


/* gint button_press (GtkWidget *widget, GdkEvent *event)
{

    if (event->type == GDK_BUTTON_PRESS) {
        GdkEventButton *bevent = (GdkEventButton *) event; 
        gtk_menu_popup(GTK_MENU(widget), NULL, NULL, NULL, NULL, bevent->button, bevent->time);

        // Say to caller that we handle the event

        return TRUE;
    }

    // Say to caller that we don't handle the event

    return FALSE;
};

gint show_menu(GtkWidget *widget, GdkEvent *event)
{
	GdkEventButton *bevent = (GdkEventButton *) event;
	if ((event->type == GDK_BUTTON_PRESS) && (bevent->button != 1)) {
		gtk_menu_popup(GTK_MENU(widget), NULL, NULL, NULL, NULL, bevent->button, bevent->time);
		return(TRUE);
	}

	return(FALSE);
} */

void OnAbout(GtkWidget *pBtn, gpointer data)
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



