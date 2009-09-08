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

/* This event is executed when click with mouse (the Calc_Key_Map is given as parameter) */
/* Guess what key was clicked... ;D */
void mouse_event(GtkWidget* pWindow,GdkEvent *event,TilemKeyMap * Calc_Key_Map) 	// void mouse_event(GdkEvent *event) doesn't work !!Necessite first parameter (I've lost 3hours for this).
{  	
	int i,keycounter=0,keycount=0,j;
	pWindow=pWindow;	// just to stop warning when I compil (that is in part why I made the mistake above)


	/* An alternative solution (used by "tilem old generation") */
	//GdkEventButton *bevent = (GdkEventButton *) event;
	//printf("%G %G",bevent->x,bevent->y);
	//if((event->button.x>40)&&(event->button.y>100)) 
	/* end */
	
	if(event->button.button==3)  {	//detect a right click to build menu
		printf("right click !\n");

	}else {						
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
		       
		/* the 2 first lines of real key  (only 3 buttons by line)*/
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
		/* the other lines of real key  (5 buttons by line). "real key" is a name given for the keys exept "window menu" and "arrows" */
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


