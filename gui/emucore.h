/*
 * TilEm II
 *
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

/* IMPORTANT: The following functions may ONLY be used within
   tilem_em_main() or a task function, and must be called while
   holding the emulator lock. */

#define TILEM_EM_ALWAYS_FF 0xffffffff

/* Run one iteration of the emulator.  LINKMODE is the link port
   emulation mode.  EVENTS is a mask of events we are interested in;
   emulation will stop early if any of these events occur, or possibly
   for other reasons (e.g., a breakpoint being hit.)

   FF_EVENTS is a mask of events we want to fast-forward through
   (i.e., not apply speed limiting even if enabled.)  If FF_EVENTS is
   set to the constant ALWAYS_FF, speed limiting is completely
   disabled.

   TIMEOUT is the length of time (microseconds) to run the emulator.
   If ELAPSED is non-null, *ELAPSED will be set to the actual number
   of microseconds elapsed.

   The return value is a mask indicating which of the requested events
   occurred. */
dword tilem_em_run(TilemCalcEmulator *emu, int linkmode,
                   dword events, dword ff_events,
                   int timeout, int *elapsed);

/* Main loop */
gpointer tilem_em_main(gpointer data);

/* Run the calculator for a short time. */
void tilem_em_delay(TilemCalcEmulator *emu, int timeout, gboolean ff);

/* Send a byte to the calculator. */
int tilem_em_send_byte(TilemCalcEmulator *emu, unsigned value,
                       int timeout, gboolean ff);

/* Receive a byte from the calculator. */
int tilem_em_get_byte(TilemCalcEmulator *emu, int timeout, gboolean ff);

/* Wake up calculator if currently turned off. */
void tilem_em_wake_up(TilemCalcEmulator *emu, gboolean ff);

/* Lock emulator. */
#define tilem_em_lock(emu) \
	g_mutex_lock(emu->calc_mutex)

/* Unlock temporarily if another thread is waiting. */
#define tilem_em_check_yield(emu) \
	do { \
		if (g_atomic_int_get(&emu->calc_lock_waiting)) \
			g_cond_wait(emu->calc_wakeup_cond, emu->calc_mutex); \
	} while (0)

/* Unlock emulator. */
#define tilem_em_unlock(emu) \
	do { \
		tilem_em_check_yield(emu); \
		g_mutex_unlock(emu->calc_mutex); \
	} while (0)

