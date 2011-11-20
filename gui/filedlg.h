/*
 * TilEm II
 *
 * Copyright (c) 2011 Benjamin Moody
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

/* Run a file chooser dialog, allowing user to select a single
   existing file to open.

   TITLE is the title of the dialog (UTF-8.)

   PARENT is the "parent" (transient-for) window, if any.

   SUGGEST_DIR is the directory to start in (GLib filename encoding.)

   Remaining arguments are a series of pairs of strings describing the
   permitted file types.  First string in each pair is the
   description; second is a pattern set (consisting of one or more
   glob-style patterns, separated by semicolons.)  Patterns must be
   lowercase; they will be checked case-insensitively.  The list is
   terminated by NULL.

   A pattern may be the empty string (""); if so, that file type is
   disabled.

   Result is NULL if dialog was cancelled; otherwise, a string in
   filename encoding, which must be freed with g_free().
 */
char * prompt_open_file(const char *title,        /* UTF-8 */
                        GtkWindow *parent,
                        const char *suggest_dir,  /* filename encoding */
                        const char *desc1,        /* UTF-8 */
                        const char *pattern1,     /* ASCII */
                        ...)
	G_GNUC_NULL_TERMINATED;

/* Run a file chooser dialog, allowing user to select one or more
   files to open.  Result is either NULL or an array of strings, which
   must be freed with g_strfreev().  */
char ** prompt_open_files(const char *title,        /* UTF-8 */
                          GtkWindow *parent,
                          const char *suggest_dir,  /* filename encoding */
                          const char *desc1,        /* UTF-8 */
                          const char *pattern1,     /* ASCII */
                          ...)
	G_GNUC_NULL_TERMINATED;

/* Run a file chooser dialog, allowing user to enter a new filename to
   be created.  SUGGEST_NAME is a suggested name for the new file;
   note that this is UTF-8. */
char * prompt_save_file(const char *title,         /* UTF-8 */
                        GtkWindow *parent,
                        const char *suggest_name,  /* UTF-8 (!) */
                        const char *suggest_dir,   /* filename encoding */
                        const char *desc1,        /* UTF-8 */
                        const char *pattern1,     /* ASCII */
                        ...)
	G_GNUC_NULL_TERMINATED;

/* Create a file entry or file-chooser button widget, allowing user to
   select a single existing file to open. */
GtkWidget * file_entry_new(const char *title, /* UTF-8 */
                           const char *desc1, /* UTF-8 */
                           const char *pattern1, /* ASCII */
                           ...)
	G_GNUC_NULL_TERMINATED;

/* Set filename in a file entry. */
void file_entry_set_filename(GtkWidget *fe,
                             const char *filename); /* filename encoding */

/* Get filename in a file entry.  Result is NULL if no file is
   selected; otherwise, a string in filename encoding, which must be
   freed with g_free(). */
char * file_entry_get_filename(GtkWidget *fe);

/* Run a directory chooser dialog, allowing user to select a directory. */
char * prompt_select_dir(const char *title, GtkWindow *parent, const char *suggest_dir);
