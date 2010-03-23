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
* ---18/03/10 ---
* - Resize the (printed) lcd area (gsi->emu->lcdwin) to fit(perfectly) into the skin.
* - Replace a lot of printf by DEBUGGING****_L*_A* to easily switch what debug infos were printed.
* - Try to make a nice debugging output (frames in ASCII ^^) :p
* - WahooOO , load a skin works perfectly.You can easily change skin _while running_, no error, no warning.
* - Could load automatically the good skin and run the good core using the choose_rom_popup() function and choose_skin_filename() function.
*/


/* #########  MAIN  ######### */

int main(int argc, char **argv)
{
	
	const char* romname="xp.rom";
	char* savname;
	char* p;
	FILE *romfile,*savfile;
	char calc_id;
	

	
	/* Init GTK+ */
	g_thread_init(NULL);
	gtk_init(&argc, &argv);
	gtk_rc_parse_string(rcstr);
	printf("Running tilem ^^ \n");
	
	/* Get the romname */
	if (argc >= 2)  /* If they are 2 parameters or more, the romfile is the second. */
	{
		romname = argv[1];
	}
	/* end */
	
	/* Create the savname */
	savname = g_malloc(strlen(romname) + 5);
	strcpy(savname, romname);
	
	if ((p = strrchr(savname, '.'))) 
	{
		strcpy(p, ".sav");
		DEBUGGINGGLOBAL_L0_A0("**************** fct : main ****************************\n");
		DEBUGGINGGLOBAL_L0_A2("*  romname=%s savname=%s           *\n",romname,savname);	
		DEBUGGINGGLOBAL_L0_A0("********************************************************\n");
	}
	else 
	{
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
	
	
	
	
	GLOBAL_SKIN_INFOS *gsi;
	gsi=malloc(sizeof(GLOBAL_SKIN_INFOS));
	gsi->si=malloc(sizeof(SKIN_INFOS));
	gsi->romfile=romfile;
	gsi->romname=(char*)malloc(sizeof(strlen(romname)));
	strcpy(gsi->romname,romname);
	
	
	
	/* The program must wait user choice before continuing */
	if((calc_id=choose_rom_popup())=='0') {
		calc_id=tilem_guess_rom_type(romfile);
		DEBUGGINGGLOBAL_L0_A0(" ---------> Let Tilem guess for you ! <---------\n");
	}
	gsi->emu=malloc(sizeof(TilemCalcEmulator));
	gsi->emu->calc = tilem_calc_new(calc_id);
	
	tilem_calc_load_state(gsi->emu->calc, romfile, savfile);
	gsi->emu->run_mutex = g_mutex_new();
	gsi->emu->calc_mutex = g_mutex_new();
	gsi->emu->lcd_mutex = g_mutex_new();
	gsi->emu->exiting = FALSE;
	gsi->emu->forcebreak = FALSE;
	gsi->emu->calc->lcd.emuflags = TILEM_LCD_REQUIRE_DELAY;
	gsi->emu->calc->flash.emuflags = TILEM_FLASH_REQUIRE_DELAY;
	
	DEBUGGINGGLOBAL_L0_A0("**************** fct : main ****************************\n");
	DEBUGGINGGLOBAL_L0_A1("*  calc_id= %c                                            *\n",calc_id);
	DEBUGGINGGLOBAL_L0_A1("*  emu.calc->hw.model= %c                               *\n",gsi->emu->calc->hw.model_id);	
	DEBUGGINGGLOBAL_L0_A1("*  emu.calc->hw.name= %s                             *\n",gsi->emu->calc->hw.name);		
	DEBUGGINGGLOBAL_L0_A1("*  emu.calc->hw.name[3]= %c                             *\n",gsi->emu->calc->hw.name[3]);
	DEBUGGINGGLOBAL_L0_A0("********************************************************\n");
	
	choose_skin_filename(gsi->emu->calc,gsi);
		
	gsi->pWindow=draw_screen(gsi);
		       
	/* ####### BEGIN THE GTK_MAIN_LOOP ####### */
	gtk_main();
	


	
	
    return EXIT_SUCCESS;
}












