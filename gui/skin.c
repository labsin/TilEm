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
#include "msgbox.h"

char* hwname[11] = {"ti73", "ti76", "ti81", "ti82", "ti83", "ti83p", "ti84p", "ti84pse", "ti84pns", "ti85", "ti86"};
char* defaultskin[11] = {"ti73.skn","ti76.skn","ti81.skn","ti82.skn","ti83.skn","ti83plus.skn","ti84plus.skn","ti84plus.skn","ti84plus.skn","ti82.skn","ti82.skn" };

/* choose_skin_filename is used to give the name of the default skin file name to load when the emulator starts */
void tilem_choose_skin_filename_by_default(GLOBAL_SKIN_INFOS *gsi) {

	int i;

	/* Compare the emu->calc->hw.name with a tab defined into skin.h
	   Set the associated skinname */
	for(i = 0; hwname[i]; i++) {
		if(strcmp(gsi->emu->calc->hw.name, hwname[i]) == 0){
			//printf("found: %s\n", gsi->emu->calc->hw.name);

			/* Get the default directory wich contains the skins */
			char* basedir = tilem_config_universal_getter("skin", "basedir");
			printf("basedir : %s\n", basedir);
			gsi->emu->cmdline->SkinFileName = (char*) malloc(strlen(basedir) * sizeof(char) + strlen(defaultskin[i]) * sizeof(char) +1);
			strcpy(gsi->emu->cmdline->SkinFileName, basedir);	
			strcat(gsi->emu->cmdline->SkinFileName, defaultskin[i]);	
		}
	}

	/* Load a default if no correspondance found (to do not crash)
	   User should change after by loading skin */
	if(!gsi->emu->cmdline->SkinFileName)	{
			char* basedir = tilem_config_universal_getter("skin", "basedir");
			printf("basedir : %s\n", basedir);
			gsi->emu->cmdline->SkinFileName = (char*) malloc(strlen(basedir) * sizeof(char) + strlen(defaultskin[i]) * sizeof(char) +1);
			strcpy(gsi->emu->cmdline->SkinFileName, basedir);	
			gsi->emu->cmdline->SkinFileName = (char*) malloc(strlen(defaultskin[i]) * sizeof(char) +1);
			strcpy(gsi->emu->cmdline->SkinFileName, defaultskin[i]);
	}
		
	//printf("skinfilename: %s\n", gsi->SkinFileName);
}

/* GtkFileSelection */
void tilem_user_change_skin(GLOBAL_SKIN_INFOS *gsi) {
	
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
			DSKIN_L0_A2("gsi->si->name : %s gsi->si->type  %d\n",gsi->si->name,gsi->si->type);
			DSKIN_L0_A1("file to load : %s\n", file_selected);
		
			gsi->emu->cmdline->SkinFileName= (char*) malloc(strlen(file_selected) * sizeof(char));
			strcpy(gsi->emu->cmdline->SkinFileName, file_selected);
			printf("Just before loading skin : gsi->SkinFileName : %s\n ", gsi->emu->cmdline->SkinFileName);
			free(file_selected);
			
			/* redraw the skin into the Window (here gsi->pWindow) */
			redraw_screen(gsi);
		}
	}
}

