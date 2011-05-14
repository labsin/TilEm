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
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <ticalcs.h>
#include <tilem.h>
#include <scancodes.h>

#include "gui.h"
#include "files.h"
#include "filedlg.h"

/* Table for translating skin-file key number (based on actual
   position, and defined by the VTI/TiEmu file formats) into a
   scancode.  Note that the TILEM_KEY_* constants are named according
   to the TI-83 keypad layout; other models use different names for
   the keys, but the same scancodes. */
static const int keycode_map[] =
	{ TILEM_KEY_YEQU,
	  TILEM_KEY_WINDOW,
	  TILEM_KEY_ZOOM,
	  TILEM_KEY_TRACE,
	  TILEM_KEY_GRAPH,

	  TILEM_KEY_2ND,
	  TILEM_KEY_MODE,
	  TILEM_KEY_DEL,
	  TILEM_KEY_LEFT,
	  TILEM_KEY_RIGHT,
	  TILEM_KEY_UP,
	  TILEM_KEY_DOWN,
	  TILEM_KEY_ALPHA,
	  TILEM_KEY_GRAPHVAR,
	  TILEM_KEY_STAT,

	  TILEM_KEY_MATH,
	  TILEM_KEY_MATRIX,
	  TILEM_KEY_PRGM,
	  TILEM_KEY_VARS,
	  TILEM_KEY_CLEAR,

	  TILEM_KEY_RECIP,
	  TILEM_KEY_SIN,
	  TILEM_KEY_COS,
	  TILEM_KEY_TAN,
	  TILEM_KEY_POWER,

	  TILEM_KEY_SQUARE,
	  TILEM_KEY_COMMA,
	  TILEM_KEY_LPAREN,
	  TILEM_KEY_RPAREN,
	  TILEM_KEY_DIV,

	  TILEM_KEY_LOG,
	  TILEM_KEY_7,
	  TILEM_KEY_8,
	  TILEM_KEY_9,
	  TILEM_KEY_MUL,

	  TILEM_KEY_LN,
	  TILEM_KEY_4,
	  TILEM_KEY_5,
	  TILEM_KEY_6,
	  TILEM_KEY_SUB,

	  TILEM_KEY_STORE,
	  TILEM_KEY_1,
	  TILEM_KEY_2,
	  TILEM_KEY_3,
	  TILEM_KEY_ADD,

	  TILEM_KEY_ON,
	  TILEM_KEY_0,
	  TILEM_KEY_DECPNT,
	  TILEM_KEY_CHS,
	  TILEM_KEY_ENTER };

/* Find the keycode for the key (if any) at the given position.  If
   the keys overlap, choose the "nearest" (according to Manhattan
   distance to the midpoint.) */
static int scan_click(const SKIN_INFOS* skin, double x, double y)
{
	guint ix, iy, nearest = 0, i;
	int dx, dy, d, best_d = G_MAXINT;

	ix = (x * skin->sx + 0.5);
	iy = (y * skin->sy + 0.5);

	for (i = 0; i < G_N_ELEMENTS(keycode_map); i++) {
		if (ix >= skin->keys_pos[i].left
		    && ix < skin->keys_pos[i].right
		    && iy >= skin->keys_pos[i].top
		    && iy < skin->keys_pos[i].bottom) {
			dx = (skin->keys_pos[i].left + skin->keys_pos[i].right
			      - 2 * ix);
			dy = (skin->keys_pos[i].top + skin->keys_pos[i].bottom
			      - 2 * iy);
			d = ABS(dx) + ABS(dy);

			if (d < best_d) {
				best_d = d;
				nearest = keycode_map[i];
			}
		}
	}

	return nearest;
}

/* Retrieve pointer coordinates for an input device. */
static void get_device_pointer(GdkWindow *win, GdkDevice *dev,
                               gdouble *x, gdouble *y, GdkModifierType *mask)
{
	gdouble *axes;
	int i;

	axes = g_new(gdouble, dev->num_axes);
	gdk_device_get_state(dev, win, axes, mask);

	for (i = 0; i < dev->num_axes; i++) {
		if (x && dev->axes[i].use == GDK_AXIS_X)
			*x = axes[i];
		else if (y && dev->axes[i].use == GDK_AXIS_Y)
			*y = axes[i];
	}

	g_free(axes);
}

/* Just close the window (freeing allocated memory maybe in the futur?)*/
void on_destroy()
{
	DGLOBAL_L2_A0("**************** SAVE_STATE ****************************\n");
	SAVE_STATE=0;
	DGLOBAL_L2_A1("*  NO (%d)                                              *\n",SAVE_STATE);
	DGLOBAL_L2_A0("********************************************************\n\n");

	printf("\nThank you for using tilem...\n");
	
	gtk_main_quit();
}


/* Save state */
void save_state(TilemCalcEmulator * emu)
{
	DGLOBAL_L2_A0("**************** SAVE_STATE ****************************\n");
	DGLOBAL_L2_A1("*  YES (%d)                                             *\n",SAVE_STATE);
	DGLOBAL_L2_A0("********************************************************\n\n");
	tilem_calc_emulator_save_state(emu);
}

/* Save tilem and save state */
void quit_with_save(TilemCalcEmulator* emu)
{
	DGLOBAL_L2_A0("**************** SAVE_STATE ****************************\n");
	SAVE_STATE=1;
	DGLOBAL_L2_A1("*  YES (%d)                                             *\n",SAVE_STATE);
	DGLOBAL_L2_A0("********************************************************\n\n");
	save_root_window_dimension(emu);
	printf("\nThank you for using tilem...\n");
	gtk_main_quit();
}

/* Save the dimension before exit for next times we use tilem */
void save_root_window_dimension(TilemCalcEmulator* emu)
{
	gint width, height;
	gtk_window_get_size(GTK_WINDOW(emu->gw->tw->pWindow), &width, &height);
	tilem_config_set("settings",
	                 "width/i", width,
	                 "height/i", height,
	                 NULL);
}
	

/* Show a nice GtkAboutDialog */
void show_about()
{
	GtkWidget *dialog = gtk_about_dialog_new();
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), "2.0"); 
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog), "(c) Benjamin Moody\n(c) Thibault Duponchelle\n(c) Luc Bruant\n");
	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), "TilEm is a TI Linux Emulator.\n It emulates all current z80 models.\n TI73, TI76, TI81, TI82, TI83(+)(SE), TI84+(SE), TI85 and TI86 ;D");
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dialog), "http://lpg.ticalc.org/prj_tilem/");

	/* Add the logo */	
	char* tilem_ban = get_shared_file_path("pixs", "tilem_ban.png", NULL);
	if(tilem_ban) {
		GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(tilem_ban, NULL);
		gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(dialog), pixbuf);
		g_object_unref(pixbuf), pixbuf = NULL;
	}

	gtk_dialog_run(GTK_DIALOG (dialog));
	gtk_widget_destroy(dialog);

}

/* Reset the calc */
void on_reset(TilemCalcEmulator * emu)
{
	g_mutex_lock(emu->calc_mutex);
	tilem_calc_reset(emu->calc);
	g_cond_broadcast(emu->calc_wakeup_cond);
	g_mutex_unlock(emu->calc_mutex);
}

void launch_debugger(TilemCalcEmulator * emu)
{
	if (!emu->dbg)
		emu->dbg = tilem_debugger_new(emu);
	tilem_debugger_show(emu->dbg);
}

/* If currently recording a macro, record a keypress */
static void record_key(TilemCalcEmulator* emu, int code)
{
	/* This WAS seriously broken ;)
	In fact, macro_file must be set to NULL when you start. And isMacroRecording to 0
	Switch isMacroRecording to 1 means : "record my key press/send file etc...
	Switch to 0 is to stop recording
	If macro file doesn't exist, tilem creates it
	The key press is represented as an int, separate by a comma, the int could contains 0 before to be like this 0031,0022,0012
	*/

	char* codechar;

	if(emu->gw->mc->isMacroRecording) {     
		codechar= (char*) malloc(sizeof(int));
		sprintf(codechar, "%04d", code);
		add_event_in_macro_file(emu, codechar);     
		free(codechar);
	}
}

/* Press a key, ensuring that at most one key is "pressed" at a time
   due to this function (if pointer moves or is released, we don't
   want the old key held down.)

   FIXME: on multi-pointer displays, allow each input device to act
   separately */
static void press_mouse_key(TilemCalcEmulator* emu, int key)
{
	if (emu->kh->mouse_key == key)
		return;

	g_mutex_lock(emu->calc_mutex);

	if (emu->kh->mouse_key)
		tilem_keypad_release_key(emu->calc, emu->kh->mouse_key);

	if (key)
		tilem_keypad_press_key(emu->calc, key);

	g_cond_broadcast(emu->calc_wakeup_cond);
	g_mutex_unlock(emu->calc_mutex);

	if (key)
		record_key(emu, key);

	emu->kh->mouse_key = key;
}

/* Mouse button pressed */
gboolean mouse_press_event(G_GNUC_UNUSED GtkWidget* w, GdkEventButton *event,
                           gpointer data)
{  	
	TilemCalcEmulator* emu = data;
	int key;

	key = scan_click(emu->si, event->x, event->y);

	if (event->button == 1) {
		/* button 1: press key until button is released or pointer moves away */
		press_mouse_key(emu, key);
		return TRUE;
	}
	else if (event->button == 2) {
		/* button 2: hold key down permanently */
		if (key) {
			g_mutex_lock(emu->calc_mutex);
			tilem_keypad_press_key(emu->calc, key);
			g_cond_broadcast(emu->calc_wakeup_cond);
			g_mutex_unlock(emu->calc_mutex);
			record_key(emu, key);
		}
		return TRUE;
	}
	else if (event->button == 3) {
		/* button 3: popup menu */
		show_popup_menu(emu, (GdkEvent*) event);
		return TRUE;
	}
	else
		return FALSE;
}

/* Mouse pointer moved */
gboolean pointer_motion_event(G_GNUC_UNUSED GtkWidget* w, GdkEventMotion *event,
                              gpointer data)
{
	TilemCalcEmulator* emu = data;
	int key;

	if (event->is_hint)
		get_device_pointer(event->window, event->device,
		                   &event->x, &event->y, &event->state);

	if (event->state & GDK_BUTTON1_MASK)
		key = scan_click(emu->si, event->x, event->y);
	else
		key = 0;

	press_mouse_key(emu, key);

	return FALSE;
}

/* Mouse button released */
gboolean mouse_release_event(G_GNUC_UNUSED GtkWidget* w, GdkEventButton *event,
                             gpointer data)
{
	TilemCalcEmulator* emu = data;

	if (event->button == 1)
		press_mouse_key(emu, 0);

	return FALSE;
}

/* Timer callback for key sequences */
static void tmr_key_queue(TilemCalc* calc, void* data)
{
	TilemCalcEmulator *emu = data;
	int nextkey = emu->kh->key_queue[emu->kh->key_queue_len - 1];

	if (emu->kh->key_queue_pressed) {
		tilem_keypad_release_key(calc, nextkey);
		emu->kh->key_queue_len--;
		if (emu->kh->key_queue_len == 0) {
			tilem_z80_remove_timer(calc, emu->kh->key_queue_timer);
			emu->kh->key_queue_timer = 0;
		}
	}
	else {
		tilem_keypad_press_key(calc, nextkey);
	}

	emu->kh->key_queue_pressed = !emu->kh->key_queue_pressed;
}

/* Find key binding matching the given event */
static TilemKeyBinding* find_key_binding(TilemCalcEmulator* emu,
                                         GdkEventKey* event)
{
	GdkDisplay *dpy;
	GdkKeymap *km;
	guint keyval;
	GdkModifierType consumed, mods;
	int i;

	dpy = gdk_drawable_get_display(event->window);
	km = gdk_keymap_get_for_display(dpy);

	/* determine the relevant set of modifiers */

	gdk_keymap_translate_keyboard_state(km, event->hardware_keycode,
	                                    event->state, event->group,
	                                    &keyval, NULL, NULL, &consumed);

	mods = (event->state & ~consumed
	        & (GDK_SHIFT_MASK | GDK_CONTROL_MASK | GDK_MOD1_MASK));

	for (i = 0; i < emu->nkeybindings; i++)
		if (keyval == emu->keybindings[i].keysym
		    && mods == emu->keybindings[i].modifiers)
			return &emu->keybindings[i];

	return NULL;
}

/* Key-press event */
gboolean key_press_event(GtkWidget* w, GdkEventKey* event,
                         gpointer data)
{
	TilemCalcEmulator *emu = data;
	TilemKeyBinding *kb;
	byte *q;
	int i, key;
	w = w;

	/* Ignore repeating keys */
	for (i = 0; i < 64; i++)
		if (emu->kh->keypress_keycodes[i] == event->hardware_keycode)
			return FALSE;
	if (emu->kh->sequence_keycode == event->hardware_keycode)
		return FALSE;

	if (!(kb = find_key_binding(emu, event)))
		return FALSE;

	g_mutex_lock(emu->calc_mutex);

	if (emu->kh->key_queue_len == 0 && kb->nscancodes == 1) {
		/* press single key */
		key = kb->scancodes[0];
		tilem_keypad_press_key(emu->calc, key);
		emu->kh->keypress_keycodes[key] = event->hardware_keycode;
		record_key(emu, key);
	}
	else {
		/* add key sequence to queue */
		q = g_new(byte, kb->nscancodes + emu->kh->key_queue_len);

		for (i = 0; i < kb->nscancodes; i++) {
			q[kb->nscancodes - i - 1] = kb->scancodes[i];
			record_key(emu, kb->scancodes[i]);
		}

		if (emu->kh->key_queue_len)
			memcpy(q + kb->nscancodes, emu->kh->key_queue,
			       emu->kh->key_queue_len);

		g_free(emu->kh->key_queue);
		emu->kh->key_queue = q;
		emu->kh->key_queue_len += kb->nscancodes;

		if (!emu->kh->key_queue_timer) {
			emu->kh->key_queue_timer
				= tilem_z80_add_timer(emu->calc,
				                      1, 50000, 1,
				                      &tmr_key_queue, emu);
			emu->kh->key_queue_pressed = 0;
		}

		emu->kh->sequence_keycode = event->hardware_keycode;
	}

	g_cond_broadcast(emu->calc_wakeup_cond);
	g_mutex_unlock(emu->calc_mutex);

	return TRUE;
}

/* Key-release event */
gboolean key_release_event(G_GNUC_UNUSED GtkWidget* w, GdkEventKey* event,
                           gpointer data)
{
	TilemCalcEmulator *emu = data;
	int i;

	/* Check if the key that was just released was one that
	   activated a calculator keypress.  (Do not try to look up
	   event->keyval; modifiers may have changed since the key was
	   pressed.) */
	for (i = 0; i < 64; i++) {
		if (emu->kh->keypress_keycodes[i] == event->hardware_keycode) {
			g_mutex_lock(emu->calc_mutex);
			tilem_keypad_release_key(emu->calc, i);
			g_cond_broadcast(emu->calc_wakeup_cond);
			g_mutex_unlock(emu->calc_mutex);
			emu->kh->keypress_keycodes[i] = 0;
		}
	}

	if (emu->kh->sequence_keycode == event->hardware_keycode)
		emu->kh->sequence_keycode = 0;

	return FALSE;
}

/* This function hide the border window, even if you load another skin, or switch view (debugger is NOT borderless because... this is useless?!) */
void switch_borderless(TilemCalcEmulator* emu) {
	
	if(gtk_window_get_decorated(GTK_WINDOW(emu->gw->tw->pWindow)))
		gtk_window_set_decorated(GTK_WINDOW(emu->gw->tw->pWindow) , FALSE);
	 else 
		gtk_window_set_decorated(GTK_WINDOW(emu->gw->tw->pWindow) , TRUE);
}

#define PAT_TI81       "*.prg"
#define PAT_TI73       "*.73?"
#define PAT_TI73_NUM   "*.73n;*.73l;*.73m;*.73i"
#define PAT_TI82       "*.82?"
#define PAT_TI82_NUM   "*.82n;*.82l;*.82m;*.82i"
#define PAT_TI82_TEXT  "*.82s;*.82y;*.82p"
#define PAT_TI83       "*.83?"
#define PAT_TI83_NUM   "*.83n;*.83l;*.83m;*.83i"
#define PAT_TI83_TEXT  "*.83s;*.83y;*.83p"
#define PAT_TI83P      "*.8x?;*.8xgrp"
#define PAT_TI83P_NUM  "*.8xn;*.8xl;*.8xm;*.8xi"
#define PAT_TI83P_TEXT "*.8xs;*.8xy;*.8xp"
#define PAT_TI85       "*.85?"
#define PAT_TI86       "*.86?"
#define PAT_TIG        "*.tig"

#define FLT_TI81       "TI-81 programs", PAT_TI81
#define FLT_TI73       "TI-73 files", PAT_TI73
#define FLT_TI82       "TI-82 files", PAT_TI82
#define FLT_TI83       "TI-83 files", PAT_TI83
#define FLT_TI83P      "TI-83 Plus files", PAT_TI83P
#define FLT_TI85       "TI-85 files", PAT_TI85
#define FLT_TI86       "TI-86 files", PAT_TI86
#define FLT_TIG        "TIGroup files", PAT_TIG
#define FLT_ALL        "All files", "*"

#define DESC_COMPAT "All compatible files"

#define FLT_TI73_COMPAT    DESC_COMPAT, (PAT_TI73 ";" PAT_TIG ";" \
                                         PAT_TI82_NUM ";" \
                                         PAT_TI83_NUM ";" \
                                         PAT_TI83P_NUM)

#define FLT_TI82_COMPAT    DESC_COMPAT, (PAT_TI82 ";" PAT_TIG ";" \
                                         PAT_TI83_TEXT ";" PAT_TI83_NUM ";" \
                                         PAT_TI83P_TEXT ";" PAT_TI83P_NUM ";" \
                                         PAT_TI73_NUM)

#define FLT_TI83_COMPAT    DESC_COMPAT, (PAT_TI83 ";" PAT_TIG ";" \
                                         PAT_TI82_TEXT ";" PAT_TI82_NUM ";" \
                                         PAT_TI83P_TEXT ";" PAT_TI83P_NUM ";" \
                                         PAT_TI73_NUM)

#define FLT_TI83P_COMPAT   DESC_COMPAT, (PAT_TI83P ";" PAT_TIG ";" \
                                         PAT_TI82_TEXT ";" PAT_TI82_NUM ";" \
                                         PAT_TI83_TEXT ";" PAT_TI83_NUM ";" \
                                         PAT_TI73_NUM)

#define FLT_TI8586_COMPAT  DESC_COMPAT, (PAT_TI85 ";" PAT_TI86 ";" PAT_TIG)

static char ** prompt_link_files(const char *title,
                                 GtkWindow *parent,
                                 const char *dir,
                                 int model)
{
	switch (model) {
	case TILEM_CALC_TI73:
		return prompt_open_files(title, parent, dir,
		                         FLT_TI73_COMPAT, FLT_TI73,
		                         FLT_TI82, FLT_TI83, FLT_TI83P,
		                         FLT_TIG, FLT_ALL, NULL);
	case TILEM_CALC_TI81:
		return prompt_open_files(title, parent, dir,
		                         FLT_TI81, FLT_ALL, NULL);
	case TILEM_CALC_TI82:
		return prompt_open_files(title, parent, dir,
		                         FLT_TI82_COMPAT, FLT_TI73,
		                         FLT_TI82, FLT_TI83, FLT_TI83P,
		                         FLT_TIG, FLT_ALL, NULL);
	case TILEM_CALC_TI83:
	case TILEM_CALC_TI76:
		return prompt_open_files(title, parent, dir,
		                         FLT_TI83_COMPAT, FLT_TI73,
		                         FLT_TI82, FLT_TI83, FLT_TI83P,
		                         FLT_TIG, FLT_ALL, NULL);
	case TILEM_CALC_TI83P:
	case TILEM_CALC_TI83P_SE:
	case TILEM_CALC_TI84P:
	case TILEM_CALC_TI84P_SE:
	case TILEM_CALC_TI84P_NSPIRE:
		return prompt_open_files(title, parent, dir,
		                         FLT_TI83P_COMPAT, FLT_TI73,
		                         FLT_TI82, FLT_TI83, FLT_TI83P,
		                         FLT_TIG, FLT_ALL, NULL);
	case TILEM_CALC_TI85:
	case TILEM_CALC_TI86:
		return prompt_open_files(title, parent, dir,
		                         FLT_TI8586_COMPAT, FLT_TI85,
		                         FLT_TI86, FLT_TIG, FLT_ALL, NULL);
	default:
		return prompt_open_files(title, parent, dir, FLT_ALL, NULL);
	}
}

/* Load a file */
void load_file(TilemCalcEmulator *emu)
{
	char **filenames, *dir;
	int i;

	tilem_config_get("upload",
	                 "sendfile_recentdir/f", &dir,
	                 NULL);

	filenames = prompt_link_files("Send File",
	                              GTK_WINDOW(emu->gw->tw->pWindow),
	                              dir, emu->calc->hw.model_id);
	g_free(dir);

	if (filenames && filenames[0]) {
		dir = g_path_get_dirname(filenames[0]);
		tilem_config_set("upload",
		                 "sendfile_recentdir/f", dir,
		                 NULL);
		g_free(dir);
	}

	for (i = 0; filenames && filenames[i]; i++) {
		load_file_from_file(emu, filenames[i]);

		if(emu->gw->mc->isMacroRecording)
			add_load_file_in_macro_file(emu, strlen(filenames[i]), filenames[i]);
	}

	g_strfreev(filenames);
}

/* Load a file without file_selector */
void load_file_from_file(TilemCalcEmulator *emu, char* filename)
{
	tilem_calc_emulator_send_file(emu, filename);
}

/* Load a file without file_selector old method without thread */
void tilem_load_file_from_file_at_startup(TilemCalcEmulator *emu, char* filename)
{
		CableHandle* cbl;
	
		/* Init the libtis */
		ticables_library_init();
		tifiles_library_init();
		ticalcs_library_init();
		
		/* Create cable (here an internal an dvirtual cabla) */
		cbl = internal_link_handle_new(emu);
		if (!cbl) 
			fprintf(stderr, "Cannot create ilp handle\n");
		
		send_file(emu, cbl, filename); /* See link.c for send_file function */
		
		ticables_handle_del(cbl);

		/* Exit the libtis */
		ticalcs_library_exit();
		tifiles_library_exit();
		ticables_library_exit();

}

/* Toggle limit speed */
void tilem_change_speed(TilemCalcEmulator *emu) {
	tilem_calc_emulator_set_limit_speed(emu, !emu->limit_speed);
}

/* Callback function for the drag and drop event */
gboolean on_drag_and_drop(G_GNUC_UNUSED GtkWidget *win, G_GNUC_UNUSED GdkDragContext *dc, G_GNUC_UNUSED gint x, G_GNUC_UNUSED gint y, G_GNUC_UNUSED GtkSelectionData *data, G_GNUC_UNUSED guint info, G_GNUC_UNUSED guint t, TilemCalcEmulator * emu) {
	
	/* FIXME : this should really be refactorised because it just a "proof of concept" currently :)
	   The string returned by gtk_selection_data_get_text look like "file:///[path]\r\0" that's why we should use 
	   a glib function to clean the "\r" and "file:///" but I don't know wich function will do that ? */
	printf("drag and drop !!\n");
	char* filename = (char*)gtk_selection_data_get_text(data);
	printf("data : %s\n", filename);
	filename = filename + 7;
	printf("data : %s\n", filename);
	int size = strlen(filename);
	filename[size - 2] = '\0';
	printf("emu : %s", emu->rom_file_name);
	load_file_from_file(emu, filename );
	return FALSE;

}
