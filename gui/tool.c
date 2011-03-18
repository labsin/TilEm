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

#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <gui.h>

static GtkWidget* new_gnome_style_frame(const gchar* label, GtkWidget* contents)
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
	gtk_alignment_set_padding(GTK_ALIGNMENT(align), 0, 0, 12, 0);
	gtk_widget_show(align);
	gtk_container_add(GTK_CONTAINER(frame), align);
	gtk_container_add(GTK_CONTAINER(align), contents);
	gtk_widget_show(frame);

	return frame;
}

void popup_error(char* msg, GLOBAL_SKIN_INFOS * gsi)
{
	GtkWidget *pPopup;

	pPopup = gtk_message_dialog_new (GTK_WINDOW(gsi->pWindow),GTK_DIALOG_MODAL,GTK_MESSAGE_ERROR,GTK_BUTTONS_OK,msg);

	/* Show the popup*/
	gtk_dialog_run(GTK_DIALOG(pPopup));
	
	/* Destroy the popup */
	gtk_widget_destroy(pPopup);
}

char choose_rom_popup(GtkWidget *parent_window, const char *filename,
                      char default_model)
{
	const TilemHardware **models;
	GtkWidget *dlg, *vbox, *frame, *btn;
	GtkToggleButton **btns;
	char *ids, id = 0;
	int nmodels, noptions, i, j, defoption, response;
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
	frame = new_gnome_style_frame(msg, vbox);
	g_free(fn);
	g_free(msg);

	gtk_container_set_border_width(GTK_CONTAINER(frame), 6);
	gtk_widget_show_all(frame);

	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dlg)->vbox), frame,
	                   FALSE, FALSE, 0);

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

#if 0
/* File chooser */
char * select_file(GLOBAL_SKIN_INFOS *gsi) {

GtkWidget *dialog;


dialog = gtk_file_chooser_dialog_new ("Open File", GTK_WINDOW(gsi->pWindow), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);

/* Launch the dialog and get the result (dialog widget is "modal") */
if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename;
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		printf(" filename: %s", filename);
		return filename;
		g_free (filename);
	}

gtk_widget_destroy (dialog);
return "";

}
#endif

/* File chooser with a different base directory */
char* select_file(GLOBAL_SKIN_INFOS *gsi, char* basedir) {

GtkWidget *dialog;
GtkFileChooser *pFileChooser;
char* filename = NULL;
gint result;

dialog = gtk_file_chooser_dialog_new ("Open File", GTK_WINDOW(gsi->pWindow), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
pFileChooser=GTK_FILE_CHOOSER(dialog);

if(basedir != NULL)
	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), basedir);
result = gtk_dialog_run (GTK_DIALOG (dialog)); 

/* ######## SIGNALS ######## */
/* Connect the signal to get the filename (when OK button is clicked) */
//gtk_signal_connect_object(GTK_OBJECT(dialog),"response",G_CALLBACK(get_selected_file),(gpointer)gsi);
/* Connect the destroy signal to OK */
//gtk_signal_connect(GTK_OBJECT(dialog),"response",G_CALLBACK(gtk_widget_destroy),(gpointer)gsi);

	if(result == GTK_RESPONSE_ACCEPT)
	{	
		filename=(gchar*)gtk_file_chooser_get_filename(pFileChooser);
		printf("Selected file : %s\n", filename);
	} else {
		printf("Cancelled ...\n");
	}	
	
	gtk_widget_destroy(GTK_WIDGET(pFileChooser));
	
	return filename;	

}

/* File chooser with a different base directory */
char* select_file_for_save(GLOBAL_SKIN_INFOS *gsi, char* basedir) {

GtkWidget *dialog;
GtkFileChooser *pFileChooser;
char* filename = NULL;
gint result;

dialog = gtk_file_chooser_dialog_new ("Save File", GTK_WINDOW(gsi->pWindow), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
pFileChooser=GTK_FILE_CHOOSER(dialog);

if(basedir != NULL)
	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), basedir);
result = gtk_dialog_run (GTK_DIALOG (dialog)); 

/* ######## SIGNALS ######## */
/* Connect the signal to get the filename (when OK button is clicked) */
//gtk_signal_connect_object(GTK_OBJECT(dialog),"response",G_CALLBACK(get_selected_file),(gpointer)gsi);
/* Connect the destroy signal to OK */
//gtk_signal_connect(GTK_OBJECT(dialog),"response",G_CALLBACK(gtk_widget_destroy),(gpointer)gsi);

	if(result == GTK_RESPONSE_ACCEPT)
	{	
		filename=(gchar*)gtk_file_chooser_get_filename(pFileChooser);
		printf("Selected file : %s\n", filename);
	} else {
		printf("Cancelled ...\n");
	}	
	
	gtk_widget_destroy(GTK_WIDGET(pFileChooser));
	
	return filename;	

}





/* File chooser with a different base directory */
void select_file_with_basedir(GLOBAL_SKIN_INFOS *gsi, char* basedir) {

GtkWidget *dialog;


dialog = gtk_file_chooser_dialog_new ("Open File", GTK_WINDOW(gsi->pWindow), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
gsi->pFileChooser=GTK_FILE_CHOOSER(dialog);

gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), basedir);
gsi->FileChooserResult = gtk_dialog_run (GTK_DIALOG (dialog)); 

/* ######## SIGNALS ######## */
/* Connect the signal to get the filename (when OK button is clicked) */
//gtk_signal_connect_object(GTK_OBJECT(dialog),"response",G_CALLBACK(get_selected_file),(gpointer)gsi);
/* Connect the destroy signal to OK */
//gtk_signal_connect(GTK_OBJECT(dialog),"response",G_CALLBACK(gtk_widget_destroy),(gpointer)gsi);

	printf("filechooserResult : %d", gsi->FileChooserResult);
	printf("GTK_RESPONSE_OK : %d", GTK_RESPONSE_OK);
	printf("GTK_RESPONSE_ACCEPT : %d\n", GTK_RESPONSE_ACCEPT);
	if(gsi->FileChooserResult == GTK_RESPONSE_ACCEPT)
	{
		gsi->FileSelected=(gchar*)gtk_file_chooser_get_filename(gsi->pFileChooser);
		printf("get_selected_file:  FileSelected : %s\n",gsi->FileSelected);
	} else {
		printf("Cancelled ...\n");		
	}	
/* Launch the dialog and get the result (dialog widget is "modal") */
	gtk_widget_destroy(GTK_WIDGET(gsi->pFileChooser));
	
//return gsi->FileSelected;							

}

void get_selected_file(GLOBAL_SKIN_INFOS *gsi) {

	/* Just get the file wich was selected */
	printf("filechooserResult : %d", gsi->FileChooserResult);
	printf("GTK_RESPONSE_OK : %d", GTK_RESPONSE_OK);
	printf("GTK_RESPONSE_ACCEPT : %d", GTK_RESPONSE_ACCEPT);
	if(gsi->FileChooserResult == GTK_RESPONSE_ACCEPT)
	{
		gsi->FileSelected=(gchar*)gtk_file_chooser_get_filename(gsi->pFileChooser);
		printf("ACCEPT !!");
		printf("get_selected_file:  FileSelected : %s\n",gsi->FileSelected);
	} else {
		printf("Cancelled ...\n");		
	}	
	gtk_widget_destroy(GTK_WIDGET(gsi->pFileChooser));
}


void copy_paste(char* src, char* dest){ 

	FILE * fsrc;
	FILE * fdest;
	char buffer[1024];
	int n;

	if((fdest = fopen(dest, "w+")) == NULL) {
		printf("Can't open destination : %s\n", dest);
	}

	if((fsrc = fopen(src, "r")) == NULL) {
		printf("Can't open src : %s\n", src);
	}


	while ((n = fread(buffer, 1, 1024, fsrc)) != 0) {
		fwrite(buffer, 1, n, fdest);
	}
	if(fsrc)	
		fclose(fsrc);
	if(fdest)
		fclose(fdest);

}




