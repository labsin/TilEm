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
#include <glib.h>
#include <ticalcs.h>
#include <tilem.h>

#include "gui.h"
#include "files.h"
#include "icons.h"
#include "msgbox.h"

/* CMD LINE OPTIONS */
static gchar* cl_romfile = NULL;
static gchar* cl_skinfile = NULL;
static gint cl_model = 2;
static gchar* cl_statefile = NULL;
static gchar* cl_file_to_load = NULL;
static gboolean cl_skinless_flag = FALSE;
static gboolean cl_reset_flag = FALSE;
static gchar* cl_file_to_exec = NULL;
static gchar* cl_getvar = NULL;
static gchar* cl_macro_to_run = NULL;
static gboolean cl_debug_flag = FALSE;
static gchar* cl_listingfile = NULL;
static gchar* cl_symbolfile = NULL;
static gchar* cl_link = NULL;
static gboolean cl_fullspeed_flag = FALSE;
static gchar* cl_batch = NULL;
static gboolean cl_nogui_flag = FALSE;
static gboolean cl_save_flag = FALSE;


static GOptionEntry entries[] =
{
	{ "rom", 'r', 0, G_OPTION_ARG_FILENAME, &cl_romfile, "The rom file to run", "N" },
	{ "skin", 'k', 0, G_OPTION_ARG_FILENAME, &cl_skinfile, "The skin file to use", "N" },
	{ "model", 'm', 0, G_OPTION_ARG_INT, &cl_model, "The model to use", "N" },
	{ "state-file", 's', 0, G_OPTION_ARG_FILENAME, &cl_statefile, "The state-file to use", "M" },
	{ "load-file", 'f', 0, G_OPTION_ARG_FILENAME, &cl_file_to_load, "Load this file at startup (the file is not executed)", "M" },
	{ "without-skin", 'l', 0, G_OPTION_ARG_NONE, &cl_skinless_flag, "Start in skinless mode", "M" },
	{ "reset", 0, 0, G_OPTION_ARG_NONE, &cl_reset_flag, "Reset the calc at startup", NULL },
	{ "execute", 'x', 0, G_OPTION_ARG_FILENAME, &cl_file_to_exec, "The file to exec at startup", NULL },
	{ "get-var", 0, 0, G_OPTION_ARG_STRING, &cl_getvar, "Get a var at startup", NULL },
	{ "play-macro", 'p', 0, G_OPTION_ARG_FILENAME, &cl_macro_to_run, "Run this macro at startup", NULL },
	{ "debug", 'd', 0, G_OPTION_ARG_NONE, &cl_debug_flag, "Launch debugger", NULL },
	{ "listing-file", 'L', 0, G_OPTION_ARG_STRING, &cl_listingfile, "Load listing file in the debugger", NULL },
	{ "symbol-file", 'S', 0, G_OPTION_ARG_STRING, &cl_symbolfile, "Load symbol file in the debugger", NULL },
	{ "set-full-speed", 0, 0, G_OPTION_ARG_NONE, &cl_fullspeed_flag, "Limit the speed", NULL },
	{ "link", 0, 0, G_OPTION_ARG_STRING, &cl_link, "Link cable", NULL },
	{ "batch", 0, 0, G_OPTION_ARG_FILENAME, &cl_batch, "Launch this batch file", NULL },
	{ "no-gui", 0, 0, G_OPTION_ARG_NONE, &cl_nogui_flag, "Don't display the gui", NULL },
	{ "save", 0, 0, G_OPTION_ARG_NONE, &cl_save_flag, "Save state (if running in batch mode and the program finishes normally) ", NULL },
	{ NULL }
};


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
	
	/* >>>> CMD LINE PARSING */
	GError *error = NULL;
	GOptionContext *context;

	context = g_option_context_new ("-= TIlEm Is a Linux EMulator =-");
	g_option_context_add_main_entries (context, entries, NULL);
	g_option_context_add_group (context, gtk_get_option_group (TRUE));
	if (!g_option_context_parse (context, &argc, &argv, &error))
	{
		g_print ("option parsing failed: %s\n", error->message);
		exit (1);
	}
	/* <<<< */

	load_initial_rom(emu, cl_romfile, cl_statefile);

	emu->ewin = tilem_emulator_window_new(emu);

	/* >>>> Command line */
	if (cl_skinfile)
		tilem_emulator_window_set_skin(emu->ewin, cl_skinfile);

	tilem_emulator_window_set_skin_disabled(emu->ewin, cl_skinless_flag);
	/* <<<< */

	gtk_widget_show(emu->ewin->window);

	tilem_calc_emulator_run(emu);

	/* >>>> Command line */
	if(cl_reset_flag) 
		tilem_calc_emulator_reset(emu);
	tilem_calc_emulator_set_limit_speed(emu, !cl_fullspeed_flag);
	/* <<<< */
	
	tilem_keybindings_init(emu, emu->calc->hw.name);

	ticables_library_init();
	tifiles_library_init();
	ticalcs_library_init();
	
	/* >>>> Command line */
	if (cl_file_to_load) /* Priority : High */
		tilem_link_send_file(emu, cl_file_to_load, -1, TRUE, TRUE);
	if (cl_macro_to_run) { /* Priority : Medium */
		printf("macro to load : %s\n", cl_macro_to_run);
		tilem_macro_load(emu, cl_macro_to_run); 		
	}
	if(cl_debug_flag) /* Priority : low */
		launch_debugger(emu->ewin);
	if(cl_getvar) {
		tilem_link_get_dirlist(emu);	
		tilem_calc_emulator_begin(emu, &tilem_link_get_var_at_startup, &tilem_link_get_var_at_startup_finished, cl_getvar); 
		
	
	}
	/* <<<< */
		

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
