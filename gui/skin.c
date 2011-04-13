/*
 * TilEm II
 *
 * Copyright (c) 2010-2011 Thibault Duponchelle
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
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <ticalcs.h>
#include <tilem.h>

#include "gui.h"
#include "files.h"
#include "msgbox.h"

/* choose_skin_filename is used to give the name of the default skin file name to load when the emulator starts */
void tilem_choose_skin_filename_by_default(GLOBAL_SKIN_INFOS *gsi)
{
	char *name;

	/* FIXME: try fallbacks if the default skin doesn't exist */

	if (!gsi->emu->cmdline->SkinFileName) {
		name = g_strconcat(gsi->emu->calc->hw.name, ".skn", NULL);
		gsi->emu->cmdline->SkinFileName = get_shared_file_path("skins", name, NULL);
		g_free(name);
	}
}

/* GtkFileSelection */
void tilem_user_change_skin(GLOBAL_SKIN_INFOS *gsi)
{
	char* file_selected = NULL ;
	DSKIN_L0_A0("\nSKINSELECTION\n");
	
	if(gsi->view==1) 
	{
		DGLOBAL_L0_A0("Use >>Switch view<< before !\n");
		popup_error("Use >>Switch view<< before !\n", gsi);
	} else {
		
		/* Show a nice chooser dialog, and get the filename selected */	

		file_selected = select_file(gsi, "./skn/");
		if(file_selected != NULL) {
			g_free(gsi->emu->cmdline->SkinFileName);
			gsi->emu->cmdline->SkinFileName = file_selected;
			redraw_screen(gsi);
		}
	}
}

