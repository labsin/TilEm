/*
 * libtilemcore - Graphing calculator emulation library
 *
 * Copyright (C) 2001 Solignac Julien
 * Copyright (C) 2004-2013 Benjamin Moody
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <tilem.h>

#include "xc.h"

static const TilemFlashSector flashsectors[] = {
	{0x000000, 0x10000, 0}, {0x010000, 0x10000, 0},
	{0x020000, 0x10000, 0},	{0x030000, 0x10000, 0},
	{0x040000, 0x10000, 0},	{0x050000, 0x10000, 0},
	{0x060000, 0x10000, 0},	{0x070000, 0x10000, 0},
	{0x080000, 0x10000, 0},	{0x090000, 0x10000, 0},
	{0x0A0000, 0x10000, 0},	{0x0B0000, 0x10000, 0},
	{0x0C0000, 0x10000, 0},	{0x0D0000, 0x10000, 0},
	{0x0E0000, 0x10000, 0},	{0x0F0000, 0x10000, 0},
	{0x100000, 0x10000, 0},	{0x110000, 0x10000, 0},
	{0x120000, 0x10000, 0},	{0x130000, 0x10000, 0},
	{0x140000, 0x10000, 0},	{0x150000, 0x10000, 0},
	{0x160000, 0x10000, 0},	{0x170000, 0x10000, 0},
	{0x180000, 0x10000, 0}, {0x190000, 0x10000, 0},
	{0x1A0000, 0x10000, 0}, {0x1B0000, 0x10000, 0},
	{0x1C0000, 0x10000, 0}, {0x1D0000, 0x10000, 0},
	{0x1E0000, 0x10000, 0}, {0x1F0000, 0x10000, 0},
	{0x200000, 0x10000, 0}, {0x210000, 0x10000, 0},
	{0x220000, 0x10000, 0},	{0x230000, 0x10000, 0},
	{0x240000, 0x10000, 0},	{0x250000, 0x10000, 0},
	{0x260000, 0x10000, 0},	{0x270000, 0x10000, 0},
	{0x280000, 0x10000, 0},	{0x290000, 0x10000, 0},
	{0x2A0000, 0x10000, 0},	{0x2B0000, 0x10000, 0},
	{0x2C0000, 0x10000, 0},	{0x2D0000, 0x10000, 0},
	{0x2E0000, 0x10000, 0},	{0x2F0000, 0x10000, 0},
	{0x300000, 0x10000, 0},	{0x310000, 0x10000, 0},
	{0x320000, 0x10000, 0},	{0x330000, 0x10000, 0},
	{0x340000, 0x10000, 0},	{0x350000, 0x10000, 0},
	{0x360000, 0x10000, 0},	{0x370000, 0x10000, 0},
	{0x380000, 0x10000, 0}, {0x390000, 0x10000, 0},
	{0x3A0000, 0x10000, 0}, {0x3B0000, 0x10000, 2},
	{0x3C0000, 0x10000, 0}, {0x3D0000, 0x10000, 0},
	{0x3E0000, 0x10000, 0},

	{0x3F0000, 0x02000, 0}, {0x3F2000, 0x02000, 0},
	{0x3F4000, 0x02000, 0}, {0x3F6000, 0x02000, 0},
	{0x3F8000, 0x02000, 0}, {0x3FA000, 0x02000, 0},
	{0x3FC000, 0x04000, 2}};

#define NUM_FLASH_SECTORS (sizeof(flashsectors) / sizeof(TilemFlashSector))

static const char* hwregnames[NUM_HW_REGS] = HW_REG_NAMES;

static const char* hwtimernames[NUM_HW_TIMERS] = HW_TIMER_NAMES;

extern const char* xp_keynames[];

TilemHardware hardware_ti84pcse = {
	'c', "ti84pcse", "TI-84 Plus C Silver Edition",
	(TILEM_CALC_HAS_LINK | TILEM_CALC_HAS_LINK_ASSIST
	 | TILEM_CALC_HAS_FLASH | TILEM_CALC_HAS_MD5_ASSIST
	 | TILEM_CALC_HAS_COLOR),
	320, 240, 256 * 0x4000, 8 * 0x4000, 320 * 240 * 3, 0, 0x00, 0xff,
	NUM_FLASH_SECTORS, flashsectors, 3,
	NUM_HW_REGS, hwregnames,
	NUM_HW_TIMERS, hwtimernames,
	xp_keynames,
	xc_reset, xc_stateloaded,
	xc_z80_in, xc_z80_out,
	xc_z80_wrmem, xc_z80_rdmem, xc_z80_rdmem_m1, NULL,
	xc_z80_ptimer, xc_get_lcd, xc_get_frame,
	xc_mem_ltop, xc_mem_ptol };
