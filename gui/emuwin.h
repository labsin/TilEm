/*
 * TilEm II
 *
 * Copyright (c) 2010-2011 Thibault Duponchelle
 * Copyright (c) 2010-2012 Benjamin Moody
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

/* Root window view (widgets and flags) */
typedef struct _TilemEmulatorWindow {
	TilemCalcEmulator *emu;

	GtkWidget *window; /* The top level window */
	GtkWidget *layout; /* Layout */
	GtkWidget *lcd;
	GtkWidget *background;
	GtkWidget *popup_menu;

	GtkActionGroup *actions;

	GdkGeometry geomhints;
	GdkWindowHints geomhintmask;

	byte* lcd_image_buf;
	int lcd_image_width;
	int lcd_image_height;
	GdkRgbCmap* lcd_cmap;
	gboolean lcd_smooth_scale;

	char *skin_file_name;
	SKIN_INFOS *skin;
	gboolean skin_disabled; /* A flag to know if skinless or not */
	gdouble base_zoom;
	gdouble zoom_factor;
	GdkWindowState window_state;

	int mouse_key;		/* Key currently pressed by mouse button */

	/* Host keycode used to activate each key, if any */
	int keypress_keycodes[64];
	int sequence_keycode;

} TilemEmulatorWindow;

/* Create a new TilemEmulatorWindow. */
TilemEmulatorWindow *tilem_emulator_window_new(TilemCalcEmulator *emu);

/* Free a TilemEmulatorWindow. */
void tilem_emulator_window_free(TilemEmulatorWindow *ewin);

/* Load a skin file. */
void tilem_emulator_window_set_skin(TilemEmulatorWindow *ewin,
                                    const char *filename);

/* Enable or disable skin. */
void tilem_emulator_window_set_skin_disabled(TilemEmulatorWindow *ewin,
                                             gboolean disabled);

/* New calculator loaded. */
void tilem_emulator_window_calc_changed(TilemEmulatorWindow *ewin);

/* Redraw LCD contents. */
void tilem_emulator_window_refresh_lcd(TilemEmulatorWindow *ewin);

/* Prompt for a ROM file to open */
gboolean tilem_emulator_window_prompt_open_rom(TilemEmulatorWindow *ewin);
