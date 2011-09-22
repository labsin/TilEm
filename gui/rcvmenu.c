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
#include "msgbox.h"

static GtkTreeModel* fill_varlist();

/* Stack list */
enum
{
	COL_NAME = 0,
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
	GtkTreeViewColumn *column;
	
	/* Create the stack list tree view and set title invisible */
	treeview = gtk_tree_view_new();
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(treeview), FALSE);
	gtk_tree_view_set_fixed_height_mode(GTK_TREE_VIEW(treeview), TRUE);

	/* Create the columns */
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("NAME", renderer, "text", COL_NAME, NULL);

	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_expand(column, TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
	
	
	return treeview;
}

static void tilem_rcvmenu_on_receive(GtkWidget* w, G_GNUC_UNUSED gpointer data) {
	printf("receive !!!!\n");
	gtk_widget_destroy(w);
}

/* Create a new menu for receiving vars. */
void tilem_rcvmenu_new(TilemCalcEmulator *emu)
{
	int defwidth, defheight;

	//GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	GtkWidget *window = gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(window), "TilEm receive Menu");
	GtkWidget* button_save = gtk_dialog_add_button(GTK_DIALOG(window), "Save file to disk", 0);
	GtkWidget* button_close = gtk_dialog_add_button(GTK_DIALOG(window), "Close", 1);

	/* Set the size of the dialog */
	defwidth = 200;
	defheight = 400;
	gtk_window_set_default_size(GTK_WINDOW(window), defwidth, defheight);
	
	/* Create and fill tree view */
	GtkWidget* treeview = create_varlist();  	
	GtkTreeModel *model = fill_varlist(tilem_get_dirlist(emu));
        gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), model);	

	/* Allow scrolling the list because we can't know how many vars the calc contains */
	GtkWidget * scroll = new_scrolled_window(treeview);
	gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(window))), scroll);

	g_signal_connect_swapped (window, "response", G_CALLBACK (gtk_widget_hide), window);
	

	
	gtk_widget_show_all(GTK_WIDGET(window));


}

static GtkTreeModel* fill_varlist(char** list)
{
	GtkListStore  *store;
	GtkTreeIter    iter;

	store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
	int i = 0;
	for(i = 0; list[i]; i++) {
		char* name = g_strdup(list[i]);
		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter, COL_NAME, name, -1);
	}
	return GTK_TREE_MODEL (store);
}

