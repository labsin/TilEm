/*
 * TilEm II
 *
 * Copyright (c) 2010-2011 Thibault Duponchelle 
 * Copyright (c) 2010 Benjamin Moody
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
#include <glib/gstdio.h>
#include <ticalcs.h>
#include <tilem.h>

#include "gui.h"

/* Create a frame around the given widget, with a boldface label in
   the GNOME style */
GtkWidget* new_frame(const gchar* label, GtkWidget* contents)
{
	GtkWidget *frame, *align;
	char *str;

	str = g_strconcat("<b>", label, "</b>", NULL);
	frame = gtk_frame_new(str);
	g_free(str);

	g_object_set(gtk_frame_get_label_widget(GTK_FRAME(frame)),
	             "use-markup", TRUE, NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_NONE);

	align = gtk_alignment_new(0.5, 0.5, 1.0, 1.0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(align), 6, 0, 12, 0);
	gtk_widget_show(align);
	gtk_container_add(GTK_CONTAINER(frame), align);
	gtk_container_add(GTK_CONTAINER(align), contents);
	gtk_widget_show(frame);

	return frame;
}

/* A popup which is used to let the user choose the model at startup */
char choose_rom_popup(GtkWidget *parent_window, const char *filename,
                      char default_model)
{
	const TilemHardware **models;
	GtkWidget *dlg, *vbox, *frame, *btn;
	GtkToggleButton **btns;
	char *ids, id = 0;
	int nmodels, noptions, i, j, defoption = 0, response;
	dword romsize;
	char *fn, *msg;

	tilem_get_supported_hardware(&models, &nmodels);

	/* determine ROM size for default model */
	for (i = 0; i < nmodels; i++)
		if (models[i]->model_id == default_model)
			break;

	g_return_val_if_fail(i < nmodels, 0);

	romsize = models[i]->romsize;

	/* all other models with same ROM size are candidates */
	noptions = 0;
	for (i = 0; i < nmodels; i++) {
		if (models[i]->model_id == default_model)
			defoption = noptions;
		if (models[i]->romsize == romsize)
			noptions++;
	}

	if (noptions < 2) /* no choice */
		return default_model;

	dlg = gtk_dialog_new_with_buttons("Select Calculator Type",
	                                  GTK_WINDOW(parent_window),
	                                  GTK_DIALOG_MODAL,
	                                  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
	                                  GTK_STOCK_OK, GTK_RESPONSE_OK,
	                                  NULL);
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(dlg),
	                                        GTK_RESPONSE_OK,
	                                        GTK_RESPONSE_CANCEL,
	                                        -1);
	gtk_dialog_set_default_response(GTK_DIALOG(dlg),
	                                GTK_RESPONSE_OK);

	vbox = gtk_vbox_new(TRUE, 0);

	/* create radio buttons */

	btns = g_new(GtkToggleButton*, noptions);
	ids = g_new(char, noptions);
	btn = NULL;
	for (i = j = 0; i < nmodels; i++) {
		if (models[i]->romsize == romsize) {
			btn = gtk_radio_button_new_with_label_from_widget
				(GTK_RADIO_BUTTON(btn), models[i]->desc);
			btns[j] = GTK_TOGGLE_BUTTON(btn);
			ids[j] = models[i]->model_id;
			gtk_box_pack_start(GTK_BOX(vbox), btn, TRUE, TRUE, 3);
			j++;
		}
	}

	gtk_toggle_button_set_active(btns[defoption], TRUE);

	fn = g_filename_display_basename(filename);
	msg = g_strdup_printf("Calculator type for %s:", fn);
	frame = new_frame(msg, vbox);
	g_free(fn);
	g_free(msg);

	gtk_container_set_border_width(GTK_CONTAINER(frame), 6);
	gtk_widget_show_all(frame);

	vbox = gtk_dialog_get_content_area(GTK_DIALOG(dlg));
	gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

	response = gtk_dialog_run(GTK_DIALOG(dlg));

	if (response == GTK_RESPONSE_OK) {
		for (i = 0; i < noptions; i++) {
			if (gtk_toggle_button_get_active(btns[i])) {
				id = ids[i];
				break;
			}
		}
	}
	else {
		id = 0;
	}

	gtk_widget_destroy(dlg);
	g_free(btns);
	g_free(ids);

	return id;
}
