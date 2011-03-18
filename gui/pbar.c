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

	GtkWidget* cancel_button;
	GtkWidget* vbox = NULL;

	GtkWidget* pw = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	emu->ilp_progress_win = pw; 
	GtkWidget *pb = gtk_progress_bar_new();
	emu->ilp_progress_bar = pb; 
	gtk_widget_show (pb);
	
	gtk_window_set_title(GTK_WINDOW(pw), "Progress");
	gtk_window_set_default_size(GTK_WINDOW(pw), 400,30);

	cancel_button = gtk_button_new_with_label("Cancel");

	/* create vertical box to separate progress bar and button */	
	vbox = gtk_vbox_new(FALSE, 3);
	gtk_box_pack_start(GTK_BOX(vbox), pb, FALSE, FALSE, 0);
	//gtk_box_pack_end(GTK_BOX(vbox), cancel_button, FALSE, FALSE, 30);

	/* Center the button */	
	GtkWidget* table= gtk_table_new(1, 3, TRUE);
	gtk_table_attach_defaults(GTK_TABLE(table), cancel_button, 1, 2, 0, 1);
	gtk_box_pack_end(GTK_BOX(vbox), GTK_WIDGET(table), FALSE, FALSE, 3);

	gtk_container_add(GTK_CONTAINER(pw), vbox);

	//gtk_container_add(GTK_CONTAINER(pw), cancel_button);
	

	
	g_signal_connect(pw, "delete-event", G_CALLBACK(destroy_progress), emu);
	g_signal_connect_swapped(cancel_button, "clicked", G_CALLBACK(tilem_calc_emulator_cancel_link), emu);

		
	gtk_widget_show_all(pw);
}


