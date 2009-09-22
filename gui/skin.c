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

/* Set a new skin */
void tilem_set_skin(TilemCalcSkin * Calc_Skin,TilemCalcSkin * skin_perso) {
	strcpy(Calc_Skin->top,skin_perso->top);
	printf("%s\n",Calc_Skin->top);	// debug
	strcpy(Calc_Skin->left,skin_perso->left);
	printf("%s\n",Calc_Skin->left);	// debug
	strcpy(Calc_Skin->right,skin_perso->right);
	printf("%s\n",Calc_Skin->right);	// debug
	strcpy(Calc_Skin->bot,skin_perso->bot);
	printf("%s\n",Calc_Skin->bot);	// debug
}
	

/* Copy the TilemKeyCoord in the TilemKeyMap */
void tilem_set_coord(TilemKeyMap* keymap,TilemKeyCoord test) {
	
	keymap->Calc_Key_Coord=test;
}




/* Guess the keymap with an id (in the futur I 'll will probably use a skin name getting from TilemCalcSkin struct) */
TilemKeyMap* tilem_guess_key_map(TilemCalc* calc) {
	
	struct TilemKeyMap *k;
	k= tilem_try_new0(TilemKeyMap, 1);
	//printf("%c",calc->hw.model_id);		// debug
	
	if(calc->hw.model_id=='2') {
		*k=x2_keymap;			// it's 82 series skinset
	} else {
		*k=x3_keymap;			// 83 series skinset.
		k->Calc_Key_Coord=x3_coord;

		
	}		
	return  k;
}


void create_menus(GtkWidget *window,GdkEvent *event, GtkItemFactoryEntry * menu_items, int thisitems, const char *menuname)
{
	GtkAccelGroup *accel_group;
	GtkItemFactory *factory;
	GtkWidget *menu;
	GdkEventButton *bevent = (GdkEventButton *) event;

	/*gtk_image_factory_parse_rc()*/

	accel_group = gtk_accel_group_new();
	factory = gtk_item_factory_new(GTK_TYPE_MENU, menuname, accel_group);
	/* translatefunc */
	gtk_item_factory_create_items(factory, thisitems, menu_items, window);
	menu = factory->widget;

	gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);
	gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, bevent->button, bevent->time);
	gtk_widget_add_events(window, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);

}







