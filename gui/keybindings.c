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


#include "keybindings.h"


void tilem_keybindings_init(GLOBAL_SKIN_INFOS *gsi, const char* model) {

	//tilem_ksc_get_scancode("GDK_A", "ti83") ;
	gchar ** keys = tilem_ksc_get_all_keys(model);

	int i = 0;
	int n = 0;
	for(i = 0; keys[i]; i++) {
		n++;
		printf("keys %d : %s\n", i, keys[i]);
	}

	gsi->nkeybindings = n;
	gsi->keybindings = g_new(TilemKeyBinding, n);

	for(i = 0; keys[i]; i++) {
		gsi->keybindings[i].keysym = tilem_get_gdkkeysyms(gks, keys[i]);
		char* scancodestr = tilem_ksc_get_scancode(keys[i], model);
		printf("scancodestr : %s \n", scancodestr);
	
		if(scancodestr) {
			char* p = strrchr(scancodestr, ':');
			if(p) {
				gsi->keybindings[i].modifiers = 0;
				gsi->keybindings[i].nscancodes = 2;
				gsi->keybindings[i].scancodes = g_new(byte, 2);
				gsi->keybindings[i].scancodes[1] = tilem_get_tilemkeysyms(tks, ++p);
				--p;
				p[0] = '\0';
				gsi->keybindings[i].scancodes[0] = tilem_get_tilemkeysyms(tks, scancodestr);
				printf("keysym= %d\tscancode[0]= %d\tscancode[1]= %d\n", gsi->keybindings[i].keysym, gsi->keybindings[i].scancodes[0],gsi->keybindings[i].scancodes[1]);
			} else {	
				gsi->keybindings[i].modifiers = 0;
				gsi->keybindings[i].nscancodes = 1;
				gsi->keybindings[i].scancodes = g_new(byte, 1);
				gsi->keybindings[i].scancodes[0] = tilem_get_tilemkeysyms(tks, tilem_ksc_get_scancode(keys[i], model));
				printf("keysym= %d\tscancode[0]= %d\n", gsi->keybindings[i].keysym, gsi->keybindings[i].scancodes[0]);
			}
			p = NULL;
		}
		free(scancodestr);
	}
}

/* Translating a string name (by example "GDK_A") to a unsigned int value (code for GDK_A) */
unsigned int tilem_get_gdkkeysyms(GdkKeySyms *gks, char* string) {
	int i= 0;

	for(i= 0; gks[i].keysymstr; i++) {
		if(strcmp(string, gks[i].keysymstr) == 0) 
			return gks[i].keysym;
	}
	
	return 0;

}

/* Translating a string name (by example "TILEM_KEY_A") to a unsigned int value (code for TILEM_KEY_A) */
unsigned int tilem_get_tilemkeysyms(TilemKeySyms *tks, char* string) {
	int i= 0;
	
	for(i= 0; tks[i].scancodestr; i++) {
		if(strcmp(string, tks[i].scancodestr) == 0) {
			//printf("%s : code %d\n", string, tks[i].scancode);
			return tks[i].scancode;
		}
	}
	return 0;
}

/* Search the scancode into the keysyms config file */
char* tilem_ksc_get_scancode(char* gdkkeysymstr, const char* model) {

	GKeyFile * gkf;
	gkf = g_key_file_new();
	
	if (! g_key_file_load_from_file(gkf, KEYBINDINGS_FILE, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, NULL)) {
		fprintf(stderr, "Could not read config file '%s'\n", KEYBINDINGS_FILE);
		exit(EXIT_FAILURE);
	}

	char* scancodestr = g_key_file_get_string(gkf, model, gdkkeysymstr, NULL);
	//printf("scancodestr : %s\n", scancodestr);
	
	g_key_file_free(gkf);
	return scancodestr;
}

/* Get all the keys from the group model */
static gchar** tilem_ksc_get_all_keys(const char* model) {

	GKeyFile * gkf;
	gchar** keys;
	
	gkf = g_key_file_new();
	
	if (! g_key_file_load_from_file(gkf, KEYBINDINGS_FILE, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, NULL)) {
		fprintf(stderr, "Could not read config file '%s'\n", KEYBINDINGS_FILE);
		exit(EXIT_FAILURE);
	}

	keys = g_key_file_get_keys(gkf, model, NULL, NULL);
	
//	for(i = 0; keys[i]; i++) 
//		printf("keys %d : %s\n", i, keys[i]);
		
	g_key_file_free(gkf);
	
	return keys;
}


