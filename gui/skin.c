#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>

#include <tilem.h>
#include <gui.h>

/* With the rom we get the informations on the model (char* tilem_guess_rom_file(FILE* romfile)) returning a char* calc_id , so we create a new structure TilemCalcEmu.
This structure contains a other structure TilemCalc created by TilemCalc* tilem_calc_new(char * calc_id).
So we use this structure in TilemCalcSkin* tilem_guess_skin_set(TilemCalc* calc) for defining the filenames for the pictures... ;D */

TilemCalcSkin* tilem_guess_skin_set(TilemCalc* calc) {

	char * pixbasename;	// this part will not change ("./pixmaps/x")
	printf("In function tilem_guess_skin_set() : \n emu.calc->hw.name= %s \n",calc->hw.name);	//debug
	TilemCalcSkin *Calc_Skin_temp;
	Calc_Skin_temp= tilem_try_new0(TilemCalcSkin, 1);

	Calc_Skin_temp->top=g_malloc(23);
	Calc_Skin_temp->left=g_malloc(23);
	Calc_Skin_temp->right=g_malloc(23);
	Calc_Skin_temp->bot=g_malloc(23);
	
	pixbasename=g_malloc(13);	// "./pixmaps/x" + modelname = 11char + 1char + 1 char for "\0"
	strcpy(pixbasename,"./pixmaps/x");
	strcat(pixbasename,&calc->hw.name[3]);	// so we have "./pixmaps/x3" (3 is an example ;D)
	printf("pixbasename : %s\n",pixbasename);	//debug

	
	strcpy(Calc_Skin_temp->top,pixbasename);	//pixbasename is the invariant prefix name for the calc that was already being choosed.
	strcat(Calc_Skin_temp->top,"_top.jpg");
	printf("%s\n",Calc_Skin_temp->top);	// debug
	strcpy(Calc_Skin_temp->left,pixbasename);
	strcat(Calc_Skin_temp->left,"_left.jpg");
	printf("%s\n",Calc_Skin_temp->left);	// debug
	strcpy(Calc_Skin_temp->right,pixbasename);
	strcat(Calc_Skin_temp->right,"_right.jpg");
	printf("%s\n",Calc_Skin_temp->right);	// debug
	strcpy(Calc_Skin_temp->bot,pixbasename);
	strcat(Calc_Skin_temp->bot,"_bot.jpg");
	printf("%s\n",Calc_Skin_temp->bot);	// debug
	/* end */ 
	
	
	
	return Calc_Skin_temp;
	
}

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

/* This event is executed when click with mouse  */
void mouse_event() 
{ 
	printf("You click with mouse : mouse_event\n");	//debug
}

