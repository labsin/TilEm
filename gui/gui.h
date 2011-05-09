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

#include "emulator.h"
//#include "skinops.h"
#include "debuginfo.h"
#include "debugger.h"

#include "gtk-compat.h"
 
/* A global boolean to say "save the state" */
gboolean SAVE_STATE;

#define LABEL_X_ALIGN 0.0

/* ###### event.c ##### */

/* Detect and handle a "destroy" event */
void on_destroy(); /* close the pWindow */

/* like on_destroy but save state */
void quit_with_save(TilemCalcEmulator* emu);

/* Save state of current rom */
void save_state(TilemCalcEmulator * emu);

/* The window about in the right_click_menu */
void on_about(GtkWidget *pBtn);

/* Dialog mesg */
void show_about();

/* Launch the debugger */
void launch_debugger(TilemCalcEmulator * emu);

/* Reset the calc */
void on_reset(TilemCalcEmulator * emu);

/* Button-press event */
gboolean mouse_press_event(GtkWidget* w, GdkEventButton *event, gpointer data);

/* Pointer-motion event */
gboolean pointer_motion_event(GtkWidget* w, GdkEventMotion *event, gpointer data);

/* Button-release event */
gboolean mouse_release_event(GtkWidget* w, GdkEventButton *event, gpointer data);

/* Key-press event */
gboolean key_press_event(GtkWidget* w, GdkEventKey *event, gpointer data);

/* Key-release event */
gboolean key_release_event(GtkWidget* w, GdkEventKey *event, gpointer data);

/* Switch view to lcd only or skin + lcd */
void switch_view(TilemCalcEmulator * emu) ;

/* Switch borderless. */
void switch_borderless(TilemCalcEmulator* emu); 

/* Create the right click menu */
void create_menus(GtkWidget *window,GdkEvent *event, GtkWidget *items);

/* Adapt the style */
void screen_restyle(GtkWidget* w, GtkStyle* oldstyle G_GNUC_UNUSED,TilemCalcEmulator * emu);

/* Resize screen */
void screen_resize(GtkWidget* w G_GNUC_UNUSED,GtkAllocation* alloc, TilemCalcEmulator * emu);

/* Load a file from PC to TI */
void load_file(TilemCalcEmulator *emu);

/* Load the file designed by filename */
void load_file_from_file(TilemCalcEmulator *emu, char* filename) ;

/* Load a file at startup using old method (no thread) */
void tilem_load_file_from_file_at_startup(TilemCalcEmulator *emu, char* filename);

/* Toggle limit speed */
void tilem_change_speed(TilemCalcEmulator *emu);

/* Handle drag and drop */
gboolean on_drag_and_drop(G_GNUC_UNUSED GtkWidget *win, G_GNUC_UNUSED GdkDragContext *dc, G_GNUC_UNUSED gint x, G_GNUC_UNUSED gint y, G_GNUC_UNUSED GtkSelectionData *data, G_GNUC_UNUSED guint info, G_GNUC_UNUSED guint t, TilemCalcEmulator * emu);

/* Save the dimension before exit for next times we use tilem */
void save_root_window_dimension(TilemCalcEmulator* emu);



/* ###### skin.c ##### */
	
/* Create the SKIN file selector */
void tilem_user_change_skin(TilemCalcEmulator *emu);

/* Choose automatically wich skin tilem must load */
void tilem_choose_skin_filename_by_default(TilemCalcEmulator *emu);



/* ###### screen.c ##### */

/* Create Screen (skin and lcd) */
GtkWidget* draw_screen(TilemCalcEmulator * emu) ;

/* Redraw_screen when modify the skin */
void redraw_screen(TilemCalcEmulator *emu);

/* Create the lcd area */
GtkWidget * create_draw_area(TilemCalcEmulator * emu);

/* Repaint another skin */
gboolean screen_repaint(GtkWidget* w G_GNUC_UNUSED,GdkEventExpose* ev G_GNUC_UNUSED,TilemCalcEmulator * emu);

/* update the screen.Repaint the drawing_area widget */
gboolean screen_update(gpointer data);

/* refresh the lcd content */
void update_lcdimage(TilemCalcEmulator *emu);

/* Display the lcd image into the terminal */
void display_lcdimage_into_terminal(TilemCalcEmulator* emu);



/* ##### tool.c ##### */

/* The popup to choose what kind of rom you are trying to load  (at startup)*/
char choose_rom_popup(GtkWidget *parent_window, const char *filename, char default_model);

/* File chooser */
char * select_file(TilemCalcEmulator *emu, const char* basedir);

/* Folder chooser with a different base directory */
char* select_dir(TilemCalcEmulator *emu, const char* basedir);

/* File chooser with a different folder */
void select_file_with_basedir(TilemCalcEmulator *emu, char* basedir);

/* Get the skin file selected */
void get_selected_file(TilemCalcEmulator *emu);

/* Choose a filename to save */
char* select_file_for_save(TilemCalcEmulator *emu, char* basedir);

/* Copy paste a file */
void copy_paste(const char* src, const char* dest);



/* ##### config.c ##### */

/* Retrieve settings from configuration file.  GROUP is the
   configuration group; following arguments are a series of OPTION
   strings, each followed by a pointer to a variable that will receive
   the value.  The list of options is terminated by NULL.

   Each OPTION is a string of the form "KEY/TYPE", where KEY is the
   name of the configuration property, and TYPE is either 'f' for a
   filename (char*), 's' for a UTF-8 string (char*), 'i' for an
   integer (int), 'r' for a real number (double), or 'b' for a boolean
   (int).

   Values that have not been set by the user are set to zero or NULL.
   Strings returned by this function must be freed by the caller
   (using g_free().) */
void tilem_config_get(const char *group, const char *option, ...)
	G_GNUC_NULL_TERMINATED;

/* Save settings to the configuration file.  Arguments are a series of
   option names, as above, each followed by the new value of the
   option.  The list is terminated by NULL. */
void tilem_config_set(const char *group, const char *option, ...)
	G_GNUC_NULL_TERMINATED;



/* ##### link.c ##### */

/* Init libtis, create ch/cbl, attach cable, and send file to TI */
void send_file(TilemCalcEmulator* emu, CableHandle* cbl, const char* filename);

/* Init libtis, create ch/cbl, attach cable */ 
CableHandle* internal_link_handle_new(TilemCalcEmulator* emu);

/* Test if the calc is ready */
int is_ready(CalcHandle* h);

/* Simply print the error */
void print_lc_error(int errnum);

/* Get calc model from calc_id */
int get_calc_model(TilemCalc* calc);

/* Simply emulate a click on key (use to prepare link -> come into receive mode) */
void run_with_key(TilemCalc* calc, int key);



/* ##### pbar.c ##### */

/* Create a progress bar */
void progress_bar_init(TilemCalcEmulator* emu);

/* Update the progress bar */
void progress_bar_update_activity(TilemCalcEmulator* emu);



/* ##### macro.c ##### */

/* Create the macro_file */
void create_or_replace_macro_file(TilemCalcEmulator* emu) ;

/* Recording macro */
void add_event_in_macro_file(TilemCalcEmulator* emu, char* string) ;

/* Add a load file */
void add_load_file_in_macro_file(TilemCalcEmulator* emu, int length, char* filename) ;

/* Not used ...? */
void save_macro_file(TilemCalcEmulator* emu) ;

/* Play it ! And play it again ! */
void play_macro(TilemCalcEmulator* emu) ;

/* Play it ! And play it again ! */
void play_macro_from_file(TilemCalcEmulator* emu) ;

/* Play it ! And play it again ! */
int play_macro_default(TilemCalcEmulator* emu, char* macro_name) ;

/* Turn on the recording */
void start_record_macro(TilemCalcEmulator* emu) ;

/* Turn off the recording */
void stop_record_macro(TilemCalcEmulator* emu) ;

/* Run slowly to play macro */
void run_with_key_slowly(TilemCalc* calc, int key);



/* ##### args.c ##### */

/* Help (usage: how to use in command line) */
void tilem_cmdline_help(char *name, int ret) ;

/* Create a structure to handle cmdline args and set all fields to null */
TilemCmdlineArgs* tilem_cmdline_new();

/* Command line argument handling */
int tilem_cmdline_get_args(int argc, char* argv[], TilemCmdlineArgs* cmdline) ;

/* Create the SavName */
void create_savname(TilemCmdlineArgs* cmdline) ;



/* ##### animatedgif.c ##### */

/* Create a animated screenshot */
void tilem_animation_start(TilemCalcEmulator* emu) ;

/* Add a frame to animation */
void tilem_animation_add_frame(TilemCalcEmulator* emu) ;

/* Record the screenshot  (called in screen.c by a timer)*/
gboolean tilem_animation_record(gpointer data);

/* Stop recording screenshot */
void tilem_animation_stop(TilemCalcEmulator* emu);



/* ##### gifencod.c ##### */

/* Encode gif data */
void GifEncode(FILE *fout, unsigned char *pixels, int depth, int siz);



/* ##### screenshot.c ##### */

/* create the screenshot popup */
void create_screenshot_window(TilemCalcEmulator* emu);

/* Take a single screenshot */
void screenshot(TilemCalcEmulator *emu);


/* ##### keybindings.c ##### */

/* Load the keybindings */
void tilem_keybindings_init(TilemCalcEmulator* emu, const char* model);


/* ##### menu.c ##### */

/* Print the menu */
void show_popup_menu(TilemCalcEmulator* emu, GdkEvent* event);

/* Build the menu (do not print it) */
GtkWidget * build_menu(TilemCalcEmulator* emu);


