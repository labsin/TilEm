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


/* Update the progress_bar (activity mode) */
gboolean progress_bar_update_activity(TilemCalcEmulator* emu) {
	
	if(!emu->ilp_active)
		destroy_progress(emu->ilp_progress_win, (gpointer) emu);
		
	if(!GTK_IS_WIDGET(emu->ilp_progress_win))
		return FALSE;
	if(GTK_IS_PROGRESS_BAR(emu->ilp_progress_bar))
		gtk_progress_bar_pulse(GTK_PROGRESS_BAR(emu->ilp_progress_bar));
	return TRUE;

}

void destroy_progress(GtkWidget *widget, gpointer* data) {
	TilemCalcEmulator* emu = (TilemCalcEmulator*) data;
	if(GTK_IS_WIDGET(widget)) {
		gtk_widget_destroy(GTK_WIDGET(widget));
		emu->ilp_progress_bar = NULL;
		widget = NULL;
	}

}


/* Create the progress bar window */
void create_progress_window(TilemCalcEmulator* emu) {

	GtkWidget* pw = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	emu->ilp_progress_win = pw; 
	GtkWidget *pb = gtk_progress_bar_new();
	emu->ilp_progress_bar = pb; 
	gtk_widget_show (pb);
	
	gtk_window_set_title(GTK_WINDOW(pw), "Progress");
	gtk_window_set_default_size(GTK_WINDOW(pw), 400,30);
	
	gtk_container_add(GTK_CONTAINER(pw), emu->ilp_progress_bar);

	g_signal_connect(pw, "delete-event", G_CALLBACK(destroy_progress), emu);

		
	gtk_widget_show_all(pw);
}


