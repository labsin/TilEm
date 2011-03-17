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


#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <gui.h>

#include <getopt.h>

/* Print help */
void help(char *name, int ret) 
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

/* Get args using getopt */
int getargs(int argc, char* argv[], TilemCmdlineArgs* cmdline) {
       	
       	char options;

	if (argc <= 2)
                help(argv[0],-1);
        else if (strcmp(argv[1], "--help") == 0)
                help(argv[0],0);

        while((options = getopt(argc,argv, "s:f:r:k:m:l")) != -1)
        {
                switch(options) //options -X de la ligne
                {		//de commande
			case 's':
                        printf("arg : s, optarg = %s\n", optarg);
			cmdline->SavName = optarg;
                        printf("cmdline->SavName = %s\n", cmdline->SavName);
			break;
                        case 'r':
                        printf("arg : r, optarg = %s\n", optarg);
			cmdline->RomName = optarg;
                        printf("cmdline->RomName = %s\n", cmdline->RomName);
                        break;
                        case 'f':
                        printf("arg : f, optarg = %s\n", optarg);
			cmdline->FileToLoad = optarg;
                        printf("cmdline->FileToLoad = %s\n", cmdline->FileToLoad);
                        break;
                        case 'k':
                        printf("arg : k, optarg = %s\n", optarg);
			cmdline->SkinFileName = optarg;
                        printf("cmdline->SkinFileName = %s\n", cmdline->SkinFileName);
                        break;
                        case 'm':
                        printf("arg : m, optarg = %s\n", optarg);
			cmdline->MacroToPlay = optarg;
                        printf("cmdline->MacroName = %s\n", cmdline->MacroToPlay);
                        break;
			case 'l':
                        printf("arg : l\n");
			cmdline->isStartingSkinless = TRUE;
                        break;
			
                        default :
                        fprintf(stderr,"Erreur d'option\n"); 
                        help(argv[0],-1);
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
				DGLOBAL_L0_A0("**************** fct : main ****************************\n");
				DGLOBAL_L0_A2("*  cmdline->RomName=%s cmdline->SavName=%s           *\n",cmdline->RomName, cmdline->SavName);	
				DGLOBAL_L0_A0("********************************************************\n");
			} else {
				strcat(cmdline->SavName, ".sav");
		}
	}
}
