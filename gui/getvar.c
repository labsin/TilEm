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
#include <stdlib.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <ticalcs.h>
#include <ticonv.h>
#include <string.h>
#include <tilem.h>
#include <scancodes.h>

#include "gui.h"
#include "ti81prg.h"




/* Display the list of vars/app */		
/* This is a modified version of get_dirlist (ticalc) 
 * because tiz80 never have folders (can you confirm that Benjamin...?) 
 */
void tilem_dirlist_display(GNode* tree)
{
	GNode *vars = tree;
	TreeInfo *info = (TreeInfo *)(tree->data);
	int i, j, k;
	char *utf8;
  
	if (tree == NULL)
		return;

	printf(  "+------------------+----------+----------+\n");
	printf(  "| B. name          | T. name  | Size     |\n");
	printf(  "+------------------+----------+----------+\n");
	i = 0;

	GNode *parent = g_node_nth_child(vars, i);


	for (j = 0; j < (int)g_node_n_children(parent); j++)	//parse variables
	{
		GNode *child = g_node_nth_child(parent, j);
		VarEntry *ve = (VarEntry *) (child->data);

		utf8 = ticonv_varname_to_utf8(info->model, ve->name, ve->type);

	printf("| ");
	for (k = 0; k < 8; k++) 
		printf("%02X", (uint8_t) (ve->name)[k]);
	printf(" | ");
	printf("%8s", utf8);
	printf(" | ");
	printf("%08X", ve->size);
	printf(" | ");
		printf("\n");
		g_free(utf8);
	}
	printf("+------------------+----------+----------+");
	printf("\n");
}

/* Get the list of varname. I plan to use it into a list (in a menu) */
/* Terminated by NULL */
void tilem_get_dirlist_ns(TilemCalcEmulator *emu)
{
	CableHandle* cbl;
	CalcHandle* ch;
	
	/* Init the libtis */
	ticables_library_init();
	tifiles_library_init();
	ticalcs_library_init();
	
	/* Create cable (here an internal an dvirtual cabla) */
	cbl = internal_link_handle_new(emu);
	if (!cbl) 
		fprintf(stderr, "Cannot create ilp handle\n");
	

	ch = ticalcs_handle_new(get_calc_model(emu->calc));
	if (!ch) {
		fprintf(stderr, "INTERNAL ERROR: unsupported calc\n");
	}
	

	ticalcs_cable_attach(ch, cbl);
	
	FileContent *content = tifiles_content_create_regular(ch->model);
	VarEntry* ve;
	ticalcs_calc_recv_var_ns(ch, MODE_NORMAL, content, &ve);

	emu->varapp = (TilemVarApp*)g_new(TilemVarApp*, 1);
	char** list = g_new(char*, content->num_entries + 1);
	int i = 0;
	for(i = 0; i < content->num_entries; i++) {
		char* utf8 = ticonv_varname_to_utf8(ch->model, content->entries[i]->name, content->entries[i]->type);
		printf("content->entries[%d] : ve.name = %s\n", i, utf8);
		list[i] = utf8;
	
	}

	list[i] = NULL;
	printf("\n");
	emu->varapp->vlist_utf8 = list;
	emu->varapp->vlist = content->entries;
	
	
	/* Detach and delete cable. Delete calc handle*/	
	ticalcs_cable_detach(ch);
	ticalcs_handle_del(ch);
	ticables_handle_del(cbl);

	/* Exit the libtis */
	ticalcs_library_exit();
	tifiles_library_exit();
	ticables_library_exit();
}




/* Get the list of varname. I plan to use it into a list (in a menu) */
/* Terminated by NULL */
void tilem_get_dirlist(TilemCalcEmulator *emu)
{
	CableHandle* cbl;
	CalcHandle* ch;
	
	/* Init the libtis */
	ticables_library_init();
	tifiles_library_init();
	ticalcs_library_init();
	
	/* Create cable (here an internal an dvirtual cabla) */
	cbl = internal_link_handle_new(emu);
	if (!cbl) 
		fprintf(stderr, "Cannot create ilp handle\n");
	

	ch = ticalcs_handle_new(get_calc_model(emu->calc));
	if (!ch) {
		fprintf(stderr, "INTERNAL ERROR: unsupported calc\n");
	}
	

	ticalcs_cable_attach(ch, cbl);
	
	GNode *vars, *apps;
	ticalcs_calc_get_dirlist(ch, &vars, &apps);

	TreeInfo *info = (TreeInfo *)(vars->data);
	int i, j;
	char *utf8;
  
	i = 0;
	/* Tree for vars */
	GNode *varparent = g_node_nth_child(vars, i);
	/* Tree for app */
	GNode *appparent = g_node_nth_child(apps, i);

	
	emu->varapp = (TilemVarApp*)g_new(TilemVarApp*, 1);
	emu->varapp->vlist = (VarEntry**) g_new(VarEntry**, (int)g_node_n_children(varparent) + (int)g_node_n_children(appparent) + 1);
		
	char** list = g_new(char*, g_node_n_children(varparent) + g_node_n_children(appparent) + 1);
	//printf("Number of children : %d\n", g_node_n_children(parent));

	/* Get vars */
	for (j = 0; j < (int)g_node_n_children(varparent); j++)	//parse variables
	{
		GNode *child = g_node_nth_child(varparent, j);
		VarEntry *ve = (VarEntry *) (child->data);

		utf8 = ticonv_varname_to_utf8(info->model, ve->name, ve->type);
		
		emu->varapp->vlist[j] = (VarEntry*) g_new(VarEntry*, 1);
		emu->varapp->vlist[j] = ve; 

		list[j] = g_strdup(utf8);
		printf ("utf8 : %s\n", utf8);
		g_free(utf8);
	}
	
	int k = j;	
	int l = 0;	

	/* Get apps */	
	for (j = j; j < k + (int)g_node_n_children(appparent); j++)
	{
		GNode *child = g_node_nth_child(appparent, l);
		VarEntry *ve = (VarEntry *) (child->data);

		utf8 = ticonv_varname_to_utf8(info->model, ve->name, ve->type);
		
		emu->varapp->vlist[j] = (VarEntry*) g_new(VarEntry*, 1);
		emu->varapp->vlist[j] = ve; 

		list[j] = g_strdup(utf8);
		printf ("utf8 : %s\n", utf8);
		g_free(utf8);
		l++;
	}
	
	list[j] = NULL;
	emu->varapp->vlist[j] = NULL;
	emu->varapp->vlist_utf8 = list;
	
	printf("\n");
	
	/* Detach and delete cable. Delete calc handle*/	
	ticalcs_cable_detach(ch);
	ticalcs_handle_del(ch);
	ticables_handle_del(cbl);

	/* Exit the libtis */
	ticalcs_library_exit();
	tifiles_library_exit();
	ticables_library_exit();
}


/* Return the number of var */ 
gint tilem_get_dirlist_size(GNode* tree)
{
	GNode *vars = tree;
	int i = 0;
	
	GNode *parent = g_node_nth_child(vars, i);
		
	return g_node_n_children(parent);

}

/* Just print the list content (debug)*/
void dirlist_print_debug(char **list) {
	int i = 0;
	for(i = 0; list[i]; i++) {
		printf("%d. Var : %s\n", i, list[i]);
	}
}

/* Get a var from calc and save it into filename on PC
 * This function should really use a separate thread because it freeze the calc
 * a long time. 
 */
int tilem_receive_var(TilemCalcEmulator* emu, VarEntry* varentry, char* destination) {
	
	CableHandle* cbl;
	CalcHandle* ch;
	
	/* Init the libtis */
	ticables_library_init();
	tifiles_library_init();
	ticalcs_library_init();

	
	
	/* Create cable (here an internal an dvirtual cabla) */
	cbl = internal_link_handle_new(emu);
	if (!cbl) 
		fprintf(stderr, "Cannot create ilp handle\n");
	

	ch = ticalcs_handle_new(get_calc_model(emu->calc));
	if (!ch) {
		fprintf(stderr, "INTERNAL ERROR: unsupported calc\n");
	}
	

	ticalcs_cable_attach(ch, cbl);
	
	ticalcs_update_set(ch, emu->link_update);
	

	/* Currently, this code is based on romain lievins example */
	
	FileContent* filec;

	filec = tifiles_content_create_regular(ch->model);

        ticalcs_calc_recv_var(ch, MODE_NORMAL, filec, varentry);	
	tifiles_file_display_regular(filec);
	tifiles_file_write_regular(destination, filec, NULL);
	
	ticalcs_cable_detach(ch);
	ticalcs_handle_del(ch);
	ticables_handle_del(cbl);


	/* Exit the libtis */
	ticalcs_library_exit();
	tifiles_library_exit();
	ticables_library_exit();


	return 0;
}

/****************/
/* PROGRESS BAR */
/****************/

/* idle callback to start progress bar */
static gboolean pbar_start(gpointer data)
{
	TilemCalcEmulator *emu = data;

	if (!emu->linkpb->ilp_progress_win)
		progress_bar_init(emu);

	return FALSE;
}

/* idle callback to close progress bar */
static gboolean pbar_stop(gpointer data)
{
	TilemCalcEmulator *emu = data;

	if (emu->linkpb->ilp_progress_win) {
		gtk_widget_destroy(emu->linkpb->ilp_progress_win);
		emu->linkpb->ilp_progress_win = NULL;
		emu->linkpb->ilp_progress_bar1 = NULL;
		emu->linkpb->ilp_progress_bar2 = NULL;
		emu->linkpb->ilp_progress_label = NULL;
	}

	return FALSE;
}

/* idle callback to update progress bar */
static gboolean pbar_update(gpointer data)
{
	TilemCalcEmulator *emu = data;

	if (emu->linkpb->ilp_progress_win)
		progress_bar_update_activity(emu);

	return FALSE;
}

static GStaticPrivate current_emu_key = G_STATIC_PRIVATE_INIT;

/* ticalcs progress bar callback */
static void pbar_do_update()
{
	TilemCalcEmulator *emu = g_static_private_get(&current_emu_key);
	g_idle_add(&pbar_update, emu);
}


/* Link thread main loop */
static gpointer link_main2(gpointer data)
{
	TilemCalcEmulator *emu = data;
	VarEntry* varentry;
	char *fname;
	CableHandle *cbl;

	ticables_library_init();
	tifiles_library_init();
	ticalcs_library_init();

	cbl = internal_link_handle_new(emu);

	g_static_private_set(&current_emu_key, emu, NULL);

	g_mutex_lock(emu->link_queue_mutex);
	while (!emu->link_cancel) {
		
		if (!(varentry = (VarEntry*) g_queue_pop_head(emu->link_queue))) {
			g_cond_wait(emu->link_queue_cond, emu->link_queue_mutex);
			continue;
		}
		printf("varentry : ve.name = %s", varentry->name);
		
		
		if (!(fname = g_queue_pop_head(emu->link_queue))) {
			g_cond_wait(emu->link_queue_cond, emu->link_queue_mutex);
			continue;
		}
		printf("fname : %s\n", fname);
	
	
		g_mutex_unlock(emu->link_queue_mutex);

		emu->link_update->max1 = 0;
		emu->link_update->max2 = 0;
		emu->link_update->text[0] = 0;

		g_idle_add(&pbar_start, emu);

		emu->link_update->pbar = &pbar_do_update;
		emu->link_update->label = &pbar_do_update;

		tilem_receive_var(emu, varentry, fname);

		g_free(fname);

		g_mutex_lock(emu->link_queue_mutex);

		if (g_queue_is_empty(emu->link_queue))
			g_idle_add(&pbar_stop, emu);
	}
	g_mutex_unlock(emu->link_queue_mutex);

	ticables_handle_del(cbl);

	ticalcs_library_exit();
	tifiles_library_exit();
	ticables_library_exit();

	return NULL;
}

/* Send a file creating and using a separate thread */
void tilem_calc_emulator_receive_file(TilemCalcEmulator *emu,
                                   VarEntry* varentry, char* destination)
{
	g_return_if_fail(emu != NULL);
	g_return_if_fail(emu->calc != NULL);
	g_return_if_fail(varentry != NULL);
	g_return_if_fail(destination != NULL);

	emu->ilp.abort = FALSE;
	emu->link_cancel = FALSE;

	g_mutex_lock(emu->link_queue_mutex);
	g_queue_push_tail(emu->link_queue, varentry);
	g_queue_push_tail(emu->link_queue, g_strdup(destination));
	g_cond_broadcast(emu->link_queue_cond);
	g_mutex_unlock(emu->link_queue_mutex);
	printf("Avant thread\n");

	if (!emu->link_thread)
		emu->link_thread = g_thread_create(&link_main2, emu, TRUE, NULL);
	printf("Apres thread\n");
}
