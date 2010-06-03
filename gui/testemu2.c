#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>

//#include <tilem.h>

#include <gui.h>
//#include <skinops.h>


/*  contra-sh : 
* ---18/08/09---
* - Draw the TI83 
* - Copy from testemu.c : Create the savname, Open the Rom, Get the model, Draw the lcdscreen (no animation for the moment)
* - Function event OnDestroy 
* - Modification of the Makefile (I hope it's good !? If you can control this... thx)
* - LEARNING HOW COMMIT !!   :D
* ---19/08/09---
* - New structure TilemCalcSkin : define the different filenames for the SkinSet (4 pieces).
* - Draw the other models _automatically_ . ;D
* ---20/08/09---
* - Create skin.c
* - Create gui.h (equivalent of tilem.h for the gui directory)
* - Move the code struct in gui.h and TilemCalcSkin* tilem_guess_skin_set(TilemCalc* calc) into skin.c (only one call in the main file to define the skin set) ;D
* - Detect a keyboard event (function keyboard_event() in skin.c actually).No treatment.
* - Detect an event click on mouse
* ---21/08/09---
* - Get the x and y values when click on mouse (now it will be easy to know how key is click on the pixmap, todo in priority : detect right click)
* ---24/08/09---
* - Detect right click.
* ---26/08/09---
* - New function : TilemKeyMap* tilem_guess_key_map(int id).Choose a TilemKeyMap with an id given in parameter. 
* ---27/08/09---
* - Extract the choice of the key_map from the mouse_event function.Execute only one time and it's more properly (and it will be easier to add the possibility to manage many skins and many keymaps).
* ---01/09/09---
* - Choose automatically the key_list. The TilemKeyList is already included in the TilemKeyMap structure...
* - New structure TilemKeyCoord (old TilemKeyMap).TilemKeyMap already contains TilemKeyCoord and TilemKeyList... 
* ---08/09/09---
* - New function tilem_set_coord to change the keypad coord.
* - New file event.c to group the GDKevent handling.
* - New function tilem_set_skin to change the skin.
* ---10/09/09---
* - Add the right click menu :D (0 signal connected without OnDestroy).Largely inspired from Tilem 0.973 and http://www.linux-france.org/article/devl/gtk/gtk_tut-11.html was a great inspiration too. 
* ---21/09/09---
* - Aouch I had seen that the left click doesn't work now! Problem : two callback for the only one button_press_event. (sorry for this version...)
* ---22/09/09---
* - Correction : only one callback now. mouse_event contains the create_menu and the gtk_menu_popup. (lot of time was spent on this problem)*
* ---23/09/09---
* - Change TilemKeyCoord to the 82 stats skin.Change Event.c too.
* ---06/10/09---
* - Beginning the correction : change the method for testing coordinates clicks (one line now) . The coordinates will be imported from a file soon.
* ---20/11/09---
* - After 1 week to learn Tiemu skin format...I have made my own Tilem skin Generator.Inspired from skinedit.It just generate compatible file with 0 coordinates values, and an default lcd coordinates.Not really necessary but very useful for testing and for learning how this f****** skin format works.It will be called skintool for the moment.
* ---27/11/09---
* - After blocking a problem for a week grrr... I succeed to adapt the TiEmu skinloader to TilEm (skinops.c and skinops.h).Little modifications
* ---28/11/09---
* - The mouse_event function now use a SKIN_INFOS struct. So delete TilemKeyCoord struct.
* ---02/12/09---
* - Add a GtkFileSelection (access by right click menu) and try to load another skin with this method.
* ---03/12/09---
* - Create a GLOBAL_SKIN_INFOS that contains a KEY_LIST struct (old TilemKeyList) and a SKIN_INFOS struct.
* ----04/12/09---
* - Delete the TilemKeyCoord, TilemKeyMap, TilemCalcSkin and TilemKeyList structs...
* ---05/12/09---
* - The GtkWidget *pWindow creation was moved from testemu2.c to event.c .The function is called GtkWidget* Draw_Screen(GLOBAL_SKIN_INFOS *gsi);
* ---07/12/09---
* - New feature : TilEm could load a skin by the right_click_menu ! It use GtkWidget* ReDraw_Screen(GLOBAL_SKIN_INFOS *gsi) in event.c. WAOUH !
* ---08/12/09---
* - Move Draw_Screen, ReDraw_Screen and create_menus in a new screen.c file. Change Makefile.
* ---14/12/09---
* - New feature : add a popup rom type selector (radio button) at the start of tilem. Showed but not used for the moment.
* - Connect the thread (no animation yet). Thanks to the "battle programming" 's spirit (hey bob ^^)
* ---18/12/09---
* - Launch the interactivity with emulation core. Could print into the draw area.
* ---09/03/10---
* - Restart working on this program ;D
* ---11/03/10---
* - I finally succeeded to connect the core. To print something into the lcd screen ! WahoOO ! This day is a great day !
* - I succeded to type numbers etc...
* - And now it works very well !! (the "button-release-event" is not catched by pWindow but by pLayout)
* ---15/03/10---
* - Create the scan_click function.Return wich keys was clicked.Print debug infos.
* - Create the debuginfos.h to group the ifdef for debugging. (different level and different type of debug msg)
* ---17/03/10---
* - Create the rom_choose_popup function to replace choose_rom_type.It use GtkDialog instead of GtkWindow.
* - rom_choose_popup _freeze_ the system... and get wich radio button is selected. So it will be easy to create the good emu.calc (and choose the default skin).
* ---18/03/10---
* - Resize the (printed) lcd area (gsi->emu->lcdwin) to fit(perfectly) into the skin.
* - Replace a lot of printf by D****_L*_A* to easily switch what debug infos were printed.
* - Try to make a nice debugging output (frames in ASCII ^^) :p
* - WahooOO , load a skin works perfectly.You can easily change skin _while running_, no error, no warning.
* - Could load automatically the good skin and run the good core using the choose_rom_popup() function and choose_skin_filename() function.
* ---30/03/10---
* - Create skin for following models : 73, 81, 82, 83+ and 84+.
* - Fix bug in tool.c .Modification of tool.c to create radio button more properly.
* ---31/03/10---
* - Create skin for following model : 83 . Based on my own calc (take a foto with my iphone 3GS :D)
* ---01/04/10---
* - New feature : Save calc state and load state. State are stored in a separate dir called sav/ . (using benjamin 's function)
* - New feature : Change view to see only the lcd. I finally choose to add it into a GtkLayout. So you can maximize it, but there was problem with add_event.
* ---02/04/10---
* - Add popup function to just print error message.You must give the message and gsi as parameter, and it run a modal dialog.
* - Some cleaning.
* ---23/04/10---
* - Add config.c file to manage config.dat (create, read, modif etc...).
* ---31/05/10---
* - Start from scratch a totally new debugger :D.Just draw button with nice GtkImages.Actually in essai2 directory.
* - Get and resize pixmaps (png) to 36 * 36 pixels for the debugger.
* ---01/06/10---
* - Add the debugger to tilem. Load registers values.
* - Add a new feature : switch the top level window "borderless".It can be switch by clicking on right click menu entry.
* ---02/06/10---
* - Create the GtkTreeView (debugger).
*/


/* #########  MAIN  ######### */

int main(int argc, char **argv)
{
	
	const char* romname="xp.rom";
	char* savname;
	char* p;
	FILE *romfile,*savfile, *config_file;
	char calc_id;
	
	GLOBAL_SKIN_INFOS *gsi;
	gsi=malloc(sizeof(GLOBAL_SKIN_INFOS));
	gsi->si=malloc(sizeof(SKIN_INFOS));
	
	
	/* Init GTK+ */
	g_thread_init(NULL);
	gtk_init(&argc, &argv);
	gtk_rc_parse_string(rcstr);/* change style */
	printf("Running tilem ^^ \n");
	
	/* Init isDebuggerRunning */
	gsi->isDebuggerRunning=FALSE;
	
	/* Init tilem config file */
	config_file = g_fopen("config.dat", "rt");
	if (!config_file) 
	{
		printf("config.dat not exists \n");
		create_config_dat();
	}
	/* end */
	
	/* Get the romname */
	if (argc >= 2)  /* If they are 2 parameters or more, the romfile is the second. */
	{
		romname = argv[1];
		gsi->RomName=(char*) malloc(strlen(romname)+1); /* save for config.c */
		strcpy(gsi->RomName,romname);
	}
	/* end */
	
	/* Create the savname */
	savname = g_malloc(4 + strlen(romname) + 5); /* sav/ (4 char) + romname + .sav (4 char) + \0 (1 char) */
	strcpy(savname,"sav/");
	strcat(savname, romname);
	
	if ((p = strrchr(savname, '.'))) 
	{
		strcpy(p, ".sav");
		DGLOBAL_L0_A0("**************** fct : main ****************************\n");
		DGLOBAL_L0_A2("*  romname=%s savname=%s           *\n",romname,savname);	
		DGLOBAL_L0_A0("********************************************************\n");
	} else {
		strcat(savname, ".sav");
	}
	/* end */
	
	
	/* Open the romfile */
	romfile = g_fopen(romname, "rb");
	if (!romfile) 
	{
		perror(romname);
		return 1;
	}
	/* end */
	
	savfile = g_fopen(savname, "rt");
	
	
	/* The program must wait user choice before continuing */
	if((calc_id=choose_rom_popup())=='0') {	/* Query for the model */
			DGLOBAL_L0_A0(" ---------> Let Tilem guess for you ! <---------\n");
			if (!(calc_id=tilem_guess_rom_type(romfile))) {	
				fprintf(stderr, "%s: unknown calculator type\n", romname);
				if(romfile!=NULL)
					fclose(romfile);
				return 1;
			} 
	}
	
	gsi->emu=malloc(sizeof(TilemCalcEmulator));
	gsi->emu->calc = tilem_calc_new(calc_id);
	
	tilem_calc_load_state(gsi->emu->calc, romfile, savfile);
	
	if (savfile)
		fclose(savfile);
		

	

	gsi->emu->run_mutex = g_mutex_new();
	gsi->emu->calc_mutex = g_mutex_new();
	gsi->emu->lcd_mutex = g_mutex_new();
	gsi->emu->exiting = FALSE;
	gsi->emu->forcebreak = FALSE;
	gsi->emu->calc->lcd.emuflags = TILEM_LCD_REQUIRE_DELAY;
	gsi->emu->calc->flash.emuflags = TILEM_FLASH_REQUIRE_DELAY;
	
	DGLOBAL_L0_A0("**************** fct : main ****************************\n");
	DGLOBAL_L0_A1("*  calc_id= %c                                            *\n",calc_id);
	DGLOBAL_L0_A1("*  emu.calc->hw.model= %c                               *\n",gsi->emu->calc->hw.model_id);	
	DGLOBAL_L0_A1("*  emu.calc->hw.name= %s                             *\n",gsi->emu->calc->hw.name);		
	DGLOBAL_L0_A1("*  emu.calc->hw.name[3]= %c                             *\n",gsi->emu->calc->hw.name[3]);
	DGLOBAL_L0_A0("********************************************************\n");
	
	choose_skin_filename(gsi->emu->calc,gsi);
		
	gsi->pWindow=draw_screen(gsi);
	
	
	
	/* ####### BEGIN THE GTK_MAIN_LOOP ####### */
	gtk_main();
	
	/* Save the state */
	DGLOBAL_L2_A1("Save state ? %d\n",SAVE_STATE);
	if(SAVE_STATE==1) {
		romfile = g_fopen(romname, "wb");
		savfile = g_fopen(savname, "wt");
		tilem_calc_save_state(gsi->emu->calc, romfile, savfile);
	}
	


	
	
    return EXIT_SUCCESS;
}












