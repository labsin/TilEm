/*
 * TilEm II
 *
 * Copyright (c) 2010-2011 Thibault Duponchelle
 * Copyright (c) 2010-2011 Benjamin Moody
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
#include <gtk/gtk.h>
#include <ticalcs.h>
#include <tilem.h>
#include <tilemdb.h>

#include "gui.h"
#include "disasmview.h"
#include "memmodel.h"

/* Stack list */
enum
{
	COL_OFFSET_STK = 0,
	COL_VALUE_STK,
  	NUM_COLS_STK
};

/* Indices in reg_entries */
enum {
	R_AF, R_BC, R_DE, R_HL, R_IX, R_SP,
	R_AF2, R_BC2, R_DE2, R_HL2, R_IY, R_PC,
	R_IM, R_I,
	NUM_REGS
};

/* Labels for the entries */
static const char * const reg_labels[] = {
	"A_F:", "B_C:", "D_E:", "H_L:", "I_X:", "SP:",
	"AF':", "BC':", "DE':", "HL':", "I_Y:", "PC:",
	"IM:", "I:"
};

/* Labels for the flag buttons */
static const char flag_labels[][2] = {
	"C", "N", "P", "X", "H", "Y", "Z", "S"
};

/* Register edited */
static void reg_edited(GtkEntry *ent, gpointer data)
{
	/* FIXME */
}

/* Flag button toggled */
static void flag_edited(GtkToggleButton *btn, gpointer data)
{
	/* FIXME */
}

/* IM edited */
static void im_edited(GtkEntry *ent, gpointer data)
{
	/* FIXME */
}

/* IFF button toggled */
static void iff_edited(GtkToggleButton *btn, gpointer data)
{
	/* FIXME */
}

/* Main window received a "delete" message */
static gboolean delete_win(G_GNUC_UNUSED GtkWidget *w,
                           G_GNUC_UNUSED GdkEvent *ev,
                           gpointer data)
{
	TilemDebugger *dbg = data;
	tilem_debugger_hide(dbg);
	return TRUE;
}

/* Create table of widgets for editing registers */
static GtkWidget *create_registers(TilemDebugger *dbg)
{
	GtkWidget *vbox, *tbl, *lbl, *hbox, *ent, *btn;
	int i;

	vbox = gtk_vbox_new(FALSE, 6);

	tbl = gtk_table_new(6, 4, FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(tbl), 6);
	gtk_table_set_col_spacings(GTK_TABLE(tbl), 6);
	gtk_table_set_col_spacing(GTK_TABLE(tbl), 1, 12);

	for (i = 0; i < 12; i++) {
		lbl = gtk_label_new_with_mnemonic(reg_labels[i]);
		gtk_misc_set_alignment(GTK_MISC(lbl), LABEL_X_ALIGN, 0.5);
		gtk_table_attach(GTK_TABLE(tbl), lbl,
		                 2 * (i / 6), 2 * (i / 6) + 1,
		                 (i % 6), (i % 6) + 1,
		                 GTK_FILL, GTK_FILL, 0, 0);

		dbg->reg_entries[i] = ent = gtk_entry_new();
		gtk_entry_set_width_chars(GTK_ENTRY(ent), 5);
		g_signal_connect(ent, "changed", G_CALLBACK(reg_edited), dbg);
		gtk_table_attach(GTK_TABLE(tbl), ent,
		                 2 * (i / 6) + 1, 2 * (i / 6) + 2,
		                 (i % 6), (i % 6) + 1,
		                 GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);

		gtk_label_set_mnemonic_widget(GTK_LABEL(lbl), ent);
	}

	gtk_box_pack_start(GTK_BOX(vbox), tbl, FALSE, FALSE, 0);

	hbox = gtk_hbox_new(TRUE, 0);

	for (i = 7; i >= 0; i--) {
		btn = gtk_toggle_button_new_with_label(flag_labels[i]);
		dbg->flag_buttons[i] = btn;
		g_signal_connect(btn, "toggled", G_CALLBACK(flag_edited), dbg);
		gtk_box_pack_start(GTK_BOX(hbox), btn, TRUE, TRUE, 0);
	}

	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

	hbox = gtk_hbox_new(FALSE, 6);

	for (i = 12; i < 14; i++) {
		lbl = gtk_label_new(reg_labels[i]);
		gtk_box_pack_start(GTK_BOX(hbox), lbl, FALSE, FALSE, 0);

		dbg->reg_entries[i] = ent = gtk_entry_new();
		gtk_box_pack_start(GTK_BOX(hbox), ent, TRUE, TRUE, 0);
	}

	g_signal_connect(dbg->reg_entries[R_I], "changed",
	                 G_CALLBACK(reg_edited), dbg);
	g_signal_connect(dbg->reg_entries[R_IM], "changed",
	                 G_CALLBACK(im_edited), dbg);

	gtk_entry_set_width_chars(GTK_ENTRY(dbg->reg_entries[R_IM]), 2);
	gtk_entry_set_width_chars(GTK_ENTRY(dbg->reg_entries[R_I]), 3);

	dbg->iff_checkbox = btn = gtk_check_button_new_with_label("EI");
	g_signal_connect(btn, "toggled", G_CALLBACK(iff_edited), dbg);
	gtk_box_pack_start(GTK_BOX(hbox), btn, TRUE, TRUE, 0);

	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

	return vbox;
}

/* Create the GtkTreeView to show the stack */
static GtkWidget *create_stack_view()
{
	GtkCellRenderer   *renderer;
	GtkWidget         *treeview;
	GtkTreeViewColumn *column;
	
	/* Create the stack list tree view and set title invisible */
	treeview = gtk_tree_view_new();
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(treeview), TRUE);
	gtk_tree_view_set_fixed_height_mode(GTK_TREE_VIEW(treeview), TRUE);
	gtk_tree_view_set_search_column(GTK_TREE_VIEW(treeview),
	                                COL_VALUE_STK);

	/* Create the columns */
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes
		("ADDR", renderer, "text", COL_OFFSET_STK, NULL);

	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_expand(column, TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes
		("VAL", renderer, "text", COL_VALUE_STK, NULL);

	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_expand(column, TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

	return treeview;
}

/* Create the GtkTreeView to show the memory */
static GtkWidget *create_memory_view()
{
	GtkCellRenderer     *renderer;
	GtkTreeViewColumn   *column;
	GtkWidget           *treeview;
	int i;

	/* Create the memory list tree view and set title invisible */
	treeview = gtk_tree_view_new();
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(treeview), FALSE);
	gtk_tree_view_set_fixed_height_mode(GTK_TREE_VIEW(treeview), TRUE);

	/* Create the columns */
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes
		("ADDR", renderer, "text", MM_COL_ADDRESS(0), NULL);

	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width(column, 70);
	gtk_tree_view_column_set_expand(column, TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

	/* FIXME: column sizes should be determined by font */

	for (i = 0; i < 8; i++) {
		renderer = gtk_cell_renderer_text_new ();
		column = gtk_tree_view_column_new_with_attributes
			(NULL, renderer, "text", MM_COL_HEX(i), NULL);

		gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
		gtk_tree_view_column_set_fixed_width(column, 20);
		gtk_tree_view_column_set_expand(column, TRUE);
		gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
	}

	for (i = 0; i < 8; i++) {
		renderer = gtk_cell_renderer_text_new ();
		column = gtk_tree_view_column_new_with_attributes
			(NULL, renderer, "text", MM_COL_CHAR(i), NULL);

		gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
		gtk_tree_view_column_set_fixed_width(column, 16);
		gtk_tree_view_column_set_expand(column, TRUE);
		gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
	}
	
	return treeview;
}

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

/* Create a new TilemDebugger. */
TilemDebugger *tilem_debugger_new(TilemCalcEmulator *emu)
{
	TilemDebugger *dbg;
	GtkWidget *hbox, *vbox, *vpaned, *sw;

	g_return_val_if_fail(emu != NULL, NULL);

	dbg = g_slice_new0(TilemDebugger);
	dbg->emu = emu;
	dbg->dasm = tilem_disasm_new();

	dbg->mem_rowsize = 8;
	dbg->mem_start = 0;
	dbg->mem_logical = TRUE;

	dbg->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(dbg->window), "TilEm Debugger");
	gtk_window_set_role(GTK_WINDOW(dbg->window), "Debugger");

	g_signal_connect(dbg->window, "delete-event",
	                 G_CALLBACK(delete_win), dbg);

	hbox = gtk_hbox_new(FALSE, 6);

	vpaned = gtk_vpaned_new();

	/* Disassembly view */

	dbg->disasm_view = tilem_disasm_view_new(dbg);
	sw = new_scrolled_window(dbg->disasm_view);
	gtk_paned_pack1(GTK_PANED(vpaned), sw, TRUE, TRUE);

	/* Memory view */

	dbg->mem_view = create_memory_view();
	sw = new_scrolled_window(dbg->mem_view);
	gtk_paned_pack2(GTK_PANED(vpaned), sw, TRUE, TRUE);

	gtk_box_pack_start(GTK_BOX(hbox), vpaned, TRUE, TRUE, 0);

	vbox = gtk_vbox_new(FALSE, 6);

	/* Registers */

	dbg->regbox = create_registers(dbg);
	gtk_box_pack_start(GTK_BOX(vbox), dbg->regbox, FALSE, FALSE, 0);

	/* Stack view */

	dbg->stack_view = create_stack_view();
	sw = new_scrolled_window(dbg->stack_view);
	gtk_box_pack_start(GTK_BOX(vbox), sw, TRUE, TRUE, 0);

	gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);
	gtk_widget_show_all(hbox);

	gtk_container_add(GTK_CONTAINER(dbg->window), hbox);

	tilem_debugger_calc_changed(dbg);

	return dbg;
}

/* Free a TilemDebugger. */
void tilem_debugger_free(TilemDebugger *dbg)
{
	g_return_if_fail(dbg != NULL);

	if (dbg->emu && dbg->emu->dbg == dbg)
		dbg->emu->dbg = NULL;
	if (dbg->window)
		gtk_widget_destroy(dbg->window);
	if (dbg->dasm)
		tilem_disasm_free(dbg->dasm);

	g_slice_free(TilemDebugger, dbg);
}

static void entry_printf(GtkWidget *ent, const char *s, ...)
{
	char buf[20];
	va_list ap;
	va_start(ap, s);
	g_vsnprintf(buf, sizeof(buf), s, ap);
	va_end(ap);
	gtk_entry_set_text(GTK_ENTRY(ent), buf);
}

static void refresh_regs(TilemDebugger *dbg)
{
	TilemCalc *calc = dbg->emu->calc;
	int i;
	GtkToggleButton *btn;

	entry_printf(dbg->reg_entries[R_AF], "%04X", calc->z80.r.af.w.l);
	entry_printf(dbg->reg_entries[R_BC], "%04X", calc->z80.r.bc.w.l);
	entry_printf(dbg->reg_entries[R_DE], "%04X", calc->z80.r.de.w.l);
	entry_printf(dbg->reg_entries[R_HL], "%04X", calc->z80.r.hl.w.l);
	entry_printf(dbg->reg_entries[R_AF2], "%04X", calc->z80.r.af2.w.l);
	entry_printf(dbg->reg_entries[R_BC2], "%04X", calc->z80.r.bc2.w.l);
	entry_printf(dbg->reg_entries[R_DE2], "%04X", calc->z80.r.de2.w.l);
	entry_printf(dbg->reg_entries[R_HL2], "%04X", calc->z80.r.hl2.w.l);
	entry_printf(dbg->reg_entries[R_SP], "%04X", calc->z80.r.sp.w.l);
	entry_printf(dbg->reg_entries[R_PC], "%04X", calc->z80.r.pc.w.l);
	entry_printf(dbg->reg_entries[R_IX], "%04X", calc->z80.r.ix.w.l);
	entry_printf(dbg->reg_entries[R_IY], "%04X", calc->z80.r.iy.w.l);
	entry_printf(dbg->reg_entries[R_I], "%02X", calc->z80.r.ir.b.h);
	entry_printf(dbg->reg_entries[R_IM], "%d", calc->z80.r.im);

	for (i = 0; i < 8; i++) {
		btn = GTK_TOGGLE_BUTTON(dbg->flag_buttons[i]);
		gtk_toggle_button_set_active(btn, calc->z80.r.af.d & (1 << i));
	}

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dbg->iff_checkbox),
	                             calc->z80.r.iff1);
}

/* Create GtkListStore and attach it */
static GtkTreeModel* fill_stk_list(TilemDebugger *dbg)
{
	GtkListStore  *store;
	GtkTreeIter    iter;
	char stack_offset[10];
	char stack_value[10];
 	gushort i=0; 
	dword phys;

	store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
	i = dbg->emu->calc->z80.r.sp.w.l;
        while  (i > 0x0010) {
	        g_snprintf(stack_offset, sizeof(stack_offset), "%04X:", i - 1);
		phys = dbg->emu->calc->hw.mem_ltop(dbg->emu->calc, i - 1);
		g_snprintf(stack_value, sizeof(stack_value), "%02X%02X", dbg->emu->calc->mem[phys + 1],dbg->emu->calc->mem[phys] );

		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter, COL_OFFSET_STK, stack_offset, COL_VALUE_STK, stack_value, -1);
 
                i += 0x0002;
        }
    
	return GTK_TREE_MODEL (store);
}

static void refresh_stack(TilemDebugger *dbg)
{
	GtkTreeModel *model = fill_stk_list(dbg);
	gtk_tree_view_set_model(GTK_TREE_VIEW(dbg->stack_view), model);
	g_object_unref(model);
}

static void refresh_all(TilemDebugger *dbg, gboolean updatemem)
{
	TilemCalc *calc;
	gboolean paused;

	dbg->refreshing = TRUE;

	g_mutex_lock(dbg->emu->calc_mutex);
	calc = dbg->emu->calc;
	paused = dbg->emu->paused;

	if (calc) {
		refresh_regs(dbg);

		if (dbg->lastwrite != calc->z80.lastwrite)
			updatemem = TRUE;

		if (updatemem || dbg->lastsp != calc->z80.r.sp.d)
			refresh_stack(dbg);

		dbg->lastwrite = calc->z80.lastwrite;
		dbg->lastsp = calc->z80.r.sp.d;
	}

	g_mutex_unlock(dbg->emu->calc_mutex);

	gtk_widget_queue_draw(dbg->mem_view);

	if (updatemem)
		tilem_disasm_view_refresh(TILEM_DISASM_VIEW(dbg->disasm_view));

	if (paused != dbg->paused) {
		dbg->paused = paused;
		gtk_widget_set_sensitive(dbg->regbox, paused);
		gtk_widget_set_sensitive(dbg->disasm_view, paused);
		gtk_widget_set_sensitive(dbg->mem_view, paused);
		gtk_widget_set_sensitive(dbg->stack_view, paused);
	}

	dbg->refreshing = FALSE;
}

/* Show debugger, and pause emulator if not already paused. */
void tilem_debugger_show(TilemDebugger *dbg)
{
	g_return_if_fail(dbg != NULL);
	g_return_if_fail(dbg->emu->calc != NULL);
	tilem_calc_emulator_pause(dbg->emu);
	tilem_disasm_view_go_to_address(TILEM_DISASM_VIEW(dbg->disasm_view),
	                                dbg->emu->calc->z80.r.pc.d);
	refresh_all(dbg, TRUE);
	gtk_window_present(GTK_WINDOW(dbg->window));
}

/* Hide debugger, and resume emulation if not already running. */
void tilem_debugger_hide(TilemDebugger *dbg)
{
	g_return_if_fail(dbg != NULL);
	gtk_widget_hide(dbg->window);
	tilem_calc_emulator_run(dbg->emu);
}

/* New calculator loaded. */
void tilem_debugger_calc_changed(TilemDebugger *dbg)
{
	TilemCalc *calc;
	GtkTreeModel *model;

	g_return_if_fail(dbg != NULL);

	tilem_disasm_free(dbg->dasm);
	dbg->dasm = tilem_disasm_new();

	calc = dbg->emu->calc;
	if (!calc)
		return;

	model = tilem_mem_model_new(dbg->emu, dbg->mem_rowsize,
	                            dbg->mem_start, dbg->mem_logical);
	gtk_tree_view_set_model(GTK_TREE_VIEW(dbg->mem_view), model);
	g_object_unref(model);

	tilem_debugger_refresh(dbg, TRUE);
}

/* Update display. */
void tilem_debugger_refresh(TilemDebugger *dbg, gboolean updatemem)
{
	g_return_if_fail(dbg != NULL);

	if (!GTK_WIDGET_VISIBLE(dbg->window))
		return;

	refresh_all(dbg, updatemem);
}

