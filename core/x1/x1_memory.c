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

void x1_z80_wrmem(TilemCalc* calc, dword A, byte v)
{
	if (A > 0xc000)
		*(calc->mem + 0x8000 + (A & 0x1fff)) = v;
}

byte x1_z80_rdmem(TilemCalc* calc, dword A)
{
	if (A > 0xc000)
		return (*(calc->mem + 0x8000 + (A & 0x1fff)));
	else
		return (*(calc->mem + (A & 0x7fff)));
}

dword x1_mem_ltop(TilemCalc* calc TILEM_ATTR_UNUSED, dword A)
{
	if (A & 0xc000)
		return (0x8000 + (A & 0x1fff));
	else
		return (A & 0x7fff);
}

dword x1_mem_ptol(TilemCalc* calc TILEM_ATTR_UNUSED, dword A)
{
	if (A & 0x8000)
		return (0xE000 + (A & 0x1fff));
	else
		return (A);
}
