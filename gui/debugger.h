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

	unsigned int mode;
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

	/* Temporary breakpoint info */
	int step_bp; /* Breakpoint ID */
	dword step_next_addr; /* Target address */

	dword lastwrite;
	dword lastsp;
	dword lastpc;
	gboolean paused;
	gboolean refreshing;
	gboolean delayed_refresh;
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
