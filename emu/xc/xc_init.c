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

void xc_reset(TilemCalc* calc)
{
	calc->hwregs[PORT3] = 0x0B;
	calc->hwregs[PORT4] = 0x07;
	calc->hwregs[PORT6] = 0x7F;
	calc->hwregs[PORT7] = 0x7F;
	calc->hwregs[PORTE] = 0x01;
	calc->hwregs[PORTF] = 0x01;

	calc->mempagemap[0] = 0x00;
	calc->mempagemap[1] = 0xFE;
	calc->mempagemap[2] = 0xFF;
	calc->mempagemap[3] = 0xFF;

	calc->z80.r.pc.d = 0x8000;

	calc->hwregs[PORT8] = 0x80;

	calc->hwregs[PORT20] = 0;
	calc->hwregs[PORT21] = 1;
	calc->hwregs[PORT22] = 0x08;
	calc->hwregs[PORT23] = 0x69;
	calc->hwregs[PORT25] = 0x10;
	calc->hwregs[PORT26] = 0x20;
	calc->hwregs[PORT27] = 0;
	calc->hwregs[PORT28] = 0;
	calc->hwregs[PORT29] = 0x14;
	calc->hwregs[PORT2A] = 0x27;
	calc->hwregs[PORT2B] = 0x2F;
	calc->hwregs[PORT2C] = 0x3B;
	calc->hwregs[PORT2D] = 0x01;
	calc->hwregs[PORT2E] = 0x44;
	calc->hwregs[PORT2F] = 0x4A;
	calc->hwregs[PORT39] = 0xF0;
	calc->hwregs[PORT3A] = 0x00;

	calc->hwregs[FLASH_READ_DELAY] = 0;
	calc->hwregs[FLASH_WRITE_DELAY] = 0;
	calc->hwregs[FLASH_EXEC_DELAY] = 0;
	calc->hwregs[RAM_READ_DELAY] = 0;
	calc->hwregs[RAM_WRITE_DELAY] = 0;
	calc->hwregs[RAM_EXEC_DELAY] = 0;
	calc->hwregs[LCD_PORT_DELAY] = 5;
	calc->hwregs[NO_EXEC_RAM_MASK] = 0x7C00;
	calc->hwregs[NO_EXEC_RAM_LOWER] = 0x4000;
	calc->hwregs[NO_EXEC_RAM_UPPER] = 0x8000;

	calc->hwregs[BACKLIGHT_ON] = 0;
	calc->hwregs[BACKLIGHT_LEVEL] = 0;

	calc->hwregs[LCD_READ_STATE] = 0;
	calc->hwregs[LCD_READ_BUF1] = 0;
	calc->hwregs[LCD_READ_BUF2] = 0;
	calc->hwregs[LCD_WRITE_STATE] = 0;
	calc->hwregs[LCD_WRITE_BUFFER] = 0;
	calc->hwregs[LCD_REG_INDEX] = 0;
	calc->hwregs[LCD_R01] = 0x0000;
	calc->hwregs[LCD_R02] = 0x0000;
	calc->hwregs[LCD_R03] = 0x0030;
	calc->hwregs[LCD_R04] = 0x0000;
	calc->hwregs[LCD_R05] = 0x0000;
	calc->hwregs[LCD_R06] = 0x0001;
	calc->hwregs[LCD_R07] = 0x0000;
	calc->hwregs[LCD_R08] = 0x0808;
	calc->hwregs[LCD_R09] = 0x0000;
	calc->hwregs[LCD_R0A] = 0x0000;
	calc->hwregs[LCD_R0C] = 0x0000;
	calc->hwregs[LCD_R0D] = 0x0000;
	calc->hwregs[LCD_R0F] = 0x0000;
	calc->hwregs[LCD_R10] = 0x0000;
	calc->hwregs[LCD_R11] = 0x0770;
	calc->hwregs[LCD_R12] = 0x0000;
	calc->hwregs[LCD_R13] = 0x0000;
	calc->hwregs[LCD_R20] = 0x0000;
	calc->hwregs[LCD_R21] = 0x0000;
	calc->hwregs[LCD_R2B] = 0x000B;
	calc->hwregs[LCD_R50] = 0;
	calc->hwregs[LCD_R51] = 239;
	calc->hwregs[LCD_R52] = 0;
	calc->hwregs[LCD_R53] = 319;
	calc->hwregs[LCD_R60] = 0x2700;
	calc->hwregs[LCD_R61] = 0x0000;
	calc->hwregs[LCD_R6A] = 0x0000;
	calc->hwregs[LCD_R80] = 0x0000;
	calc->hwregs[LCD_R81] = 0x0000;
	calc->hwregs[LCD_R82] = 0x0000;
	calc->hwregs[LCD_R83] = 0x0000;
	calc->hwregs[LCD_R84] = 0x0000;
	calc->hwregs[LCD_R85] = 0x0000;
	calc->hwregs[LCD_R90] = 0x0010;
	calc->hwregs[LCD_R92] = 0x0600;
	calc->hwregs[LCD_R95] = 0x0200;
	calc->hwregs[LCD_R97] = 0x0C00;
	calc->hwregs[LCD_RE2] = 0x0000;
	calc->hwregs[LCD_RE3] = 0x0001;
	calc->hwregs[LCD_RE4] = 0x1400;
	calc->hwregs[LCD_RE5] = 0x01C0;
	calc->hwregs[LCD_RE6] = 0x0000;
	calc->hwregs[LCD_RE7] = 0x1004;
	calc->hwregs[LCD_RE9] = 0x0000;
	calc->hwregs[LCD_REA] = 0x0000;
	calc->hwregs[LCD_REB] = 0x8000;
	calc->hwregs[LCD_REC] = 0x148F;
	calc->hwregs[LCD_RED] = 0x269F;
	calc->hwregs[LCD_REE] = 0x0830;
	calc->hwregs[LCD_REF] = 0x1221;
	calc->hwregs[LCD_RFE] = 0x0000;
	calc->hwregs[LCD_RFF] = 0x0000;
	calc->hwregs[LCD_CUR_X] = 0;
	calc->hwregs[LCD_CUR_Y] = 0;

	calc->hwregs[PROTECTSTATE] = 0;

	calc->flash.overridegroup = 1;

	tilem_z80_set_speed(calc, 6000);

	tilem_z80_set_timer(calc, TIMER_INT1, 1600, 9277, 1);
	tilem_z80_set_timer(calc, TIMER_INT2A, 1300, 9277, 1);
	tilem_z80_set_timer(calc, TIMER_INT2B, 1000, 9277, 1);
}

void xc_stateloaded(TilemCalc* calc, int savtype TILEM_ATTR_UNUSED)
{
	tilem_calc_fix_certificate(calc, calc->mem + (0xFE * 0x4000L),
	                           0xe3, 0x0c, 0x1fc8, 0);
}
