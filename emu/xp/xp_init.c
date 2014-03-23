/*
 * libtilemcore - Graphing calculator emulation library
 *
 * Copyright (C) 2001 Solignac Julien
 * Copyright (C) 2004-2011 Benjamin Moody
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

#include "xp.h"

void xp_reset(TilemCalc* calc)
{
	calc->hwregs[PORT3] = 0x0B;
	calc->hwregs[PORT4] = 0x77;
	calc->hwregs[PORT6] = 0x1F;
	calc->hwregs[PORT7] = 0x1F;

	calc->mempagemap[0] = 0x00;
	calc->mempagemap[1] = 0x1E;
	calc->mempagemap[2] = 0x1F;
	calc->mempagemap[3] = 0x1F;

	calc->z80.r.pc.d = 0x8000;

	calc->hwregs[PORT5] = 0;
	calc->hwregs[NOEXEC0] = 0;
	calc->hwregs[NOEXEC1] = 0;
	calc->hwregs[NOEXEC2] = 0;
	calc->hwregs[NOEXEC3] = 0;
	calc->hwregs[NOEXEC4] = 0;

	calc->hwregs[PROTECTSTATE] = 0;

	tilem_linkport_set_mode(calc, TILEM_LINK_MODE_NO_TIMEOUT);

	tilem_z80_set_speed(calc, 6000);

	tilem_z80_set_timer(calc, TIMER_INT1, 1600, 8474, 1);
	tilem_z80_set_timer(calc, TIMER_INT2A, 1300, 8474, 1);
	tilem_z80_set_timer(calc, TIMER_INT2B, 1000, 8474, 1);
}

void xp_stateloaded(TilemCalc* calc, int savtype TILEM_ATTR_UNUSED)
{
	tilem_calc_fix_certificate(calc, calc->mem + (0x1E * 0x4000L),
	                           0x15, 0x0c, 0x1fe0, 0x1f18);
}
