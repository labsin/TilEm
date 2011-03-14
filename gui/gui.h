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

#include <tilem.h>
#include <z80.h>
#include <skinops.h>
#include <scancodes.h>
#include <debuginfo.h>
#include <tilemdb.h>

#include <ticalcs.h>
#include <ticables.h>
#include <tifiles.h>
#define TI73   "TI73"
#define TI82   "TI82"
#define TI83   "TI83"
#define TI83P  "TI83+"
#define TI85   "TI85"
#define TI86   "TI86"
#define X_FRINGE 2
#define Y_FRINGE 1

#include "emulator.h"
 
/* A global boolean to say "save the state" */
int SAVE_STATE;

typedef struct _TilemDebuggerRegister {
	GtkWidget* reg[12];
	
} TilemDebuggerRegister;

typedef struct _TilemKeyBinding {
	unsigned int keysym;     /* host keysym value */
	unsigned int modifiers;  /* modifier mask */
	int nscancodes;          /* number of calculator scancodes */
	byte *scancodes;         /* calculator scancodes */
} TilemKeyBinding;

/* Internal data structure for gui */
typedef struct GLOBAL_SKIN_INFOS {
	SKIN_INFOS *si;
	GtkWidget *pWindow;
	GtkWidget *pLayout;
	GtkWidget *pFrame;
	gchar* FileSelected;
	GtkFileChooser *pFileChooser;
	gint FileChooserResult;
	gchar* SkinFileName;
	GtkWidget *pRadio;
	int view;
	char calc_id;
	char *RomName;
	char *SavName;
	char RomType;
	char *FileToLoad; 
	char *MacroName;
	TilemCalcEmulator *emu;
	TilemDebuggerRegister *reg_entry;
	gboolean isDebuggerRunning;
	GtkWidget* stack_treeview;	
	FILE * macro_file;
	FILE * animation_file;
	gboolean isMacroRecording;
	gboolean isMacroPlaying;
	gboolean isStartingSkinless;
	gboolean isAnimScreenshotRecording;

	int mouse_key;		/* Key currently pressed by mouse button */

	/* List of key bindings */
	TilemKeyBinding *keybindings;
	int nkeybindings;

	/* Host keycode used to activate each key, if any */
	int keypress_keycodes[64];
	int sequence_keycode;

	/* Sequence of keys to be pressed
	   (used by core thread; guarded by calc_mutex) */
	byte *key_queue;
	int key_queue_len;
	int key_queue_timer;
	int key_queue_pressed;

}GLOBAL_SKIN_INFOS;


/* ###### event.c ##### */

/* Detect and handle a "destroy" event */
void on_destroy(); /* close the pWindow */

/* Save state of current rom */
void save_state(GLOBAL_SKIN_INFOS * gsi);

/* The window about in the right_click_menu */
void on_about(GtkWidget *pBtn);

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

/* Load a file from PC to TI */
void load_file(GLOBAL_SKIN_INFOS *gsi);

/* Load the file designed by filename */
void load_file_from_file(GLOBAL_SKIN_INFOS *gsi, char* filename) ;

/* Take a screenshot i*/
void screenshot(GLOBAL_SKIN_INFOS *gsi);




/* ###### skin.c ##### */
	
/* Create the SKIN file selector */
void tilem_user_change_skin(GLOBAL_SKIN_INFOS *gsi);

/* Choose automatically wich skin tilem must load */
void tilem_choose_skin_filename_by_default(GLOBAL_SKIN_INFOS *gsi);




/* ###### screen.c ##### */

/* Create Screen (skin and lcd) */
GtkWidget* draw_screen(GLOBAL_SKIN_INFOS * gsi) ;

/* Redraw_screen when modify the skin */
void redraw_screen(GLOBAL_SKIN_INFOS *gsi);

/* Create the lcd area */
GtkWidget * create_draw_area(GLOBAL_SKIN_INFOS * gsi);

/* Repaint another skin */
gboolean screen_repaint(GtkWidget* w G_GNUC_UNUSED,GdkEventExpose* ev G_GNUC_UNUSED,GLOBAL_SKIN_INFOS * gsi);

/* update the screen.Repaint the drawing_area widget */
gboolean screen_update(gpointer data);

/* refresh the lcd content */
void update_lcdimage(TilemCalcEmulator *emu);

/* Display the lcd image into the terminal */
void display_lcdimage_into_terminal(GLOBAL_SKIN_INFOS* gsi);



/* ##### event.c ##### */

/* Switch view to lcd only or skin + lcd */
void switch_view(GLOBAL_SKIN_INFOS * gsi) ;

/* Switch borderless. */
void switch_borderless(GLOBAL_SKIN_INFOS* gsi); 

/* Create the right click menu */
void create_menus(GtkWidget *window,GdkEvent *event,GtkItemFactoryEntry *items, int this_items, const char *menuname,gpointer* gsi);

/* Adapt the style */
void screen_restyle(GtkWidget* w, GtkStyle* oldstyle G_GNUC_UNUSED,GLOBAL_SKIN_INFOS * gsi);

/* Resize screen */
void screen_resize(GtkWidget* w G_GNUC_UNUSED,GtkAllocation* alloc, GLOBAL_SKIN_INFOS * gsi);




/* ##### tool.c ##### */

/* Generic popup (could be use for all kind of msg) */
void popup_error(char* msg, GLOBAL_SKIN_INFOS * gsi);

/* The popup to choose what kind of rom you are trying to load  (at startup)*/
char choose_rom_popup(GtkWidget *parent_window, const char *filename,
                      char default_model);

/* like on_destroy but save state */
void quit_with_save();

/* Dialog mesg */
void show_about();

/* File chooser */
char * select_file(GLOBAL_SKIN_INFOS *gsi, char* basedir);

/* File chooser with a different folder */
void select_file_with_basedir(GLOBAL_SKIN_INFOS *gsi, char* basedir);

/* Get the skin file selected */
void get_selected_file(GLOBAL_SKIN_INFOS *gsi);




/* ##### config.c ##### */

/* Search and return the default skin for this model */
char* get_defaultskin(char* romname);

/* Set a default skin, or add it if not exists */
void set_defaultskin(char* romname, char* skinname);

/* Search the most recent rom */
char* get_recentrom(char* romname);

/* Set a default skin, or add it if not exists */
void set_recentrom(char* romname);

/* get the model */
int get_modelcalcid(const char* romname);

/* Set model calc id */
void set_modelcalcid(char* romname, char id);

/* search, write, and save config on right click menu */
void add_or_modify_defaultskin(GLOBAL_SKIN_INFOS* gsi);

/* search, write, and save config on right click menu */
void add_or_modify_defaultmodel(GLOBAL_SKIN_INFOS* gsi);

/* Search and return the last directory opened to send a file*/
char* get_sendfile_recentdir();

/* Set the last dir opened to send file */
void set_sendfile_recentdir(char* recentdir);

/* Get a key from a group from config file */
char* tilem_config_universal_getter(char* group, char* key);



/* ##### debugger.c ##### */

/* Main function for the debugger. */
void launch_debugger(GLOBAL_SKIN_INFOS *gsi);

/* Refresh on click the register in the debugger */
void refresh_register(GLOBAL_SKIN_INFOS* gsi);

/* Refresh on click the stack in the debugger */
void refresh_stack(GLOBAL_SKIN_INFOS* gsi);

/* Print state register in terminal */
void printstate(TilemCalcEmulator* emu);




/* ##### link.c ##### */

/* Init libtis, create ch/cbl, attach cable, and send file to TI */
void send_file(TilemCalcEmulator* emu, CalcHandle* ch, const char* filename);

/* Init libtis, create ch/cbl, attach cable */ 
CableHandle* internal_link_handle_new(TilemCalcEmulator* emu);

/* test if the calc is ready */
int is_ready(CalcHandle* h);

/* Simply print the error */
void print_lc_error(int errnum);

/* Get calc model from calc_id */
int get_calc_model(TilemCalc* calc);

/* Simply emulate a click on key (use to prepare link -> come into receive mode) */
void run_with_key(TilemCalc* calc, int key);



/* ##### macro.c ##### */

/* Create the macro_file */
void create_or_replace_macro_file(GLOBAL_SKIN_INFOS* gsi) ;

/* Recording macro */
void add_event_in_macro_file(GLOBAL_SKIN_INFOS* gsi, char* string) ;

/* Add a load file */
void add_load_file_in_macro_file(GLOBAL_SKIN_INFOS* gsi, int length, char* filename) ;

/* Not used ...? */
void save_macro_file(GLOBAL_SKIN_INFOS* gsi) ;

/* Play it ! And play it again ! */
void play_macro(GLOBAL_SKIN_INFOS* gsi) ;

/* Play it ! And play it again ! */
void play_macro_from_file(GLOBAL_SKIN_INFOS* gsi) ;

/* Play it ! And play it again ! */
int play_macro_default(GLOBAL_SKIN_INFOS* gsi, char* macro_name) ;

/* Turn on the recording */
void start_record_macro(GLOBAL_SKIN_INFOS* gsi) ;

/* Turn off the recording */
void stop_record_macro(GLOBAL_SKIN_INFOS* gsi) ;

/* Run slowly to play macro */
void run_with_key_slowly(TilemCalc* calc, int key);



/* ##### args.c ##### */

/* Help (usage: how to use in command line) */
void help(char *name, int ret) ;

/* Command line argument handling */
int getargs(int argc, char* argv[], GLOBAL_SKIN_INFOS* gsi) ;

/* Create the SavName */
void create_savname(GLOBAL_SKIN_INFOS* gsi) ;



/* ##### animatedgif.c ##### */

/* Create a animated screenshot */
void tilem_animation_start(GLOBAL_SKIN_INFOS* gsi) ;

/* Add a frame to animation */
void tilem_animation_add_frame(GLOBAL_SKIN_INFOS* gsi) ;

/* Record the screenshot  (called in screen.c by a timer)*/
gboolean tilem_animation_record(gpointer data);

/* Stop recording screenshot */
void tilem_animation_stop(GLOBAL_SKIN_INFOS* gsi);



/* ##### gifencod.c ##### */

/* Encode gif data */
void GifEncode(FILE *fout, unsigned char *pixels, int depth, int siz);



/* ##### screenshot.c ##### */

/* create the screenshot popup */
void create_screenshot_window(GLOBAL_SKIN_INFOS* gsi);
