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


#include "debugger.h"
#include "memmodel.h"

/* This function is the first to be called. Start the debugger */
void launch_debugger(GLOBAL_SKIN_INFOS* gsi){

	//printf("%s", gsi->RomName);
	dasm= tilem_disasm_new();
	//gsi->reg_entry=malloc(sizeof(TilemDisasm));
	//gsi->dasm= tilem_disasm_new();	
	//gsi->dasm= dasm;
	create_debug_window(gsi);
	
}

/* Destroy the debugger, and turn to FALSE the isDebuggerRunning variable (use by other function to know if debuuger is running) */
void on_debug_destroy(GtkWidget* debug_win, GLOBAL_SKIN_INFOS* gsi)
{
	gsi->isDebuggerRunning=FALSE;
	if(GTK_IS_WIDGET(debug_win))
		gtk_widget_destroy(GTK_WIDGET(debug_win));

}

/* Print register value in terminal (copy paste of Benjamin Moody's work) */
void printstate(TilemCalcEmulator* emu)
{
       printf("*  PC=%02X:%04X AF=%04X BC=%04X DE=%04X                  *\n"
		"*  HL=%04X IX=%04X IY=%04X SP=%04X                     *\n",
       emu->calc->mempagemap[emu->calc->z80.r.pc.b.h >> 6],
       emu->calc->z80.r.pc.w.l,
       emu->calc->z80.r.af.w.l,
       emu->calc->z80.r.bc.w.l,
       emu->calc->z80.r.de.w.l,
       emu->calc->z80.r.hl.w.l,
       emu->calc->z80.r.ix.w.l,
       emu->calc->z80.r.iy.w.l,
       emu->calc->z80.r.sp.w.l);
}

/* Return the string value of the register(function used by create_register_list) */
void getreg(GLOBAL_SKIN_INFOS* gsi , int i, char* string)
{
	switch(i)
	{
	case 0:
		snprintf(string, 5, "%04X", gsi->emu->calc->z80.r.af2.d);
		break;
	case 1:
		snprintf(string, 5, "%04X", gsi->emu->calc->z80.r.af.d);
		break;
	case 2:
		snprintf(string, 5, "%04X", gsi->emu->calc->z80.r.bc2.d);
		break;
	case 3:
		snprintf(string, 5, "%04X", gsi->emu->calc->z80.r.bc.d);
		break;
	case 4:
		snprintf(string, 5, "%04X", gsi->emu->calc->z80.r.de2.d);
		break;
	case 5:
		snprintf(string, 5, "%04X", gsi->emu->calc->z80.r.de.d);
		break;
	case 6:
		snprintf(string, 5, "%04X", gsi->emu->calc->z80.r.hl2.d);
		break;
	case 7:
		snprintf(string, 5, "%04X", gsi->emu->calc->z80.r.hl.d);
		break;
	case 8:
		snprintf(string, 5, "%04X", gsi->emu->calc->z80.r.iy.d);
		break;
	case 9:
		snprintf(string, 5, "%04X", gsi->emu->calc->z80.r.ix.d);
		break;
	case 10:
		snprintf(string, 5, "%04X", gsi->emu->calc->z80.r.sp.d);
		break;
	case 11:
		snprintf(string, 5, "%04X", gsi->emu->calc->z80.r.pc.d);
		break;
	default:
		snprintf(string, 5, "%s", "----");
		break;

	}

}

/* Return the string value of the register */
gushort getreg_int(GLOBAL_SKIN_INFOS* gsi , int i)
{
	switch(i)
	{
	case 0:
		return gsi->emu->calc->z80.r.af2.d;
		break;
	case 1:
		return gsi->emu->calc->z80.r.af.d;
		break;
	case 2:
		return gsi->emu->calc->z80.r.bc2.d;
		break;
	case 3:
		return gsi->emu->calc->z80.r.bc.d;
		break;
	case 4:
		return gsi->emu->calc->z80.r.de2.d;
		break;
	case 5:
		return gsi->emu->calc->z80.r.de.d;
		break;
	case 6:
		return gsi->emu->calc->z80.r.hl2.d;
		break;
	case 7:
		return gsi->emu->calc->z80.r.hl.d;
		break;
	case 8:
		return gsi->emu->calc->z80.r.iy.d;
		break;
	case 9:
		return gsi->emu->calc->z80.r.ix.d;
		break;
	case 10:
		return gsi->emu->calc->z80.r.sp.d;
		break;
	case 11:
		return gsi->emu->calc->z80.r.pc.d;
		break;
	default:
		return 0;
		break;

	}

}

/* Create the debugger window */
void create_debug_window(GLOBAL_SKIN_INFOS* gsi) 
{
	
	GtkWidget* debug_win= gtk_window_new(GTK_WINDOW_TOPLEVEL);
	//gtk_window_set_modal(GTK_WINDOW(debug_win), TRUE);
	gtk_widget_set_size_request(GTK_WIDGET(debug_win) , 600, 400);	
	gtk_window_set_resizable(GTK_WINDOW(debug_win), TRUE);	
	gtk_window_set_title(GTK_WINDOW(debug_win), "TilEm debugger");
	GtkWidget* debug_table= create_debug_table(gsi);
	
	/* Create the central dasm scrolled widget */
	GtkWidget* debug_dasmscroll= gtk_scrolled_window_new(NULL, NULL);	
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(debug_dasmscroll), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
	create_dasm_list(debug_dasmscroll, gsi);	
		
	/* Create the stack list scrolled widget */
	GtkWidget* debug_stackframe= gtk_frame_new("Stack");
	gtk_frame_set_label_align(GTK_FRAME(debug_stackframe), 1.0, 0.5);
	GtkWidget* debug_stackscroll= gtk_scrolled_window_new(NULL, NULL);	
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(debug_stackscroll), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
	create_stack_list(debug_stackscroll, gsi);
	gtk_container_add(GTK_CONTAINER(debug_stackframe), debug_stackscroll);
	
	/* Create the memory list scrolled widget */
	GtkWidget* debug_memoryframe= gtk_frame_new("Memory");
	gtk_frame_set_label_align(GTK_FRAME(debug_memoryframe), 0.5, 0.5);
	//gtk_widget_set_size_request(GTK_WIDGET(debug_memoryframe) , 400, 160);	
	GtkWidget* debug_memoryscroll= gtk_scrolled_window_new(NULL, NULL);	
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(debug_memoryscroll), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
	create_memory_list(debug_memoryscroll, gsi);
	gtk_container_add(GTK_CONTAINER(debug_memoryframe), debug_memoryscroll);
	
	GtkWidget* debug_label= gtk_label_new("Tilem-dbg (c) Duponchelle Thibault, Moody Benjamin, Bruant Luc");
	gtk_table_attach_defaults(GTK_TABLE(debug_table), debug_label, 0, 7, 13, 14);
	gtk_table_attach_defaults(GTK_TABLE(debug_table),debug_dasmscroll, 1 , 6, 1, 7); 
	gtk_table_attach_defaults(GTK_TABLE(debug_table),debug_stackframe, 6 , 7, 0, 7); 
	gtk_table_attach_defaults(GTK_TABLE(debug_table), debug_memoryframe, 0, 7, 7, 13);
	gtk_container_add(GTK_CONTAINER(debug_win), debug_table);
	//gtk_signal_connect(GTK_OBJECT(debug_win), "delete-event", G_CALLBACK(on_debug_destroy), gsi);
	g_signal_connect(GTK_OBJECT(debug_win), "delete-event", G_CALLBACK(on_debug_destroy), gsi);	
	gtk_widget_show_all(debug_win);
}

/* The debugger windows contains a GtkTable.This is the high layout. */
GtkWidget* create_debug_table(GLOBAL_SKIN_INFOS* gsi) 
{
	GtkWidget* debug_table= gtk_table_new(14, 8, FALSE);
	create_debug_button(debug_table);
	create_register_list(debug_table, gsi);
	return debug_table;
}

/* Create navigation button */
void create_debug_button(GtkWidget* debug_table) 
{	
	/* Create the navigation buttons (5 buttons)*/
	GtkWidget* buttonleft2= gtk_button_new();
	GtkWidget* left2= gtk_image_new_from_file("./pix/rewindleft2.png"); 
	gtk_button_set_image(GTK_BUTTON(buttonleft2), left2);
	gtk_table_attach(GTK_TABLE(debug_table), buttonleft2, 1, 2, 0, 1, 0, 0, 0, 0);
	
	GtkWidget* buttonleft1= gtk_button_new();
	GtkWidget* left1= gtk_image_new_from_file("./pix/rewindleft1.png"); 
	gtk_button_set_image(GTK_BUTTON(buttonleft1), left1);
	gtk_table_attach(GTK_TABLE(debug_table), buttonleft1, 2, 3, 0, 1, 0, 0, 0, 0);
	
	GtkWidget* buttonpause= gtk_button_new();
	GtkWidget* pause= gtk_image_new_from_file("./pix/pause.png"); 
	gtk_button_set_image(GTK_BUTTON(buttonpause), pause);
	gtk_table_attach(GTK_TABLE(debug_table), buttonpause, 3, 4, 0, 1, 0, 0, 0, 0);
	
	GtkWidget* buttonright1= gtk_button_new();
	GtkWidget* right1= gtk_image_new_from_file("./pix/rewindright1.png"); 
	gtk_button_set_image(GTK_BUTTON(buttonright1), right1);
	gtk_table_attach(GTK_TABLE(debug_table), buttonright1, 4, 5, 0, 1, 0, 0, 0, 0);
	
	GtkWidget* buttonright2= gtk_button_new();
	GtkWidget* right2= gtk_image_new_from_file("./pix/rewindright2.png"); 
	gtk_button_set_image(GTK_BUTTON(buttonright2), right2);
	gtk_table_attach(GTK_TABLE(debug_table), buttonright2, 5, 6, 0, 1, 0, 0, 0, 0);

}


/* Refresh the register value in the GtkEntry (could be called out of debugger.c)*/
void refresh_register(GLOBAL_SKIN_INFOS* gsi)
{
	DDEBUGGER_L0_A0("******************** fct: refresh_register *************\n");
	DDEBUGGER_L0_A0("*  Refresh register :                                  *\n");
	printstate(gsi->emu);
	DDEBUGGER_L0_A0("********************************************************\n");

	if(gsi->reg_entry!=NULL)
	{
		char string[50];
		int i=0;
		for(i=0; i<12; i++) 
		{
			getreg(gsi, i, string);
			gtk_entry_set_text(GTK_ENTRY(gsi->reg_entry->reg[i]), string);
		}
	}

}

/* Create another GtkTable which contain a one GtkFrame per register */
void create_register_list(GtkWidget* debug_table, GLOBAL_SKIN_INFOS* gsi) 
{
	GtkWidget* debug_register_frame= gtk_frame_new("Registers");
	GtkWidget* debug_register_table= gtk_table_new(5, 8, FALSE);
	
	gtk_table_attach_defaults(GTK_TABLE(debug_table), debug_register_frame, 0, 1, 0, 7);
	gtk_container_add(GTK_CONTAINER(debug_register_frame), debug_register_table);
	
	char string[50]; /* Useful to get and format (snprintf) the registers */
	int i=0;
	int j=0;
	int n=0;

	/* Init gsi->reg_entry. This is a save of the register widget.It 's easier to refresh it from another function with it. */
	gsi->reg_entry=malloc(sizeof(TilemDebuggerRegister));
	gsi->isDebuggerRunning=TRUE;
	for(i=0; i<12; i++)
		gsi->reg_entry->reg[i]= gtk_entry_new();

	/* Create the register table */
	for(i=0; i<6; i++) {
		for(j=0; j<2; j++) {

			GtkWidget* frame= gtk_frame_new(rlabel[n]);
			GtkWidget* entry= gtk_entry_new();
			gsi->reg_entry->reg[n]=entry;
			gtk_widget_set_size_request(GTK_WIDGET(entry), 40,20);
			//snprintf(string, 5, "%04X", gsi->emu->calc->z80.r.af.d);
			getreg(gsi, n, string);
			gtk_entry_set_text(GTK_ENTRY(entry), string);
			gtk_container_add(GTK_CONTAINER(frame), entry);
			gtk_table_attach(GTK_TABLE(debug_register_table), frame, j, j+1, i, i+1, 0, 0, 0, 0);
			n++;
		}
	}
	DDEBUGGER_L0_A0("********************* fct: create_register_list ********\n");
	printstate(gsi->emu);
	DDEBUGGER_L0_A0("********************************************************\n");
}

/* Create the GtkTreeView to show the dasm_list */
void create_dasm_list(GtkWidget* debug_dasmscroll, GLOBAL_SKIN_INFOS* gsi) {
	
	GtkCellRenderer     *renderer;
	GtkTreeModel        *model;
	GtkWidget* debug_treeview;
	GtkTreeViewColumn *column;
	gsi=gsi;	
	
	debug_treeview= gtk_tree_view_new();

	/* Create the columns */
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_renderer_set_fixed_size(renderer, -1, 16);
	column = gtk_tree_view_column_new_with_attributes("ADDR" ,renderer, "text", COL_OFFSET_DASM, NULL);
	gtk_tree_view_column_set_expand(column, TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(debug_treeview), column);
	
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_renderer_set_fixed_size(renderer, -1, 16);
	column = gtk_tree_view_column_new_with_attributes("OP" ,renderer, "text", COL_OP_DASM, NULL);
	gtk_tree_view_column_set_expand(column, TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(debug_treeview), column);

	
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_renderer_set_fixed_size(renderer, -1, 16);
	column = gtk_tree_view_column_new_with_attributes("ARGS" ,renderer, "text", COL_ARGS_DASM, NULL);
	gtk_tree_view_column_set_expand(column, TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(debug_treeview), column);

	/*renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (debug_treeview), -1, "OFFSET", renderer, "text", COL_OFFSET_DASM, NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (debug_treeview), -1, "OP", renderer, "text", COL_OP_DASM, NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (debug_treeview), -1, "VALUE", renderer, "text", COL_ARGS_DASM, NULL);
	*/
	/* Get the list */
	model = fill_dasm_list ();

	/* Add the list */
	gtk_tree_view_set_model (GTK_TREE_VIEW (debug_treeview), model);

	gtk_container_add(GTK_CONTAINER(debug_dasmscroll), debug_treeview);

}

/* Create GtkListStore and attach it */
static GtkTreeModel* fill_dasm_list(void)
{
	GtkListStore  *store;
	GtkTreeIter    iter;
  
	store = gtk_list_store_new (NUM_COLS_DASM, G_TYPE_STRING, G_TYPE_STRING , G_TYPE_STRING);

	/* Append a row and fill in some data (here is just for debugging) */
	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter, COL_OFFSET_DASM, "0000", COL_OP_DASM, "INC", COL_ARGS_DASM, "$12", -1);
	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter, COL_OFFSET_DASM, "0002", COL_OP_DASM, "ADD", COL_ARGS_DASM, "$A3", -1);
	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter, COL_OFFSET_DASM, "0004", COL_OP_DASM, "DEC", COL_ARGS_DASM, "$11", -1);
	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter, COL_OFFSET_DASM, "0006", COL_OP_DASM, "CP", COL_ARGS_DASM, "$08", -1);
	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter, COL_OFFSET_DASM, "0008", COL_OP_DASM, "JP", COL_ARGS_DASM, "$FF", -1);
	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter, COL_OFFSET_DASM, "000A", COL_OP_DASM, "LD", COL_ARGS_DASM, "$FF", -1);
	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter, COL_OFFSET_DASM, "000C", COL_OP_DASM, "LD", COL_ARGS_DASM, "$FF", -1);
	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter, COL_OFFSET_DASM, "000E", COL_OP_DASM, "JNZ", COL_ARGS_DASM, "$FF", -1);
	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter, COL_OFFSET_DASM, "0010", COL_OP_DASM, "JNZ", COL_ARGS_DASM, "$FF", -1);
	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter, COL_OFFSET_DASM, "0012", COL_OP_DASM, "JNZ", COL_ARGS_DASM, "$FF", -1);
  
  
	return GTK_TREE_MODEL (store);
}

/* Create the GtkTreeView to show the stack_list */
void create_stack_list(GtkWidget* debug_stackscroll, GLOBAL_SKIN_INFOS* gsi) {
	
	GtkCellRenderer     *renderer;
	GtkTreeModel        *model;
	GtkWidget* 	debug_treeview;
	GtkTreeViewColumn *column;
	
	/* Create the stack list tree view and set title invisible */
	debug_treeview= gtk_tree_view_new();
	gsi->stack_treeview= debug_treeview;
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(debug_treeview), TRUE);	
	gtk_tree_view_set_search_column(GTK_TREE_VIEW(debug_treeview), COL_OP_DASM);

	/* Create the columns */
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_renderer_set_fixed_size(renderer, -1, 16);
	column = gtk_tree_view_column_new_with_attributes("ADDR" ,renderer, "text", COL_OFFSET_STK, NULL);
	gtk_tree_view_column_set_expand(column, TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(debug_treeview), column);
	
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_renderer_set_fixed_size(renderer, -1, 16);
	column = gtk_tree_view_column_new_with_attributes("VAL" ,renderer, "text", COL_VALUE_STK, NULL);
	gtk_tree_view_column_set_expand(column, TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(debug_treeview), column);

	/*renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (debug_treeview), -1, NULL, renderer, "text", COL_OFFSET_STK, NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (debug_treeview), -1, NULL, renderer, "text", COL_VALUE_STK, NULL);
	*/

	/* Get the list */
	model = fill_stk_list(gsi);

	/* Add the list */
	gtk_tree_view_set_model (GTK_TREE_VIEW (debug_treeview), model);

	gtk_container_add(GTK_CONTAINER(debug_stackscroll), debug_treeview);

}

void refresh_stack(GLOBAL_SKIN_INFOS * gsi)
{
	char* sp;
	sp= malloc(sizeof(char*));
	getreg(gsi, 10, sp);
	DDEBUGGER_L0_A0("******************** fct: refresh_stack ****************\n");
	DDEBUGGER_L0_A1("*  SP : %s                                           *\n", sp);
	DDEBUGGER_L0_A0("********************************************************\n");
	GtkTreeModel * model;
	model = fill_stk_list(gsi);
	gtk_tree_view_set_model (GTK_TREE_VIEW (gsi->stack_treeview), model);
}


/* Create GtkListStore and attach it */
static GtkTreeModel* fill_stk_list(GLOBAL_SKIN_INFOS* gsi)
{
	GtkListStore  *store;
	GtkTreeIter    iter;
	char* stack_offset;
	char* stack_value;
	stack_offset = (char*) malloc(sizeof(char*));
	stack_value = (char*) malloc(sizeof(char*));
 	gushort i=0; 
	dword phys;
	
	g_mutex_lock(gsi->emu->calc_mutex);
	store = gtk_list_store_new (NUM_COLS_STK, G_TYPE_STRING, G_TYPE_STRING);
	i = getreg_int(gsi, 10) + 1;
        while  (i > 0x0010) {
        	sprintf(stack_offset, "%04X:", i - 1);
		phys = gsi->emu->calc->hw.mem_ltop(gsi->emu->calc, i - 1);
                sprintf(stack_value, "%02X%02X", gsi->emu->calc->mem[phys + 1],gsi->emu->calc->mem[phys] );
		

		/* Append a row and fill in some data (here is just for debugging) */
		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter, COL_OFFSET_STK, stack_offset, COL_VALUE_STK, stack_value, -1);
 
                i += 0x0002;
      }
  
  
	g_mutex_unlock(gsi->emu->calc_mutex);
	return GTK_TREE_MODEL (store);
}

/* Create the GtkTreeView to show the memory_list */
void create_memory_list(GtkWidget* debug_memoryscroll, GLOBAL_SKIN_INFOS* gsi) {
	
	GtkCellRenderer     *renderer;
	GtkTreeModel        *model;
	GtkTreeViewColumn   *column;
	gsi=gsi;
	
	/* Create the memory list tree view and set title invisible */
	GtkWidget* debug_treeview= gtk_tree_view_new();
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(debug_treeview), FALSE);	
	gtk_tree_view_set_fixed_height_mode(GTK_TREE_VIEW(debug_treeview), TRUE);

	/* Create the columns */
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("ADDR",renderer, "text", MM_COL_ADDRESS(0), NULL);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width(column, 70);
	gtk_tree_view_column_set_expand(column, TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(debug_treeview), column);

	int i=0;
	for(i=0; i<8; i++)
	{
		renderer = gtk_cell_renderer_text_new ();
		column = gtk_tree_view_column_new_with_attributes(NULL ,renderer, "text", MM_COL_HEX(i), NULL);
		gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
		gtk_tree_view_column_set_fixed_width(column, 20);
		gtk_tree_view_column_set_expand(column, TRUE);
		gtk_tree_view_append_column(GTK_TREE_VIEW(debug_treeview), column);
	}	

	for (i = 0; i < 8; i++) {
		renderer = gtk_cell_renderer_text_new ();
		column = gtk_tree_view_column_new_with_attributes("ASCII",renderer, "text", MM_COL_CHAR(i), NULL);
		gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
		gtk_tree_view_column_set_fixed_width(column, 16);
		gtk_tree_view_column_set_expand(column, TRUE);
		gtk_tree_view_append_column(GTK_TREE_VIEW(debug_treeview), column);
	}
	

	/* Get the list */
	model = (GtkTreeModel*)tilem_mem_model_new(gsi->emu, 8, 0, TRUE);
	//model = fill_memory_list();

	/* Add the list */
	/* FIXME : THERE'S A BUG HERE ! 
	It works only with fill_memory_list, I don't know why??
	*/
	gtk_tree_view_set_model (GTK_TREE_VIEW (debug_treeview), (GtkTreeModel*)model);
	g_object_unref(model);
	
	gtk_container_add(GTK_CONTAINER(debug_memoryscroll), debug_treeview);
	gtk_tree_view_columns_autosize (GTK_TREE_VIEW(debug_treeview));


}

/* Create GtkListStore and attach it */
/*static GtkTreeModel* fill_memory_list(void)
{
	GtkListStore  *store;
	GtkTreeIter    iter;
  
	store = gtk_list_store_new (10, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
*/
	/* Append a row and fill in some data (here is just for debugging) */
/*	int i=0;
	for(i=0; i<30; i++)
	{
		gtk_list_store_append (store, &iter);
		gtk_list_store_set(store,&iter, 0, "0000", 1, "AE", 2, "EE", 3, "FF", 4, "EF", 5, "3F", 6, "FE", 7, "A1", 8, "2B", 9, ". . . . . .", -1);
	}
  
  
	return GTK_TREE_MODEL (store);
}
*/

