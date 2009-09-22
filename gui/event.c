#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>

#include <tilem.h>
#include <gui.h>


/* Just close the window (freeing allocation maybe in the futur?)*/
void OnDestroy(GtkWidget * pWidget, gpointer pData) 
{
	pWidget=0;	// just to delete the "warning" while compilation.Benjamin : If you know what's the utility of pWidget and pData you can correct this...
	pData=0;		// Thank you. 
	printf("Thank you for using tilem...\n");
	gtk_main_quit();
}


/* This event is executed when key (on keyboard not a click with mouse) is pressed */
void keyboard_event() 
{ 
	printf("You press a key : keyboard_event\n");	//debug
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
	int i,keycounter=0,keycount=0,j;
	pWindow=pWindow;	// just to stop warning when I compil (that is in part why I made the mistake above)


	/* An alternative solution (used by "tilem old generation") */
	//GdkEventButton *bevent = (GdkEventButton *) event;
	//printf("%G %G",bevent->x,bevent->y);
	//if((event->button.x>40)&&(event->button.y>100)) 
	/* end */
	
	
//#################### Right Click Menu	####################
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
			{"/---", NULL, NULL, 0, "<Separator>", NULL},
			{"/Quit without saving", "<control>Q", OnDestroy, 0, NULL, NULL},
			{"/Exit and save state", "<alt>X", NULL, 1, NULL, NULL}
};
create_menus(pWindow,event,right_click_menu, sizeof(right_click_menu) / sizeof(GtkItemFactoryEntry), "<magic_right_click_menu>");
//#################### END ####################

	}else {	
		
		//printf ("%d %d",Calc_Key_Map->Calc_Key_Coord.x_begin_btn_w,Calc_Key_Map->Calc_Key_Coord.x_jump_btn_w);
			for(i=0;i<5;i++) {		//detect if a key was pressed in "window menu" zone
				if((event->button.x>Calc_Key_Map->Calc_Key_Coord.x_begin_btn_w+i*Calc_Key_Map->Calc_Key_Coord.x_jump_btn_w) && (event->button.x<(Calc_Key_Map->Calc_Key_Coord.x_begin_btn_w+i*Calc_Key_Map->Calc_Key_Coord.x_jump_btn_w+Calc_Key_Map->Calc_Key_Coord.x_size_btn_w)) 
					&& (event->button.y>Calc_Key_Map->Calc_Key_Coord.y_begin_btn_w) && (event->button.y<(Calc_Key_Map->Calc_Key_Coord.y_begin_btn_w+Calc_Key_Map->Calc_Key_Coord.y_size_btn_w))) {
					// if x>leftbutton + i*jump && x< leftbutton+i*jump+sizebutton && y>topbutton && y<topbutton+sizebutton 
					// Simple is beautiful ...? Oh that's probably a bad example...
					keycount=keycounter;
					printf("Window : %s \n\n",Calc_Key_Map->Calc_Key_List[keycount].label);
				} else {
					keycounter++;
					//printf("%d\n",keycount);
				}
			}
		}
		       
		// the 2 first lines of real key  (only 3 buttons by line)
		if(keycount==0) // if we had not found what key is pressed
		{
			for(j=0;j<2;j++) {
				
				for(i=0;i<3;i++) {		//detect if a key was pressed in "real key" zone.
					if((event->button.x   >  Calc_Key_Map->Calc_Key_Coord.x_begin_btn_rk   +   i  *Calc_Key_Map->Calc_Key_Coord.x_jump_btn_rk)   &&  
						(event->button.x   <   (Calc_Key_Map->Calc_Key_Coord.x_begin_btn_rk   +   i  *Calc_Key_Map->Calc_Key_Coord.x_jump_btn_rk   + Calc_Key_Map->Calc_Key_Coord.x_size_btn_rk))   &&   
						(event->button.y   > Calc_Key_Map->Calc_Key_Coord.y_begin_btn_rk   +  j  *Calc_Key_Map->Calc_Key_Coord.y_jump_btn_rk)   &&   
						(event->button.y   <   (Calc_Key_Map->Calc_Key_Coord.y_begin_btn_rk  +    j  *Calc_Key_Map->Calc_Key_Coord.y_jump_btn_rk  +Calc_Key_Map->Calc_Key_Coord.y_size_btn_rk))) {
						keycount=keycounter;
						printf("Key : %s \n\n",Calc_Key_Map->Calc_Key_List[keycount].label);
					} else {
						keycounter++;
						//printf("%d\n",keycount);
					}
					
			       }
		       }
	       }
		// the other lines of real key  (5 buttons by line). "real key" is a name given for the keys exept "window menu" and "arrows" 
		if(keycount==0) // if we had not found what key is pressed
		{
			for(j=2;j<9;j++) {
				
				for(i=0;i<5;i++) {		//detect a key press in "arrow" zone
					if((event->button.x   >  Calc_Key_Map->Calc_Key_Coord.x_begin_btn_rk   +   i  * Calc_Key_Map->Calc_Key_Coord.x_jump_btn_rk)   &&  
						(event->button.x   <   (Calc_Key_Map->Calc_Key_Coord.x_begin_btn_rk   +   i  * Calc_Key_Map->Calc_Key_Coord.x_jump_btn_rk   +  Calc_Key_Map->Calc_Key_Coord.x_size_btn_rk))   &&   
						(event->button.y   >  Calc_Key_Map->Calc_Key_Coord.y_begin_btn_rk   +  j  * Calc_Key_Map->Calc_Key_Coord.y_jump_btn_rk)   &&   
						(event->button.y   <   (Calc_Key_Map->Calc_Key_Coord.y_begin_btn_rk  +    j  * Calc_Key_Map->Calc_Key_Coord.y_jump_btn_rk  + Calc_Key_Map->Calc_Key_Coord.y_size_btn_rk))) {
						keycount=keycounter;
						printf("Key : %s \n\n",Calc_Key_Map->Calc_Key_List[keycount].label);
					} else {
						keycounter++;
					}
					
			       }
		       }
	       
		}
		printf("click :     x=%G    y=%G\n",event->button.x,event->button.y);	//debug
}


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


