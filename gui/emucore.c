/*
 * TilEm II
 *
 * Copyright (c) 2011-2012 Benjamin Moody
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <gtk/gtk.h>
#include <ticalcs.h>
#include <tilem.h>
#include <scancodes.h>

#include "gui.h"
#include "emucore.h"

#define MICROSEC_PER_TICK 10000

typedef struct {
	TilemCalcEmulator *emu;
	TilemTaskMainFunc mainf;
	TilemTaskFinishedFunc finishedf;
	gpointer userdata;
	gboolean cancelled;
} Task;

/* Add a task to the queue. */
void tilem_calc_emulator_begin(TilemCalcEmulator *emu,
                               TilemTaskMainFunc mainf,
                               TilemTaskFinishedFunc finishedf,
                               gpointer data)
{
	Task *task;

	g_return_if_fail(emu != NULL);
	g_return_if_fail(mainf != NULL);

	task = g_slice_new0(Task);
	task->emu = emu;
	task->mainf = mainf;
	task->finishedf = finishedf;
	task->userdata = data;

	tilem_calc_emulator_lock(emu);
	g_queue_push_tail(emu->task_queue, task);
	tilem_calc_emulator_unlock(emu);
}

/* Cancel all pending tasks.  If a task is currently running, this
   will wait for it to finish. */
void tilem_calc_emulator_cancel_tasks(TilemCalcEmulator *emu)
{
	GQueue *oldqueue;
	Task *task;

	tilem_calc_emulator_lock(emu);
	emu->task_abort = TRUE;
	emu->link_update->cancel = TRUE;
	oldqueue = emu->task_queue;
	emu->task_queue = g_queue_new();
	tilem_calc_emulator_unlock(emu);

	while ((task = g_queue_pop_head(oldqueue))) {
		if (task->finishedf)
			(*task->finishedf)(emu, task->userdata, TRUE);
		g_slice_free(Task, task);
	}

	g_queue_free(oldqueue);

	g_mutex_lock(emu->calc_mutex);
	while (emu->task_busy)
		g_cond_wait(emu->task_finished_cond, emu->calc_mutex);
	emu->task_abort = FALSE;
	emu->link_update->cancel = FALSE;
	g_cond_broadcast(emu->calc_wakeup_cond);
	g_mutex_unlock(emu->calc_mutex);
}

/* Check if calculator is powered off */
static gboolean calc_asleep(TilemCalcEmulator *emu)
{
	return (emu->calc->z80.halted
	        && !emu->calc->z80.interrupts
	        && !emu->calc->poweronhalt
	        && !emu->key_queue_timer);
}

static gboolean refresh_lcd(gpointer data)
{
	TilemCalcEmulator* emu = data;

	if (emu->ewin)
		tilem_emulator_window_refresh_lcd(emu->ewin);

	return FALSE;
}

/* Update screen for display while paused */
static void update_screen_mono(TilemCalcEmulator *emu)
{
	g_mutex_lock(emu->lcd_mutex);

	tilem_lcd_get_frame(emu->calc, emu->lcd_buffer);

	if (!emu->lcd_update_pending) {
		emu->lcd_update_pending = TRUE;
		g_idle_add_full(G_PRIORITY_DEFAULT, &refresh_lcd, emu, NULL);
	}

	g_mutex_unlock(emu->lcd_mutex);
}

/* idle callback to update progress bar */
static gboolean pbar_update(gpointer data)
{
	TilemCalcEmulator *emu = data;
	progress_bar_update(emu);
	return FALSE;
}

static void update_progress(TilemCalcEmulator *emu, gboolean force)
{
	if (force || emu->progress_changed)
		g_idle_add(&pbar_update, emu);
	emu->progress_changed = FALSE;
}

static gboolean show_debugger(gpointer data)
{
	TilemCalcEmulator* emu = data;

	if (emu->dbg)
		tilem_debugger_show(emu->dbg);

	return FALSE;
}

struct error_info {
	TilemCalcEmulator *emu;
	char *title;
	char *message;
};

static gboolean show_cable_error(gpointer data)
{
	struct error_info *info = data;

	show_error(info->emu, info->title, info->message);
	g_free(info->title);
	g_free(info->message);
	g_slice_free(struct error_info, info);
	return FALSE;
}

static void cable_error(TilemCalcEmulator *emu,
                        const char *title, int errcode)
{
	struct error_info *info;
	char *message = NULL;

	ticables_error_get(errcode, &message);
	if (!message)
		message = g_strdup_printf(_("Unknown error (%d)"), errcode);

	info = g_slice_new0(struct error_info);
	info->emu = emu;
	info->title = g_strdup(title);
	info->message = message;
	g_idle_add(&show_cable_error, info);
}

static void init_ext_cable(TilemCalcEmulator *emu)
{
	CableModel cur_model;
	CablePort cur_port;
	CableOptions options;
	int e;

	options = emu->ext_cable_options;
	emu->ext_cable_changed = FALSE;

	if (emu->ext_cable) {
		cur_model = emu->ext_cable->model;
		cur_port = emu->ext_cable->port;
	}
	else {
		cur_model = CABLE_NUL;
		cur_port = 0;
	}

	/* If a different model and/or port is selected, close and re-open
	   cable */

	if (cur_model != options.model || cur_port != options.port) {
		if (emu->ext_cable) {
			ticables_cable_close(emu->ext_cable);
			ticables_handle_del(emu->ext_cable);
			emu->ext_cable = NULL;
		}

		if (options.model == CABLE_NUL)
			return;

		emu->ext_cable = ticables_handle_new(options.model, options.port);
		g_return_if_fail(emu->ext_cable != NULL);

		tilem_em_unlock(emu);
		e = ticables_cable_open(emu->ext_cable);
		if (e) {
			cable_error(emu, _("Unable to open link cable"), e);
			tilem_em_lock(emu);
			ticables_handle_del(emu->ext_cable);
			emu->ext_cable = NULL;
			return;
		}
		tilem_em_lock(emu);
	}

	if (!emu->ext_cable)
		return;

	ticables_options_set_timeout(emu->ext_cable, options.timeout);
	ticables_options_set_delay(emu->ext_cable, options.delay);
}

static void try_reset_ext_cable(TilemCalcEmulator *emu,
                                CableHandle *cable)
{
	int e;
	e = ticables_cable_reset(cable);
	if (e) {
		cable_error(emu, _("Link cable I/O error"), e);
		ticables_cable_close(cable);
		ticables_handle_del(cable);
		emu->ext_cable = NULL;
	}
}

static void update_ext_cable_cooked(TilemCalcEmulator *emu,
                                    CableHandle *cable)
{
	CableStatus status = 0;
	int e;
	uint8_t value;
	int svalue;

	if (tilem_linkport_graylink_ready(emu->calc)) {
		tilem_em_unlock(emu);
		if (ticables_cable_check(cable, &status)) {
			try_reset_ext_cable(emu, cable);
			tilem_em_lock(emu);
			return;
		}

		if (!(status & STATUS_RX)) {
			tilem_em_lock(emu);
			return;
		}

		e = ticables_cable_get(cable, &value);
		if (e && e != ERROR_READ_TIMEOUT) {
			try_reset_ext_cable(emu, cable);
			tilem_em_lock(emu);
			return;
		}

		tilem_em_lock(emu);
		if (!e)
			tilem_linkport_graylink_send_byte(emu->calc, value);
	}
	else {
		svalue = tilem_linkport_graylink_get_byte(emu->calc);
		if (svalue < 0)
			return;

		tilem_em_unlock(emu);
		e = ticables_cable_put(cable, svalue);
		if (e) {
			try_reset_ext_cable(emu, cable);
			tilem_em_lock(emu);
			/* FIXME: should also freeze the link port to
			   notify the calc of the error */
			return;
		}
		tilem_em_lock(emu);
	}
}

#define BREAK_MASK (TILEM_STOP_BREAKPOINT \
                    | TILEM_STOP_INVALID_INST \
                    | TILEM_STOP_UNDOCUMENTED_INST)

#define GRAY_LINK_MASK (TILEM_STOP_LINK_READ_BYTE \
                        | TILEM_STOP_LINK_WRITE_BYTE \
                        | TILEM_STOP_LINK_ERROR)

/* Run one iteration of the emulator. */
dword tilem_em_run(TilemCalcEmulator *emu, int linkmode,
                   dword events, dword ff_events, gboolean keep_awake,
                   int timeout, int *elapsed)
{
	dword all_events, ev_auto, ev_user;
	int rem;
	gulong tcur, delaytime;
	CableHandle *cable = NULL;

	if (emu->exiting || emu->task_abort) {
		if (elapsed) *elapsed = 0;
		return 0;
	}
	else if (emu->paused) {
		update_screen_mono(emu);
		update_progress(emu, TRUE);
		g_cond_wait(emu->calc_wakeup_cond, emu->calc_mutex);
		update_progress(emu, TRUE);
		g_timer_elapsed(emu->timer, &emu->timevalue);
		if (elapsed) *elapsed = 0;
		return 0;
	}
	else if (!keep_awake && calc_asleep(emu)) {
		update_progress(emu, FALSE);
		update_screen_mono(emu);
		g_cond_wait(emu->calc_wakeup_cond, emu->calc_mutex);
		g_timer_elapsed(emu->timer, &emu->timevalue);
		if (elapsed) *elapsed = timeout;
		return 0;
	}

	update_progress(emu, FALSE);

	all_events = events | BREAK_MASK;

	if (!emu->ilp_active) {
		if (emu->ext_cable_changed)
			init_ext_cable(emu);
		cable = emu->ext_cable;
		if (cable) {
			linkmode = TILEM_LINK_EMULATOR_GRAY;
			all_events |= GRAY_LINK_MASK;
		}
	}

	emu->calc->linkport.linkemu = linkmode;
	emu->calc->z80.stop_mask = ~all_events;

	tilem_z80_run_time(emu->calc, timeout, &rem);

	ev_user = emu->calc->z80.stop_reason & events;
	ev_auto = emu->calc->z80.stop_reason & ~events;

	if (elapsed) *elapsed = timeout - rem;

	if (ev_auto & BREAK_MASK) {
		emu->paused = TRUE;
		g_idle_add(&show_debugger, emu);
	}

	if (cable)
		update_ext_cable_cooked(emu, cable);

	if (emu->limit_speed
	    && !(ff_events & ev_user)
	    && ff_events != TILEM_EM_ALWAYS_FF) {
		emu->timevalue += timeout - rem;
		g_timer_elapsed(emu->timer, &tcur);

		/* emu->timevalue is the "ideal" time when the
		   operation should be completed.  If emu->timevalue
		   is greater than tcur, we're running faster than
		   real time.  Try to sleep for (emu->timevalue -
		   tcur) microseconds.

		   If emu->timevalue is less than tcur, we're running
		   slower than real time.  If the difference is small,
		   just keep going and hope we'll catch up later.

		   If the difference is substantial (more than 1/10
		   second in either direction), re-synchronize. */

		delaytime = emu->timevalue - tcur;

		if (delaytime <= (gulong) 100000 + timeout) {
			tilem_em_unlock(emu);
			g_usleep(delaytime);
			tilem_em_lock(emu);
		}
		else {
			tilem_em_check_yield(emu);
			if (delaytime < (gulong) -100000)
				emu->timevalue = tcur;
		}
	}
	else {
		tilem_em_check_yield(emu);
	}

	return ev_user;
}

static gboolean taskfinished(gpointer data)
{
	Task *task = data;

	if (task->finishedf)
		(*task->finishedf)(task->emu, task->userdata, task->cancelled);

	g_slice_free(Task, task);
	return FALSE;
}

static void run_task(TilemCalcEmulator *emu, Task *task)
{
	gboolean status;

	emu->task_busy = TRUE;
	status = (*task->mainf)(emu, task->userdata);

	g_idle_add(&taskfinished, task);

	if (!status) {
		while ((task = g_queue_pop_head(emu->task_queue))) {
			task->cancelled = TRUE;
			g_idle_add(&taskfinished, task);
		}
	}
	emu->task_busy = FALSE;
}

/* Main loop */
gpointer tilem_em_main(gpointer data)
{
	TilemCalcEmulator *emu = data;
	Task *task;

	tilem_em_lock(emu);

	g_timer_start(emu->timer);
	g_timer_elapsed(emu->timer, &emu->timevalue);

	while (!emu->exiting) {
		task = g_queue_pop_head(emu->task_queue);
		if (task) {
			run_task(emu, task);
		}
		else if (emu->task_abort) {
			g_cond_broadcast(emu->task_finished_cond);
			g_cond_wait(emu->calc_wakeup_cond, emu->calc_mutex);
		}
		else {
			tilem_em_run(emu, 0, 0, 0, FALSE,
			             MICROSEC_PER_TICK, NULL);
		}
	}

	if (emu->ext_cable) {
		ticables_cable_close(emu->ext_cable);
		ticables_handle_del(emu->ext_cable);
		emu->ext_cable = NULL;
		emu->ext_cable_changed = TRUE;
	}

	tilem_em_unlock(emu);
	return NULL;
}

/* Run the calculator for a short time. */
void tilem_em_delay(TilemCalcEmulator *emu, int timeout, gboolean ff)
{
	int t;
	G_GNUC_UNUSED dword events;

	while (!emu->task_abort && timeout > 0) {
		t = MIN(MICROSEC_PER_TICK, timeout);
		events = tilem_em_run(emu, 0, 0,
		                      (ff ? TILEM_EM_ALWAYS_FF : 0), TRUE,
		                      t, &t);
		timeout -= t;
	}
}

#define LINK_EVENTS (TILEM_STOP_LINK_READ_BYTE \
                     | TILEM_STOP_LINK_WRITE_BYTE \
                     | TILEM_STOP_LINK_ERROR)

static int run_until_ready(TilemCalcEmulator *emu, int timeout, gboolean ff)
{
	int t;
	dword events;

	emu->calc->linkport.linkemu = TILEM_LINK_EMULATOR_GRAY;
	while (!emu->task_abort && timeout > 0) {
		if (tilem_linkport_graylink_ready(emu->calc))
			return 0;

		t = MIN(MICROSEC_PER_TICK, timeout);
		events = tilem_em_run(emu, TILEM_LINK_EMULATOR_GRAY,
		                      LINK_EVENTS, (ff ? LINK_EVENTS : 0), TRUE,
		                      t, &t);

		timeout -= t;
		if (events & TILEM_STOP_LINK_ERROR)
			break;
	}
	return -1;
}

/* Send a byte to the calculator. */
int tilem_em_send_byte(TilemCalcEmulator *emu, unsigned value,
                       int timeout, gboolean ff)
{
	if (run_until_ready(emu, timeout, ff))
		return -1;
	if (tilem_linkport_graylink_send_byte(emu->calc, value))
		return -1;
	if (run_until_ready(emu, timeout, ff))
		return -1;
	return 0;
}

/* Receive a byte from the calculator. */
int tilem_em_get_byte(TilemCalcEmulator *emu, int timeout, gboolean ff)
{
	int t, v;
	dword events;

	while (!emu->task_abort && timeout > 0) {
		v = tilem_linkport_graylink_get_byte(emu->calc);
		if (v >= 0)
			return v;

		t = MIN(MICROSEC_PER_TICK, timeout);
		events = tilem_em_run(emu, TILEM_LINK_EMULATOR_GRAY,
		                      LINK_EVENTS, (ff ? LINK_EVENTS : 0), FALSE,
		                      t, &t);
		timeout -= t;
		if (events & TILEM_STOP_LINK_ERROR)
			break;
	}
	return -1;
}

/* Wake up calculator if currently turned off. */
void tilem_em_wake_up(TilemCalcEmulator *emu, gboolean ff)
{
	tilem_em_delay(emu, 1000000, ff);

	if (!calc_asleep(emu))
		return;

	tilem_keypad_press_key(emu->calc, TILEM_KEY_ON);
	tilem_em_delay(emu, 500000, ff);
	tilem_keypad_release_key(emu->calc, TILEM_KEY_ON);
	tilem_em_delay(emu, 500000, ff);
}

/* Set progress window title.  Set TITLE to NULL to disable progress
   window. */
void tilem_em_set_progress_title(TilemCalcEmulator *emu, const char *title)
{
	g_mutex_lock(emu->pbar_mutex);
	g_free(emu->pbar_title);
	g_free(emu->pbar_status);
	emu->pbar_title = title ? g_strdup(title) : NULL;
	emu->pbar_status = NULL;
	emu->pbar_progress = 0.0;
	if (!emu->pbar_update_pending)
		emu->progress_changed = TRUE;
	emu->pbar_update_pending = TRUE;
	g_mutex_unlock(emu->pbar_mutex);
}

/* Set current progress information.  FRAC is the estimated fraction
   of the task completed; STATUS is a text description of the current
   operation. */
void tilem_em_set_progress(TilemCalcEmulator *emu, gdouble frac,
                           const char *status)
{
	g_mutex_lock(emu->pbar_mutex);

	if (!emu->pbar_status || !status
	    || strcmp(status, emu->pbar_status)) {
		g_free(emu->pbar_status);
		emu->pbar_status = status ? g_strdup(status) : NULL;
	}

	emu->pbar_progress = frac;

	if (!emu->pbar_update_pending)
		emu->progress_changed = TRUE;
	emu->pbar_update_pending = TRUE;

	g_mutex_unlock(emu->pbar_mutex);
}
