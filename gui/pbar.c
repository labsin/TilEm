/*
 * TilEm II
 *
 * Copyright (c) 2010-2011 Thibault Duponchelle
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <gtk/gtk.h>
#include <ticalcs.h>
#include <tilem.h>

#include "gui.h"

/* Update the progress_bar */
void progress_bar_update_activity(TilemCalcEmulator* emu)
{
	gdouble f;

	if (!emu->linkpb->ilp_progress_win)
		return;

	if (emu->link_update->max1 > 0) {
		f = (gdouble) emu->link_update->cnt1 / emu->link_update->max1;
		f = CLAMP(f, 0.0, 1.0);
		gtk_progress_bar_set_fraction(emu->linkpb->ilp_progress_bar1, f);
	}
	else {
		gtk_progress_bar_pulse(emu->linkpb->ilp_progress_bar1);
	}

	if (emu->link_update->max2 > 0) {
		f = (gdouble) emu->link_update->cnt2 / emu->link_update->max2;
		f = CLAMP(f, 0.0, 1.0);
		gtk_progress_bar_set_fraction(emu->linkpb->ilp_progress_bar2, f);
	}

	gtk_label_set_text(emu->linkpb->ilp_progress_label, emu->link_update->text);
}

/* Callback to destroy the progress bar */
static void destroy_progress(G_GNUC_UNUSED GtkDialog *dlg,
                             G_GNUC_UNUSED gint response,
                             gpointer data)
{
	TilemCalcEmulator* emu = data;
	tilem_calc_emulator_cancel_link(emu);
}

/* Create the progress bar window */
void progress_bar_init(TilemCalcEmulator* emu)
{
	GtkWidget *pw, *parent, *vbox, *tbl, *lbl, *pb;

	g_return_if_fail(emu != NULL);

	if (emu->ewin)
		parent = emu->ewin->window;
	else
		parent = NULL;

	pw = gtk_dialog_new_with_buttons("Sending files", GTK_WINDOW(parent),
	                                 GTK_DIALOG_DESTROY_WITH_PARENT,
	                                 GTK_STOCK_CANCEL,
	                                 GTK_RESPONSE_CANCEL,
	                                 NULL);
	emu->linkpb->ilp_progress_win = pw;

	vbox = gtk_dialog_get_content_area(GTK_DIALOG(pw));

	tbl = gtk_table_new(2, 2, FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(tbl), 6);
	gtk_table_set_row_spacings(GTK_TABLE(tbl), 6);
	gtk_table_set_col_spacings(GTK_TABLE(tbl), 6);

	lbl = gtk_label_new("Current:");
	gtk_misc_set_alignment(GTK_MISC(lbl), LABEL_X_ALIGN, 0.5);
	gtk_table_attach(GTK_TABLE(tbl), lbl, 0, 1, 0, 1,
	                 GTK_FILL, GTK_FILL, 0, 0);

	pb = gtk_progress_bar_new();
	emu->linkpb->ilp_progress_bar1 = GTK_PROGRESS_BAR(pb);
	gtk_table_attach(GTK_TABLE(tbl), pb, 1, 2, 0, 1,
	                 GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);

	lbl = gtk_label_new("Total:");
	gtk_misc_set_alignment(GTK_MISC(lbl), LABEL_X_ALIGN, 0.5);
	gtk_table_attach(GTK_TABLE(tbl), lbl, 0, 1, 1, 2,
	                 GTK_FILL, GTK_FILL, 0, 0);

	pb = gtk_progress_bar_new();
	emu->linkpb->ilp_progress_bar2 = GTK_PROGRESS_BAR(pb);
	gtk_table_attach(GTK_TABLE(tbl), pb, 1, 2, 1, 2,
	                 GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);

	gtk_box_pack_start(GTK_BOX(vbox), tbl, FALSE, FALSE, 0);

	lbl = gtk_label_new(emu->link_update->text);
	gtk_misc_set_alignment(GTK_MISC(lbl), 0.5, 0.5);
	emu->linkpb->ilp_progress_label = GTK_LABEL(lbl);

	gtk_box_pack_start(GTK_BOX(vbox), lbl, FALSE, FALSE, 6);

	g_signal_connect(pw, "response", G_CALLBACK(destroy_progress), emu);

	gtk_widget_show_all(pw);
}


