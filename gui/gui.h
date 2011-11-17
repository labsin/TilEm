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

#include "animation.h"
#include "emulator.h"
#include "skinops.h"
#include "debuginfo.h"
#include "emuwin.h"
#include "debugger.h"

#include "gtk-compat.h"

/* This struct is used to handle cmd line args */
typedef struct _TilemCmdlineArg {
	char *SkinFileName;
	char *RomName;  
	char *SavName; 
	char *FileToLoad;  
	char *MacroToPlay;  
	/* Flags */
	gboolean isStartingSkinless; 
} TilemCmdlineArgs;

/* Screenshot view (widgets and flags) */
typedef struct _TilemScreenshotDialog {
	TilemCalcEmulator *emu;
	
	GtkWidget* window;

	/* Buttons */
	GtkWidget* screenshot;
	GtkWidget* record;
	GtkWidget* stop;

	/* Screenshot menu */
	GtkWidget* screenshot_preview_image; /* Review pixbuf */
	GtkWidget* ss_ext_combo; /* Combo box for file format */
	GtkWidget* ss_size_combo; /* Combo box for size */
	GtkWidget* width_spin;
	GtkWidget* height_spin;
	GtkWidget* grayscale_tb;
	GtkWidget* animation_speed;
	GtkWidget* background_color;
	GtkWidget* foreground_color;

	TilemAnimation *current_anim;
	gboolean current_anim_grayscale;
} TilemScreenshotDialog;


typedef struct _TilemReceiveDialog {
	TilemCalcEmulator *emu;

	GSList *vars;

	GtkWidget* window;

	GtkWidget* button_refresh;
	GtkWidget* button_save;
	GtkWidget* button_close;

	GtkWidget* treeview;
	GtkTreeModel* model;
} TilemReceiveDialog;

/* Handle the ilp progress stuff */
typedef struct _TilemLinkProgress {
	TilemCalcEmulator *emu;

	GtkProgressBar* ilp_progress_bar1; /* progress bar (current item) */
	GtkProgressBar* ilp_progress_bar2; /* progress bar (total) */
	GtkLabel* ilp_progress_label; /* status message */
	GtkWidget* ilp_progress_win;
} TilemLinkProgress;

/* A global boolean to say "save the state" */
gboolean SAVE_STATE;

#define LABEL_X_ALIGN 0.0


/* ###### event.c ##### */

/* Detect and handle a "destroy" event */
void on_destroy(); /* close the pWindow */

/* Dialog mesg */
void show_about();

/* Launch the debugger */
void launch_debugger(TilemEmulatorWindow *ewin);

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
void load_file(TilemEmulatorWindow *ewin);

/* Handle drag and drop */
gboolean on_drag_and_drop(G_GNUC_UNUSED GtkWidget *win, G_GNUC_UNUSED GdkDragContext *dc, G_GNUC_UNUSED gint x, G_GNUC_UNUSED gint y, G_GNUC_UNUSED GtkSelectionData *data, G_GNUC_UNUSED guint info, G_GNUC_UNUSED guint t, TilemEmulatorWindow * ewin);

/* Save the dimension before exit for next times we use tilem */
void save_root_window_dimension(TilemEmulatorWindow *ewin);


/* ###### emuwin.c ##### */

/* Display the lcd image into the terminal */
void display_lcdimage_into_terminal(TilemEmulatorWindow *ewin);

/* Redraw the screen with or without skin */
void redraw_screen(TilemEmulatorWindow *ewin);


/* ##### preferences.c ##### */

/* Run preferences dialog. */
void tilem_preferences_dialog(TilemEmulatorWindow *ewin);


/* ##### address.c ##### */

/* Convert address to a displayable string. */
char * tilem_format_addr(TilemDebugger *dbg, dword addr, gboolean physical);

/* Parse physical address expressed as page and offset. */
gboolean tilem_parse_paged_addr(TilemDebugger *dbg, const char *pagestr,
                                const char *offsstr, dword *value);

/* Parse an address or hex constant.  If PHYSICAL is null, only a
   logical address (simple hex value or symbol) is allowed.  If
   PHYSICAL is non-null, physical addresses in the form "PAGE:OFFSET"
   are also allowed.  *PHYSICAL will be set to true if the user
   entered a physical address. */
gboolean tilem_parse_addr(TilemDebugger *dbg, const char *string,
                          dword *value, gboolean *physical);

/* Open a dialog box prompting the user to enter an address.  PARENT
   is the transient-for window; TITLE is the dialog's title; PROMPT is
   a label for the input. */
gboolean tilem_prompt_address(TilemDebugger *dbg, GtkWindow *parent,
                              const char *title, const char *prompt,
                              dword *value, gboolean physical,
                              gboolean usedefault);


/* ##### tool.c ##### */

/* Create a frame around the given widget */
GtkWidget* new_frame(const gchar* label, GtkWidget* contents);

/* The popup to choose what kind of rom you are trying to load  (at startup)*/
char choose_rom_popup(GtkWidget *parent_window, const char *filename, char default_model);


/* ##### config.c ##### */

/* Retrieve settings from configuration file.  GROUP is the
   configuration group; following arguments are a series of OPTION
   strings, each followed by a pointer to a variable that will receive
   the value.  The list of options is terminated by NULL.

   Each OPTION is a string of the form "KEY/TYPE" or "KEY/TYPE=VALUE",
   where KEY is the name of the configuration property, and TYPE is
   either 'f' for a filename (char*), 's' for a UTF-8 string (char*),
   'i' for an integer (int), 'r' for a real number (double), or 'b'
   for a boolean (int).

   VALUE, if specified, is the default value for the option if it has
   not been defined by the user.  If no VALUE is specified, the option
   defaults to zero or NULL.

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

typedef struct {
	int model;

	VarEntry *ve;   /* Original variable info retrieved
	                   from calculator */
	int slot;       /* Slot number */

	/* Strings for display (UTF-8) */
	char *name_str; /* Variable name */
	char *type_str; /* Variable type */
	char *slot_str; /* Program slot */
	char *file_ext; /* Default file extension */
	char *filetype_desc; /* File format description */

	int size;            /* Variable size */
	gboolean archived;   /* Is archived */
	gboolean can_group;  /* Can be stored in group file */

} TilemVarEntry;

TilemVarEntry *tilem_var_entry_copy(const TilemVarEntry *tve);
void tilem_var_entry_free(TilemVarEntry *tve);

/* Send a file to the calculator through the GUI.  SLOT is the
   destination program slot (for TI-81.)  FIRST must be true if this
   is the first variable in a series; LAST must be true if this is the
   last in a series. */
void tilem_link_send_file(TilemCalcEmulator *emu, const char *filename,
                          int slot, gboolean first, gboolean last);

/* Request directory listing. */
void tilem_link_get_dirlist(TilemCalcEmulator *emu);

void begin_link(TilemCalcEmulator *emu, CableHandle **cbl, CalcHandle **ch);
void end_link(TilemCalcEmulator *emu, CableHandle *cbl, CalcHandle *ch);
int get_calc_model(TilemCalc *calc);
void show_error(TilemCalcEmulator *emu, const char *title, const char *message);

/* Receive a variable and write it to a file. */
void tilem_link_receive_file(TilemCalcEmulator *emu,
                             const TilemVarEntry* varentry,
                             const char* destination);


/* ##### pbar.c ##### */

/* Create a progress bar */
void progress_bar_init(TilemCalcEmulator* emu);

/* Update the progress bar */
void progress_bar_update_activity(TilemCalcEmulator* emu);


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

/* Save a TilemAnimation to a GIF file. */
void tilem_animation_write_gif(TilemAnimation *anim, byte* palette, int palette_size, FILE *fp);


/* ##### gifencod.c ##### */

/* Encode gif data */
void GifEncode(FILE *fout, unsigned char *pixels, int depth, int siz);


/* ##### screenshot.c ##### */

/* create the screenshot popup */
void popup_screenshot_window(TilemEmulatorWindow* ewin);

/* Take a single screenshot */
void quick_screenshot(TilemEmulatorWindow *ewin);


/* ##### keybindings.c ##### */

/* Load the keybindings */
void tilem_keybindings_init(TilemCalcEmulator* emu, const char* model);


/* ##### menu.c ##### */

/* Build the menu (do not print it) */
void build_menu(TilemEmulatorWindow* ewin);


/* ##### rcvmenu.c ##### */

/* Createe the popup dialog */
void popup_receive_menu(TilemEmulatorWindow *ewin);

/* Create a TilemReceiveDialog */
TilemReceiveDialog* tilem_receive_dialog_new(TilemCalcEmulator *emu);

/* Destroy a TilemReceiveDialog */
void tilem_receive_dialog_free(TilemReceiveDialog *rcvdlg);

/* Update TilemReceiveDialog with directory listing.  VARLIST is a
   GSList of TilemVarEntries; the dialog assumes ownership of this
   list.  Display the dialog if it's currently hidden. */
void tilem_receive_dialog_update(TilemReceiveDialog *rcvdlg,
                                 GSList *varlist);
