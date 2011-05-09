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
# include <config.h>
#endif

#include <stdio.h>
#include <gtk/gtk.h>
#include <ticalcs.h>
#include <tilem.h>

#include "gui.h"
#include "icons.h"



/* #########  MAIN  ######### */

int main(int argc, char **argv)
{
	GLOBAL_SKIN_INFOS *gsi;

	g_thread_init(NULL);
	gtk_init(&argc, &argv);

	g_set_application_name("TilEm");

	init_custom_icons();

	gsi = g_new0(GLOBAL_SKIN_INFOS, 1);
	


	/* TilemCalcEmu part */
	gsi->emu = tilem_calc_emulator_new();

	gsi->emu->guiwidget = g_new0(TilemGuiWidget, 1);
	gsi->emu->guiwidget->pWindow = NULL;
	gsi->emu->si=NULL;
	gsi->emu->macro_file = NULL;
	gsi->emu->keyhandle = g_new0(TilemKeyHandle, 1);
	gsi->emu->keyhandle->mouse_key = 0;
	gsi->emu->keyhandle->key_queue = NULL;
	gsi->emu->keyhandle->key_queue_len = 0;
	gsi->emu->keyhandle->key_queue_timer = 0;
	gsi->emu->guiflags = g_new0(TilemGuiStateFlags, 1);
	gsi->emu->guiflags->isMacroRecording = FALSE;
	gsi->emu->guiflags->isAnimScreenshotRecording = FALSE;
	gsi->emu->guiflags->isDebuggerRunning=FALSE;

	TilemCmdlineArgs * cmdline = tilem_cmdline_new();
	tilem_cmdline_get_args(argc, argv, cmdline);
	gsi->emu->cmdline = cmdline;
	

	if (!tilem_calc_emulator_load_state(gsi->emu, cmdline->RomName)) {
		tilem_calc_emulator_free(gsi->emu);
		return 1;
	}

	DGLOBAL_L0_A0("**************** fct : main ****************************\n");
	DGLOBAL_L0_A1("*  calc_id= %c                                            *\n",gsi->calc_id);
	DGLOBAL_L0_A1("*  emu.calc->hw.model= %c                               *\n",gsi->emu->calc->hw.model_id);	
	DGLOBAL_L0_A1("*  emu.calc->hw.name= %s                             *\n",gsi->emu->calc->hw.name);		
	DGLOBAL_L0_A1("*  emu.calc->hw.name[3]= %c                             *\n",gsi->emu->calc->hw.name[3]);
	DGLOBAL_L0_A0("********************************************************\n");
	

	if (cmdline->SkinFileName == NULL) {
		tilem_choose_skin_filename_by_default(gsi);
		tilem_config_get("settings",
		                 "skin_disabled/b", &gsi->emu->guiflags->isSkinDisabled,
		                 NULL);
	}

	if (cmdline->isStartingSkinless)
		gsi->emu->guiflags->isSkinDisabled = TRUE;

	/* Draw skin */	
	gsi->emu->guiwidget->pWindow=draw_screen(gsi);

	tilem_calc_emulator_run(gsi->emu);

	
	tilem_keybindings_init(gsi, gsi->emu->calc->hw.name);
		
	if(cmdline->FileToLoad != NULL) /* Given as parameter ? */
		tilem_load_file_from_file_at_startup(gsi, cmdline->FileToLoad);
	if(cmdline->MacroToPlay != NULL) { /* Given as parameter ? */
		play_macro_default(gsi, cmdline->MacroToPlay); 		
	}

	gtk_main();

	tilem_calc_emulator_pause(gsi->emu);
	
	/* Save the state */
	if(SAVE_STATE==1)
		tilem_calc_emulator_save_state(gsi->emu);

	tilem_calc_emulator_free(gsi->emu);

	return 0;
}












