/*
 * TilEm II
 *
 * Copyright (c) 2010-2011 Thibault Duponchelle
 * Copyright (c) 2010-2011 Benjamin Moody
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <gtk/gtk.h>
#include <ticalcs.h>
#include <tilem.h>

#include "gui.h"
#include "files.h"
#include "icons.h"
#include "filedlg.h"



/* #########  MAIN  ######### */

int main(int argc, char **argv)
{
	TilemCalcEmulator* emu;
	TilemCmdlineArgs* cl;
	char *menurc_path;

	g_thread_init(NULL);
	gtk_init(&argc, &argv);

	g_set_application_name("TilEm");

	menurc_path = get_shared_file_path("menurc", NULL);
	if (menurc_path)
		gtk_accel_map_load(menurc_path);
	g_free(menurc_path);

	init_custom_icons();

	emu = tilem_calc_emulator_new();

	emu->linkpb = g_slice_new0(TilemLinkProgress);
	emu->linkpb->emu = emu;

	cl = tilem_cmdline_new();
	tilem_cmdline_get_args(argc, argv, cl);
	

	/* Check if user gives a romfile as cmd line parameter */
	/* In this cas, try to load a saved rom */
	if(!cl->RomName)
		tilem_config_get("recent", "rom/f", &cl->RomName, NULL);

	/* If no saved romfile */
	/* In this case, prompt the user wich file he want to load */
	if(!cl->RomName) {
		char * basename;
		tilem_config_get("recent", "basedir/f", &basename, NULL);
		cl->RomName = prompt_open_file("Open rom", NULL, basename, "ROM files", "*.rom", "All files", "*", NULL);
		g_free(basename);
	}
	
	/* Check if user cancelled the prompt */
	/* In this case, jut close tilem2 (can't choose for him) */
	if(!cl->RomName) {	
		return 0;
	} else {
		/* Save the directory */
		gchar* folder = g_path_get_dirname(cl->RomName);
		printf("basename : %s\n", folder);
		tilem_config_set("recent", "rom/f", cl->RomName, "basedir/f", folder, NULL);		
		g_free(folder);
	}
	
	if (!tilem_calc_emulator_load_state(emu, cl->RomName)) {
		tilem_calc_emulator_free(emu);
		return 1;
	}

	emu->ewin = tilem_emulator_window_new(emu);

	if (cl->SkinFileName)
		tilem_emulator_window_set_skin(emu->ewin, cl->SkinFileName);

	if (cl->isStartingSkinless)
		tilem_emulator_window_set_skin_disabled(emu->ewin, TRUE);

	gtk_widget_show(emu->ewin->window);

	tilem_calc_emulator_run(emu);
	
	tilem_keybindings_init(emu, emu->calc->hw.name);
		
	if (cl->FileToLoad) /* Given as parameter ? */
		tilem_load_file_from_file_at_startup(emu, cl->FileToLoad);
	if (cl->MacroToPlay) { /* Given as parameter ? */
		tilem_macro_load(emu, cl->MacroToPlay); 		
	}

	gtk_main();

	tilem_calc_emulator_pause(emu);
	
	/* Save the state */
	if(SAVE_STATE==1)
		tilem_calc_emulator_save_state(emu);

	tilem_emulator_window_free(emu->ewin);
	tilem_calc_emulator_free(emu);

	menurc_path = get_config_file_path("menurc", NULL);
	gtk_accel_map_save(menurc_path);
	g_free(menurc_path);

	return 0;
}
