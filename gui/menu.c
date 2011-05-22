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
#include <gtk/gtk.h>
#include <ticalcs.h>
#include <tilem.h>

#include "gui.h"

static void action_send_file(G_GNUC_UNUSED GtkAction *act, gpointer data)
{
	TilemEmulatorWindow *ewin = data;
	load_file(ewin);
}

static void action_receive_file(G_GNUC_UNUSED GtkAction *act, gpointer data)
{
	TilemEmulatorWindow *ewin = data;
	tilem_rcvmenu_new(ewin->emu);
}

static void action_start_debugger(G_GNUC_UNUSED GtkAction *act, gpointer data)
{
	TilemEmulatorWindow *ewin = data;
	launch_debugger(ewin);
}

static void action_open_calc(G_GNUC_UNUSED GtkAction *act, gpointer data)
{
	/* FIXME */
}

static void action_save_calc(G_GNUC_UNUSED GtkAction *act, gpointer data)
{
	/* FIXME */
}

static void action_revert_calc(G_GNUC_UNUSED GtkAction *act, gpointer data)
{
	/* FIXME */
}

static void action_reset_calc(G_GNUC_UNUSED GtkAction *act, gpointer data)
{
	TilemEmulatorWindow *ewin = data;

	g_return_if_fail(ewin != NULL);
	g_return_if_fail(ewin->emu != NULL);
	g_return_if_fail(ewin->emu->calc != NULL);

	g_mutex_lock(ewin->emu->calc_mutex);
	tilem_calc_reset(ewin->emu->calc);
	g_cond_broadcast(ewin->emu->calc_wakeup_cond);
	g_mutex_unlock(ewin->emu->calc_mutex);
}

static void action_begin_macro(G_GNUC_UNUSED GtkAction *act, gpointer data)
{
	TilemEmulatorWindow *ewin = data;
	start_record_macro(ewin);
}

static void action_end_macro(G_GNUC_UNUSED GtkAction *act, gpointer data)
{
	TilemEmulatorWindow *ewin = data;
	stop_record_macro(ewin);
}

static void action_play_macro(G_GNUC_UNUSED GtkAction *act, gpointer data)
{
	TilemEmulatorWindow *ewin = data;
	play_macro(ewin);
}

static void action_open_macro(G_GNUC_UNUSED GtkAction *act, gpointer data)
{
	/* FIXME */
}

static void action_save_macro(G_GNUC_UNUSED GtkAction *act, gpointer data)
{
	/* FIXME */
}

static void action_screenshot(G_GNUC_UNUSED GtkAction *act, gpointer data)
{
	TilemEmulatorWindow *ewin = data;
	popup_screenshot_window(ewin);
}

static void action_quick_screenshot(G_GNUC_UNUSED GtkAction *act,
                                    gpointer data)
{
	TilemEmulatorWindow *ewin = data;
	quick_screenshot(ewin);
}

static void action_quit(G_GNUC_UNUSED GtkAction *act,
                        G_GNUC_UNUSED gpointer data)
{
	gtk_main_quit();
}

static const GtkActionEntry main_action_ents[] =
	{{ "send-file",
	   GTK_STOCK_OPEN, "Send _File...", "<ctrl>O",
	   "Send a program or variable file to the calculator",
	   G_CALLBACK(action_send_file) },

	 { "receive-file",
	   GTK_STOCK_SAVE_AS, "Re_ceive File...", "<ctrl>S",
	   "Receive a program or variable from the calculator",
	   G_CALLBACK(action_receive_file) },
 
	 { "open-calc",
	   GTK_STOCK_OPEN, "_Open Calculator...", "<shift><ctrl>O",
	   "Open a calculator ROM file",
	   G_CALLBACK(action_open_calc) },
	 { "save-calc",
	   GTK_STOCK_SAVE, "_Save Calculator", "<shift><ctrl>S",
	   "Save current calculator state",
	   G_CALLBACK(action_save_calc) },
	 { "revert-calc",
	   GTK_STOCK_REVERT_TO_SAVED, "Re_vert Calculator State", 0,
	   "Revert to saved calculator state",
	   G_CALLBACK(action_revert_calc) },
	 { "reset-calc",
	   GTK_STOCK_CLEAR, "_Reset Calculator", "<shift><ctrl>Delete",
	   "Reset the calculator",
	   G_CALLBACK(action_reset_calc) },

	 { "start-debugger",
	   0, "_Debugger", "Pause",
	   "Pause emulation and start the debugger",
	   G_CALLBACK(action_start_debugger) },

	 { "begin-macro",
	   GTK_STOCK_MEDIA_RECORD, "_Record", 0,
	   "Begin recording a macro",
	   G_CALLBACK(action_begin_macro) },
	 { "end-macro",
	   GTK_STOCK_MEDIA_STOP, "S_top", 0,
	   "Begin recording a macro",
	   G_CALLBACK(action_end_macro) },
	 { "play-macro",
	   GTK_STOCK_MEDIA_PLAY, "_Play", 0,
	   "Play back the current macro",
	   G_CALLBACK(action_play_macro) },
	 { "open-macro",
	   GTK_STOCK_OPEN, "_Open Macro File...", "",
	   "Load a macro from a file",
	   G_CALLBACK(action_open_macro) },
	 { "save-macro",
	   GTK_STOCK_SAVE_AS, "_Save Macro File...", "",
	   "Save current macro to a file",
	   G_CALLBACK(action_save_macro) },

	 { "screenshot",
	   0, "S_creenshot...", "<ctrl>Print",
	   "Save a screenshot",
	   G_CALLBACK(action_screenshot) },
	 { "quick-screenshot",
	   0, "_Quick Screenshot", "<shift><ctrl>Print",
	   "Save a screenshot using default settings",
	   G_CALLBACK(action_quick_screenshot) },

	 { "quit",
	   GTK_STOCK_QUIT, "_Quit", "<ctrl>Q",
	   "Quit the application",
	   G_CALLBACK(action_quit) }};

static GtkWidget *add_item(GtkWidget *menu, GtkAccelGroup *accelgrp,
                           GtkActionGroup *actions, const char *name)
{
	GtkAction *action;
	GtkWidget *item;

	action = gtk_action_group_get_action(actions, name);
	g_return_val_if_fail(action != NULL, NULL);

	gtk_action_set_accel_group(action, accelgrp);
	item = gtk_action_create_menu_item(action);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	gtk_widget_show(item);
	return item;
}

static GtkWidget *add_separator(GtkWidget *menu)
{
	GtkWidget *item;
	item = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	gtk_widget_show(item);
	return item;
}

static GtkWidget *add_submenu(GtkWidget *menu, const char *label)
{
	GtkWidget *item, *submenu;

	item = gtk_menu_item_new_with_mnemonic(label);
	submenu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), submenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	gtk_widget_show(item);
	return submenu;
}

/* Build the menu */
void build_menu(TilemEmulatorWindow* ewin)
{
	GtkActionGroup *acts;
	GtkAccelGroup *ag;
	GtkWidget *menu, *submenu;

	ewin->actions = acts = gtk_action_group_new("Emulator");
	gtk_action_group_add_actions(ewin->actions, main_action_ents,
	                             G_N_ELEMENTS(main_action_ents), ewin);

	ag = gtk_accel_group_new();
	gtk_window_add_accel_group(GTK_WINDOW(ewin->window), ag);

	ewin->popup_menu = menu = gtk_menu_new();

	add_item(menu, ag, acts, "send-file");
	add_item(menu, ag, acts, "receive-file");
	add_separator(menu);

	add_item(menu, ag, acts, "open-calc");
	add_item(menu, ag, acts, "save-calc");
	add_item(menu, ag, acts, "revert-calc");
	add_item(menu, ag, acts, "reset-calc");
	add_separator(menu);

	add_item(menu, ag, acts, "start-debugger");

	submenu = add_submenu(menu, "_Macro");
	add_item(submenu, ag, acts, "begin-macro");
	add_item(submenu, ag, acts, "end-macro");
	add_item(submenu, ag, acts, "play-macro");
	add_separator(submenu);
	add_item(submenu, ag, acts, "open-macro");
	add_item(submenu, ag, acts, "save-macro");

	add_item(menu, ag, acts, "screenshot");
	add_item(menu, ag, acts, "quick-screenshot");
	add_separator(menu);

	add_item(menu, ag, acts, "quit");
}	
