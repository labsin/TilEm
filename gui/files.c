/*
 * TilEm II
 *
 * Copyright (c) 2011-2012 Benjamin Moody
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
#include <stdarg.h>
#include <glib.h>
#include <glib/gstdio.h>

#include "files.h"

static char *program_dir;

/* Set the name used to invoke this program */
void set_program_path(const char *path)
{
	if (path && strchr(path, G_DIR_SEPARATOR))
		program_dir = g_path_get_dirname(path);
}

/* Build a filename out of varargs */
static char *build_filenamev(const char *start, va_list rest)
{
	char *args[10];
	int i;

	args[0] = (char*) start;
	for (i = 1; i < 10; i++) {
		args[i] = (char*) va_arg(rest, const char *);
		if (!args[i])
			break;
	}
	g_assert(i < 10);

	return g_build_filenamev(args);
}

/* Get the default configuration directory.

   On Unix, this is $XDG_CONFIG_HOME/tilem2 (where $XDG_CONFIG_HOME
   defaults to $HOME/.config/ if not set.)

   On Windows, this is $CSIDL_APPDATA/tilem2 (where $CSIDL_APPDATA is
   typically the "Application Data" directory within the user
   profile.)

   Result is cached and should not be freed. */
static char * get_default_config_dir()
{
	static char *result;

	if (!result)
		result = g_build_filename(g_get_user_config_dir(),
		                          "tilem2", NULL);

	return result;
}

/* Search for an existing file.

   The default package configuration directory (defined above) is
   searched first; if the file is not found there, try to find the
   file that was installed along with the package, or (in case the
   package hasn't yet been installed) the copy included in the source
   package. */
static char * find_filev(GFileTest test, const char *name, va_list rest)
{
	char *fullname, *dname, *path;
	const char *userdir;
	const char * const *sysdirs;

	fullname = build_filenamev(name, rest);

	dname = get_default_config_dir();
	path = g_build_filename(dname, fullname, NULL);
	if (g_file_test(path, test)) {
		g_free(fullname);
		return path;
	}
	g_free(path);

#ifdef G_OS_WIN32
	if ((dname = g_win32_get_package_installation_directory(NULL, NULL))) {
		path = g_build_filename(dname, "share", "tilem2", fullname, NULL);
		g_free(dname);
		if (g_file_test(path, test)) {
			g_free(fullname);
			return path;
		}
		g_free(path);
	}
#endif

#ifdef UNINSTALLED_SHARE_DIR
	if (program_dir) {
		path = g_build_filename(program_dir, UNINSTALLED_SHARE_DIR,
		                        fullname, NULL);
		if (g_file_test(path, test)) {
			g_free(fullname);
			return path;
		}
		g_free(path);
	}
#endif

#ifdef SHARE_DIR
	path = g_build_filename(SHARE_DIR, fullname, NULL);
	if (g_file_test(path, test)) {
		g_free(fullname);
		return path;
	}
	g_free(path);
#endif

	userdir = g_get_user_data_dir();
	if (userdir) {
		path = g_build_filename(userdir, "tilem2", fullname, NULL);
		if (g_file_test(path, test)) {
			g_free(fullname);
			return path;
		}
	}

	sysdirs = g_get_system_data_dirs();
	while (sysdirs && sysdirs[0]) {
		path = g_build_filename(sysdirs[0], "tilem2", fullname, NULL);
		if (g_file_test(path, test)) {
			g_free(fullname);
			return path;
		}
		sysdirs++;
	}

	g_free(fullname);
	return NULL;
}

/* Locate an existing configuration or data file */
char * get_shared_file_path(const char *name, ...)
{
	va_list ap;
	char *path;
	va_start(ap, name);
	path = find_filev(G_FILE_TEST_IS_REGULAR, name, ap);
	va_end(ap);
	return path;
}

/* Locate an existing configuration or data directory */
char * get_shared_dir_path(const char *name, ...)
{
	va_list ap;
	char *path;
	va_start(ap, name);
	path = find_filev(G_FILE_TEST_IS_DIR, name, ap);
	va_end(ap);
	return path;
}

/* Return the path to the user's configuration directory, where any
   new or modified config files should be written.  Result is cached
   and should not be freed. */
static char * get_config_dir()
{
	static char *result;
	char *fname;
	FILE *f;

	if (result)
		return result;

	/* If config.ini already exists, in any of the standard
	   locations, and is writable, use the directory containing
	   it.  This will allow building the package as a relocatable
	   bundle. */
	fname = get_shared_file_path("config.ini", NULL);
	if (fname) {
		f = g_fopen(fname, "r+");
		if (f) {
			result = g_path_get_dirname(fname);
			fclose(f);
		}
		g_free(fname);
	}

	/* Otherwise use default config directory */
	if (!result)
		result = g_strdup(get_default_config_dir());

	return result;
}

/* Get path for writing a new or modified configuration file */
char * get_config_file_path(const char *name, ...)
{
	va_list ap;
	const char *cfgdir;
	char *fullname, *path;

	cfgdir = get_config_dir();
	g_mkdir_with_parents(cfgdir, 0775);

	va_start(ap, name);
	fullname = build_filenamev(name, ap);
	va_end(ap);
	path = g_build_filename(cfgdir, fullname, NULL);
	g_free(fullname);
	return path;
}

