/*
 * libtilemcore - Graphing calculator emulation library
 *
 * Copyright (C) 2001 Solignac Julien
 * Copyright (C) 2004-2009 Benjamin Moody
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

#include "x1.h"

byte x1_z80_in(TilemCalc* calc, dword port)
{
	byte v;
	
	switch(port&0x07) {
	case 0x01:
		return(tilem_keypad_read_keys(calc));

	case 0x03:
		v = (calc->keypad.onkeydown ? 0x00: 0x08);

		if (calc->z80.interrupts & TILEM_INTERRUPT_ON_KEY)
			v |= 0x01;
		if (calc->z80.interrupts & TILEM_INTERRUPT_TIMER1)
			v |= 0x02;

		return(v);

	case 0x05:
		return(calc->hwregs[PORT5]);

	case 0x06:
		return(calc->hwregs[PORT6]);
	}

	tilem_warning(calc, "Input from port %x", port);
	return(0x00);
}


void x1_z80_out(TilemCalc* calc, dword port, byte value)
{
	switch(port&0x07) {
	case 0x00:
		calc->lcd.addr = ((value & 0x1f) << 8);
		break;

	case 0x01:
		tilem_keypad_set_group(calc, value);
		break;

	case 0x02:
		calc->lcd.contrast = 16 + (value & 0x1f);
		break;

	case 0x03:
		if (value & 0x01) {
			calc->keypad.onkeyint = 1;
		}
		else {
			calc->z80.interrupts &= ~TILEM_INTERRUPT_ON_KEY;
			calc->keypad.onkeyint = 0;
		}

		if (!(value & 0x02)) {
			calc->z80.interrupts &= ~TILEM_INTERRUPT_TIMER1;
		}

		calc->hwregs[PORT3] = value;
		calc->lcd.active = calc->poweronhalt = ((value & 8) >> 3);
		break;

	case 0x04:
		calc->hwregs[PORT4] = value;
		break;

	case 0x05:
		calc->hwregs[PORT5] = value;
		break;

	case 0x06:
		calc->hwregs[PORT6] = value;
		break;
	}

	return;
}

void x1_z80_ptimer(TilemCalc* calc, int id)
{
	switch (id) {
	case TIMER_INT:
		if (calc->hwregs[PORT3] & 0x02)
			calc->z80.interrupts |= TILEM_INTERRUPT_TIMER1;
		break;
	}
}
