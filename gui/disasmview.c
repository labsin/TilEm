/*
 * TilEm II
 *
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
#include <string.h>
#include <gtk/gtk.h>
#include <ticalcs.h>
#include <tilem.h>
#include <tilemdb.h>

#include "gui.h"
#include "disasmview.h"

G_DEFINE_TYPE(TilemDisasmView, tilem_disasm_view, GTK_TYPE_TREE_VIEW);

/*
  This is a HORRIBLE kludge.  Don't ever do anything like this.  ;)

  We want a widget that has the look and feel of a GtkTreeView.  But
  our "data model" isn't consistent with a GtkTreeModel, since it
  changes depending on where we are.

  This widget keeps track of how high each row will be once rendered,
  and uses that to construct a GtkListStore with the appropriate
  number of rows to fill the window.  We also override the move-cursor
  signal so that we can handle the boundaries.
 */

/* Model columns */
enum {
	COL_POSITION,
	COL_ADDRESS,
	COL_MNEMONIC,
	COL_ARGUMENTS,
	COL_SHOW_MNEMONIC,
	NUM_COLUMNS
};

static GtkTreeViewClass *parent_class;

/* We define two "positions" for each actual address; the second is
   used if there's a label to be displayed at that address. */

#define POS_TO_ADDR(x) ((x) >> 1)
#define ADDR_TO_POS(x) ((x) << 1)

/* Disassembly */

/* Convert physical to logical address; if address is not currently
   mapped, use the bank-A address */
static dword default_ptol(TilemDisasmView *dv, dword addr)
{
	TilemCalc *calc = dv->dbg->emu->calc;
	dword addr_l;

	g_return_val_if_fail(calc != NULL, 0);

	addr_l = (*calc->hw.mem_ptol)(calc, addr);
	if (addr_l == 0xffffffff)
		addr_l = (addr & 0x3fff) | 0x4000;

	return addr_l;
}

/* Check for a label at the given address (physical or logical
   depending on the mode of the DisasmView) */
static const char *get_label(TilemDisasmView *dv, dword addr)
{
	g_return_val_if_fail(dv->dbg->dasm != NULL, NULL);
	if (!dv->use_logical)
		addr = default_ptol(dv, addr);
	return tilem_disasm_get_label_at_address(dv->dbg->dasm, addr);
}

/* Disassemble a line */
static void disassemble(TilemDisasmView *dv, dword pos,
                        dword *nextpos, char **mnemonic, char **args)
{
	TilemCalc *calc = dv->dbg->emu->calc;
	dword addr = POS_TO_ADDR(pos);
	const char *lbl;
	char buf[500], *p;

	g_return_if_fail(calc != NULL);
	g_return_if_fail(dv->dbg->dasm != NULL);

	if (!(pos & 1) && (lbl = get_label(dv, addr))) {
		if (mnemonic) {
			*mnemonic = NULL;
			*args = g_strdup_printf("%s:", lbl);
		}

		if (nextpos)
			*nextpos = pos + 1;
	}
	else if (mnemonic) {
		tilem_disasm_disassemble(dv->dbg->dasm, calc,
		                         !dv->use_logical, addr,
		                         &addr, buf, sizeof(buf));

		p = strchr(buf, '\t');
		if (p) {
			*mnemonic = g_strndup(buf, p - buf);
			*args = g_strdup(p + 1);
		}
		else {
			*mnemonic = g_strdup(buf);
			*args = NULL;
		}

		if (nextpos)
			*nextpos = ADDR_TO_POS(addr);
	}
	else {
		tilem_disasm_disassemble(dv->dbg->dasm, calc,
		                         !dv->use_logical, addr,
		                         &addr, NULL, 0);
		if (nextpos)
			*nextpos = ADDR_TO_POS(addr);
	}
}

/* Get "next" position */
static dword get_next_pos(TilemDisasmView *dv, dword pos)
{
	disassemble(dv, pos, &pos, NULL, NULL);
	return pos;
}

/* Get "previous" position */
static dword get_prev_pos(TilemDisasmView *dv, dword pos)
{
	dword addr = POS_TO_ADDR(pos);
	TilemCalc *calc = dv->dbg->emu->calc;

	g_return_val_if_fail(calc != NULL, 0);

	if (pos & 1) {
		return pos - 1;
	}
	else {
		if (addr > 0)
			addr--;
		else if (dv->use_logical)
			addr = 0xffff;
		else
			addr = (calc->hw.romsize + calc->hw.ramsize - 1);

		if (get_label(dv, addr))
			return ADDR_TO_POS(addr) + 1;
		else
			return ADDR_TO_POS(addr);
	}
}

/* Convert physical to logical position */
static dword pos_ptol(TilemDisasmView *dv, dword pos)
{
	dword addr;

	if (pos == (dword) -1)
		return pos;

	addr = default_ptol(dv, POS_TO_ADDR(pos));
	return ADDR_TO_POS(addr) + (pos & 1);
}

/* Convert logical to physical position */
static dword pos_ltop(TilemDisasmView *dv, dword pos)
{
	TilemCalc *calc = dv->dbg->emu->calc;
	dword addr;

	g_return_val_if_fail(calc != NULL, 0);

	if (pos == (dword) -1)
		return pos;

	addr = (*calc->hw.mem_ltop)(calc, POS_TO_ADDR(pos));
	return ADDR_TO_POS(addr) + (pos & 1);
}

/* List model management */

/* Create a new list store for disassembly */
static GtkTreeModel * new_dasm_model()
{
	GtkListStore *store;

	g_assert(NUM_COLUMNS == 5);
	store = gtk_list_store_new(5,
	                           G_TYPE_INT,
	                           G_TYPE_STRING,
	                           G_TYPE_STRING,
	                           G_TYPE_STRING,
	                           G_TYPE_BOOLEAN);

	return GTK_TREE_MODEL(store);
}

/* Append dummy data to the model; used for sizing */
static void append_dummy_line(GtkTreeModel *model, GtkTreeIter *iter)
{
	GtkTreeIter iter1;

	gtk_list_store_append(GTK_LIST_STORE(model), &iter1);

	gtk_list_store_set(GTK_LIST_STORE(model), &iter1,
	                   COL_ADDRESS, "DD:DDDD",
	                   COL_MNEMONIC, "ROM_CALL",
	                   COL_ARGUMENTS, "_fnord",
	                   COL_SHOW_MNEMONIC, TRUE,
	                   -1);
	if (iter)
		*iter = iter1;
}

/* Append a line to the dasm model */
static void append_dasm_line(TilemDisasmView *dv, GtkTreeModel *model,
                             GtkTreeIter *iter, dword pos, dword *nextpos)
{
	TilemCalc *calc = dv->dbg->emu->calc;
	GtkTreeIter iter1;
	char abuf[20], *mnem, *args;
	dword addr, page;

	g_return_if_fail(calc != NULL);

	gtk_list_store_append(GTK_LIST_STORE(model), &iter1);

	addr = POS_TO_ADDR(pos);
	if (dv->use_logical) {
		g_snprintf(abuf, sizeof(abuf), "%04X", addr);
	}
	else {
		if (addr >= calc->hw.romsize) {
			page = (((addr - calc->hw.romsize) >> 14)
			        + calc->hw.rampagemask);
		}
		else {
			page = addr >> 14;
		}
		g_snprintf(abuf, sizeof(abuf), "%02X:%04X",
		           page, default_ptol(dv, addr));
	}

	disassemble(dv, pos, nextpos, &mnem, &args);

	gtk_list_store_set(GTK_LIST_STORE(model), &iter1,
	                   COL_POSITION, (int) pos,
	                   COL_ADDRESS, abuf,
	                   COL_MNEMONIC, mnem,
	                   COL_SHOW_MNEMONIC, (mnem ? TRUE : FALSE),
	                   COL_ARGUMENTS, args,
	                   -1);

	g_free(mnem);
	g_free(args);

	if (iter)
		*iter = iter1;
}

/* Refresh the view by creating and populating a new model */
static void refresh_disassembly(TilemDisasmView *dv, dword pos, int nlines,
                                dword selectpos)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreePath *selectpath = NULL;
	dword nextpos;
	int i;

	model = new_dasm_model();

	if (!dv->dbg->emu->calc) {
		gtk_tree_view_set_model(GTK_TREE_VIEW(dv), model);
		g_object_unref(model);
		return;
	}

	dv->startpos = pos;

	for (i = 0; i < nlines; i++) {
		append_dasm_line(dv, model, &iter, pos, &nextpos);

		if (pos == selectpos)
			selectpath = gtk_tree_model_get_path(model, &iter);

		pos = nextpos;
	}

	dv->endpos = pos;
	dv->nlines = nlines;

	gtk_tree_view_set_model(GTK_TREE_VIEW(dv), model);
	g_object_unref(model);

	if (selectpath) {
		gtk_tree_view_set_cursor(GTK_TREE_VIEW(dv), selectpath,
		                         NULL, FALSE);
		gtk_tree_path_free(selectpath);
	}
}

/* Determine the (absolute) position and (display-relative) line
   number of the cursor, if any */
static gboolean get_cursor_line(TilemDisasmView *dv, dword *pos,
                                int *linenum)
{
	GtkTreePath *path;
	GtkTreeModel *model;
	GtkTreeIter iter;
	gint *i, p;

	gtk_tree_view_get_cursor(GTK_TREE_VIEW(dv), &path, NULL);
	if (!path) {
		if (pos) *pos = (dword) -1;
		if (linenum) *linenum = -1;
		return FALSE;
	}

	if (pos) {
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(dv));
		if (gtk_tree_model_get_iter(model, &iter, path)) {
			gtk_tree_model_get(model, &iter,
			                   COL_POSITION, &p, -1);	
			*pos = p;
		}
		else {
			*pos = (dword) -1;
		}
	}

	if (linenum) {
		i = gtk_tree_path_get_indices(path);
		*linenum = i[0];
	}

	gtk_tree_path_free(path);

	return TRUE;
}

/* Size allocation */

/* Get the desired height for the tree view (based on size of the data
   we've inserted into the model) */
static int get_parent_request_height(GtkWidget *w)
{
	GtkRequisition req;
	(*GTK_WIDGET_CLASS(parent_class)->size_request)(w, &req);
	return req.height;
}

/* Widget is assigned a size and position */
static void tilem_disasm_view_size_allocate(GtkWidget *w,
                                            GtkAllocation *alloc)
{
	TilemDisasmView *dv;
	GtkTreeModel *model;
	dword curpos;
	int n, height1, height2;

	g_return_if_fail(TILEM_IS_DISASM_VIEW(w));
	dv = TILEM_DISASM_VIEW(w);

	(*GTK_WIDGET_CLASS(parent_class)->size_allocate)(w, alloc);

	if (alloc->height < 1)
		return;

	get_cursor_line(dv, &curpos, NULL);

	/* Calculate line height */
	if (!dv->line_height) {
		model = new_dasm_model();

		append_dummy_line(model, NULL);
		gtk_tree_view_set_model(GTK_TREE_VIEW(dv), model);
		height1 = get_parent_request_height(w);

		append_dummy_line(model, NULL);
		height2 = get_parent_request_height(w);

		dv->line_height = height2 - height1;
		dv->base_height = height1 - dv->line_height;

		g_object_unref(model);

		dv->nlines = 0;

		if (dv->line_height <= 0) {
			dv->line_height = 0;
			return;
		}
	}

	n = (alloc->height - dv->base_height) / dv->line_height;

	if (n < 1)
		n = 1;

	if (n != dv->nlines)
		refresh_disassembly(dv, dv->startpos, n, curpos);
}

/* Get widget's desired size */
static void tilem_disasm_view_size_request(GtkWidget *w, GtkRequisition *req)
{
	(*GTK_WIDGET_CLASS(parent_class)->size_request)(w, req);
	req->height = 100;	/* ignore requested height */
}

/* Widget style set */
static void tilem_disasm_view_style_set(GtkWidget *w, GtkStyle *oldstyle)
{
	TilemDisasmView *dv;
	GtkTreeModel *model;
	GtkTreeIter iter;
	GList *cols, *cp;
	GtkTreeViewColumn *col;
	int width;

	g_return_if_fail(TILEM_IS_DISASM_VIEW(w));
	dv = TILEM_DISASM_VIEW(w);

	(*GTK_WIDGET_CLASS(parent_class)->style_set)(w, oldstyle);

	/* line height must be recalculated */
	dv->line_height = 0;

	/* set column widths based on a dummy model */

	model = new_dasm_model();
	append_dummy_line(model, &iter);

	cols = gtk_tree_view_get_columns(GTK_TREE_VIEW(dv));
	for (cp = cols; cp; cp = cp->next) {
		col = cp->data;
		gtk_tree_view_column_cell_set_cell_data(col, model, &iter,
		                                        FALSE, FALSE);
		gtk_tree_view_column_cell_get_size(col, NULL, NULL, NULL,
		                                   &width, NULL);
		gtk_tree_view_column_set_fixed_width(col, width + 10);
	}
	g_list_free(cols);

	g_object_unref(model);
}

/* Cursor movement commands */

/* Move up by COUNT lines */
static gboolean move_up_lines(TilemDisasmView *dv, int count)
{
	dword pos;
	int linenum;

	if (!get_cursor_line(dv, NULL, &linenum))
		linenum = 0;

	if (linenum >= count)
		return FALSE;

	pos = dv->startpos;
	count -= linenum;
	while (count > 0) {
		pos = get_prev_pos(dv, pos);
		count--;
	}

	refresh_disassembly(dv, pos, dv->nlines, pos);
	return TRUE;
}

/* Move down by COUNT lines */
static gboolean move_down_lines(TilemDisasmView *dv, int count)
{
	dword startpos, selpos;
	int linenum;

	if (!get_cursor_line(dv, NULL, &linenum))
		linenum = -1;

	if (linenum + count < dv->nlines)
		return FALSE;

	startpos = get_next_pos(dv, dv->startpos);
	selpos = dv->endpos;
	count -= dv->nlines - linenum;

	while (count > 0) {
		startpos = get_next_pos(dv, startpos);
		selpos = get_next_pos(dv, selpos);
		count--;
	}

	refresh_disassembly(dv, startpos, dv->nlines, selpos);
	return TRUE;
}

/* Move down by COUNT bytes */
static void move_bytes(TilemDisasmView *dv, int count)
{
	dword pos, addr;
	TilemCalc *calc = dv->dbg->emu->calc;

	g_return_if_fail(calc != NULL);

	if (!get_cursor_line(dv, &pos, NULL))
		pos = dv->startpos;

	addr = POS_TO_ADDR(pos);

	if (dv->use_logical)
		addr = (addr + count) & 0xffff;
	else {
		addr += calc->hw.romsize + calc->hw.ramsize + count;
		addr %= calc->hw.romsize + calc->hw.ramsize;
	}

	pos = ADDR_TO_POS(addr);
	refresh_disassembly(dv, pos, dv->nlines, pos - 1);
}

/* Move the cursor (action signal) */
static gboolean tilem_disasm_view_move_cursor(GtkTreeView *tv,
                                              GtkMovementStep step,
                                              gint count)
{
	TilemDisasmView *dv;

	g_return_val_if_fail(TILEM_IS_DISASM_VIEW(tv), FALSE);
	dv = TILEM_DISASM_VIEW(tv);

	if (!dv->dbg->emu->calc)
		return FALSE;

	switch (step) {
	case GTK_MOVEMENT_DISPLAY_LINES:
		if (count < 0) {
			if (move_up_lines(dv, -count))
				return TRUE;
		}
		else {
			if (move_down_lines(dv, count))
				return TRUE;
		}
		break;

	case GTK_MOVEMENT_PARAGRAPHS:
	case GTK_MOVEMENT_PARAGRAPH_ENDS:
	case GTK_MOVEMENT_PAGES:
		/* FIXME: might be better to move by actual "pages" of code */
		move_bytes(dv, count * 0x100);
		return TRUE;

	case GTK_MOVEMENT_BUFFER_ENDS:
		move_bytes(dv, count * 0x4000);
		return TRUE;

	case GTK_MOVEMENT_LOGICAL_POSITIONS:
	case GTK_MOVEMENT_VISUAL_POSITIONS:
	case GTK_MOVEMENT_WORDS:
	case GTK_MOVEMENT_DISPLAY_LINE_ENDS:
	case GTK_MOVEMENT_HORIZONTAL_PAGES:
	default:
		break;
	}

	return (*GTK_TREE_VIEW_CLASS(parent_class)->move_cursor)(tv, step, count);
}

/* Initialize a new TilemDisasmView */
static void tilem_disasm_view_init(TilemDisasmView *dv)
{
	GtkTreeView *tv = GTK_TREE_VIEW(dv);
	GtkCellRenderer *cell;
	GtkTreeViewColumn *col;

	dv->use_logical = TRUE;

	gtk_tree_view_set_enable_search(tv, FALSE);
	gtk_tree_view_set_fixed_height_mode(tv, TRUE);
	gtk_tree_view_set_headers_visible(tv, FALSE);

	cell = gtk_cell_renderer_text_new();
	col = gtk_tree_view_column_new_with_attributes("Addr", cell,
	                                               "text", COL_ADDRESS,
	                                               NULL);
	gtk_tree_view_column_set_sizing(col, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_append_column(tv, col);

	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, "Disassembly");

	cell = gtk_cell_renderer_text_new();
	g_object_set(cell, "xpad", 10, NULL);
	gtk_tree_view_column_pack_start(col, cell, FALSE);
	gtk_tree_view_column_set_attributes(col, cell,
	                                    "text", COL_MNEMONIC,
	                                    "visible", COL_SHOW_MNEMONIC,
	                                    NULL);

	cell = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(col, cell, TRUE);
	gtk_tree_view_column_set_attributes(col, cell,
	                                    "text", COL_ARGUMENTS,
	                                    NULL);

	gtk_tree_view_column_set_sizing(col, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_expand(col, TRUE);
	gtk_tree_view_append_column(tv, col);
}

static const char default_style[] =
	"style \"tilem-disasm-default\" { font_name = \"Monospace\" } "
	"widget \"*.TilemDisasmView\" style:application \"tilem-disasm-default\"";

/* Initialize the TilemDisasmView class */
static void tilem_disasm_view_class_init(TilemDisasmViewClass *klass)
{
	GtkTreeViewClass *tv_class = GTK_TREE_VIEW_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	gtk_rc_parse_string(default_style);

	parent_class = g_type_class_peek_parent(klass);

	widget_class->style_set = &tilem_disasm_view_style_set;
	widget_class->size_request = &tilem_disasm_view_size_request;
	widget_class->size_allocate = &tilem_disasm_view_size_allocate;
	tv_class->move_cursor = &tilem_disasm_view_move_cursor;
}

GtkWidget * tilem_disasm_view_new(TilemDebugger *dbg)
{
	TilemDisasmView *dv;

	g_return_val_if_fail(dbg != NULL, NULL);

	dv = g_object_new(TILEM_TYPE_DISASM_VIEW, NULL);
	dv->dbg = dbg;

	return GTK_WIDGET(dv);
}

/* Select memory addressing mode. */
void tilem_disasm_view_set_logical(TilemDisasmView *dv, gboolean logical)
{
	dword start, curpos;

	g_return_if_fail(TILEM_IS_DISASM_VIEW(dv));

	get_cursor_line(dv, &curpos, NULL);

	if (logical && !dv->use_logical) {
		curpos = pos_ptol(dv, curpos);
		start = pos_ptol(dv, dv->startpos);

		dv->use_logical = TRUE;
		refresh_disassembly(dv, start, dv->nlines, curpos);
	}
	else if (!logical && dv->use_logical) {
		curpos = pos_ltop(dv, curpos);
		start = pos_ltop(dv, dv->startpos);

		dv->use_logical = FALSE;
		refresh_disassembly(dv, start, dv->nlines, curpos);
	}
}

/* Refresh contents of view. */
void tilem_disasm_view_refresh(TilemDisasmView *dv)
{
	dword curpos;
	g_return_if_fail(TILEM_IS_DISASM_VIEW(dv));
	get_cursor_line(dv, &curpos, NULL);
	refresh_disassembly(dv, dv->startpos, dv->nlines, curpos);
}

/* Find tree path for the given position */
static GtkTreePath *find_path_for_position(GtkTreeModel *model, int pos)
{
	gint p;
	GtkTreeIter iter;

	if (!gtk_tree_model_get_iter_first(model, &iter))
		return NULL;

	do {
		gtk_tree_model_get(model, &iter, COL_POSITION, &p, -1);
		if (p == pos) {
			return gtk_tree_model_get_path(model, &iter);
		}
	} while (gtk_tree_model_iter_next(model, &iter));

	return NULL;
}

/* Highlight the specified Z80 address. */
void tilem_disasm_view_go_to_address(TilemDisasmView *dv, dword addr)
{
	dword pos;
	GtkTreeModel *model;
	GtkTreePath *path;

	g_return_if_fail(TILEM_IS_DISASM_VIEW(dv));

	addr &= 0xffff;
	if (dv->use_logical)
		pos = ADDR_TO_POS(addr);
	else
		pos = pos_ltop(dv, ADDR_TO_POS(addr));

	if (pos >= dv->startpos && pos < dv->endpos) {
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(dv));
		path = find_path_for_position(model, pos);
		if (path) {
			gtk_tree_view_set_cursor(GTK_TREE_VIEW(dv), path,
			                         NULL, FALSE);
			gtk_tree_path_free(path);
			return;
		}
	}

	refresh_disassembly(dv, pos, dv->nlines, pos);
}
