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

static GtkTreeModel* fill_varlist();
TilemReceiveDialog* create_receive_menu(TilemCalcEmulator *emu);

/* Columns */
enum
{
	COL_INDEX = 0, 
	COL_NAME,
	COL_TYPE,
	COL_SIZE,
  	NUM_COLS
};



/* #### SIGNALS CALLBACK #### */

/* Close the window */
static void tilem_rcvmenu_on_close(G_GNUC_UNUSED GtkWidget* w, G_GNUC_UNUSED gpointer data) {
	TilemReceiveDialog* rcvdialog = (TilemReceiveDialog*) data;

	gtk_widget_hide(rcvdialog->window);
}

static char** tilem_rcvmenu_get_selected_vars(G_GNUC_UNUSED TilemReceiveDialog* rcvdialog) {

	char* ve_list = NULL;	
	
	return NULL;
}

/* Get a default filename composed from the varname and the extension given by ticonv/tifiles (depends on var entry : is it var or app, calc model) */
static gchar* tilem_rcvmenu_get_default_filename(TilemReceiveDialog* rcvdialog, VarEntry* ve) {
	gchar* basename = ticonv_varname_to_filename(get_calc_model(rcvdialog->emu->calc),ve->name, ve->type);
        gchar* default_filename = g_strconcat(basename, ".", tifiles_vartype2fext(get_calc_model(rcvdialog->emu->calc), ve->type), NULL);
	
	return default_filename;
}
	

/* Event called on Send button click. Get the selected var/app and save it. */
static void tilem_rcvmenu_on_receive(G_GNUC_UNUSED GtkWidget* w, G_GNUC_UNUSED gpointer data) {
	
	TilemReceiveDialog* rcvdialog = (TilemReceiveDialog*) data;

	char* dir = NULL; 		/* The directory */
	gchar* default_filename = NULL; /* Default filename (without directory) with extension */
	gchar* filename = NULL; 	/* Filename */
	gchar* varname = NULL;		/* Selected varname */
	int index = 0;			/* Index of the selected row (and var entry in the vlist struct) */
	GtkTreeSelection* selection = NULL; /* GtkTreeSelection */

	/* FIXME : allow multiple var/app selection */
	/* FIXME : handle error : no row selected */

	/* Get the selected index */
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(rcvdialog->treeview));
	gtk_tree_selection_get_selected(selection, &rcvdialog->model, &rcvdialog->iter);
	GList* gl = gtk_tree_selection_get_selected_rows(selection, &rcvdialog->model);
	while(gl) {
		GtkTreePath* gtp = (GtkTreePath*) gl->data;
		printf("Tree Path : %d\n", *gtk_tree_path_get_indices(gtp));
		gl = gl->next;
	}
	gtk_tree_selection_get_selected(selection, &rcvdialog->model, &rcvdialog->iter);
	gtk_tree_model_get (rcvdialog->model, &rcvdialog->iter, COL_INDEX, &index, COL_NAME, &varname, -1);

	/*  Get the recent directory */	
	tilem_config_get("download", "receivefile_recentdir/f", &dir, NULL);	
	if(!dir) dir = g_get_current_dir();

	/* Get a default filename with a correct extension (to be used as default in the prompt file dialog) */
	default_filename = tilem_rcvmenu_get_default_filename(rcvdialog, rcvdialog->emu->varapp->vlist[index]); 
		

	filename = prompt_save_file("Save file", GTK_WINDOW(rcvdialog->window), default_filename, dir, "All files", "*.*", "TI82 file", "*.82p", "TI83 file", "*.83p", "TI83+ or TI84+ file","*.8xp", "TI83+ or TI84+ falsh app", "*.8xk", NULL); /* FIXME : add the other extension */ 
	if(filename == NULL) 
		return;
	
	

	dir = g_path_get_dirname(filename);

	/* Save config */
	tilem_config_set("download", "receivefile_recentdir/f", dir, NULL);
	//tilem_receive_var(rcvdialog->emu, rcvdialog->emu->varapp->vlist[index], filename);
	
	tilem_calc_emulator_receive_file(rcvdialog->emu, rcvdialog->emu->varapp->vlist[index], filename);

	if(filename)	
		g_free(filename);	
	if(default_filename)	
		g_free(default_filename);	
	if(dir)
		g_free(dir);	
	if(varname)
		g_free(varname);
}

/* This function is executed when the dirlist asked by the refresh button is finished */
static void tilem_get_dirlist_refresh_finished(G_GNUC_UNUSED TilemCalcEmulator *emu, G_GNUC_UNUSED gpointer data, G_GNUC_UNUSED gboolean cancelled) {
	TilemReceiveDialog* rcvdialog = (TilemReceiveDialog*) data;
	
	rcvdialog->model = fill_varlist(rcvdialog, rcvdialog->emu->varapp->vlist_utf8);
        gtk_tree_view_set_model(GTK_TREE_VIEW(rcvdialog->treeview), rcvdialog->model);	
}

/* This function is executed when user click on refresh button */
static void tilem_rcvmenu_on_refresh(G_GNUC_UNUSED GtkWidget* w, G_GNUC_UNUSED gpointer data) {
	TilemReceiveDialog* rcvdialog = (TilemReceiveDialog*) data;

	/* Freeing previous allocated varlist and applist */	
	if(rcvdialog->emu->varapp->vlist)
		g_free(rcvdialog->emu->varapp->vlist);
	if(rcvdialog->emu->varapp)
		g_free(rcvdialog->emu->varapp);
	
	/* Get the varlist and the applist */
	tilem_calc_emulator_begin(rcvdialog->emu, &tilem_get_dirlist_main, &tilem_get_dirlist_refresh_finished, rcvdialog);	

}



/* A popup wich is needed because of the fact that ti82 and ti85 need to be in the "transmit" sate to get vars */
static void on_ask_prepare_receive_response(G_GNUC_UNUSED GtkWidget* w, G_GNUC_UNUSED GtkResponseType t,   G_GNUC_UNUSED gpointer data) {
	TilemCalcEmulator* emu = (TilemCalcEmulator*) data;

	
	/*if (!emu->link_thread)
		emu->link_thread = g_thread_create(&tilem_get_dirlist_ns, emu, TRUE, NULL);
	*/
	
	/*g_thread_join(emu->link_thread);*/ /* Do not create the menu if getting vars is not done */
	
	
	
 	/* Destroy the popup */
	gtk_widget_destroy(GTK_WIDGET(w));

	/* Print the window */
	emu->rcvdlg = create_receive_menu(emu);
	gtk_window_present(GTK_WINDOW(emu->rcvdlg->window));
	
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
static GtkWidget *create_varlist()
{
	GtkCellRenderer   *renderer;
	GtkWidget         *treeview;
	GtkTreeViewColumn *c1, *c2, *c3, *c4;
	
	/* Create the stack list tree view and set title invisible */
	treeview = gtk_tree_view_new();
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(treeview), TRUE);
	gtk_tree_view_set_fixed_height_mode(GTK_TREE_VIEW(treeview), TRUE);

	/* Create the columns */
	renderer = gtk_cell_renderer_text_new ();
	c1 = gtk_tree_view_column_new_with_attributes ("INDEX", renderer, "text", COL_INDEX, NULL);
	c2 = gtk_tree_view_column_new_with_attributes ("NAME", renderer, "text", COL_NAME, NULL);
	c3 = gtk_tree_view_column_new_with_attributes ("TYPE", renderer, "text", COL_TYPE, NULL);
	c4 = gtk_tree_view_column_new_with_attributes ("SIZE", renderer, "text", COL_SIZE, NULL);

	gtk_tree_view_column_set_sizing(c1, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_expand(c1, TRUE);
	gtk_tree_view_column_set_visible(c1, FALSE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), c1);


	gtk_tree_view_column_set_sizing(c2, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_expand(c2, TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), c2);

		
	gtk_tree_view_column_set_sizing(c3, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_expand(c3, TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), c3);
		
	gtk_tree_view_column_set_sizing(c4, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_expand(c4, TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), c4);
	
	return treeview;
}

/* Fill the list of vars. In fact, add all vars from list to a GtkListStore */
static GtkTreeModel* fill_varlist(TilemReceiveDialog * rcvdialog, char** list)
{
	
	rcvdialog->store = gtk_list_store_new (4, G_TYPE_INT, G_TYPE_STRING, G_TYPE_CHAR, G_TYPE_INT);
	int i = 0;
	if(list){
		for(i = 0; list[i]; i++) {
			/* Append a row */
			gtk_list_store_append (rcvdialog->store, &rcvdialog->iter);
			/* Fill the row */ 
			printf("list[%d] : %s\n", i, list[i]);
			gtk_list_store_set (rcvdialog->store, &rcvdialog->iter, COL_INDEX, i, COL_NAME, list[i], COL_TYPE, rcvdialog->emu->varapp->vlist[i]->type, COL_SIZE, rcvdialog->emu->varapp->vlist[i]->size, -1);
			
		}
	}
	return GTK_TREE_MODEL (rcvdialog->store);
}

/* Create a new menu for receiving vars. */
/* Previous allocated and filled varlist is needed */
TilemReceiveDialog* create_receive_menu(TilemCalcEmulator *emu)
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
	rcvdialog->treeview = create_varlist();  	
	if(!rcvdialog->model) {
		if(emu->varapp) {
			rcvdialog->model = fill_varlist(rcvdialog, emu->varapp->vlist_utf8);
		} else { 
			rcvdialog->model = fill_varlist(rcvdialog, NULL);
		}
			
        	gtk_tree_view_set_model(GTK_TREE_VIEW(rcvdialog->treeview), rcvdialog->model);	
	}

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



/* #### ENTRY POINT #### */

/* Ask the user to to some action to have the calc in the send state */
void ask_prepare_receive (TilemCalcEmulator* emu)
{
	GtkWidget *dialog, *label, *content_area;

	/* Start to wait the transmit state */
	if (!emu->link_thread)
		emu->link_thread = g_thread_create(&tilem_get_dirlist_ns, emu, TRUE, NULL);

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


/* Popup the receive window */
/* This is the entry point */
void popup_receive_menu(TilemEmulatorWindow *ewin)
{
	TilemReceiveDialog* rcvdlg;

	g_return_if_fail(ewin != NULL);
	g_return_if_fail(ewin->emu != NULL);


	if (ewin->emu->calc->hw.model_id == TILEM_CALC_TI81) {
		/* FIXME : do something for ti81 */
	} else if (ewin->emu->calc->hw.model_id == TILEM_CALC_TI82) {
		ask_prepare_receive(ewin->emu); /* This function will create the receive menu when preparation is ok */
	} else if (ewin->emu->calc->hw.model_id == TILEM_CALC_TI85) {
		ask_prepare_receive(ewin->emu); /* This function will create the receive menu when preparation is ok */
	} else {
		/* If it's the first time we ask for this dialog, task manager do get dir list then finished function print the popup*/
		if(!ewin->emu->rcvdlg) {
			tilem_calc_emulator_begin(ewin->emu, &tilem_get_dirlist_main, &tilem_get_dirlist_finished, NULL);	
		} else {
			ewin->emu->rcvdlg = create_receive_menu(ewin->emu);
			rcvdlg = ewin->emu->rcvdlg;
			gtk_window_present(GTK_WINDOW(rcvdlg->window));
		}
	}
}






