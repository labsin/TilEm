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

#include <tilem.h>
#include <malloc.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <gui.h>


/* Create a window with progress bar */
void progress_bar_init(TilemCalcEmulator* emu) {
	create_progress_window(emu);
}

/* Update the progress bar (FIXME : gtk_progress_bar_update is deprecated) */
void progress_bar_update(TilemCalcEmulator* emu, gfloat percentage) {
	gtk_progress_bar_update(GTK_PROGRESS_BAR(emu->ilp_progress_bar), percentage );

}

/* Destroy the widget */
void destroy_progress_win(GtkWidget* progress_win)   {
	gtk_widget_destroy(GTK_WIDGET(progress_win));
}

/* Create the progress bar window */
void create_progress_window(TilemCalcEmulator* emu) {

	GtkWidget* pw = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	emu->progress_win = pw; 
	emu->ilp_progress_bar = gtk_progress_bar_new();
	gtk_window_set_title(GTK_WINDOW(emu->progress_win), "Progress");
	gtk_window_set_default_size(GTK_WINDOW(emu->progress_win), 400,30);
	
	gtk_container_add(GTK_CONTAINER(emu->progress_win), emu->ilp_progress_bar);

	g_signal_connect(emu->progress_win, "destroy", G_CALLBACK(destroy_progress_win), NULL);



		
	gtk_widget_show_all(emu->progress_win);
}


