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

/* Key binding */
typedef struct _TilemKeyBinding {
	unsigned int keysym;     /* host keysym value */
	unsigned int modifiers;  /* modifier mask */
	int nscancodes;          /* number of calculator scancodes */
	byte *scancodes;         /* calculator scancodes */
} TilemKeyBinding;

/* A single action */
typedef struct _TilemMacroAtom {
	char* value;
	int type;
} TilemMacroAtom;

/* All the actions */
typedef struct _TilemMacro {
	TilemMacroAtom** actions;
	int n;
} TilemMacro;
	

/* Internal link cable state */
typedef struct _TilemInternalLink {
	gboolean active;       /* internal link cable active */
	GCond *finished_cond;  /* used to signal when transfer finishes */
	gboolean error;        /* error (collision or timeout) */
	gboolean abort;        /* transfer aborted */
	int timeout_max;       /* time allowed per byte */
	int timeout;           /* time left for next byte */
	byte *read_queue;      /* buffer for received data */
	int read_count;        /* number of bytes left to read */
	const byte *write_queue; /* data to be sent */
	int write_count;         /* number of bytes left to send */
} TilemInternalLink;

typedef struct _TilemCalcEmulator {
	GThread *z80_thread;

	GMutex *calc_mutex;
	GCond *calc_wakeup_cond;
	TilemCalc *calc;
	gboolean paused;
	gboolean exiting;
	gboolean limit_speed;   /* limit to actual speed */

	/* Internal link cable */
	TilemInternalLink ilp;

	/* Sequence of keys to be pressed */
	byte *key_queue;
	int key_queue_len;
	int key_queue_timer;
	int key_queue_pressed;
	int key_queue_cur;
	int key_queue_hold;

	GMutex *lcd_mutex;
	TilemLCDBuffer *lcd_buffer;
	TilemLCDBuffer *tmp_lcd_buffer;
	TilemGrayLCD *glcd;

	TilemAnimation *anim; /* animation being recorded */
	gboolean anim_grayscale; /* use grayscale in animation */

	char *rom_file_name;

	/* List of key bindings */
	TilemKeyBinding *keybindings;
	int nkeybindings;
	
	struct _TilemMacro *macro;

	/* Link transfer state */
	GThread *link_thread;
	GMutex *link_queue_mutex;
	GCond *link_queue_cond;
	GQueue *link_queue;      /* queue of filenames to be sent */
	gboolean link_cancel;    /* cancel_link() has been called */
	CalcUpdate *link_update; /* CalcUpdate (status and callbacks for ticalcs) */

	/* GUI widgets */
	struct _TilemDebugger *dbg;
	struct _TilemEmulatorWindow *ewin;
	struct _TilemScreenshotDialog *ssdlg;
	struct _TilemLinkProgress *linkpb;

	FILE * macro_file;	/* The macro file */
	gboolean isMacroRecording; /* A flag to know everywhere that macro is recording */
	gboolean isMacroPlaying; /* A flag to know if a macro is currently palying */

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

/* Press a single key. */
void tilem_calc_emulator_press_key(TilemCalcEmulator *emu, int key);

/* Release a single key. */
void tilem_calc_emulator_release_key(TilemCalcEmulator *emu, int key);

/* Add keys to the input queue. */
void tilem_calc_emulator_queue_keys(TilemCalcEmulator *emu,
                                    const byte *keys, int nkeys);

/* Release final key in input queue. */
void tilem_calc_emulator_release_queued_key(TilemCalcEmulator *emu);

/* If input queue is empty, press key immediately; otherwise, add to
   the input queue.  Return TRUE if key was added to the queue. */
gboolean tilem_calc_emulator_press_or_queue(TilemCalcEmulator *emu, int key);

/* Retrieve a static screenshot of current calculator screen.
   Returned object has a reference count of 1 (free it with
   g_object_unref().) */
TilemAnimation * tilem_calc_emulator_get_screenshot(TilemCalcEmulator *emu,
                                                    gboolean grayscale);

/* Begin recording an animated screenshot. */
void tilem_calc_emulator_begin_animation(TilemCalcEmulator *emu,
                                         gboolean grayscale);

/* Finish recording an animated screenshot.  Returned object has a
   reference count of 1 (free it with g_object_unref().) */
TilemAnimation * tilem_calc_emulator_end_animation(TilemCalcEmulator *emu);

/* Add a file to the link queue. */
void tilem_calc_emulator_send_file(TilemCalcEmulator *emu,
                                   const char *filename);

/* Abort any pending link transfers. */
void tilem_calc_emulator_cancel_link(TilemCalcEmulator *emu);


/* Run slowly to play macro */
void run_with_key_slowly(TilemCalc* calc, int key);

/* Macros */
void tilem_macro_start(TilemCalcEmulator *emu);
void tilem_macro_add_action(TilemMacro* macro, int type, char * value);
void tilem_macro_stop(TilemCalcEmulator *emu);
void tilem_macro_print(TilemMacro *macro);
void tilem_macro_write_file(TilemCalcEmulator *emu);
void tilem_macro_play(TilemCalcEmulator *emu);
void tilem_macro_load(TilemCalcEmulator *emu);
