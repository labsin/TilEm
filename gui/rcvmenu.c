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
  	NUM_COLS
};

/* Create the GtkTreeView to show the stack */
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

/* Create a new TilemDebugger. */
void tilem_rcvmenu_new(char** list, int list_size)
{
	int defwidth, defheight;


	//GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	GtkWidget *window = gtk_dialog_new();
	//gtk_window_set_title(GTK_WINDOW(window), "TilEm receive Menu");
	gtk_window_set_title(GTK_WINDOW(window), "TilEm receive Menu");
	gtk_dialog_add_button(GTK_DIALOG(window), "Save", 0);
	gtk_dialog_add_button(GTK_DIALOG(window), "Cancel", 1);

	defwidth = 200;
	defheight = 100;

	gtk_window_set_default_size(GTK_WINDOW(window), defwidth, defheight);
	
	GtkWidget* treeview = create_varlist();  	
	GtkTreeModel *model = fill_varlist(list, list_size);
        gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), model);	


	gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(window))), treeview);

	
	gtk_widget_show_all(GTK_WIDGET(window));


}

static GtkTreeModel* fill_varlist(char** list, int list_size)
{
	GtkListStore  *store;
	GtkTreeIter    iter;

	store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
	int i = 0;
	for(i = 0; i < list_size; i++) {
		char* name = g_strdup( list[i]);
		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter, COL_NAME, name, -1);
	}

	return GTK_TREE_MODEL (store);
}

