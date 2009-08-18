#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>

#include <tilem.h>

/*  contra-sh : 
* ---18/08/09---
* - Draw the TI83 (the other model will be made soon with tilem_guess_rom_type()" I think)
* - Copy from testemu.c : Create the savname, Open the Romfile, Get the model, Draw the lcdscreen (added in the "pHBox" - no animation for the moment)
* - Function event OnDestroy 
* - Modification of the Makefile (I hope it's good !? If you can control this... thx)
* - LEARNING HOW COMMIT !!   :D 
*/

void OnDestroy(GtkWidget *pWidget, gpointer pData);	// close the pWindow

typedef struct _TilemCalcEmulator {
	GMutex* run_mutex;
	gboolean exiting;
	gboolean forcebreak;

	GMutex* calc_mutex;

	GMutex* lcd_mutex;
	guchar* lcdlevel;
	guchar* lcdimage;
	GdkColor lcdfg, lcdbg;
	GdkPixbuf* lcdpb;
	GdkPixbuf* lcdscaledpb;

	GtkWidget* lcdwin;
} TilemCalcEmulator;


/* Benjamin : 
* I want to move this part of code (memory management) in another file but i'm not a "Makefile 's expert" and "linking expert" too :p
* When I tried, I had a "unreference to function _start" (like normally when you don't have "int main() { }") or an equivalent message... 
* If you had 5 minutes can you modify the Makefile, correct and move the code for me (for the makefile I don't know if it's necessary to modify...) ?
* (move in the gui directory)
* But don't worry, I'll probably find the solution alone soon and it does not matter if you do not have time for this.
* Thank you for your help.
*/

/* CUT HERE */

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
/* end */

/* CUT END */

int main(int argc, char **argv)
{
	
	const char* romname = "xp.rom";
	char* savname;
	char* p;
	char calc_id;	
	FILE *romfile;//*savfile;
	
	
	TilemCalcEmulator emu;
	
	/* Init GTK+ */
	GtkWidget *pWindow;
	gtk_init(&argc, &argv);
	printf("Running tilem ^^ \n");
	
	/* Get the romname */
	if (argc >= 2) {	// If they are 2 parameters, the romfile is the first of two.
		romname = argv[1];
	}
	savname = g_malloc(strlen(romname) + 5);
	strcpy(savname, romname);
	if ((p = strrchr(savname, '.'))) {
		strcpy(p, ".sav");
		printf("romname=%s savname=%s \n",romname,savname);	//debug
	}
	else {
		strcat(savname, ".sav");
	}
	/* end */
	
	/* Open the romfile */
	romfile = g_fopen(romname, "rb");
	if (!romfile) {
		perror(romname);
		return 1;
	}
	/* end */
	
	/* Get the model */
	calc_id = tilem_guess_rom_type(romfile);
	printf("clac_id = %c\n",calc_id);	//debug
	/* end */
	

	
	/* Create the window */
	GtkWidget *gtk_window_new(GtkWindowType type);
	pWindow=gtk_window_new(GTK_WINDOW_TOPLEVEL);	// GTK_WINDOW_LEVEL : define how is the window 
		gtk_window_set_title(GTK_WINDOW(pWindow),"tilem");	// define title of the window 
		gtk_window_set_position(GTK_WINDOW(pWindow),GTK_WIN_POS_CENTER); // GTK_WIN_POS_CENTER : define how the window is displayed 
		gtk_window_set_default_size(GTK_WINDOW(pWindow),60,320);	// define size of the window
	
	g_signal_connect(G_OBJECT(pWindow),"destroy",G_CALLBACK(OnDestroy),NULL); 
	//gtk_widget_show(pWindow);
	/* fin */
	
	
	/* Draw TI83  */
	GtkWidget *pSkinset,*pVBox,*pHBox; 
	GtkWidget *pTop,*pLeft,*pAf,*pRight,*pBot;
	
	/* TOP *********************************************************************/
	pVBox=gtk_vbox_new(FALSE,0);
	pTop=gtk_image_new_from_file("./pixmaps/x3_top.jpg");	// top of the screen
	gtk_box_pack_start(GTK_BOX(pVBox),pTop,FALSE,FALSE,0);
	
	/* CENTER ****************************************************************/
	pHBox=gtk_hbox_new(FALSE,0);	// this horizontal box is for the 3 elements of the screen
		gtk_box_pack_start(GTK_BOX(pVBox),pHBox,FALSE,FALSE,0); // pVBox > pHBox
	pLeft=gtk_image_new_from_file("./pixmaps/x3_left.jpg");	// left of the screen
		
	
	int screenwidth=94;	// fixed for the test
	int screenheight=61;	// idem
	pAf = gtk_aspect_frame_new(NULL, 0.5, 0.5, 1.0, TRUE);		// et pAf ! ça fait des Chocapic !! 
                         gtk_frame_set_shadow_type(GTK_FRAME(pAf),
                                                   GTK_SHADOW_NONE);
                         {
                                 emu.lcdwin = gtk_drawing_area_new();
                                 gtk_widget_set_name(emu.lcdwin, "tilem-lcd");
 
                                 gtk_widget_set_size_request(emu.lcdwin,
                                                             screenwidth * 2,
                                                             screenheight * 2);
                                 gtk_container_add(GTK_CONTAINER(pAf),
                                                   emu.lcdwin);
                               gtk_widget_show(emu.lcdwin);
                       }
 


	pRight=gtk_image_new_from_file("./pixmaps/x3_right.jpg");	 // right of the screen
		gtk_box_pack_start(GTK_BOX(pHBox),pLeft,FALSE,FALSE,0);
		gtk_box_pack_start(GTK_BOX(pHBox),pAf,FALSE,FALSE,0);
		gtk_box_pack_start(GTK_BOX(pHBox),pRight,FALSE,FALSE,0);
	
	/* BOTTOM ****************************************************************/	       
	pBot=gtk_image_new_from_file("./pixmaps/x3_bot.jpg");	// bottom of the screen (keyboard)
		gtk_box_pack_start(GTK_BOX(pVBox),pBot,FALSE,FALSE,0);
	pSkinset=gtk_hbox_new(FALSE,0);	// the complete set to fit them
		gtk_box_pack_start(GTK_BOX(pSkinset),pVBox,FALSE,FALSE,0);
	gtk_container_add(GTK_CONTAINER(pWindow),pSkinset);	// just add the box to the window
	/* end */
	

	
	gtk_widget_show_all(pWindow);	// display the window and all that it contains.
	gtk_main();

    return EXIT_SUCCESS;
}

void OnDestroy(GtkWidget * pWidget, gpointer pData) 
{
	pWidget=0;	// just to delete the "warning" while compilation.Benjamin : If you know what's the utility of pWidget and pData you can correct this...
	pData=0;	// Thank you. 
	printf("Thank you for using tilem...\n");
	gtk_main_quit();
}





