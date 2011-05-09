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
#include <string.h>
#include <gtk/gtk.h>
#include <ticalcs.h>
#include <tilem.h>

#include "gui.h"
#include "msgbox.h"
#include "files.h"

/* Get the associated calculator key name */
static int calc_key_from_name(const TilemCalc *calc, const char *name)
{
	int i;

	for (i = 0; i < 64; i++)
		if (calc->hw.keynames[i]
		    && !strcmp(calc->hw.keynames[i], name))
			return i + 1;

	return 0;
}

/* Parse a line of the group (model) in the keybindings file */
static gboolean parse_binding(TilemKeyBinding *kb,
                              const char *pckeys, const char *tikeys,
                              const TilemCalc *calc)
{
	const char *p;
	char *s;
	int n, k;

	kb->modifiers = 0;
	kb->keysym = 0;
	kb->nscancodes = 0;
	kb->scancodes = NULL;

	/* Parse modifiers */

	while ((p = strchr(pckeys, '+'))) {
		s = g_strndup(pckeys, p - pckeys);
		g_strstrip(s);
		if (!g_ascii_strcasecmp(s, "ctrl")
		    || !g_ascii_strcasecmp(s, "control"))
			kb->modifiers |= GDK_CONTROL_MASK;
		else if (!g_ascii_strcasecmp(s, "shift"))
			kb->modifiers |= GDK_SHIFT_MASK;
		else if (!g_ascii_strcasecmp(s, "alt")
		         || !g_ascii_strcasecmp(s, "mod1"))
			kb->modifiers |= GDK_MOD1_MASK;
		else if (!g_ascii_strcasecmp(s, "mod2"))
			kb->modifiers |= GDK_MOD2_MASK;
		else if (!g_ascii_strcasecmp(s, "mod3"))
			kb->modifiers |= GDK_MOD3_MASK;
		else if (!g_ascii_strcasecmp(s, "mod4"))
			kb->modifiers |= GDK_MOD4_MASK;
		else if (!g_ascii_strcasecmp(s, "mod5"))
			kb->modifiers |= GDK_MOD5_MASK;
		else {
			g_free(s);
			return FALSE;
		}
		g_free(s);
		pckeys = p + 1;
	}

	/* Parse keysym */

	s = g_strstrip(g_strdup(pckeys));
	kb->keysym = gdk_keyval_from_name(s);
	g_free(s);
	if (!kb->keysym)
		return FALSE;

	/* Parse calculator keys */

	/* FIXME: allow combinations of simultaneous keys (separated
	   by '+'); current TilemKeyBinding struct doesn't provide for
	   this */

	n = 0;
	do {
		if ((p = strchr(tikeys, ',')))
			s = g_strndup(tikeys, p - tikeys);
		else
			s = g_strdup(tikeys);
		g_strstrip(s);

		k = calc_key_from_name(calc, s);
		g_free(s);

		if (!k) {
			g_free(kb->scancodes);
			kb->scancodes = NULL;
			return FALSE;
		}

		kb->nscancodes++;
		if (kb->nscancodes >= n) {
			n = kb->nscancodes * 2;
			kb->scancodes = g_renew(byte, kb->scancodes, n);
		}
		kb->scancodes[kb->nscancodes - 1] = k;

		tikeys = (p ? p + 1 : NULL);
	} while (tikeys);

	return TRUE;
}

/* Parse a group (model) in the keybindings file */
static void parse_binding_group(TilemCalcEmulator *emu, GKeyFile *gkf,
                                const char *group)
{
	gchar **keys;
	char *k, *v;
	int i, n;

	keys = g_key_file_get_keys(gkf, group, NULL, NULL);
	if (!keys) {
		printf("no bindings for %s\n", group);
		return;
	}

	for (i = 0; keys[i]; i++)
		;

	n = emu->nkeybindings;
	emu->keybindings = g_renew(TilemKeyBinding, emu->keybindings, n + i);

	for(i = 0; keys[i]; i++) {
		k = keys[i];
		v = g_key_file_get_value(gkf, group, k, NULL);
		if (!v)
			continue;

		if (parse_binding(&emu->keybindings[n], k, v, emu->calc))
			n++;
		else
			g_printerr("syntax error in key bindings: '%s=%s'\n",
			           k, v);
		g_free(v);
	}

	emu->nkeybindings = n;

	g_strfreev(keys);
}

/* Init the keybindings struct and open the keybindings file */
void tilem_keybindings_init(TilemCalcEmulator *emu, const char *model)
{
	char *kfname = get_shared_file_path("keybindings.ini", NULL);
	char *dname;
	GKeyFile *gkf;
	GError *err = NULL;

	g_return_if_fail(emu != NULL);
	g_return_if_fail(emu != NULL);
	g_return_if_fail(emu->calc != NULL);

	if (kfname == NULL) {
		messagebox00(NULL, GTK_MESSAGE_ERROR,
		             "Unable to load key bindings",
		             "The file keybindings.ini could not be found."
		             "  TilEm may not have been installed correctly.");
		return;
	}

	gkf = g_key_file_new();
	if (!g_key_file_load_from_file(gkf, kfname, 0, &err)) {
		dname = g_filename_display_name(kfname);
		messagebox02(NULL, GTK_MESSAGE_ERROR,
		             "Unable to load key bindings",
		             "An error occurred while reading %s: %s",
		             dname, err->message);
		g_error_free(err);
		g_free(dname);
		g_free(kfname);
		return;
	}

	g_free(emu->keybindings);
	emu->keybindings = NULL;
	emu->nkeybindings = 0;


	/* To avoid giving the same keybindings for each models, 
	   keybindings are grouped into categories...
	   "all" : all the models
	   "all-alpha" : all models with alpha key
	   "all-except-ti73" : all models ... except ti73
	*/
	parse_binding_group(emu, gkf, "all");
	
	if((strcmp(model, "ti73") !=0)) {
		printf("all-with-alpha\n");
		parse_binding_group(emu, gkf, "all-with-alpha");
	}

	if(strcmp(model, "ti73") != 0){
		printf("all-except-ti73\n");
		parse_binding_group(emu, gkf, "all-except-ti73");
	}
	
	if((strcmp(model, "ti82") == 0) || (strcmp(model, "ti83") == 0) || (strcmp(model, "ti83+") == 0) ) {
		printf("ti82-ti83(+)-ti84+\n");
		parse_binding_group(emu, gkf, "ti82-ti83(+)-ti84+");
	} else if((strcmp(model, "ti85") == 0) || (strcmp(model, "ti86") == 0)) { 
		printf("ti85-ti86\n");
		parse_binding_group(emu, gkf, "ti85-ti86");
	} else {	 
		printf("%s\n", model);
		parse_binding_group(emu, gkf, model);
	}

	g_key_file_free(gkf);
	g_free(kfname);
}
