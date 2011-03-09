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


#include <stdio.h>
#include <gtk/gtk.h>
#include <string.h>
#include <malloc.h>
#include <gui.h>
#include <glib/gstdio.h>



/* Callback event */
void on_screenshot();
void on_record(GtkWidget* win, GLOBAL_SKIN_INFOS* gsi);
void on_add_frame(GtkWidget* win, GLOBAL_SKIN_INFOS* gsi);
void on_stop(GtkWidget* win, GLOBAL_SKIN_INFOS* gsi);
void on_play(GLOBAL_SKIN_INFOS* gsi);
void on_destroy_playview(GtkWidget* playwin);

void screenshot(GLOBAL_SKIN_INFOS *gsi);

static gboolean save_screenshot(GLOBAL_SKIN_INFOS *gsi, const char *filename, const char *format);
