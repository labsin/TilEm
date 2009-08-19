#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>

#include <tilem.h>

/*  contra-sh : 
* ---18/08/09---
* - Draw the TI83 
* - Copy from testemu.c : Create the savname, Open the Rom, Get the model, Draw the lcdscreen (no animation for the moment)
* - Function event OnDestroy 
* - Modification of the Makefile (I hope it's good !? If you can control this... thx)
* - LEARNING HOW COMMIT !!   :D
* ---18/08/09---
* - New structure TilemCalcSkin : define the different filename for the SkinSet.
* - Draw the other models _automatically_ . ;D
*/

void OnDestroy(GtkWidget *pWidget, gpointer pData);	// close the pWindow

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

typedef struct _TileCalcSkin {
	char * top;
	char * left;
	char * right;
	char * bot;
} TilemCalcSkin;
	


/* #########  MAIN  ######### */

int main(int argc, char **argv)
{
	
	const char* romname = "xp.rom";
	char* savname;
	char* p;
	char calc_id;
	char* pixbasename;
	FILE *romfile;//*savfile;
	
	
	
	TilemCalcEmulator emu;
	
	/* Init GTK+ */
	GtkWidget *pWindow;
	gtk_init(&argc, &argv);
	printf("Running tilem ^^ \n");
	
	/* Get the romname */
	if (argc >= 2) {	// If they are 2 parameters or more, the romfile is the first.
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
	
	/* Get the romtype*/
	calc_id = tilem_guess_rom_type(romfile);
	printf("clac_id = %c\n",calc_id);	//debug
	/* end */
	
	/* Create the calc with the romtype so get the modelname */
	emu.calc = tilem_calc_new(calc_id);
	printf("emu.calc->hw.model= %c \n",emu.calc->hw.model_id);	// debug :)
	printf("emu.calc->hw.name= %s \n",emu.calc->hw.name);		// debug :D it works !
	printf("emu.calc->hw.name= %c \n",emu.calc->hw.name[3]);	// that's what I'll use for the pixmap choice !
	/* end */
	

	

	
	/* Create the window */
	GtkWidget *gtk_window_new(GtkWindowType type);
	pWindow=gtk_window_new(GTK_WINDOW_TOPLEVEL);	// GTK_WINDOW_LEVEL : define how is the window 
		gtk_window_set_title(GTK_WINDOW(pWindow),"tilem");	// define title of the window 
		gtk_window_set_position(GTK_WINDOW(pWindow),GTK_WIN_POS_CENTER); // GTK_WIN_POS_CENTER : define how the window is displayed 
		gtk_window_set_default_size(GTK_WINDOW(pWindow),60,320);	// define size of the window
	
	g_signal_connect(G_OBJECT(pWindow),"destroy",G_CALLBACK(OnDestroy),NULL); 
	//gtk_widget_show(pWindow);
	/* end */
	
	
	/* Draw Calc  */
	GtkWidget *pSkinset,*pVBox,*pHBox; 
	GtkWidget *pTop,*pLeft,*pAf,*pRight,*pBot;
	
	/* Choose the filename for the pix top,left,right,bot in function of the model */
	TilemCalcSkin Calc_Skin;		// Calc_Skni will contain the different element for the SkinSet
	Calc_Skin.top=g_malloc(23);
	Calc_Skin.left=g_malloc(23);
	Calc_Skin.right=g_malloc(23);
	Calc_Skin.bot=g_malloc(23);
	
	pixbasename=g_malloc(13);	// "./pixmaps/x" + modelname = 11char + 1char + 1 char for "\0"
	strcpy(pixbasename,"./pixmaps/x");
	strcat(pixbasename,&emu.calc->hw.name[3]);	// so we have "./pixmaps/x3" (3 is an example ;D)
	//printf("pixbasename : %s\n",pixbasename);	//debug

	
	strcpy(Calc_Skin.top,pixbasename);	//pixbasename is the invariant prefix name for the calc that was already being choosed.
	strcat(Calc_Skin.top,"_top.jpg");
	//printf("%s\n",Calc_Skin.top);	// debug
	strcpy(Calc_Skin.left,pixbasename);
	strcat(Calc_Skin.left,"_left.jpg");
	//printf("%s\n",Calc_Skin.left);	// debug
	strcpy(Calc_Skin.right,pixbasename);
	strcat(Calc_Skin.right,"_right.jpg");
	//printf("%s\n",Calc_Skin.right);	// debug
	strcpy(Calc_Skin.bot,pixbasename);
	strcat(Calc_Skin.bot,"_bot.jpg");
	//printf("%s\n",Calc_Skin.bot);	// debug
	/* end */ 
	
	
	
	/* TOP *********************************************************************/
	pVBox=gtk_vbox_new(FALSE,0);
	pTop=gtk_image_new_from_file(Calc_Skin.top);	// top of the screen
	gtk_box_pack_start(GTK_BOX(pVBox),pTop,FALSE,FALSE,0);
	
	/* CENTER ****************************************************************/
	pHBox=gtk_hbox_new(FALSE,0);	// this horizontal box is for the 3 elements of the screen
		gtk_box_pack_start(GTK_BOX(pVBox),pHBox,FALSE,FALSE,0); // pVBox > pHBox
	pLeft=gtk_image_new_from_file(Calc_Skin.left);	// left of the screen
		
	
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
	pRight=gtk_image_new_from_file(Calc_Skin.right);	 // right of the screen
		gtk_box_pack_start(GTK_BOX(pHBox),pLeft,FALSE,FALSE,0);
		gtk_box_pack_start(GTK_BOX(pHBox),pAf,FALSE,FALSE,0);
		gtk_box_pack_start(GTK_BOX(pHBox),pRight,FALSE,FALSE,0);
	
	/* BOTTOM ****************************************************************/	   

	pBot=gtk_image_new_from_file(Calc_Skin.bot);	// bottom of the screen (keyboard)
		gtk_box_pack_start(GTK_BOX(pVBox),pBot,FALSE,FALSE,0);
	pSkinset=gtk_hbox_new(FALSE,0);	// the complete set to fit them
		gtk_box_pack_start(GTK_BOX(pSkinset),pVBox,FALSE,FALSE,0);
	gtk_container_add(GTK_CONTAINER(pWindow),pSkinset);	// just add the box to the window
	/* end */
	

	
	gtk_widget_show_all(pWindow);	// display the window and all that it contains.
	gtk_main();
	
	free(Calc_Skin.top);
	free(Calc_Skin.left);
	free(Calc_Skin.right);
	free(Calc_Skin.bot);

    return EXIT_SUCCESS;
}

void OnDestroy(GtkWidget * pWidget, gpointer pData) 
{
	pWidget=0;	// just to delete the "warning" while compilation.Benjamin : If you know what's the utility of pWidget and pData you can correct this...
	pData=0;	// Thank you. 
	printf("Thank you for using tilem...\n");
	gtk_main_quit();
}





