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

#include "gui.h"
#include "memmodel.h"

/* Get flags for the model */
static GtkTreeModelFlags
tilem_mem_model_get_flags(G_GNUC_UNUSED GtkTreeModel *model)
{
	return (GTK_TREE_MODEL_LIST_ONLY | GTK_TREE_MODEL_ITERS_PERSIST);
}

/* Get the number of columns */
static int
tilem_mem_model_get_n_columns(GtkTreeModel *model)
{
	TilemMemModel *mm;
	g_return_val_if_fail(TILEM_IS_MEM_MODEL(model), 0);
	mm = TILEM_MEM_MODEL(model);
	return (MM_COLUMNS_PER_BYTE * mm->row_size);
}

/* Get type of data for the given column.  Currently all columns are
   strings. */
static GType
tilem_mem_model_get_column_type(G_GNUC_UNUSED GtkTreeModel *model,
                                G_GNUC_UNUSED int index)
{
	return G_TYPE_STRING;
}

/* Get an iterator pointing to the nth row */
static gboolean get_nth_iter(GtkTreeModel *model, GtkTreeIter *iter, int n)
{
	TilemMemModel *mm;

	g_return_val_if_fail(TILEM_IS_MEM_MODEL(model), FALSE);
	mm = TILEM_MEM_MODEL(model);

	if (n >= mm->num_rows)
		return FALSE;

	iter->stamp = mm->stamp;
	iter->user_data = GINT_TO_POINTER(n);
	iter->user_data2 = NULL;
	iter->user_data3 = NULL;
	return TRUE;
}

/* Get row number for the given iterator */
static int get_row_number(GtkTreeModel *model, GtkTreeIter *iter)
{
	TilemMemModel *mm;
	int n;

	g_return_val_if_fail(TILEM_IS_MEM_MODEL(model), 0);
	mm = TILEM_MEM_MODEL(model);
	g_return_val_if_fail(iter != NULL, 0);
	g_return_val_if_fail(iter->stamp == mm->stamp, 0);
	n = GPOINTER_TO_INT(iter->user_data);
	g_return_val_if_fail(n < mm->num_rows, 0);
	return n;
}

/* Get iterator for a given path */
static gboolean tilem_mem_model_get_iter(GtkTreeModel *model,
                                         GtkTreeIter *iter,
                                         GtkTreePath *path)
{
	int *indices;

	if (gtk_tree_path_get_depth(path) != 1)
		return FALSE;

	indices = gtk_tree_path_get_indices(path);
	return get_nth_iter(model, iter, indices[0]);
}

/* Get path for an iterator */
static GtkTreePath * tilem_mem_model_get_path(GtkTreeModel *model,
                                              GtkTreeIter *iter)
{
	int n;
	n = get_row_number(model, iter);
	return gtk_tree_path_new_from_indices(n, -1);
}

/* Get next (sibling) iterator */
static gboolean tilem_mem_model_iter_next(GtkTreeModel *model,
                                          GtkTreeIter *iter)
{
	int n;
	n = get_row_number(model, iter);
	return get_nth_iter(model, iter, n + 1);
}

/* Check if iterator has a child */
static gboolean
tilem_mem_model_iter_has_child(G_GNUC_UNUSED GtkTreeModel *model,
			       G_GNUC_UNUSED GtkTreeIter *iter)
{
	return FALSE;
}

/* Get number of children (iter = NULL means get number of root
   nodes) */
static gint tilem_mem_model_iter_n_children(GtkTreeModel *model,
                                            GtkTreeIter *iter)
{
	TilemMemModel *mm;

	g_return_val_if_fail(TILEM_IS_MEM_MODEL(model), 0);
	mm = TILEM_MEM_MODEL(model);

	if (iter)
		return 0;
	else
		return (mm->num_rows);
}

/* Get nth child (parent = NULL means get nth root node */
static gboolean tilem_mem_model_iter_nth_child(GtkTreeModel *model,
                                               GtkTreeIter *iter,
                                               GtkTreeIter *parent,
                                               gint n)
{
	TilemMemModel* mm;

	g_return_val_if_fail(TILEM_IS_MEM_MODEL(model), FALSE);
	mm = TILEM_MEM_MODEL(model);

	if (parent)
		return FALSE;
	else
		return get_nth_iter(model, iter, n);
}

/* Get first child */
static gboolean tilem_mem_model_iter_children(GtkTreeModel *model,
                                              GtkTreeIter *iter,
                                              GtkTreeIter *parent)
{
	return tilem_mem_model_iter_nth_child(model, iter, parent, 0);
}

/* Get parent */
static gboolean tilem_mem_model_iter_parent(G_GNUC_UNUSED GtkTreeModel *model,
                                            G_GNUC_UNUSED GtkTreeIter *iter,
                                            G_GNUC_UNUSED GtkTreeIter *child)
{
	return FALSE;
}

/* Retrieve value for a given column */
static void tilem_mem_model_get_value(GtkTreeModel *model,
                                      GtkTreeIter *iter,
                                      gint column,
                                      GValue *value)
{
	TilemMemModel *mm;
	dword n, addr, phys;
	TilemCalc *calc;
	char buf[100];
	unsigned int page;

	g_return_if_fail(TILEM_IS_MEM_MODEL(model));
	mm = TILEM_MEM_MODEL(model);

	g_return_if_fail(mm->emu != NULL);
	
	n = get_row_number(model, iter);

	g_mutex_lock(mm->emu->calc_mutex);
	calc = mm->emu->calc;
	if (!calc) {
		g_mutex_unlock(mm->emu->calc_mutex);
		return;
	}

	addr = (mm->start_addr
	        + n * mm->row_size
	        + column / MM_COLUMNS_PER_BYTE) % mm->wrap_addr;

	if (mm->use_logical)
		phys = calc->hw.mem_ltop(calc, addr);

	column %= MM_COLUMNS_PER_BYTE;

	switch (column) {
	case MM_COL_ADDRESS_0:
		if (mm->use_logical)
			g_snprintf(buf, sizeof(buf), "%04X", addr);
		else {
			if (addr >= calc->hw.romsize) {
				addr -= calc->hw.romsize;
				page = (addr >> 14) + calc->hw.rampagemask;
			}
			else {
				page = addr >> 14;
			}
			g_snprintf(buf, sizeof(buf), "%02X:%04X",
			           page, addr & 0x3fff);
		}
		break;

	case MM_COL_HEX_0:
		g_snprintf(buf, sizeof(buf), "%02X", calc->mem[phys]);
		break;

	case MM_COL_CHAR_0:
		/* FIXME */
		if (calc->mem[phys] >= 0x20 && calc->mem[phys] <= 0x7e) {
			buf[0] = calc->mem[phys];
			buf[1] = 0;
		}
		else {
			strcpy(buf, "\357\277\275");
		}
	}

	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, buf);

	g_mutex_unlock(mm->emu->calc_mutex);
}

static void tilem_mem_model_class_init(G_GNUC_UNUSED TilemMemModelClass *klass)
{
}

static void tilem_mem_model_init(TilemMemModel *mm)
{
	mm->stamp = g_random_int();
	mm->row_size = 1;
}

static void tilem_mem_tree_model_init(GtkTreeModelIface *iface)
{
	iface->get_flags = &tilem_mem_model_get_flags;
	iface->get_n_columns = &tilem_mem_model_get_n_columns;
	iface->get_column_type = &tilem_mem_model_get_column_type;
	iface->get_iter = &tilem_mem_model_get_iter;
	iface->get_path = &tilem_mem_model_get_path;
	iface->get_value = &tilem_mem_model_get_value;
	iface->iter_next = &tilem_mem_model_iter_next;
	iface->iter_children = &tilem_mem_model_iter_children;
	iface->iter_has_child = &tilem_mem_model_iter_has_child;
	iface->iter_n_children = &tilem_mem_model_iter_n_children;
	iface->iter_nth_child = &tilem_mem_model_iter_nth_child;
	iface->iter_parent = &tilem_mem_model_iter_parent;
}

GType tilem_mem_model_get_type(void)
{
	static GType type = 0;

	static const GTypeInfo type_info = {
		sizeof(TilemMemModelClass),
		NULL,
		NULL,
		(GClassInitFunc) tilem_mem_model_class_init,
		NULL,
		NULL,
		sizeof(TilemMemModel),
		0,
		(GInstanceInitFunc) tilem_mem_model_init,
		NULL
	};

	static const GInterfaceInfo tree_model_info = {
		(GInterfaceInitFunc) tilem_mem_tree_model_init,
		NULL,
		NULL
	};

	if (!type) {
		type = g_type_register_static(G_TYPE_OBJECT, "TilemMemModel",
					      &type_info, 0);
		g_type_add_interface_static(type, GTK_TYPE_TREE_MODEL,
					    &tree_model_info);
	}

	return type;
}

GtkTreeModel * tilem_mem_model_new(TilemCalcEmulator* emu,
                                   int rowsize, dword start,
                                   gboolean logical)
{
	TilemMemModel* mm;

	g_return_val_if_fail(emu != NULL, NULL);
	g_return_val_if_fail(emu->calc != NULL, NULL);
	g_return_val_if_fail(rowsize > 0, NULL);

	mm = g_object_new(TILEM_TYPE_MEM_MODEL, NULL);

	mm->emu = emu;
	mm->row_size = rowsize;
	mm->start_addr = start;

	if (logical) {
		mm->use_logical = TRUE;
		mm->wrap_addr = 0x10000;
	}
	else {
		mm->use_logical = FALSE;
		mm->wrap_addr = (emu->calc->hw.romsize
		                 + emu->calc->hw.ramsize);
	}

	mm->num_rows = (mm->wrap_addr + rowsize - 1) / rowsize;

	return GTK_TREE_MODEL(mm);
}
