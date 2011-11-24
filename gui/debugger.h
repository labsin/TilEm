/*
 * TilEm II
 *
 * Copyright (c) 2010-2011 Thibault Duponchelle
 * Copyright (c) 2011 Benjamin Moody
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

typedef struct _TilemDebugBreakpoint {
	enum {
		TILEM_DB_BREAK_LOGICAL,
		TILEM_DB_BREAK_PHYSICAL,
		TILEM_DB_BREAK_PORT,
		TILEM_DB_BREAK_OPCODE
	} type;

	enum {
		TILEM_DB_BREAK_READ = 4,
		TILEM_DB_BREAK_WRITE = 2,
		TILEM_DB_BREAK_EXEC = 1
	} mode;

	dword start;
	dword end;
	dword mask;

	int disabled;
	int id[3];
} TilemDebugBreakpoint;

typedef struct _TilemDebugger {
	struct _TilemCalcEmulator *emu;
	struct _TilemDisasm *dasm;

	/* Debugger widgets */
	GtkWidget *window; /* Main debugger window */

	GtkWidget *disasm_view;	 /* Disassembly view */
	GtkWidget *mem_view;	 /* Memory view */
	GtkWidget *stack_view;	 /* Stack view */

	GtkWidget *regbox;	    /* Box containing registers */
	GtkWidget *reg_entries[14]; /* Entries for registers */
	GtkWidget *iff_checkbox;    /* Checkbox for IFF */
	GtkWidget *flag_buttons[8]; /* Buttons for flags */

	/* Action groups */
	GtkActionGroup *run_actions;
	GtkActionGroup *paused_actions;
	GtkActionGroup *misc_actions;

	/* Memory settings */
	int mem_rowsize;
	int mem_start;
	gboolean mem_logical;

	/* Stack navigation */
	GtkAction *prev_stack_action;
	int stack_index;

	/* Breakpoints */
	GSList *breakpoints;
	int last_bp_type;
	int last_bp_mode;

	/* Temporary breakpoint info */
	int step_bp; /* Breakpoint ID */
	dword step_next_addr; /* Target address */

	dword lastwrite;
	dword lastsp;
	dword lastpc;
	gboolean paused;
	gboolean refreshing;
	gboolean delayed_refresh;

	/* Other windows */
	struct _TilemKeypadDialog *keypad_dialog;
} TilemDebugger;

/* Create a new TilemDebugger. */
TilemDebugger *tilem_debugger_new(TilemCalcEmulator *emu);

/* Free a TilemDebugger. */
void tilem_debugger_free(TilemDebugger *dbg);

/* Show debugger, and pause emulator if not already paused. */
void tilem_debugger_show(TilemDebugger *dbg);

/* Hide debugger, and resume emulation if not already running. */
void tilem_debugger_hide(TilemDebugger *dbg);

/* New calculator loaded. */
void tilem_debugger_calc_changed(TilemDebugger *dbg);

/* Update display. */
void tilem_debugger_refresh(TilemDebugger *dbg, gboolean updatemem);

/* Add a new breakpoint. */
TilemDebugBreakpoint * tilem_debugger_add_breakpoint(TilemDebugger *dbg,
                                                     const TilemDebugBreakpoint *bp);

/* Remove an existing breakpoint. */
void tilem_debugger_remove_breakpoint(TilemDebugger *dbg,
                                      TilemDebugBreakpoint *bp);

/* Modify an existing breakpoint. */
void tilem_debugger_change_breakpoint(TilemDebugger *dbg,
                                      TilemDebugBreakpoint *bp,
                                      const TilemDebugBreakpoint *newbp);

/* Show a dialog letting the user add, remove, and edit breakpoints. */
void tilem_debugger_edit_breakpoints(TilemDebugger *dbg);


/* Memory view */

/* Create the memory view */
GtkWidget *tilem_debugger_mem_view_new(TilemDebugger *dbg);

/* Set memory view settings */
void tilem_debugger_mem_view_configure(GtkWidget *mem_view,
                                       TilemCalcEmulator *emu,
                                       int rowsize, int start,
                                       gboolean logical);


/* Keypad dialog */

typedef struct _TilemKeypadDialog {
	TilemDebugger *dbg;

	gboolean refreshing;

	GtkWidget *window;
	GtkWidget *output[7];
	GtkWidget *keys[7][8];
	GtkWidget *input[8];
} TilemKeypadDialog;

/* Create a new TilemKeypadDialog. */
TilemKeypadDialog *tilem_keypad_dialog_new(TilemDebugger *dbg);

/* Free a TilemKeypadDialog. */
void tilem_keypad_dialog_free(TilemKeypadDialog *kpdlg);

/* New calculator loaded. */
void tilem_keypad_dialog_calc_changed(TilemKeypadDialog *kpdlg);

/* Refresh key states. */
void tilem_keypad_dialog_refresh(TilemKeypadDialog *kpdlg);

