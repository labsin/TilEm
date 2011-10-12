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
char ** tilem_get_dirlist(TilemCalcEmulator *emu)
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
		
	char ** list = g_new(char*, g_node_n_children(varparent) + g_node_n_children(appparent) + 1);
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
	
	printf("\n");
	
	/* Detach and delete cable. Delete calc handle*/	
	ticalcs_cable_detach(ch);
	ticalcs_handle_del(ch);
	ticables_handle_del(cbl);

	/* Exit the libtis */
	ticalcs_library_exit();
	tifiles_library_exit();
	ticables_library_exit();

	/* Return a list of entry names. terminated by NULL */
	return list;
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


/* Entry point for receive menu (should be moved into rcvmenu.c maybe) */
/*void get_var(TilemCalcEmulator *emu)
{

	tilem_rcvmenu_new(emu);
}*/



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
