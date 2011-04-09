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

#ifndef CONFIG_FILE
#define CONFIG_FILE "config.ini"
#endif

/* TODO : remove some redundant code (write a specific function to write config file) */

/* Search and return the default skin for this model */
char* get_defaultskin(char* romname) {

	GKeyFile * gkf;
	gkf = g_key_file_new();
	
	if (! g_key_file_load_from_file(gkf, CONFIG_FILE, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, NULL)) {
		fprintf(stderr, "Could not read config file '%s'\n", CONFIG_FILE);
		exit(EXIT_FAILURE);
	}

	char* SkinFileName = g_key_file_get_string(gkf, "skin", romname, NULL);
	printf("skinfilename : %s\n", SkinFileName);
	
	g_key_file_free(gkf);
	return SkinFileName;
}

/* Set a default skin, or add it if not exists */
void set_defaultskin(char* romname, char* skinname) {
	
	GKeyFile * gkf;
	gkf = g_key_file_new();
	
	if (! g_key_file_load_from_file(gkf, CONFIG_FILE, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, NULL)) {
		fprintf(stderr, "Could not read config file '%s'\n", CONFIG_FILE);
		exit(EXIT_FAILURE);
	}
	
	g_key_file_set_string(gkf, "skin", romname, skinname);
	
	/* Save the config */
	FILE *file;
        char *data;

        if (!(file = fopen (CONFIG_FILE, "w")))
        {
            fprintf(stderr, "Could not open file: %s", CONFIG_FILE);
            return;
        }
        data = g_key_file_to_data (gkf, NULL, NULL);
        fputs (data, file);
        fclose (file);
	g_key_file_free(gkf);
        g_free (data); 
}

/* Search the most recent rom */
char* get_recentrom(char* romname) {

	GKeyFile * gkf;
	gkf = g_key_file_new();
	
	if (! g_key_file_load_from_file(gkf, CONFIG_FILE, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, NULL)) {
		fprintf(stderr, "Could not read config file '%s'\n", CONFIG_FILE);
		exit(EXIT_FAILURE);
	}

	char* recentrom = g_key_file_get_string(gkf, "skin", romname, NULL);

	
	g_key_file_free(gkf);
	return recentrom;
}




/* Set a default skin, or add it if not exists */
void set_recentrom(char* romname) {
	
	GKeyFile * gkf;
	gchar** keys;
	
	gkf = g_key_file_new();
	
	if (! g_key_file_load_from_file(gkf, CONFIG_FILE, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, NULL)) {
		fprintf(stderr, "Could not read config file '%s'\n", CONFIG_FILE);
		exit(EXIT_FAILURE);
	}
	int i = 0;	
		
	keys = g_key_file_get_keys(gkf, "recent", NULL, NULL);
	
	/* Shift from top to bottom */	
	for(i = 9; i > 0; i--) {	
		printf("%s\n", g_key_file_get_string(gkf, "recent", keys[(i-1)], NULL));
		g_key_file_set_string(gkf, "recent", keys[i], g_key_file_get_string(gkf, "recent", keys[(i-1)], NULL));
		
	}
	
	/* write the most recent rom */
	g_key_file_set_string(gkf, "recent", keys[0], romname);
	
	/* Save the config */
	FILE *file;
        char *data;

        if (!(file = fopen (CONFIG_FILE, "w")))
        {
            fprintf(stderr, "Could not open file: %s", CONFIG_FILE);
            return;
        }
        data = g_key_file_to_data (gkf, NULL, NULL);
        fputs (data, file);
        fclose (file);
	
	g_strfreev(keys);
	g_key_file_free(gkf);
        g_free (data); 
}

/* Get the saved model for a rom */
int get_modelcalcid(const char* romname) {
	GKeyFile * gkf;
	gkf = g_key_file_new();
	
	if (! g_key_file_load_from_file(gkf, CONFIG_FILE, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, NULL)) {
		fprintf(stderr, "Could not read config file '%s'\n", CONFIG_FILE);
		exit(EXIT_FAILURE);
	}

	char* pcalc_id = g_key_file_get_string(gkf, "model", romname, NULL);
	if(pcalc_id == NULL) {
		printf("Not found :\n");
		return 0;
	}
	
	g_key_file_free(gkf);
	//printf("calc_id : %s\n", pcalc_id);
	return pcalc_id[0];
}

/* Set model calc id */
void set_modelcalcid(char* romname, char id) {
	
	GKeyFile * gkf;
	gkf = g_key_file_new();
	
	if (! g_key_file_load_from_file(gkf, CONFIG_FILE, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, NULL)) {
		fprintf(stderr, "Could not read config file '%s'\n", CONFIG_FILE);
		exit(EXIT_FAILURE);
	}
	
	char idstr[2];
	idstr[0] = id;
	idstr[1] = '\0';
	g_key_file_set_string(gkf, "model", romname, idstr);
	
	/* Save the config */
	FILE *file;
        char *data;

        if (!(file = fopen (CONFIG_FILE, "w")))
        {
            fprintf(stderr, "Could not open file: %s", CONFIG_FILE);
            return;
        }
        data = g_key_file_to_data (gkf, NULL, NULL);
        fputs (data, file);
        fclose (file);
	g_key_file_free(gkf);
        g_free (data); 
}

/* search, write, and save config on right click menu */
void add_or_modify_defaultskin(GLOBAL_SKIN_INFOS* gsi) {

	set_defaultskin(gsi->emu->cmdline->RomName, gsi->emu->cmdline->SkinFileName);
}

/* search, write, and save config on right click menu */
void add_or_modify_defaultmodel(GLOBAL_SKIN_INFOS* gsi) {
	set_modelcalcid(gsi->emu->cmdline->RomName, gsi->emu->calc->hw.model_id);

}


/* Search and return the last directory opened to send a file*/
char* get_sendfile_recentdir() {

	GKeyFile * gkf;
	gkf = g_key_file_new();
	
	if (! g_key_file_load_from_file(gkf, CONFIG_FILE, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, NULL)) {
		fprintf(stderr, "Could not read config file '%s'\n", CONFIG_FILE);
		exit(EXIT_FAILURE);
	}

	char* recentdir = g_key_file_get_string(gkf, "upload", "sendfile_recentdir", NULL);
	printf("send file recent dir : %s\n", recentdir);
	
	g_key_file_free(gkf);
	return recentdir;
}

/* Set the last dir opened to send file */
void set_sendfile_recentdir(char* recentdir) {
	
	GKeyFile * gkf;
	gkf = g_key_file_new();
	
	if (! g_key_file_load_from_file(gkf, CONFIG_FILE, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, NULL)) {
		fprintf(stderr, "Could not read config file '%s'\n", CONFIG_FILE);
		exit(EXIT_FAILURE);
	}
	
	g_key_file_set_string(gkf, "upload", "sendfile_recentdir", recentdir);
	
	/* Save the config */
	FILE *file;
        char *data;

        if (!(file = fopen (CONFIG_FILE, "w")))
        {
            fprintf(stderr, "Could not open file: %s", CONFIG_FILE);
            return;
        }
        data = g_key_file_to_data (gkf, NULL, NULL);
        fputs (data, file);
        fclose (file);
	g_key_file_free(gkf);
        g_free (data); 
}

/* Search the value for key into group */
char* tilem_config_universal_getter(char* group, char* key) {

	GKeyFile * gkf;
	gkf = g_key_file_new();
	
	if (! g_key_file_load_from_file(gkf, CONFIG_FILE, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, NULL)) {
		fprintf(stderr, "Could not read config file '%s'\n", CONFIG_FILE);
		exit(EXIT_FAILURE);
	}

	char* string = g_key_file_get_string(gkf, group, key, NULL);
	//printf("%s from %s : %s\n", key, group, string);
	
	g_key_file_free(gkf);
	return string;
}

/* Test if the group exists in config_file */
gboolean tilem_test_group_exist_from_config_file(char* config_file, char* group) {

	GKeyFile * gkf;
	gkf = g_key_file_new();
	
	if (! g_key_file_load_from_file(gkf, config_file, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, NULL)) {
		fprintf(stderr, "Could not read config file '%s'\n", CONFIG_FILE);
		exit(EXIT_FAILURE);
	}

	gboolean ret = g_key_file_has_group(gkf, group);
	g_key_file_free(gkf);

	return ret;
	
}


/* Set the last dir opened to send file */
void set_loadmacro_recentdir(char* recentdir) {
	
	GKeyFile * gkf;
	gkf = g_key_file_new();
	
	if (! g_key_file_load_from_file(gkf, CONFIG_FILE, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, NULL)) {
		fprintf(stderr, "Could not read config file '%s'\n", CONFIG_FILE);
		exit(EXIT_FAILURE);
	}
	
	char* p = strrchr(recentdir, '/');
	strcpy(p, "\0");
	g_key_file_set_string(gkf, "macro", "loadmacro_recentdir", recentdir);
	
	/* Save the config */
	FILE *file;
        char *data;

        if (!(file = fopen (CONFIG_FILE, "w")))
        {
            fprintf(stderr, "Could not open file: %s", CONFIG_FILE);
            return;
        }
        data = g_key_file_to_data (gkf, NULL, NULL);
        fputs (data, file);
        fclose (file);
	g_key_file_free(gkf);
        g_free (data); 
}
