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
#include <tilem.h>
#include <tilemdb.h>

#include "gui.h"
#include "disasmview.h"
#include "memmodel.h"
#include "files.h"
#include "filedlg.h"
#include "msgbox.h"

static GtkTreeModel* fill_varlist();

/* Stack list */
enum
{
	COL_INDEX = 0, 
	COL_NAME,
	COL_TYPE,
	COL_SIZE,
  	NUM_COLS
};


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



/* Create the GtkTreeView to show the vars list */
static GtkWidget *create_varlist()
{
	GtkCellRenderer   *renderer;
	GtkWidget         *treeview;
	GtkTreeViewColumn *c1;
	GtkTreeViewColumn *c2;
	GtkTreeViewColumn *c3;
	GtkTreeViewColumn *c4;
	
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

/* Event called on Send button click. Get the selected var/app and save it. */
static void tilem_rcvmenu_on_receive(G_GNUC_UNUSED GtkWidget* w, G_GNUC_UNUSED gpointer data) {
	TilemReceiveDialog* rcvdialog = (TilemReceiveDialog*) data;
	printf("receive !!!!\n");
	gchar* varname;
	int index;
	//gtk_tree_model_get (rcvdialog->model, &rcvdialog->iter, 0, &varname, -1);
	GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(rcvdialog->treeview));
	gtk_tree_selection_get_selected(selection, &rcvdialog->model, &rcvdialog->iter);
	gtk_tree_model_get (rcvdialog->model, &rcvdialog->iter, COL_INDEX, &index, COL_NAME, &varname, -1);
	printf("choice : %d\t%s\n", index, varname);
	
	gchar* path = prompt_save_file("Save file", GTK_WINDOW(rcvdialog->window), "", "", "destination", NULL, NULL);
	if(path == NULL)
		return;
	printf("Destination : %s\n", path);
	
//	tilem_receive_var(rcvdialog->emu, rcvdialog->emu->varapp->vlist[index], path);
	
	tilem_calc_emulator_receive_file(rcvdialog->emu, rcvdialog->emu->varapp->vlist[index], path);

	
	
	g_free(varname);
 
}


static void tilem_rcvmenu_on_refresh(G_GNUC_UNUSED GtkWidget* w, G_GNUC_UNUSED gpointer data) {
	TilemReceiveDialog* rcvdialog = (TilemReceiveDialog*) data;

	
	if(rcvdialog->emu->varapp->vlist)
		g_free(rcvdialog->emu->varapp->vlist);
	if(rcvdialog->emu->varapp)
		g_free(rcvdialog->emu->varapp);
	
	rcvdialog->model = fill_varlist(rcvdialog, tilem_get_dirlist(rcvdialog->emu));
        gtk_tree_view_set_model(GTK_TREE_VIEW(rcvdialog->treeview), rcvdialog->model);	
	gtk_widget_show(GTK_WIDGET(rcvdialog->window));
}
	

/* Close the window */
static void tilem_rcvmenu_on_close(G_GNUC_UNUSED GtkWidget* w, G_GNUC_UNUSED gpointer data) {
	TilemReceiveDialog* rcvdialog = (TilemReceiveDialog*) data;

	gtk_widget_hide(rcvdialog->window);
}

/* Create a new menu for receiving vars. */
TilemReceiveDialog* create_receive_menu(TilemCalcEmulator *emu)
{
	int defwidth, defheight;

	TilemReceiveDialog* rcvdialog = g_slice_new0(TilemReceiveDialog);
	rcvdialog->emu = emu;
	emu->rcvdlg = rcvdialog;
	//GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	rcvdialog->window = gtk_dialog_new();
	gtk_window_set_transient_for(GTK_WINDOW(rcvdialog->window), GTK_WINDOW(emu->ewin->window));	
	gtk_window_set_title(GTK_WINDOW(rcvdialog->window), "TilEm receive Menu");
	rcvdialog->button_refresh = gtk_dialog_add_button(GTK_DIALOG(rcvdialog->window), "Refresh", 0);
	rcvdialog->button_save = gtk_dialog_add_button(GTK_DIALOG(rcvdialog->window), "Save file to disk", 1);
	rcvdialog->button_close = gtk_dialog_add_button(GTK_DIALOG(rcvdialog->window), "Close", 2);

	/* Set the size of the dialog */
	defwidth = 200;
	defheight = 400;
	gtk_window_set_default_size(GTK_WINDOW(rcvdialog->window), defwidth, defheight);
	
	/* Create and fill tree view */
	rcvdialog->treeview = create_varlist();  	
	rcvdialog->model = fill_varlist(rcvdialog, tilem_get_dirlist(emu));
        gtk_tree_view_set_model(GTK_TREE_VIEW(rcvdialog->treeview), rcvdialog->model);	

	/* Allow scrolling the list because we can't know how many vars the calc contains */
	GtkWidget * scroll = new_scrolled_window(rcvdialog->treeview);
	gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(rcvdialog->window))), scroll);

	
	//g_signal_connect_swapped (window, "response", G_CALLBACK (gtk_widget_hide), window);
	g_signal_connect(rcvdialog->button_refresh, "clicked", G_CALLBACK (tilem_rcvmenu_on_refresh), rcvdialog);
	g_signal_connect(rcvdialog->button_save, "clicked", G_CALLBACK (tilem_rcvmenu_on_receive), rcvdialog);
	g_signal_connect(rcvdialog->button_close, "clicked", G_CALLBACK (tilem_rcvmenu_on_close), rcvdialog);
	
	
	gtk_widget_show_all(GTK_WIDGET(rcvdialog->window));

	return rcvdialog;


}



/* Popup the receive window */
void popup_receive_menu(TilemEmulatorWindow *ewin)
{
	TilemReceiveDialog* rcvdlg;

	g_return_if_fail(ewin != NULL);
	g_return_if_fail(ewin->emu != NULL);

	if (!ewin->emu->rcvdlg)
		ewin->emu->rcvdlg = create_receive_menu(ewin->emu);
	rcvdlg = ewin->emu->rcvdlg;

	gtk_window_present(GTK_WINDOW(rcvdlg->window));
}

/* Fill the list of vars. In fact, add all vars from list to a GtkListStore */
static GtkTreeModel* fill_varlist(TilemReceiveDialog * rcvdialog, char** list)
{
	
	
	
	rcvdialog->store = gtk_list_store_new (4, G_TYPE_INT, G_TYPE_STRING, G_TYPE_CHAR, G_TYPE_INT);
	int i = 0;
	for(i = 0; list[i]; i++) {
		/* Append a row */
		gtk_list_store_append (rcvdialog->store, &rcvdialog->iter);
		/* Fill the row */ 
		gtk_list_store_set (rcvdialog->store, &rcvdialog->iter, COL_INDEX, i, COL_NAME, list[i], COL_TYPE, rcvdialog->emu->varapp->vlist[i]->type, COL_SIZE, rcvdialog->emu->varapp->vlist[i]->size, -1);
		
	}
	return GTK_TREE_MODEL (rcvdialog->store);
}

