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
	const char *model = gsi->emu->calc->hw.name;
	char *name = NULL, *path;

	g_free(gsi->emu->cmdline->SkinFileName);

	tilem_config_get(model,
	                 "skin/f", &name,
	                 NULL);

	if (!name)
		name = g_strdup_printf("%s.skn", model);

	if (!g_path_is_absolute(name)) {
		path = get_shared_file_path("skins", name, NULL);
		gsi->emu->cmdline->SkinFileName = path;
		g_free(name);
	}
	else {
		gsi->emu->cmdline->SkinFileName = name;
	}
}

/* Convert to an absolute path */
static char *canonicalize_filename(const char *name)
{
#ifdef G_OS_WIN32
	static const char delim[] = "/\\";
#else
	static const char delim[] = G_DIR_SEPARATOR_S;
#endif
	char *result, **parts, *p;
	int i;

	if (name == NULL || g_path_is_absolute(name))
		return g_strdup(name);

	result = g_get_current_dir();
	parts = g_strsplit_set(name, delim, -1);
	for (i = 0; parts[i]; i++) {
		if (!strcmp(parts[i], "..")) {
			p = g_path_get_dirname(result);
			g_free(result);
			result = p;
		}
		else if (strcmp(parts[i], ".")
		         && strcmp(parts[i], "")) {
			p = g_build_filename(result, parts[i], NULL);
			g_free(result);
			result = p;
		}
	}
	g_strfreev(parts);
	return result;
}

/* GtkFileSelection */
void tilem_user_change_skin(GLOBAL_SKIN_INFOS *gsi)
{
	const char *model = gsi->emu->calc->hw.name;
	char *file_selected = NULL, *default_dir, *base, *shared, *canon;

	/* Show a nice chooser dialog, and get the filename selected */	
	default_dir = get_shared_dir_path("skins", NULL);
	file_selected = select_file(gsi, default_dir);
	g_free(default_dir);

	if (file_selected != NULL) {
		g_free(gsi->emu->cmdline->SkinFileName);
		gsi->emu->cmdline->SkinFileName = file_selected;
		gsi->emu->guiflags->isSkinDisabled = FALSE;
		redraw_screen(gsi);

		/* if file is stored in shared skins directory, save
		   only the relative path; otherwise, save the
		   absolute path */
		base = g_path_get_basename(file_selected);
		shared = get_shared_file_path("skins", base, NULL);
		canon = canonicalize_filename(shared);

		if (canon && !strcmp(canon, file_selected))
			tilem_config_set(model,
			                 "skin/f", base,
			                 NULL);
		else
			tilem_config_set(model,
			                 "skin/f", file_selected,
			                 NULL);

		g_free(base);
		g_free(shared);
		g_free(canon);
	}
}

