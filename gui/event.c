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


#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>

#include <tilem.h>
#include <scancodes.h>
#include <gui.h>

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
void save_state(GLOBAL_SKIN_INFOS * gsi)
{
	DGLOBAL_L2_A0("**************** SAVE_STATE ****************************\n");
	DGLOBAL_L2_A1("*  YES (%d)                                             *\n",SAVE_STATE);
	DGLOBAL_L2_A0("********************************************************\n\n");
	tilem_calc_emulator_save_state(gsi->emu);
}

/* Save tilem and save state */
void quit_with_save()
{
	printf("\nThank you for using tilem...\n");
	DGLOBAL_L2_A0("**************** SAVE_STATE ****************************\n");
	SAVE_STATE=1;
	DGLOBAL_L2_A1("*  YES (%d)                                             *\n",SAVE_STATE);
	DGLOBAL_L2_A0("********************************************************\n\n");
	gtk_main_quit();
}

/* Show a nice GtkAboutDialog */
void show_about()
{

  GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file("pix/tilem.png", NULL);

  GtkWidget *dialog = gtk_about_dialog_new();
  gtk_about_dialog_set_name(GTK_ABOUT_DIALOG(dialog), "TilEm");
  gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), "2.0"); 
  gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog), "(c) Benjamin Moody\n(c) Thibault Duponchelle\n(c) Luc Bruant\n");
  gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), "TilEm is a TI Linux Emulator.\n It emulates all current z80 models.\n TI73, TI76, TI81, TI82, TI83(+)(SE), TI84+(SE), TI85 and TI86 ;D");
  gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dialog), "http://lpg.ticalc.org/prj_tilem/");
  gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(dialog), pixbuf);
  g_object_unref(pixbuf), pixbuf = NULL;
  gtk_dialog_run(GTK_DIALOG (dialog));
  gtk_widget_destroy(dialog);

}

/* Reset the calc */
void on_reset(GLOBAL_SKIN_INFOS * gsi)
{
	g_mutex_lock(gsi->emu->calc_mutex);
	tilem_calc_reset(gsi->emu->calc);
	g_cond_broadcast(gsi->emu->calc_wakeup_cond);
	g_mutex_unlock(gsi->emu->calc_mutex);
}

/* Pop up context menu */
static void show_popup_menu(GLOBAL_SKIN_INFOS* gsi, GdkEvent* event)
{
	static GtkItemFactoryEntry right_click_menu[] = {
		{"/Load skin...", "F12", tilem_user_change_skin, 1, NULL,NULL},
		{"/Send file...", "F11", load_file, 1, NULL, NULL},
		{"/Enter debugger...", "F11", launch_debugger, 1, NULL, NULL},
		{"/---", NULL, NULL, 0, "<Separator>", NULL},
		{"/Toggle speed",NULL, tilem_change_speed, 1, NULL, NULL},
		{"/Screenshot !",NULL, create_screenshot_window, 1, NULL, NULL},
		{"/Display lcd into console !",NULL, display_lcdimage_into_terminal, 1, NULL, NULL},
		{"/Switch view",NULL,switch_view,1,NULL,NULL},
		{"/Switch borderless",NULL,switch_borderless,1,NULL,NULL},
		{"/Use this model as default for this rom",NULL,add_or_modify_defaultmodel, 1, NULL, NULL},
		{"/Use this skin as default for this rom ",NULL,add_or_modify_defaultskin, 1, NULL, NULL},
		{"/Save state... ",NULL,save_state, 1, NULL, NULL},
		{"/---", NULL, NULL, 0, "<Separator>", NULL},
		{"/Start recording...", "<control>Q",start_record_macro, 0, NULL, NULL},
		{"/Stop recording.", "<control>Q",stop_record_macro, 0, NULL, NULL},
		{"/Play !", "<control>Q", play_macro, 0, NULL, NULL},
		{"/Play ! (from file...)", "<control>Q", play_macro_from_file, 0, NULL, NULL},
		{"/---", NULL, NULL, 0, "<Separator>", NULL},
		{"/About", "<control>Q",show_about, 0, NULL, NULL},
		{"/Reset", "<control>R", on_reset, 0, NULL, NULL},
		{"/Quit without saving", "<control>Q", on_destroy, 0, NULL, NULL},
		{"/Exit and save state", "<alt>X", quit_with_save, 1, NULL, NULL}
	};

	right_click_menu[0].extra_data=gsi;
	create_menus(gsi->pWindow,event,right_click_menu, sizeof(right_click_menu) / sizeof(GtkItemFactoryEntry), "<magic_right_click_menu>",(gpointer)gsi);
}

/* If currently recording a macro, record a keypress */
static void record_key(GLOBAL_SKIN_INFOS* gsi, int code)
{
	/* This WAS seriously broken ;)
	In fact, gsi->macro_file must be set to NULL when you start. And gsi->isMacroRecording to 0
	Switch isMacroRecording to 1 means : "record my key press/send file etc...
	Switch to 0 is to stop recording
	If macro file doesn't exist, tilem create it
	The key press is represented as an int, separate by a comma, the int could contains 0 before to be like this 0031,0022,0012
	*/

	char* codechar;

	if(gsi->isMacroRecording) {     
		codechar= (char*) malloc(sizeof(int));
		sprintf(codechar, "%04d", code);
		add_event_in_macro_file(gsi, codechar);     
		free(codechar);
	}
}

/* Press a key, ensuring that at most one key is "pressed" at a time
   due to this function (if pointer moves or is released, we don't
   want the old key held down.)

   FIXME: on multi-pointer displays, allow each input device to act
   separately */
static void press_mouse_key(GLOBAL_SKIN_INFOS* gsi, int key)
{
	if (gsi->mouse_key == key)
		return;

	g_mutex_lock(gsi->emu->calc_mutex);

	if (gsi->mouse_key)
		tilem_keypad_release_key(gsi->emu->calc, gsi->mouse_key);

	if (key)
		tilem_keypad_press_key(gsi->emu->calc, key);

	g_cond_broadcast(gsi->emu->calc_wakeup_cond);
	g_mutex_unlock(gsi->emu->calc_mutex);

	if (key)
		record_key(gsi, key);

	gsi->mouse_key = key;
}

/* Mouse button pressed */
gboolean mouse_press_event(G_GNUC_UNUSED GtkWidget* w, GdkEventButton *event,
                           gpointer data)
{  	
	GLOBAL_SKIN_INFOS* gsi = data;
	int key;

	key = scan_click(gsi->si, event->x, event->y);

	if (event->button == 1) {
		/* button 1: press key until button is released or pointer moves away */
		press_mouse_key(gsi, key);
		return TRUE;
	}
	else if (event->button == 2) {
		/* button 2: hold key down permanently */
		if (key) {
			g_mutex_lock(gsi->emu->calc_mutex);
			tilem_keypad_press_key(gsi->emu->calc, key);
			g_cond_broadcast(gsi->emu->calc_wakeup_cond);
			g_mutex_unlock(gsi->emu->calc_mutex);
			record_key(gsi, key);
		}
		return TRUE;
	}
	else if (event->button == 3) {
		/* button 3: popup menu */
		show_popup_menu(gsi, (GdkEvent*) event);
		return TRUE;
	}
	else
		return FALSE;
}

/* Mouse pointer moved */
gboolean pointer_motion_event(G_GNUC_UNUSED GtkWidget* w, GdkEventMotion *event,
                              gpointer data)
{
	GLOBAL_SKIN_INFOS* gsi = data;
	int key;

	if (event->is_hint)
		get_device_pointer(event->window, event->device,
		                   &event->x, &event->y, &event->state);

	if (event->state & GDK_BUTTON1_MASK)
		key = scan_click(gsi->si, event->x, event->y);
	else
		key = 0;

	press_mouse_key(gsi, key);

	return FALSE;
}

/* Mouse button released */
gboolean mouse_release_event(G_GNUC_UNUSED GtkWidget* w, GdkEventButton *event,
                             gpointer data)
{
	GLOBAL_SKIN_INFOS* gsi = data;

	if (event->button == 1)
		press_mouse_key(gsi, 0);

	return FALSE;
}

/* Timer callback for key sequences */
static void tmr_key_queue(TilemCalc* calc, void* data)
{
	GLOBAL_SKIN_INFOS *gsi = data;
	int nextkey = gsi->key_queue[gsi->key_queue_len - 1];

	if (gsi->key_queue_pressed) {
		tilem_keypad_release_key(calc, nextkey);
		gsi->key_queue_len--;
		if (gsi->key_queue_len == 0) {
			tilem_z80_remove_timer(calc, gsi->key_queue_timer);
			gsi->key_queue_timer = 0;
		}
	}
	else {
		tilem_keypad_press_key(calc, nextkey);
	}

	gsi->key_queue_pressed = !gsi->key_queue_pressed;
}

/* Find key binding matching the given event */
static TilemKeyBinding* find_key_binding(GLOBAL_SKIN_INFOS* gsi,
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

	for (i = 0; i < gsi->nkeybindings; i++)
		if (keyval == gsi->keybindings[i].keysym
		    && mods == gsi->keybindings[i].modifiers)
			return &gsi->keybindings[i];

	return NULL;
}

/* Key-press event */
gboolean key_press_event(GtkWidget* w, GdkEventKey* event,
                         gpointer data)
{
	GLOBAL_SKIN_INFOS *gsi = data;
	TilemKeyBinding *kb;
	byte *q;
	int i, key;
	w = w;

	/* Ignore repeating keys */
	for (i = 0; i < 64; i++)
		if (gsi->keypress_keycodes[i] == event->hardware_keycode)
			return FALSE;
	if (gsi->sequence_keycode == event->hardware_keycode)
		return FALSE;

	if (!(kb = find_key_binding(gsi, event)))
		return FALSE;

	g_mutex_lock(gsi->emu->calc_mutex);

	if (gsi->key_queue_len == 0 && kb->nscancodes == 1) {
		/* press single key */
		key = kb->scancodes[0];
		tilem_keypad_press_key(gsi->emu->calc, key);
		gsi->keypress_keycodes[key] = event->hardware_keycode;
		record_key(gsi, key);
	}
	else {
		/* add key sequence to queue */
		q = g_new(byte, kb->nscancodes + gsi->key_queue_len);

		for (i = 0; i < kb->nscancodes; i++) {
			q[kb->nscancodes - i - 1] = kb->scancodes[i];
			record_key(gsi, kb->scancodes[i]);
		}

		if (gsi->key_queue_len)
			memcpy(q + kb->nscancodes, gsi->key_queue,
			       gsi->key_queue_len);

		g_free(gsi->key_queue);
		gsi->key_queue = q;
		gsi->key_queue_len += kb->nscancodes;

		if (!gsi->key_queue_timer) {
			gsi->key_queue_timer
				= tilem_z80_add_timer(gsi->emu->calc,
				                      1, 50000, 1,
				                      &tmr_key_queue, gsi);
			gsi->key_queue_pressed = 0;
		}

		gsi->sequence_keycode = event->hardware_keycode;
	}

	g_cond_broadcast(gsi->emu->calc_wakeup_cond);
	g_mutex_unlock(gsi->emu->calc_mutex);

	return TRUE;
}

/* Key-release event */
gboolean key_release_event(G_GNUC_UNUSED GtkWidget* w, GdkEventKey* event,
                           gpointer data)
{
	GLOBAL_SKIN_INFOS *gsi = data;
	int i;

	/* Check if the key that was just released was one that
	   activated a calculator keypress.  (Do not try to look up
	   event->keyval; modifiers may have changed since the key was
	   pressed.) */
	for (i = 0; i < 64; i++) {
		if (gsi->keypress_keycodes[i] == event->hardware_keycode) {
			g_mutex_lock(gsi->emu->calc_mutex);
			tilem_keypad_release_key(gsi->emu->calc, i);
			g_cond_broadcast(gsi->emu->calc_wakeup_cond);
			g_mutex_unlock(gsi->emu->calc_mutex);
			gsi->keypress_keycodes[i] = 0;
		}
	}

	if (gsi->sequence_keycode == event->hardware_keycode)
		gsi->sequence_keycode = 0;

	return FALSE;
}

/* This function hide the border window, even if you load another skin, or switch view (debugger is NOT borderless because... this is useless?!) */
void switch_borderless(GLOBAL_SKIN_INFOS* gsi) {
	
	if(gtk_window_get_decorated(GTK_WINDOW(gsi->pWindow)))
	{
		gtk_window_set_decorated(GTK_WINDOW(gsi->pWindow) , FALSE);
	} else {
		gtk_window_set_decorated(GTK_WINDOW(gsi->pWindow) , TRUE);
	}
}



/* Load a file */
void load_file(GLOBAL_SKIN_INFOS *gsi)
{
	char* filename= NULL;

	/* Launch and get the result of a GtkFileChooserDialog. Cancelled -> filename == NULL */
	filename = select_file(gsi, get_sendfile_recentdir());

	/* Test if FileChooser cancelled ... */
	if(filename != NULL) {
		//printf("filename = %s", filename);
		load_file_from_file(gsi, filename);

		if(gsi->isMacroRecording)
			add_load_file_in_macro_file(gsi, strlen(filename), filename);

		/* Search the directory and save it into the config file (for the next open file) */
		char* p;
		if ((p = strrchr(filename, '/'))) {
			strcpy(p, "\0");
			set_sendfile_recentdir(filename);
		}
	}
}

/* Load a file without file_selector */
void load_file_from_file(GLOBAL_SKIN_INFOS *gsi, char* filename)
{
	
	tilem_calc_emulator_send_file(gsi->emu, filename);
}

/* Load a file without file_selector old method without thread */
void tilem_load_file_from_file_at_startup(GLOBAL_SKIN_INFOS *gsi, char* filename)
{
		CableHandle* cbl;
		CalcHandle* ch;
	
		/* Init the libtis */
		ticables_library_init();
		tifiles_library_init();
		ticalcs_library_init();
		
		/* Create cable (here an internal an dvirtual cabla) */
		cbl = internal_link_handle_new(gsi->emu);
		if (!cbl) 
			fprintf(stderr, "Cannot create ilp handle\n");
		
		/* Create calc */
		ch = ticalcs_handle_new(get_calc_model(gsi->emu->calc));
		if (!ch) 
			fprintf(stderr, "Cannot create calc handle\n");
		
		/* Attach cable to the emulated calc */
		ticalcs_cable_attach(ch, cbl);
		
		send_file(gsi->emu, ch,  filename); /* See link.c for send_file function */
		
		ticalcs_cable_detach(ch);
		ticalcs_handle_del(ch);
		ticables_handle_del(cbl);

		/* Exit the libtis */
		ticalcs_library_exit();
		tifiles_library_exit();
		ticables_library_exit();

}

/* Toggle limit speed */
void tilem_change_speed(GLOBAL_SKIN_INFOS *gsi) {

	tilem_calc_emulator_set_limit_speed(gsi->emu, !gsi->emu->limit_speed);
}
