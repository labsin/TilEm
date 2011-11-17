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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <ticalcs.h>
#include <ticonv.h>
#include <tilem.h>
#include <tilemdb.h>
#include <scancodes.h>

#include "gui.h"
#include "disasmview.h"
#include "memmodel.h"
#include "files.h"
#include "filedlg.h"
#include "msgbox.h"

static GtkTreeModel* fill_varlist(TilemReceiveDialog *rcvdialog);
TilemReceiveDialog* create_receive_menu(TilemCalcEmulator *emu);

/* Columns */
enum
{
	COL_ENTRY = 0,
	COL_SLOT_STR,
	COL_NAME_STR,
	COL_TYPE_STR,
	COL_SIZE_STR,
	COL_SIZE,
  	NUM_COLS
};



/* #### SIGNALS CALLBACK #### */

/* Close the window */
static void tilem_rcvmenu_on_close(G_GNUC_UNUSED GtkWidget* w, G_GNUC_UNUSED gpointer data) {
	TilemReceiveDialog* rcvdialog = (TilemReceiveDialog*) data;

	gtk_widget_hide(rcvdialog->window);
}

/* Event called on Send button click. Get the selected var/app and save it. */
static void tilem_rcvmenu_on_receive(G_GNUC_UNUSED GtkWidget* w, G_GNUC_UNUSED gpointer data) {
	
	TilemReceiveDialog* rcvdialog = (TilemReceiveDialog*) data;

	char* dir = NULL; 		/* The directory */
	gchar* default_filename = NULL; /* Default filename (without directory) with extension */
	gchar* filename = NULL; 	/* Filename */
	TilemVarEntry *tve;		/* Variable entry */
	GtkTreeSelection* selection = NULL; /* GtkTreeSelection */
	GtkTreeModel *model;
	GtkTreeIter iter;

	/* FIXME : allow multiple var/app selection */
	/* FIXME : handle error : no row selected */

	/* Get the selected entry */
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(rcvdialog->treeview));
	
	GList* list =   gtk_tree_selection_get_selected_rows(selection, &model);
	while(list) {
	 	GtkTreePath* path = (GtkTreePath*) list->data;
	 	printf("Tree Path : %d\n", *gtk_tree_path_get_indices(path));
		gtk_tree_model_get_iter(model, &iter, path);
	 	list = list->next;


		/*gtk_tree_selection_get_selected(selection, &model, &iter); */ /* Used only when selection->type != GTK_SELECTION_MULTIPLE */

		gtk_tree_model_get(model, &iter, COL_ENTRY, &tve, -1);
		/*  Get the recent directory */	
		tilem_config_get("download", "receivefile_recentdir/f", &dir, NULL);	
		if(!dir) dir = g_get_current_dir();

		/* Get a default filename with a correct extension (to be used as default in the prompt file dialog) */
		default_filename = g_strconcat(tve->name_str, ".", tve->file_ext, NULL);

		filename = prompt_save_file("Save file", GTK_WINDOW(rcvdialog->window), default_filename, dir, "All files", "*.*", "TI82 file", "*.82p", "TI83 file", "*.83p", "TI83+ or TI84+ file","*.8xp", "TI83+ or TI84+ falsh app", "*.8xk", NULL); /* FIXME : add the other extension */ 
		if(filename == NULL) 
			return;

		dir = g_path_get_dirname(filename);

		/* Save config */
		tilem_config_set("download", "receivefile_recentdir/f", dir, NULL);

		tilem_link_receive_file(rcvdialog->emu, tve, filename);
	} 

	if(filename)	
		g_free(filename);	
	if(default_filename)	
		g_free(default_filename);	
	if(dir)
		g_free(dir);	
}

/* This function is executed when user click on refresh button */
static void tilem_rcvmenu_on_refresh(G_GNUC_UNUSED GtkWidget* w, G_GNUC_UNUSED gpointer data) {
	TilemReceiveDialog* rcvdialog = (TilemReceiveDialog*) data;
	
	/* Get the varlist and the applist */
	tilem_link_get_dirlist(rcvdialog->emu);
}


/* #### WIDGET CREATION #### */

/* Create a new scrolled window with sensible default settings. */
static GtkWidget *new_scrolled_window(GtkWidget *contents)
{
        GtkWidget *sw; 
        sw = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
                                       GTK_POLICY_AUTOMATIC,
                                       GTK_POLICY_AUTOMATIC);
        gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw),
                                            GTK_SHADOW_IN);
        gtk_container_add(GTK_CONTAINER(sw), contents);
        return sw;
}

/* Create the (empty) GtkTreeView to show the vars list */
static GtkWidget *create_varlist(TilemReceiveDialog *rcvdialog)
{
	GtkCellRenderer   *renderer;
	GtkWidget         *treeview;
	GtkTreeViewColumn *c1, *c2, *c3, *c4;
	gboolean           is_81;

	g_return_val_if_fail(rcvdialog->emu != NULL, NULL);
	g_return_val_if_fail(rcvdialog->emu->calc != NULL, NULL);

	is_81 = (rcvdialog->emu->calc->hw.model_id == TILEM_CALC_TI81);

	/* Create the stack list tree view and set title invisible */
	treeview = gtk_tree_view_new();
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(treeview), TRUE);
	gtk_tree_view_set_headers_clickable(GTK_TREE_VIEW(treeview), TRUE);
	gtk_tree_view_set_fixed_height_mode(GTK_TREE_VIEW(treeview), TRUE);
	
	/* Allow multiple selection */
	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview)), GTK_SELECTION_MULTIPLE);

	/* Create the columns */
	renderer = gtk_cell_renderer_text_new();

	if (is_81) {
		c1 = gtk_tree_view_column_new_with_attributes
			("Slot", renderer, "text", COL_SLOT_STR, NULL);

		gtk_tree_view_column_set_sizing(c1, GTK_TREE_VIEW_COLUMN_FIXED);
		gtk_tree_view_column_set_sort_column_id(c1, COL_SLOT_STR);
		gtk_tree_view_column_set_expand(c1, TRUE);
		gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), c1);
	}

	c2 = gtk_tree_view_column_new_with_attributes
		("Name", renderer, "text", COL_NAME_STR, NULL);

	gtk_tree_view_column_set_sizing(c2, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_sort_column_id(c2, COL_NAME_STR);
	gtk_tree_view_column_set_expand(c2, TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), c2);

	if (!is_81) {
		c3 = gtk_tree_view_column_new_with_attributes
			("Type", renderer, "text", COL_TYPE_STR, NULL);
		
		gtk_tree_view_column_set_sizing(c3, GTK_TREE_VIEW_COLUMN_FIXED);
		gtk_tree_view_column_set_sort_column_id(c3, COL_TYPE_STR);
		gtk_tree_view_column_set_expand(c3, TRUE);
		gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), c3);
	}

	renderer = gtk_cell_renderer_text_new();
	g_object_set(renderer, "xalign", 1.0, NULL);
	c4 = gtk_tree_view_column_new_with_attributes
		("Size", renderer, "text", COL_SIZE_STR, NULL);
		
	gtk_tree_view_column_set_sizing(c4, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_sort_column_id(c4, COL_SIZE);
	gtk_tree_view_column_set_expand(c4, TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), c4);
	
	return treeview;
}

/* Fill the list of vars. In fact, add all vars from list to a GtkListStore */
static GtkTreeModel* fill_varlist(TilemReceiveDialog *rcvdialog)
{
	GSList *l;
	TilemVarEntry *tve;
	GtkListStore *store;
	GtkTreeIter iter;
	char *size_str;

	store = gtk_list_store_new(6,
	                           G_TYPE_POINTER,
	                           G_TYPE_STRING,
	                           G_TYPE_STRING,
	                           G_TYPE_STRING,
	                           G_TYPE_STRING,
	                           G_TYPE_INT);

	for (l = rcvdialog->vars; l; l = l->next) {
		tve = l->data;
		gtk_list_store_append(store, &iter);
#ifdef G_OS_WIN32
		size_str = g_strdup_printf("%d", tve->size);
#else
		size_str = g_strdup_printf("%'d", tve->size);
#endif
		gtk_list_store_set(store, &iter,
		                   COL_ENTRY, tve,
		                   COL_SLOT_STR, tve->slot_str,
		                   COL_NAME_STR, tve->name_str,
		                   COL_TYPE_STR, tve->type_str,
		                   COL_SIZE_STR, size_str,
		                   COL_SIZE, tve->size,
		                   -1);
		g_free(size_str);
	}

	return GTK_TREE_MODEL(store);
}

/* Create a new menu for receiving vars. */
/* Previous allocated and filled varlist is needed */
TilemReceiveDialog* tilem_receive_dialog_new(TilemCalcEmulator *emu)
{

	TilemReceiveDialog* rcvdialog = g_slice_new0(TilemReceiveDialog);
	rcvdialog->emu = emu;
	emu->rcvdlg = rcvdialog;
	rcvdialog->window = gtk_dialog_new();
	gtk_window_set_transient_for(GTK_WINDOW(rcvdialog->window), GTK_WINDOW(emu->ewin->window));	
	gtk_window_set_title(GTK_WINDOW(rcvdialog->window), "TilEm receive Menu");
	rcvdialog->button_refresh = gtk_dialog_add_button(GTK_DIALOG(rcvdialog->window), "Refresh", 0);
	rcvdialog->button_save = gtk_dialog_add_button(GTK_DIALOG(rcvdialog->window), "Save file to disk", 1);
	rcvdialog->button_close = gtk_dialog_add_button(GTK_DIALOG(rcvdialog->window), "Close", 2);

	/* Set the size of the dialog */
	int defwidth = 200;
	int defheight = 400;
	gtk_window_set_default_size(GTK_WINDOW(rcvdialog->window), defwidth, defheight);
	
	/* Create and fill tree view */
	rcvdialog->treeview = create_varlist(rcvdialog);

	/* Allow scrolling the list because we can't know how many vars the calc contains */
	GtkWidget * scroll = new_scrolled_window(rcvdialog->treeview);
	gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(rcvdialog->window))), scroll);

	/* Signals callback */	
	g_signal_connect(rcvdialog->button_refresh, "clicked", G_CALLBACK (tilem_rcvmenu_on_refresh), rcvdialog);
	g_signal_connect(rcvdialog->button_save, "clicked", G_CALLBACK (tilem_rcvmenu_on_receive), rcvdialog);
	g_signal_connect(rcvdialog->button_close, "clicked", G_CALLBACK (tilem_rcvmenu_on_close), rcvdialog);
	
	gtk_widget_show_all(GTK_WIDGET(rcvdialog->window));

	return rcvdialog;
}

void tilem_receive_dialog_update(TilemReceiveDialog *rcvdialog, GSList *varlist)
{
	GSList *l;

	g_return_if_fail(rcvdialog != NULL);

	for (l = rcvdialog->vars; l; l = l->next)
		tilem_var_entry_free(l->data);
	g_slist_free(rcvdialog->vars);

	rcvdialog->vars = varlist;
	rcvdialog->model = fill_varlist(rcvdialog);
	gtk_tree_view_set_model(GTK_TREE_VIEW(rcvdialog->treeview), rcvdialog->model);
}

/* #### ENTRY POINT #### */

#if 0
/* Ask the user to to some action to have the calc in the send state */
void ask_prepare_receive (TilemCalcEmulator* emu)
{
	GtkWidget *dialog, *label, *content_area;

	/* Start to wait the transmit state */
	/*
	if (!emu->link_thread)
		emu->link_thread = g_thread_create(&tilem_get_dirlist_ns, emu, TRUE, NULL);
	*/

	/* Create the widgets */
	dialog = gtk_dialog_new_with_buttons ("Message", GTK_WINDOW(emu->ewin->window), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_OK, GTK_RESPONSE_NONE, NULL);
	content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

	/* Add the image */
	char* shared = NULL;
	if (emu->calc->hw.model_id == TILEM_CALC_TI82) {
		label = gtk_label_new ("In order to transmit vars to your computer,\n ti82 needs some preparing tasks : \n\t - Firstly go to home\n\t - Then press 2nd, link\n\t - Then press enter\n\t - Then press right arrow\n\t - Press enter then very quickly click OK...");
		shared = get_shared_file_path("pixs", "prepare_ti82.gif", NULL); /* Get the gif image */
	} else if (emu->calc->hw.model_id == TILEM_CALC_TI85) {
		label = gtk_label_new ("In order to transmit vars to your computer,\n ti85 needs some preparing tasks : \n\t - Firstly go to home\n\t - Then press 2nd, link\n\t - Then press F1\n\t - Then press F5\n\t - Then press F3\n\t - Then press F1\n\t - Then very quickly click OK...");
		shared = get_shared_file_path("pixs", "prepare_ti82.gif", NULL); /* Get the gif image */
	}
	
	if(!shared) 
		return;
	
	GtkWidget* image = gtk_image_new_from_file(shared); 

	GtkWidget* vbox = gtk_vbox_new(FALSE, 10);
	gtk_box_pack_start_defaults(GTK_BOX(vbox), image);
	gtk_box_pack_end_defaults(GTK_BOX(vbox), image);
	gtk_container_add(GTK_CONTAINER(content_area), vbox);
	gtk_widget_show(vbox);

	gtk_signal_connect (GTK_OBJECT (dialog), "response", GTK_SIGNAL_FUNC (on_ask_prepare_receive_response), emu);

	gtk_container_add (GTK_CONTAINER (content_area), label);
	gtk_widget_show_all (dialog);
	
}
#endif

/* Popup the receive window */
/* This is the entry point */
void popup_receive_menu(TilemEmulatorWindow *ewin)
{
	g_return_if_fail(ewin != NULL);
	g_return_if_fail(ewin->emu != NULL);

	if (ewin->emu->rcvdlg)
		gtk_window_present(GTK_WINDOW(ewin->emu->rcvdlg->window));
	else
		tilem_link_get_dirlist(ewin->emu);
}

