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
#include <getopt.h>
#include <gtk/gtk.h>
#include <ticalcs.h>
#include <tilem.h>

#include "gui.h"

/* Print help */
void tilem_cmdline_help(char *name, int ret) 
{
        fprintf(stdout,"Usage: %s -r <rom> [OPTIONS]\n"
                        "\n\t--== TI Linux EMulator 2.00 ==--\n"
                        "\t    Tilem Is a Linux EMulator\n"
                        "\t--help\t\tshow this message\n"
                        "\t-r <rom>\trom to run\n"
                        "\t-f <file>\tfile to load\n"
                        "\t-k <skin>\tskin to display\n"
                        "\t-m <macro>\tmacro to run\n"
                        "\t-s <save>\tsave state to load\n"
                        "\t-l \t\tstart in skinless mode\n", name);

        exit(ret);
}

/* Create a structure to handle cmdline args and set all fields to null */
TilemCmdlineArgs* tilem_cmdline_new() {
	/* TilemCmdlineArgs */
	TilemCmdlineArgs *cmdline = g_new0(TilemCmdlineArgs, 1);
	cmdline->SkinFileName = NULL;
	cmdline->RomName = NULL;
	cmdline->SavName = NULL;
	cmdline->FileToLoad = NULL;
	cmdline->SavName = NULL;
	cmdline->MacroToPlay = NULL;
	cmdline->isStartingSkinless = FALSE;
	
	if(!cmdline)
		exit(EXIT_FAILURE);	
	return cmdline;
}

/* Get args using getopt */
int tilem_cmdline_get_args(int argc, char* argv[], TilemCmdlineArgs* cmdline) {
       	
       	char options;

	if (argc <= 2)
                return 0;
        else if (strcmp(argv[1], "--help") == 0)
                tilem_cmdline_help(argv[0],0);

        while((options = getopt(argc,argv, "s:f:r:k:m:l")) != -1)
        {
                switch(options) //options -X de la ligne
                {		//de commande
			case 's':
                        /*printf("arg : s, optarg = %s\n", optarg);*/
			cmdline->SavName = optarg;
			break;
                        case 'r':
                        /*printf("arg : r, optarg = %s\n", optarg);*/
			cmdline->RomName = optarg;
                        break;
                        case 'f':
                        /*printf("arg : f, optarg = %s\n", optarg);*/
			cmdline->FileToLoad = optarg;
                        break;
                        case 'k':
                        /*printf("arg : k, optarg = %s\n", optarg);*/
			cmdline->SkinFileName = optarg;
                        break;
                        case 'm':
                        /*printf("arg : m, optarg = %s\n", optarg);*/
			cmdline->MacroToPlay = optarg;
                        break;
			case 'l':
                        /*printf("arg : l\n");*/
			cmdline->isStartingSkinless = TRUE;
                        break;
			
                        default :
                        fprintf(stderr,"Erreur d'option\n"); 
                        tilem_cmdline_help(argv[0],-1);
                        break;
                                                                                                                   
                }
        }

	return 0;
}

/* Create the savname */
void create_savname(TilemCmdlineArgs* cmdline) {

	char* p;
	
	if(cmdline->RomName != NULL) 
		if(cmdline->SavName == NULL) {
			
			cmdline->SavName = g_malloc(strlen(cmdline->RomName) + 5); /* sav/ (4 char) + romname + .sav (4 char) + \0 (1 char) */
			memset(cmdline->SavName, 0 , strlen(cmdline->RomName));
			strcat(cmdline->SavName, cmdline->RomName);
			
			if ((p = strrchr(cmdline->SavName, '.'))) 
			{
				strcpy(p, ".sav");
				printf("RomName=%s SavName=%s\n",cmdline->RomName, cmdline->SavName);	
			} else {
				strcat(cmdline->SavName, ".sav");
		}
	}
}
