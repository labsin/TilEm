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
	TilemCalcEmulator* emu;
	TilemCmdlineArgs* cl;

	g_thread_init(NULL);
	gtk_init(&argc, &argv);

	g_set_application_name("TilEm");

	init_custom_icons();

	emu = tilem_calc_emulator_new();

	emu->ewin = g_slice_new0(TilemEmulatorWindow);
	emu->ewin->emu = emu;
	emu->ssdlg = g_slice_new0(TilemScreenshotDialog);
	emu->ssdlg->emu = emu;
	emu->linkpb = g_slice_new0(TilemLinkProgress);
	emu->linkpb->emu = emu;

	cl = tilem_cmdline_new();
	tilem_cmdline_get_args(argc, argv, cl);

	if (!tilem_calc_emulator_load_state(emu, cl->RomName)) {
		tilem_calc_emulator_free(emu);
		return 1;
	}

	DGLOBAL_L0_A0("**************** fct : main ****************************\n");
	DGLOBAL_L0_A1("*  calc_id= %c                                            *\n",calc_id);
	DGLOBAL_L0_A1("*  emu.calc->hw.model= %c                               *\n",emu->calc->hw.model_id);	
	DGLOBAL_L0_A1("*  emu.calc->hw.name= %s                             *\n",emu->calc->hw.name);		
	DGLOBAL_L0_A1("*  emu.calc->hw.name[3]= %c                             *\n",emu->calc->hw.name[3]);
	DGLOBAL_L0_A0("********************************************************\n");

	if (cl->SkinFileName) {
		emu->ewin->skin_file_name = g_strdup(cl->SkinFileName);
	}
	else {
		tilem_choose_skin_filename_by_default(emu->ewin);
		tilem_config_get("settings",
		                 "skin_disabled/b", &emu->ewin->isSkinDisabled,
		                 NULL);
	}

	if (cl->isStartingSkinless)
		emu->ewin->isSkinDisabled = TRUE;

	/* Draw skin */	
	draw_screen(emu->ewin);

	tilem_calc_emulator_run(emu);

	
	tilem_keybindings_init(emu, emu->calc->hw.name);
		
	if (cl->FileToLoad) /* Given as parameter ? */
		tilem_load_file_from_file_at_startup(emu, cl->FileToLoad);
	if (cl->MacroToPlay) { /* Given as parameter ? */
		play_macro_default(emu, cl->MacroToPlay); 		
	}

	gtk_main();

	tilem_calc_emulator_pause(emu);
	
	/* Save the state */
	if(SAVE_STATE==1)
		tilem_calc_emulator_save_state(emu);

	tilem_calc_emulator_free(emu);

	return 0;
}












