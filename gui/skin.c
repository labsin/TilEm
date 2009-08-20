#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>

#include <tilem.h>
#include <gui.h>

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


