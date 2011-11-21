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
#include <stdlib.h>
#include <gtk/gtk.h>
#include <ticalcs.h>
#include <tilem.h>

#include "gui.h"
#include "files.h"
#include "icons.h"
#include "msgbox.h"

/* #########  MAIN  ######### */

static void load_initial_rom(TilemCalcEmulator *emu,
                             const char *cmdline_rom_name,
                             const char *cmdline_state_name)
{
	GError *err = NULL;
	char *modelname;
	int model;

	/* If a ROM file is specified on the command line, use that
	   (and no other) */

	if (cmdline_rom_name) {
		if (tilem_calc_emulator_load_state(emu, cmdline_rom_name,
		                                   cmdline_state_name, 0, &err))
			return;
		else if (!err)
			exit(0);
		else {
			g_printerr("%s\n", err->message);
			exit(1);
		}
	}

	/* Try to load the most recently used model */
	tilem_config_get("recent", "last_model/s", &modelname, NULL);
	if (modelname && (model = name_to_model(modelname))) {
		if (tilem_calc_emulator_load_state(emu, NULL, NULL,
		                                   model, &err))
			return;
		else if (!err)
			exit(0);
		else {
			messagebox01(NULL, GTK_MESSAGE_ERROR,
			             "Unable to load calculator state",
			             "%s", err->message);
			g_clear_error(&err);
		}
	}

	#if 0
	/* Try to load the most recently used rom */
	char* rom_name = NULL;
	tilem_config_get("recent", "rom/f", &rom_name, NULL);
	if(rom_name) {
		if (tilem_calc_emulator_load_state(emu, rom_name, NULL, 0, &err)) {
			g_free(rom_name);
			return;
		} else if (!err) {
			g_free(rom_name);
			exit(0);
		} else {
			g_free(rom_name);
			messagebox01(NULL, GTK_MESSAGE_ERROR,
			             "Unable to load calculator rom",
			             "%s", err->message);
			g_clear_error(&err);
		}
	}
	#endif
	

	

	/* Prompt user for a ROM file */

	while (!emu->calc) {
		if (!tilem_calc_emulator_prompt_open_rom(emu))
			exit(0);
	}
}

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

	cl = tilem_cmdline_new();
	tilem_cmdline_get_args(argc, argv, cl);

	load_initial_rom(emu, cl->RomName, cl->SavName);

	emu->ewin = tilem_emulator_window_new(emu);

	if (cl->SkinFileName)
		tilem_emulator_window_set_skin(emu->ewin, cl->SkinFileName);

	if (cl->isStartingSkinless)
		tilem_emulator_window_set_skin_disabled(emu->ewin, TRUE);

	gtk_widget_show(emu->ewin->window);

	tilem_calc_emulator_run(emu);
	
	tilem_keybindings_init(emu, emu->calc->hw.name);

	ticables_library_init();
	tifiles_library_init();
	ticalcs_library_init();
		
	if (cl->FileToLoad) /* Given as parameter ? */
		tilem_link_send_file(emu, cl->FileToLoad, -1, TRUE, TRUE);
	if (cl->MacroToPlay) { /* Given as parameter ? */
		printf("macro to load : %s\n", cl->MacroToPlay);
		tilem_macro_load(emu, cl->MacroToPlay); 		
	}

	g_signal_connect(emu->ewin->window, "destroy",
	                 G_CALLBACK(gtk_main_quit), NULL);

	gtk_main();

	tilem_calc_emulator_pause(emu);
	
	tilem_emulator_window_free(emu->ewin);
	tilem_calc_emulator_free(emu);

	menurc_path = get_config_file_path("menurc", NULL);
	gtk_accel_map_save(menurc_path);
	g_free(menurc_path);

	ticables_library_exit();
	tifiles_library_exit();
	ticalcs_library_exit();

	return 0;
}
