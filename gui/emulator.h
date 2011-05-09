/*
 * TilEm II
 *
 * Copyright (c) 2011 Benjamin Moody
 * Copyright (c) 2011 Duponchelle Thibault
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

#include "skinops.h"

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


typedef struct _TilemGuiStateFlags {
	gboolean isSkinDisabled; /* A flag to know if skinless or not */
	gboolean isAnimScreenshotRecording; /* A flag to know everywhere that screenshot is recording (gif) */
	gboolean isMacroRecording; /* A flag to know everywhere that macro is recording */
	gboolean isMacroPlaying; /* A flag to know if a macro is currently palying */
	gboolean isDebuggerRunning; /* A flag to know if debugger is runnig */
} TilemGuiStateFlags;

typedef struct _TilemKeyBinding {
	unsigned int keysym;     /* host keysym value */
	unsigned int modifiers;  /* modifier mask */
	int nscancodes;          /* number of calculator scancodes */
	byte *scancodes;         /* calculator scancodes */
} TilemKeyBinding;

/* Key press related stuff */
typedef struct _TilemKeyHandle {
	int mouse_key;		/* Key currently pressed by mouse button */

	/* Host keycode used to activate each key, if any */
	int keypress_keycodes[64];
	int sequence_keycode;

	/* Sequence of keys to be pressed
	   (used by core thread; guarded by calc_mutex) */
	byte *key_queue;
	int key_queue_len;
	int key_queue_timer;
	int key_queue_pressed;
} TilemKeyHandle;

/* Widgets  and some related things */
typedef struct _TilemGuiWidget {
	GtkWidget *pWindow; /* The top level window */
	GtkWidget *pLayout; /* Layout */

	/* Screenshot menu */
	GtkWidget* screenshot_preview_image; /* Review pixbuf */
	GtkWidget* folder_chooser_screenshot; /* Folder chooser dialog (static screenshot)*/
	GtkWidget* folder_chooser_animation; /* Folder chooser dialog (animated gif)*/

} TilemGuiWidget;




typedef struct _TilemCalcEmulator {
	GThread *z80_thread;

	GMutex *calc_mutex;
	GCond *calc_wakeup_cond;
	TilemCalc *calc;
	gboolean paused;
	gboolean exiting;
	gboolean limit_speed;   /* limit to actual speed */

	GMutex *lcd_mutex;
	TilemLCDBuffer *lcd_buffer;
	TilemGrayLCD *glcd;

	char *rom_file_name;

	struct _TilemDebugger *dbg;

	/* Skin infos  */
	SKIN_INFOS *si; /* A structure which contains all the information about a skin (see skinops.h) */

	/* This struct contains some useful gui widget */	
	TilemGuiWidget * gw;

	/* The gui flags */
	TilemGuiStateFlags *gf;

	/* This struct handle command line arguments */
	TilemCmdlineArgs *cl;

	/* All key press relating stuff (maybe we should add TilemkeyBindings inside?) */
	TilemKeyHandle * kh;
	
	/* List of key bindings */
	TilemKeyBinding *keybindings;
	int nkeybindings;

	/* Macros */
	FILE * macro_file;	/* The macro file */

	GtkProgressBar* ilp_progress_bar1; /* progress bar (current item) */
	GtkProgressBar* ilp_progress_bar2; /* progress bar (total) */
	GtkLabel* ilp_progress_label; /* status message */
	GtkWidget* ilp_progress_win;

	gboolean ilp_active;       /* internal link cable active */
	GCond *ilp_finished_cond;  /* used to signal when transfer finishes */
	gboolean ilp_error;        /* error (collision or timeout) */
	gboolean ilp_abort;        /* transfer aborted */
	int ilp_timeout_max;       /* time allowed per byte */
	int ilp_timeout;           /* time left for next byte */
	byte *ilp_read_queue;      /* buffer for received data */
	int ilp_read_count;        /* number of bytes left to read */
	const byte *ilp_write_queue; /* data to be sent */
	int ilp_write_count;         /* number of bytes left to send */

	GThread *link_thread;
	GMutex *link_queue_mutex;
	GCond *link_queue_cond;
	GQueue *link_queue;      /* queue of filenames to be sent */
	gboolean link_cancel;    /* cancel_link() has been called */
	CalcUpdate *link_update; /* CalcUpdate (status and callbacks for ticalcs) */

	/* FIXME: following stuff belongs elsewhere */

	byte* lcd_image_buf;
	int lcd_image_width;
	int lcd_image_height;
	GdkRgbCmap* lcd_cmap;

	GtkWidget* lcdwin;
	GtkWidget* background;

	GdkGeometry geomhints;
	GdkWindowHints geomhintmask;
} TilemCalcEmulator;

/* Create a new TilemCalcEmulator. */
TilemCalcEmulator *tilem_calc_emulator_new(void);

/* Free a TilemCalcEmulator. */
void tilem_calc_emulator_free(TilemCalcEmulator *emu);

/* Load the calculator state from the given ROM file (and accompanying
   sav file, if any.) */
gboolean tilem_calc_emulator_load_state(TilemCalcEmulator *emu,
                                        const char *filename);

/* Save the calculator state. */
gboolean tilem_calc_emulator_save_state(TilemCalcEmulator *emu);

/* Reset the calculator. */
void tilem_calc_emulator_reset(TilemCalcEmulator *emu);

/* Pause emulation (if currently running.) */
void tilem_calc_emulator_pause(TilemCalcEmulator *emu);

/* Resume emulation (if currently paused.) */
void tilem_calc_emulator_run(TilemCalcEmulator *emu);

/* Enable/disable speed limiting (TRUE means attempt to run at the
   actual CPU speed; FALSE means run as fast as we can.) */
void tilem_calc_emulator_set_limit_speed(TilemCalcEmulator *emu,
                                         gboolean limit);

/* Add a file to the link queue. */
void tilem_calc_emulator_send_file(TilemCalcEmulator *emu,
                                   const char *filename);

/* Abort any pending link transfers. */
void tilem_calc_emulator_cancel_link(TilemCalcEmulator *emu);
