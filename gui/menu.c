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


/* Build the menu */
GtkWidget * build_menu(GLOBAL_SKIN_INFOS* gsi) {

	GtkWidget* right_click_menu = gtk_menu_new ();    

	/* Create the items for the menu */
	GtkWidget* send_file_item =  gtk_image_menu_item_new_from_stock(GTK_STOCK_ADD, NULL);
	gtk_menu_item_set_label(GTK_MENU_ITEM(send_file_item), "Send file...");
	GtkWidget* load_skin_item =  gtk_image_menu_item_new_from_stock(GTK_STOCK_OPEN, NULL);
	gtk_menu_item_set_label(GTK_MENU_ITEM(load_skin_item), "Load skin...");
	GtkWidget* launch_debugger_item =  gtk_image_menu_item_new_from_stock(GTK_STOCK_FIND, NULL);
	gtk_menu_item_set_label(GTK_MENU_ITEM(launch_debugger_item), "Launch debugger...");
	GtkWidget* toggle_speed_item;
	if(gsi->emu->limit_speed) {
		toggle_speed_item =  gtk_image_menu_item_new_from_stock(GTK_STOCK_MEDIA_FORWARD, NULL);
	} else {
		toggle_speed_item =  gtk_image_menu_item_new_from_stock(GTK_STOCK_MEDIA_PLAY, NULL);
	}
		
	gtk_menu_item_set_label(GTK_MENU_ITEM(toggle_speed_item), "Toggle speed");

	/* >>>> Sub menu screenshot */
	GtkWidget* screenshot_submenu = gtk_menu_new();
	GtkWidget* screenshot_item =  gtk_image_menu_item_new_from_stock(GTK_STOCK_ORIENTATION_PORTRAIT, NULL);
	gtk_menu_item_set_label(GTK_MENU_ITEM(screenshot_item), "Screenshot...");
	GtkWidget* screenshot_menu_item = gtk_menu_item_new_with_label ("Screenshot menu");
	GtkWidget* quick_screenshot_item = gtk_menu_item_new_with_label ("Quick screenshot !");
	/* <<<< */
	
	GtkWidget* display_lcd_into_console_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_SORT_ASCENDING, NULL);
	gtk_menu_item_set_label(GTK_MENU_ITEM(display_lcd_into_console_item), "Display LCD into console...");
	GtkWidget* switch_view_item;
	if(gsi->view) {
		switch_view_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_FULLSCREEN, NULL);
		gtk_menu_item_set_label(GTK_MENU_ITEM(switch_view_item), "Show skin");
	} else {
		switch_view_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_LEAVE_FULLSCREEN, NULL);
		gtk_menu_item_set_label(GTK_MENU_ITEM(switch_view_item), "Hide skin");
	}
	GtkWidget* switch_borderless_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_LEAVE_FULLSCREEN, NULL);
	gtk_menu_item_set_label(GTK_MENU_ITEM(switch_borderless_item), "Switch borderless");
	

	/* >>>> Sub menu save */
	GtkWidget* save_submenu = gtk_menu_new();
	GtkWidget* save_item =  gtk_image_menu_item_new_from_stock(GTK_STOCK_PREFERENCES, NULL);
	gtk_menu_item_set_label(GTK_MENU_ITEM(save_item), "Save current state/config...");
	GtkWidget* set_default_model_item = gtk_menu_item_new_with_label ("Use this model as default for this rom");
	GtkWidget* set_default_skin_item = gtk_menu_item_new_with_label ("Use this skin as default for this rom");
	GtkWidget* save_state_item = gtk_menu_item_new_with_label ("Save state...");
	/* <<<< */
	
	/* >>>> Sub menu macro */
	GtkWidget* macro_submenu = gtk_menu_new();
	GtkWidget* macro_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_EXECUTE, NULL);
	gtk_menu_item_set_label(GTK_MENU_ITEM(macro_item), "Macro...");
	GtkWidget* start_record_macro_item = gtk_menu_item_new_with_label ("Start recording macro...");
	GtkWidget* stop_record_macro_item = gtk_menu_item_new_with_label ("Stop recording macro...");
	GtkWidget* play_item = gtk_menu_item_new_with_label ("Play macro !");
	GtkWidget* play_from_file_item = gtk_menu_item_new_with_label ("Play macro from file");
	/* <<<< */

	GtkWidget* about_item =  gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT, NULL);
	gtk_menu_item_set_label(GTK_MENU_ITEM(about_item), "About");
	GtkWidget* reset_item =  gtk_image_menu_item_new_from_stock(GTK_STOCK_DIALOG_ERROR, NULL);
	gtk_menu_item_set_label(GTK_MENU_ITEM(reset_item), "Reset");
	GtkWidget* quit_no_save_item =  gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, NULL);
	gtk_menu_item_set_label(GTK_MENU_ITEM(quit_no_save_item), "Quit without saving state");
	GtkWidget* quit_with_save_item =  gtk_image_menu_item_new_from_stock(GTK_STOCK_REVERT_TO_SAVED, NULL);
	gtk_menu_item_set_label(GTK_MENU_ITEM(quit_with_save_item), "Exit and save state");


	/* Add items to the menu */
	gtk_menu_shell_append (GTK_MENU_SHELL (right_click_menu), send_file_item);
	gtk_menu_shell_append (GTK_MENU_SHELL (right_click_menu), load_skin_item);
	gtk_menu_shell_append (GTK_MENU_SHELL (right_click_menu), launch_debugger_item);
	gtk_menu_shell_append (GTK_MENU_SHELL (right_click_menu), toggle_speed_item);

	/* Sub menu screenshot */
	gtk_menu_shell_append(GTK_MENU_SHELL(screenshot_submenu), screenshot_menu_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(screenshot_submenu), quick_screenshot_item);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(screenshot_item), screenshot_submenu);
	gtk_menu_shell_append (GTK_MENU_SHELL (right_click_menu), screenshot_item);
	
	gtk_menu_shell_append (GTK_MENU_SHELL (right_click_menu), display_lcd_into_console_item);
	gtk_menu_shell_append (GTK_MENU_SHELL (right_click_menu), switch_view_item);
	gtk_menu_shell_append (GTK_MENU_SHELL (right_click_menu), switch_borderless_item);

	/* Sub menu save */
	gtk_menu_shell_append (GTK_MENU_SHELL (save_submenu), save_state_item);
	gtk_menu_shell_append (GTK_MENU_SHELL (save_submenu), set_default_model_item);
	gtk_menu_shell_append (GTK_MENU_SHELL (save_submenu), set_default_skin_item);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(save_item), save_submenu);
	gtk_menu_shell_append (GTK_MENU_SHELL (right_click_menu), save_item);
	/* <<<< */
	
	/* >>>> Sub menu macro */
	gtk_menu_shell_append (GTK_MENU_SHELL (macro_submenu), start_record_macro_item);
	gtk_menu_shell_append (GTK_MENU_SHELL (macro_submenu), stop_record_macro_item);
	gtk_menu_shell_append (GTK_MENU_SHELL (macro_submenu), play_item);
	gtk_menu_shell_append (GTK_MENU_SHELL (macro_submenu), play_from_file_item);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(macro_item), macro_submenu);
	gtk_menu_shell_append (GTK_MENU_SHELL (right_click_menu), macro_item);
	/* <<<< */
	
	gtk_menu_shell_append (GTK_MENU_SHELL (right_click_menu), about_item);
	gtk_menu_shell_append (GTK_MENU_SHELL (right_click_menu), reset_item);
	gtk_menu_shell_append (GTK_MENU_SHELL (right_click_menu), quit_no_save_item);
	gtk_menu_shell_append(GTK_MENU_SHELL (right_click_menu), quit_with_save_item);


	/* Callback */
	g_signal_connect_swapped (GTK_OBJECT (load_skin_item), "activate", G_CALLBACK (tilem_user_change_skin), (gpointer) gsi);
	g_signal_connect_swapped (GTK_OBJECT (send_file_item), "activate", G_CALLBACK (load_file), (gpointer) gsi);
	g_signal_connect_swapped (GTK_OBJECT (launch_debugger_item), "activate", G_CALLBACK (launch_debugger), (gpointer) gsi);
	g_signal_connect_swapped (GTK_OBJECT (toggle_speed_item), "activate", G_CALLBACK (tilem_change_speed), (gpointer) gsi);
	g_signal_connect_swapped (GTK_OBJECT (screenshot_menu_item), "activate", G_CALLBACK (create_screenshot_window), (gpointer) gsi);
	g_signal_connect_swapped (GTK_OBJECT (quick_screenshot_item), "activate", G_CALLBACK (screenshot), (gpointer) gsi);
	g_signal_connect_swapped (GTK_OBJECT (display_lcd_into_console_item), "activate", G_CALLBACK (display_lcdimage_into_terminal), (gpointer) gsi);
	g_signal_connect_swapped (GTK_OBJECT (switch_view_item), "activate", G_CALLBACK (switch_view), (gpointer) gsi);
	g_signal_connect_swapped (GTK_OBJECT (switch_borderless_item), "activate", G_CALLBACK (switch_borderless), (gpointer) gsi);
	g_signal_connect_swapped (GTK_OBJECT (set_default_model_item), "activate", G_CALLBACK (add_or_modify_defaultmodel), (gpointer) gsi);
	g_signal_connect_swapped (GTK_OBJECT (set_default_skin_item), "activate", G_CALLBACK (add_or_modify_defaultskin), (gpointer) gsi);
	g_signal_connect_swapped (GTK_OBJECT (save_state_item), "activate", G_CALLBACK (save_state), (gpointer) gsi);
	g_signal_connect_swapped (GTK_OBJECT (start_record_macro_item), "activate", G_CALLBACK (start_record_macro), (gpointer) gsi);
	g_signal_connect_swapped (GTK_OBJECT (stop_record_macro_item), "activate", G_CALLBACK (stop_record_macro), (gpointer) gsi);
	g_signal_connect_swapped (GTK_OBJECT (play_item), "activate", G_CALLBACK (play_macro), (gpointer) gsi);
	g_signal_connect_swapped (GTK_OBJECT (play_from_file_item), "activate", G_CALLBACK (play_macro_from_file), (gpointer) gsi);
	g_signal_connect_swapped (GTK_OBJECT (about_item), "activate", G_CALLBACK (show_about), (gpointer) gsi);
	g_signal_connect_swapped (GTK_OBJECT (reset_item), "activate", G_CALLBACK (on_reset), (gpointer) gsi);
	g_signal_connect_swapped (GTK_OBJECT (quit_no_save_item), "activate", G_CALLBACK (on_destroy), (gpointer) gsi);
	g_signal_connect_swapped (GTK_OBJECT (quit_with_save_item), "activate", G_CALLBACK (quit_with_save), (gpointer) gsi);
	
	/* Show the items */
	gtk_widget_show (load_skin_item);
	gtk_widget_show (send_file_item);
	gtk_widget_show (launch_debugger_item);
	gtk_widget_show (toggle_speed_item);
	gtk_widget_show (screenshot_item);
	gtk_widget_show (screenshot_menu_item);
	gtk_widget_show (quick_screenshot_item);
	gtk_widget_show (display_lcd_into_console_item);
	gtk_widget_show (switch_view_item);
	gtk_widget_show (switch_borderless_item);
	gtk_widget_show (save_item);
	gtk_widget_show (set_default_model_item);
	gtk_widget_show (set_default_skin_item);
	gtk_widget_show (save_state_item);
	gtk_widget_show (macro_item);
	gtk_widget_show (start_record_macro_item);
	gtk_widget_show (stop_record_macro_item);
	gtk_widget_show (play_item);
	gtk_widget_show (play_from_file_item);
	gtk_widget_show (about_item);
	gtk_widget_show (reset_item);
	gtk_widget_show (quit_no_save_item);
	gtk_widget_show (quit_with_save_item);

	return right_click_menu;
}


/* Print the right click menu */
void show_popup_menu(GLOBAL_SKIN_INFOS* gsi, GdkEvent* event)
{
	GtkWidget* right_click_menu = build_menu(gsi);
	create_menus(gsi->pWindow, event, right_click_menu);
}
