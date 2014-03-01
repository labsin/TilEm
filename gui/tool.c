/*
 * TilEm II
 *
 * Copyright (c) 2010-2011 Thibault Duponchelle 
 * Copyright (c) 2010-2012 Benjamin Moody
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
#include <errno.h>
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

/* Get model name (abbreviation) for a TilEm model ID. */
const char * model_to_name(int model)
{
	const TilemHardware **models;
	int nmodels, i;

	tilem_get_supported_hardware(&models, &nmodels);
	for (i = 0; i < nmodels; i++)
		if (models[i]->model_id == model)
			return models[i]->name;

	return NULL;
}

/* Convert model name to a model ID. */
int name_to_model(const char *name)
{
	char *s;
	const TilemHardware **models;
	int nmodels, i, j;

	s = g_new(char, strlen(name) + 1);
	for (i = j = 0; name[i]; i++) {
		if (name[i] == '+')
			s[j++] = 'p';
		else if (name[i] != '-')
			s[j++] = g_ascii_tolower(name[i]);
	}
	s[j] = 0;

	tilem_get_supported_hardware(&models, &nmodels);
	for (i = 0; i < nmodels; i++) {
		if (!strcmp(s, models[i]->name)) {
			g_free(s);
			return models[i]->model_id;
		}
	}

	g_free(s);
	return 0;
}

/* Convert TilEm model ID to tifiles2 model ID. */
CalcModel model_to_calcmodel(int model)
{
	switch (model) {
	case TILEM_CALC_TI73:
		return CALC_TI73;

	case TILEM_CALC_TI82:
		return CALC_TI82;

	case TILEM_CALC_TI83:
	case TILEM_CALC_TI76:
		return CALC_TI83;

	case TILEM_CALC_TI83P:
	case TILEM_CALC_TI83P_SE:
		return CALC_TI83P;

	case TILEM_CALC_TI84P:
	case TILEM_CALC_TI84P_SE:
	case TILEM_CALC_TI84P_NSPIRE:
	case TILEM_CALC_TI84PC_SE:
		return CALC_TI84P;

	case TILEM_CALC_TI85:
		return CALC_TI85;

	case TILEM_CALC_TI86:
		return CALC_TI86;

	default:
		return CALC_NONE;
	}
}

/* Convert tifiles2 model ID to TilEm model ID. */
int calcmodel_to_model(CalcModel model)
{
	switch (model) {
	case CALC_TI73:
		return TILEM_CALC_TI73;
	case CALC_TI82:
		return TILEM_CALC_TI82;
	case CALC_TI83:
		return TILEM_CALC_TI83;
	case CALC_TI83P:
		return TILEM_CALC_TI83P;
	case CALC_TI84P:
		return TILEM_CALC_TI84P;
	case CALC_TI85:
		return TILEM_CALC_TI85;
	case CALC_TI86:
		return TILEM_CALC_TI86;
	default:
		return 0;
	}
}

/* Get model ID for a given file. */
int file_to_model(const char *name)
{
	const char *p;
	TigContent *tig;
	int model;

	p = strrchr(name, '.');
	if (!p || strlen(p) < 4 || strchr(p, '/') || strchr(p, '\\'))
		return 0;
	p++;

	if (!g_ascii_strcasecmp(p, "prg"))
		return TILEM_CALC_TI81;

	if (!g_ascii_strncasecmp(p, "73", 2))
		return TILEM_CALC_TI73;
	if (!g_ascii_strncasecmp(p, "82", 2))
		return TILEM_CALC_TI82;
	if (!g_ascii_strncasecmp(p, "83", 2))
		return TILEM_CALC_TI83;
	if (!g_ascii_strncasecmp(p, "8x", 2))
		return TILEM_CALC_TI83P;
	if (!g_ascii_strncasecmp(p, "85", 2))
		return TILEM_CALC_TI85;
	if (!g_ascii_strncasecmp(p, "86", 2))
		return TILEM_CALC_TI86;

	if (!g_ascii_strcasecmp(p, "tig")
	    || !g_ascii_strcasecmp(p, "zip")) {
		/* read file and see what tifiles thinks the type is */
		tig = tifiles_content_create_tigroup(CALC_NONE, 0);
		tifiles_file_read_tigroup(name, tig);
		model = calcmodel_to_model(tig->model);
		tifiles_content_delete_tigroup(tig);
		return model;
	}

	return 0;
}

/* Get "base" model for file type support. */
int model_to_base_model(int calc_model)
{
	switch (calc_model) {
	case TILEM_CALC_TI83:
	case TILEM_CALC_TI76:
		return TILEM_CALC_TI83;

	case TILEM_CALC_TI83P:
	case TILEM_CALC_TI83P_SE:
	case TILEM_CALC_TI84P:
	case TILEM_CALC_TI84P_SE:
	case TILEM_CALC_TI84P_NSPIRE:
		return TILEM_CALC_TI83P;

	default:
		return calc_model;
	}
}

/* Check if calc is compatible with given file type. */
gboolean model_supports_file(int calc_model, int file_model)
{
	calc_model = model_to_base_model(calc_model);
	file_model = model_to_base_model(file_model);

	if (file_model == calc_model)
		return TRUE;

	if (file_model == TILEM_CALC_TI82
	    && (calc_model == TILEM_CALC_TI83
	        || calc_model == TILEM_CALC_TI83P))
		return TRUE;

	if (file_model == TILEM_CALC_TI83
	    && (calc_model == TILEM_CALC_TI83P))
		return TRUE;

	if (file_model == TILEM_CALC_TI85
	    && (calc_model == TILEM_CALC_TI86))
		return TRUE;

	return FALSE;
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

	dlg = gtk_dialog_new_with_buttons(_("Select Calculator Type"),
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
	msg = g_strdup_printf(_("Calculator type for %s:"), fn);
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

/* Convert UTF-8 to filename encoding.  Use ASCII digits in place of
   subscripts if necessary.  If conversion fails utterly, fall back to
   the UTF-8 name, which is broken but better than nothing. */
char * utf8_to_filename(const char *utf8str)
{
	gchar *result, *ibuf, *obuf, *p;
	gsize icount, ocount;
	const gchar **charsets;
	GIConv ic;
	gunichar c;

	if (g_get_filename_charsets(&charsets))
		return g_strdup(utf8str);

	ic = g_iconv_open(charsets[0], "UTF-8");
	if (!ic) {
		g_warning(_("utf8_to_filename: unsupported charset %s"),
		          charsets[0]);
		return g_strdup(utf8str);
	}

	ibuf = (gchar*) utf8str;
	icount = strlen(utf8str);
	ocount = icount * 2; /* be generous */
	result = obuf = g_new(gchar, ocount + 1);

	while (g_iconv(ic, &ibuf, &icount, &obuf, &ocount) == (gsize) -1) {
		if (errno != EILSEQ) {
			g_warning(_("utf8_to_filename: error in conversion"));
			g_free(result);
			g_iconv_close(ic);
			return g_strdup(utf8str);
		}

		c = g_utf8_get_char(ibuf);
		if (c >= 0x2080 && c <= 0x2089)
			*obuf = c - 0x2080 + '0';
		else
			*obuf = '_';
		obuf++;
		ocount--;

		p = g_utf8_next_char(ibuf);
		icount -= p - ibuf;
		ibuf = p;
	}

	*obuf = 0;
	g_iconv_close(ic);
	return result;
}

/* Convert UTF-8 to a subset of UTF-8 that is compatible with the
   locale */
char * utf8_to_restricted_utf8(const char *utf8str)
{
	char *p, *q;
	p = utf8_to_filename(utf8str);
	q = g_filename_to_utf8(p, -1, NULL, NULL, NULL);
	g_free(p);
	if (q)
		return q;
	else
		return g_strdup(utf8str);
}

/* Generate default filename (UTF-8) for a variable */
char * get_default_filename(const TilemVarEntry *tve)
{
	GString *str = g_string_new("");

	if (tve->slot_str) {
		g_string_append(str, tve->slot_str);
		if (tve->name_str && tve->name_str[0]) {
			g_string_append_c(str, '-');
			g_string_append(str, tve->name_str);
		}
	}
	else if (tve->name_str && tve->name_str[0]) {
		g_string_append(str, tve->name_str);
	}
	else {
		g_string_append(str, _("untitled"));
	}
	g_string_append_c(str, '.');
	g_string_append(str, tve->file_ext);
	return g_string_free(str, FALSE);
}
