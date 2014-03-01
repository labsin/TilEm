/*
 * libtilemcore - Graphing calculator emulation library
 *
 * Copyright (C) 2013 Benjamin Moody
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
#include <string.h>
#include <time.h>
#include <tilem.h>

#include "xc.h"
#include "../gettext.h"

#define WIDTH 320
#define HEIGHT 240

/* NOTE:

   ILI9335 docs refer to "horizontal" and "vertical" motion - the LCD
   in the 84pc is installed sideways, so this terminology is wrong.
   We use the terms "row number" (or "Y") and "column number" (or "X")
   to refer to what the docs call "horizontal" and "vertical" position.

   Note also that "normal" image rendering on the 84pc requires
   setting the BGR and REV bits (i.e., internally, the R and B
   channels are swapped and brightness levels are inverted.)

   The 'lcdmem' array is stored in the natural format for RGB image
   rendering - i.e., row major order, with the red component first.
   This is contrary to what you might think of as the natural
   implementation from the ILI9335's perspective.
*/


/* Register 01 - row/column output mode */

/* SM = 0: normal
   SM = 1: columns interlaced */
#define R01_INTERLACED      0x0400
/* SS = 0: normal
   SS = 1: memory reads/writes flipped in Y direction */
#define R01_FLIP_COLUMNS    0x0100


/* Register 03 - pixel I/O mode */

/* TRI = 0: 16-bit output mode (2 output cycles per pixel)
   TRI = 1: 18-bit output mode (3 output cycles per pixel) */
#define R03_18BIT         0x8000
/* DFM = 0: write 18-bit data as low 18 bits of 24-bit value
   DFM = 1: write 18-bit data as high 6 bits of each of 3 bytes */
#define R03_18BIT_UNPACKED  0x4000
/* BGR = 1: swap red/blue values on read/write */
#define R03_BGR           0x1000
/* ORG = 0: normal
   ORG = 1: something weird */
#define R03_ORG           0x0080
/* ID1 = 0: transfer image right to left (column decrement)
   ID1 = 1: transfer image left to right (column increment) */
#define R03_COLINC        0x0020
/* ID0 = 0: transfer image bottom to top (row decrement)
   ID0 = 1: transfer image top to bottom (row increment) */
#define R03_ROWINC        0x0010
/* AM = 0: increment/decrement row index first
   AM = 1: increment/decrement column index first */
#define R03_COLFIRST      0x0008


/* Register 07 - main lcd control */

/* PTDE1 = 0: don't display partial image #1
   PTDE1 = 1: display partial image #1 */
#define R07_SHOW_PARTIAL1 0x2000
/* PTDE0 = 0: don't display partial image #0
   PTDE0 = 1: display partial image #0 */
#define R07_SHOW_PARTIAL0 0x1000
/* BASEE = 0: don't display base image
   BASEE = 1: display base image (overrides PTDE0/PTDE1) */
#define R07_SHOW_BASE     0x0100
/* GON: what does this do? */
#define R07_GON           0x0020
/* DTE: what does this do? */
#define R07_DTE           0x0010
/* CL = 0: normal mode
   CL = 1: limit to 8 colors */
#define R07_8COLOR        0x0008
/* D1 = 0: turn off display
   D1 = 1: turn on display */
#define R07_ENABLE1       0x0002
/* D0 = 0: halt controller
   D0 = 1: enable controller */
#define R07_ENABLE0       0x0001


/* Register 60 - base image control */

/* GS = 0: normal scan
   GS = 1: screen flipped in X direction */
#define R60_FLIP_ROWS        0x8000
/* NL = number of columns to be displayed (/ 8) */
#define R60_BASE_NLINES_MASK 0x3F00
/* SCN = first column (memory-wise) in base image (/ 8) */
#define R60_BASE_START_MASK  0x003F


/* Register 61 - color and scrolling control */

/* NDL = 0: non-display area is white
   NDL = 1: non-display area is black */
#define R61_NDL_BLACK       0x0004
/* VLE = 0: disable scrolling
   VLE = 1: scroll according to R6A */
#define R61_SCROLL_ENABLED  0x0002
/* REV = 0: normal colors
   REV = 1: inverted colors */
#define R61_LEVEL_INVERT    0x0001


/* Register 6A - scrolling control */

/* VL = [0-319]: scroll base image in X direction (if VLE set) */
#define R6A_SCROLL_MASK 0x01FF


/* Registers 80-85 - partial image buffers, positions, and sizes */

/* PTDP0: starting display position of partial image 0 */
#define R80_P0_POS_MASK   0x01FF
/* PTSA0: starting buffer address of partial image 0 */
#define R81_P0_START_MASK 0x01FF
/* PTEA0: starting buffer address of partial image 0 */
#define R82_P0_END_MASK   0x01FF
/* PTDP1: starting display position of partial image 0 */
#define R83_P1_POS_MASK   0x01FF
/* PTSA1: starting buffer address of partial image 0 */
#define R84_P1_START_MASK 0x01FF
/* PTEA1: starting buffer address of partial image 0 */
#define R85_P1_END_MASK   0x01FF


/* Write to the LCD index register (port 10) */
void xc_lcd_control(TilemCalc* calc, byte val)
{
	byte prev = calc->hwregs[LCD_REG_INDEX];
	calc->hwregs[LCD_REG_INDEX] = (prev << 8) | val;

	/* FIXME: stuff doesn't work (how?) if you only write one byte
	   to port 10.  You need to write two bytes but the first one
	   doesn't matter. */

	if (calc->hwregs[LCD_READ_STATE] != 0)
		tilem_warning(calc, _("incomplete LCD read"));
	if (calc->hwregs[LCD_WRITE_STATE] != 0)
		tilem_warning(calc, _("incomplete LCD write"));

	/* I guess writing to the index register probably resets the
	   I/O state? */
	calc->hwregs[LCD_READ_STATE] = 0;
	calc->hwregs[LCD_WRITE_STATE] = 0;
}

static dword* get_reg(TilemCalc* calc, byte index)
{
	switch (index) {
	case 0x01: return &calc->hwregs[LCD_R01];
	case 0x02: return &calc->hwregs[LCD_R02];
	case 0x03: return &calc->hwregs[LCD_R03];
	case 0x04: return &calc->hwregs[LCD_R04];
	case 0x05: return &calc->hwregs[LCD_R05];
	case 0x06: return &calc->hwregs[LCD_R06];
	case 0x07: return &calc->hwregs[LCD_R07];
	case 0x08: return &calc->hwregs[LCD_R08];
	case 0x09: return &calc->hwregs[LCD_R09];
	case 0x0A: return &calc->hwregs[LCD_R0A];
	case 0x0C: return &calc->hwregs[LCD_R0C];
	case 0x0D: return &calc->hwregs[LCD_R0D];
	case 0x0F: return &calc->hwregs[LCD_R0F];
	case 0x10: return &calc->hwregs[LCD_R10];
	case 0x11: return &calc->hwregs[LCD_R11];
	case 0x12: return &calc->hwregs[LCD_R12];
	case 0x13: return &calc->hwregs[LCD_R13];
	case 0x20: return &calc->hwregs[LCD_R20];
	case 0x21: return &calc->hwregs[LCD_R21];
	case 0x29: return &calc->hwregs[LCD_R29];
	case 0x2B: return &calc->hwregs[LCD_R2B];
	case 0x30: return &calc->hwregs[LCD_R30];
	case 0x31: return &calc->hwregs[LCD_R31];
	case 0x32: return &calc->hwregs[LCD_R32];
	case 0x35: return &calc->hwregs[LCD_R35];
	case 0x36: return &calc->hwregs[LCD_R36];
	case 0x37: return &calc->hwregs[LCD_R37];
	case 0x38: return &calc->hwregs[LCD_R38];
	case 0x39: return &calc->hwregs[LCD_R39];
	case 0x3C: return &calc->hwregs[LCD_R3C];
	case 0x3D: return &calc->hwregs[LCD_R3D];
	case 0x50: return &calc->hwregs[LCD_R50];
	case 0x51: return &calc->hwregs[LCD_R51];
	case 0x52: return &calc->hwregs[LCD_R52];
	case 0x53: return &calc->hwregs[LCD_R53];
	case 0x60: return &calc->hwregs[LCD_R60];
	case 0x61: return &calc->hwregs[LCD_R61];
	case 0x6A: return &calc->hwregs[LCD_R6A];
	case 0x80: return &calc->hwregs[LCD_R80];
	case 0x81: return &calc->hwregs[LCD_R81];
	case 0x82: return &calc->hwregs[LCD_R82];
	case 0x83: return &calc->hwregs[LCD_R83];
	case 0x84: return &calc->hwregs[LCD_R84];
	case 0x85: return &calc->hwregs[LCD_R85];
	case 0x90: return &calc->hwregs[LCD_R90];
	case 0x92: return &calc->hwregs[LCD_R92];
	case 0x95: return &calc->hwregs[LCD_R95];
	case 0x97: return &calc->hwregs[LCD_R97];
	case 0xA1: return &calc->hwregs[LCD_RA1];
	case 0xA2: return &calc->hwregs[LCD_RA2];
	case 0xA3: return &calc->hwregs[LCD_RA3];
	case 0xA4: return &calc->hwregs[LCD_RA4];
	case 0xA5: return &calc->hwregs[LCD_RA5];
	case 0xE2: return &calc->hwregs[LCD_RE2];
	case 0xE3: return &calc->hwregs[LCD_RE3];
	case 0xE4: return &calc->hwregs[LCD_RE4];
	case 0xE5: return &calc->hwregs[LCD_RE5];
	case 0xE6: return &calc->hwregs[LCD_RE6];
	case 0xE7: return &calc->hwregs[LCD_RE7];
	case 0xE9: return &calc->hwregs[LCD_RE9];
	case 0xEA: return &calc->hwregs[LCD_REA];
	case 0xEB: return &calc->hwregs[LCD_REB];
	case 0xEC: return &calc->hwregs[LCD_REC];
	case 0xED: return &calc->hwregs[LCD_RED];
	case 0xEE: return &calc->hwregs[LCD_REE];
	case 0xEF: return &calc->hwregs[LCD_REF];
	case 0xFE: return &calc->hwregs[LCD_RFE];
	case 0xFF: return &calc->hwregs[LCD_RFF];
	default: return NULL;
	}
}

/* Get mask of significant bits */
static dword get_reg_mask(byte index)
{
	switch (index) {
	case 0x01: return 0x0500;
	case 0x03: return 0xD0B8;
	case 0x04: return 0x0777;
	case 0x05: return 0x0003;
	case 0x06: return 0x0001;
	case 0x07: return 0x313B;
	case 0x08: return 0xFFFF;
	case 0x09: return 0x073F;
	case 0x0A: return 0x000F;
	case 0x0C: return 0x7133;
	case 0x0D: return 0x01FF;
	case 0x0F: return 0x001B;
	case 0x10: return 0x17F3;
	case 0x11: return 0x0777;
	case 0x12: return 0x008F;
	case 0x13: return 0x1F00;
	case 0x20: return 0x00FF;
	case 0x21: return 0x01FF;
	case 0x29: return 0x003F;
	case 0x2B: return 0x000F;
	case 0x30: return 0x0707;
	case 0x31: return 0x0707;
	case 0x32: return 0x0707;
	case 0x35: return 0x0707;
	case 0x36: return 0x1F0F;
	case 0x37: return 0x0707;
	case 0x38: return 0x0707;
	case 0x39: return 0x0707;
	case 0x3C: return 0x0707;
	case 0x3D: return 0x1F0F;
	case 0x50: return 0x00FF;
	case 0x51: return 0x00FF;
	case 0x52: return 0x01FF;
	case 0x53: return 0x01FF;
	case 0x60: return 0xBF3F;
	case 0x61: return 0x0007;
	case 0x6A: return 0x01FF;
	case 0x80: return 0x01FF;
	case 0x81: return 0x01FF;
	case 0x82: return 0x01FF;
	case 0x83: return 0x01FF;
	case 0x84: return 0x01FF;
	case 0x85: return 0x01FF;
	case 0x90: return 0x031F;
	case 0x92: return 0x0700;
	case 0x95: return 0x0300;
	case 0x97: return 0x0F00;
	case 0xA1: return 0x083F;
	case 0xA2: return 0x0001;
	case 0xA3: return 0x0003;
	case 0xA4: return 0xF7FF;
	case 0xA5: return 0xFFFF;
	case 0xE2: return 0x9FFF;
	case 0xE3: return 0xF0FF;
	case 0xE4: return 0x1F3C;
	case 0xE5: return 0x1FC7;
	case 0xE6: return 0x0001;
	case 0xE7: return 0x3FFF;
	case 0xE8: return 0x003F;
	case 0xE9: return 0x000F;
	case 0xEA: return 0xFFFF;
	case 0xEB: return 0xB37F;
	case 0xEC: return 0x7FFF;
	case 0xED: return 0x7FFF;
	case 0xEE: return 0x0F7F;
	case 0xEF: return 0xFB37;
	case 0xFE: return 0x0001;
	case 0xFF: return 0x000F;
	default:   return 0xFFFF;
	}
}

/* Read an LCD register */
static word read_reg(TilemCalc* calc, byte index)
{
	dword *reg;

	if (index == 0x00)
		return 0x9335;
	else if (index == 0xE8)
		return 0x003F;
	else if ((reg = get_reg(calc, index)))
		return *reg;
	else if (!calc->hwregs[LCD_READ_STATE])
		tilem_warning(calc, _("reading from unknown LCD register %04x"),
		              calc->hwregs[LCD_REG_INDEX]);
	return 0;
}

static void set_org(TilemCalc *calc, dword mode)
{
	if (mode & R03_ORG) {
		if (mode & R03_COLINC)
			calc->hwregs[LCD_CUR_X] = calc->hwregs[LCD_R52];
		else
			calc->hwregs[LCD_CUR_X] = calc->hwregs[LCD_R53];
		if (mode & R03_ROWINC)
			calc->hwregs[LCD_CUR_Y] = calc->hwregs[LCD_R50];
		else
			calc->hwregs[LCD_CUR_Y] = calc->hwregs[LCD_R51];
	}
}

static void set_loc(TilemCalc *calc)
{
	dword mode = calc->hwregs[LCD_R03];
	if (mode & R03_ORG)
		set_org(calc, mode);
	else {
		calc->hwregs[LCD_CUR_X] = calc->hwregs[LCD_R21];
		calc->hwregs[LCD_CUR_Y] = calc->hwregs[LCD_R20];
	}
}

/* Write an LCD register */
static void write_reg(TilemCalc* calc, byte index, word value)
{
	dword *reg;

	if ((reg = get_reg(calc, index))) {
		*reg = value & get_reg_mask(index);
		calc->z80.lastlcdwrite = calc->z80.clock;

		/* Setting either R20 or R21 resets both LCD_CUR_X and
		   LCD_CUR_Y. */
		if (index == 0x20 || index == 0x21)
			set_loc(calc);
		if (index >= 0x50 && index <= 0x53)
			set_org(calc, calc->hwregs[LCD_R03]);

		/* Turn display on/off */
		if (index == 0x07) {
			if ((value & R07_ENABLE0) && (value & R07_ENABLE1))
				calc->lcd.active = 1;
			else
				calc->lcd.active = 0;
		}

		if (index == 0x03) {
			if (value & R03_ORG)
				set_loc(calc);
		}
	}
	else if (!calc->hwregs[LCD_READ_STATE]) {
		tilem_warning(calc, _("writing to unknown LCD register %04x"),
		              calc->hwregs[LCD_REG_INDEX]);
	}
}

static dword get_pixel(TilemCalc* calc, dword mode)
{
	dword row = calc->hwregs[LCD_CUR_Y];
	dword col = calc->hwregs[LCD_CUR_X];
	const byte* restrict p;
	dword pixel;

	if (row >= HEIGHT) {
		tilem_warning(calc, "LCD row (%d) out of bounds", row);
		row %= HEIGHT;
	}
	if (col >= WIDTH) {
		tilem_warning(calc, "LCD column (%d) out of bounds", col);
		col %= WIDTH;
	}

	if (calc->hwregs[LCD_R01] & R01_FLIP_COLUMNS)
		row = HEIGHT - 1 - row;

	p = &calc->lcdmem[(row * WIDTH + col) * 3];
	if (mode & R03_BGR)
		pixel = (p[0] << 16 | p[1] << 8 | p[2]);
	else
		pixel = (p[2] << 16 | p[1] << 8 | p[0]);

	/* Note: reading does not change CUR_X/CUR_Y. */

	return pixel;
}

/* Read from LCD memory or register (port 11) */
byte xc_lcd_read(TilemCalc* calc)
{
	byte index = calc->hwregs[LCD_REG_INDEX] & 0xff;
	dword state = calc->hwregs[LCD_READ_STATE];
	dword pixel;
	word value;
	word mode;

	if (index == 0x22) {
		pixel = calc->hwregs[LCD_READ_BUFFER];
		value = (pixel >> 16);

		mode = calc->hwregs[LCD_R03];

		/* Note: TRI has no effect on reads.  There is
		   (apparently) no way for the CPU to read the least
		   significant bits of R and B channels. */

		state = !state;
		if (state == 0) {
			pixel = get_pixel(calc, mode);
			pixel = ((pixel & 0x3e0000) << 2
			         | (pixel & 0x3f00) << 5
			         | (pixel & 0x3e) << 7);
		}
		else {
			pixel <<= 8;
		}
		calc->hwregs[LCD_READ_STATE] = state;
		calc->hwregs[LCD_READ_BUFFER] = pixel;
		return value;
	}
	else {
		value = read_reg(calc, index);
		calc->hwregs[LCD_READ_STATE] = !state;
		if (state)
			return (value & 0xff);
		else
			return (value >> 8);
	}
}

static inline int update_col(TilemCalc *calc, dword mode)
{
	/* FIXME: need to test out-of-bounds behavior */

	if (mode & R03_COLINC) {
		if (calc->hwregs[LCD_CUR_X] < calc->hwregs[LCD_R53]) {
			calc->hwregs[LCD_CUR_X]++;
			return 0;
		}

		if (calc->hwregs[LCD_CUR_X] != calc->hwregs[LCD_R53])
			tilem_warning(calc, "LCD column (%d) past end of window (%d)",
			              calc->hwregs[LCD_CUR_X],
			              calc->hwregs[LCD_R53]);

		calc->hwregs[LCD_CUR_X] = calc->hwregs[LCD_R52];
		return 1;
	}
	else {
		if (calc->hwregs[LCD_CUR_X] > calc->hwregs[LCD_R52]) {
			calc->hwregs[LCD_CUR_X]--;
			return 0;
		}

		if (calc->hwregs[LCD_CUR_X] != calc->hwregs[LCD_R52])
			tilem_warning(calc, "LCD column (%d) past start of window (%d)",
			              calc->hwregs[LCD_CUR_X],
			              calc->hwregs[LCD_R52]);

		calc->hwregs[LCD_CUR_X] = calc->hwregs[LCD_R53];
		return 1;
	}
}

static inline int update_row(TilemCalc *calc, dword mode)
{
	if (mode & R03_ROWINC) {
		if (calc->hwregs[LCD_CUR_Y] < calc->hwregs[LCD_R51]) {
			calc->hwregs[LCD_CUR_Y]++;
			return 0;
		}

		if (calc->hwregs[LCD_CUR_Y] != calc->hwregs[LCD_R51])
			tilem_warning(calc, "LCD row (%d) past end of window (%d)",
			              calc->hwregs[LCD_CUR_Y],
			              calc->hwregs[LCD_R51]);

		calc->hwregs[LCD_CUR_Y] = calc->hwregs[LCD_R50];
		return 1;
	}
	else {
		if (calc->hwregs[LCD_CUR_Y] > calc->hwregs[LCD_R50]) {
			calc->hwregs[LCD_CUR_Y]--;
			return 0;
		}

		if (calc->hwregs[LCD_CUR_Y] != calc->hwregs[LCD_R50])
			tilem_warning(calc, "LCD row (%d) past start of window (%d)",
			              calc->hwregs[LCD_CUR_Y],
			              calc->hwregs[LCD_R50]);

		calc->hwregs[LCD_CUR_Y] = calc->hwregs[LCD_R51];
		return 1;
	}
}

static void put_pixel(TilemCalc* calc, dword mode, dword r, dword g, dword b)
{
	dword row = calc->hwregs[LCD_CUR_Y];
	dword col = calc->hwregs[LCD_CUR_X];
	byte* restrict p;

	if (row >= HEIGHT)
		row %= HEIGHT;
	if (col >= WIDTH)
		col %= WIDTH;

	if (calc->hwregs[LCD_R01] & R01_FLIP_COLUMNS)
		row = HEIGHT - 1 - row;

	p = &calc->lcdmem[(row * WIDTH + col) * 3];
	if (mode & R03_BGR) {
		p[0] = r & 0x3f;
		p[1] = g & 0x3f;
		p[2] = b & 0x3f;
	}
	else {
		p[0] = b & 0x3f;
		p[1] = g & 0x3f;
		p[2] = r & 0x3f;
	}

	calc->z80.lastlcdwrite = calc->z80.clock;

	if (mode & R03_COLFIRST) {
		if (update_col(calc, mode))
			update_row(calc, mode);
	}
	else {
		if (update_row(calc, mode))
			update_col(calc, mode);
	}
}

static void put_pixel16(TilemCalc* calc, dword mode, dword value)
{
	dword r, g, b;

	/* ILI9335 docs claim that R05 works as shown below (although
	   the description of mode 3 is incomplete.)  Experiments have
	   shown, however, that this is completely wrong, and "mode 2"
	   is used regardless of the value of R05.

	switch (calc->hwregs[LCD_R05] & R05_16TO18_MASK) {
	case R05_16TO18_FIXED_0:
		r = (value >> 10) & ~1;
		g = (value >> 5);
		b = (value << 1);
		break;

	case R05_16TO18_FIXED_1:
		r = (value >> 10) | 1;
		g = (value >> 5);
		b = (value << 1) | 1;
		break;

	case R05_16TO18_COPY_5:
		r = ((value >> 10) & ~1) | ((value >> 15) & 1);
		g = (value >> 5);
		b = (value << 1) | ((value >> 4) & 1);
		break;

	case R05_16TO18_COPY_G:
		r = (value >> 10) & ~1;
		g = (value >> 5);
		b = (value << 1);
		if (r == b) {
			r |= (g & 1);
			b |= (g & 1);
		}
		break;
	}
	*/

	r = ((value >> 10) & ~1) | ((value >> 15) & 1);
	g = (value >> 5);
	b = (value << 1) | ((value >> 4) & 1);

	put_pixel(calc, mode, r, g, b);
}

/* Write to LCD memory or register (port 11) */
void xc_lcd_write(TilemCalc* calc, byte val)
{
	byte index = calc->hwregs[LCD_REG_INDEX] & 0xff;
	dword state = calc->hwregs[LCD_WRITE_STATE];
	word mode;
	dword value, r, g, b;

	value = (calc->hwregs[LCD_WRITE_BUFFER] << 8 | val);
	calc->hwregs[LCD_WRITE_BUFFER] = value;

	if (index == 0x22) {
		mode = calc->hwregs[LCD_R03];
		if (mode & R03_18BIT) {
			/* 18-bit mode */
			state++;
			if (state >= 3) {
				state = 0;
				if (mode & R03_18BIT_UNPACKED) {
					value &= 0xfcfcfc;
					r = (value >> 18);
					g = (value >> 10);
					b = (value >> 2);
				}
				else {
					value &= 0x3ffff;
					r = (value >> 12);
					g = (value >> 6);
					b = value;
				}
				put_pixel(calc, mode, r, g, b);
			}
		}
		else {
			/* 16-bit mode */
			state = !state;
			if (state == 0)
				put_pixel16(calc, mode, value);
		}
		calc->hwregs[LCD_WRITE_STATE] = state;
	}
	else {
		calc->hwregs[LCD_WRITE_STATE] = !state;
		if (state)
			write_reg(calc, index, value);
	}
}

void xc_get_lcd(TilemCalc* calc, byte* data)
{
	int i, j, k;
	const byte* restrict p;
	unsigned x;

	p = calc->lcdmem;
	for (i = 0; i < 240; i++) {
		for (j = 0; j < 40; j++) {
			x = 0;
			for (k = 0; k < 8; k++) {
				if (!(p[(i+j+k*2)%3] & 0x20))
					x |= (0x80 >> k);
				p += 3;
			}
			*data = x;
			data++;
		}
	}
}

static inline void fill_floating(byte* restrict dest, int nbytes)
{
	int i;

	/* This will only work correctly if the same TilemLCDBuffer is
	   passed repeatedly & at regular intervals - I'm not sure
	   it's worth the effort to try to do this the "right way." */

	for (i = 0; i < nbytes; i++)
		if (dest[i] < 0x3f) dest[i]++;

	/*memset(dest, 0x3f, nbytes);*/
}

static inline void fill_nondisplay(byte* restrict dest, int nbytes,
                                   int ndl_black)
{
	memset(dest, (ndl_black ? 0x00 : 0x3f), nbytes);
}

static inline void fill_image(byte* restrict dest,
                              const byte* restrict src, int nbytes,
                              int flip_rows, int swap_channels,
                              int invert_levels, int msb_only)
{
	const byte* restrict r;
	const byte* restrict g;
	const byte* restrict b;
	unsigned int rev;
	int i;

	if (!(swap_channels || invert_levels)) {
		memcpy(dest, src, nbytes);
	}
	else {
		if (swap_channels) {
			b = src;
			g = src + 1;
			r = src + 2;
		}
		else {
			r = src;
			g = src + 1;
			b = src + 2;
		}

		rev = (invert_levels ? 0x3f : 0x00);

		if (flip_rows) {
			r += nbytes - 3;
			g += nbytes - 3;
			b += nbytes - 3;
			for (i = 0; i < nbytes; i += 3) {
				dest[i + 0] = r[-i] ^ rev;
				dest[i + 1] = g[-i] ^ rev;
				dest[i + 2] = b[-i] ^ rev;
			}
		}
		else {
			for (i = 0; i < nbytes; i += 3) {
				dest[i + 0] = r[i] ^ rev;
				dest[i + 1] = g[i] ^ rev;
				dest[i + 2] = b[i] ^ rev;
			}
		}
	}

	if (msb_only)
		for (i = 0; i < nbytes; i++)
			dest[i] = (dest[i] >> 5) * 0x3f;
}

static inline void fill_scrolled_image(byte* restrict dest,
                                       const byte* restrict src,
                                       int offs, int nbytes,
                                       int flip_rows, int swap_channels,
                                       int invert_levels, int msb_only)
{
	int n1, n2;

	if (offs > WIDTH * 3)
		offs %= WIDTH * 3;

	if (offs + nbytes > WIDTH * 3) {
		n1 = (WIDTH * 3) - offs;
		n2 = nbytes - n1;

		if (flip_rows) {
			fill_image(dest + n2, src + offs, n1, 1,
			           swap_channels, invert_levels, msb_only);
			fill_image(dest, src, n2, 1,
			           swap_channels, invert_levels, msb_only);
		}
		else {
			fill_image(dest, src + offs, n1, 0,
			           swap_channels, invert_levels, msb_only);
			fill_image(dest + n1, src, n2, 0,
			           swap_channels, invert_levels, msb_only);
		}
	}
	else {
		fill_image(dest, src + offs, nbytes, flip_rows, swap_channels,
		           invert_levels, msb_only);
	}
}

static inline void fill_row(byte* restrict dest,
                            const byte* restrict src,
                            int nfloat_start, int nfloat_end,
                            int imgpos1, int imgoffs1, int imgsize1,
                            int imgpos2, int imgoffs2, int imgsize2,
                            int flip_rows, int interlace_cols,
                            int swap_channels, int invert_levels,
                            int msb_only, int ndl_black)
{
	byte* restrict optr;
	byte ilbuf[WIDTH * 3];
	int i, n;

	if (interlace_cols)
		optr = ilbuf;
	else
		optr = dest;

	if (nfloat_start) {
		if (interlace_cols) /* FIXME */
			fill_nondisplay(optr, nfloat_start, 0);
		else
			fill_floating(optr, nfloat_start);
		optr += nfloat_start;
	}

	n = WIDTH * 3 - nfloat_start - nfloat_end;
	if (imgsize1 != n && imgsize2 != n) {
		fill_nondisplay(optr, n, ndl_black);
	}

	if (imgsize1) {
		fill_scrolled_image(optr + imgpos1, src, imgoffs1, imgsize1,
		                    flip_rows, swap_channels,
		                    invert_levels, msb_only);
	}

	if (imgsize2) {
		fill_scrolled_image(optr + imgpos2, src, imgoffs2, imgsize2,
		                    flip_rows, swap_channels,
		                    invert_levels, msb_only);
	}

	optr += n;

	if (nfloat_end) {
		if (interlace_cols) /* FIXME */
			fill_nondisplay(optr, nfloat_end, 0);
		else
			fill_floating(optr, nfloat_end);
		optr += nfloat_end;
	}

	if (interlace_cols) {
		optr = ilbuf;
		for (i = 0; i < WIDTH / 2; i++) {
			dest[0] = optr[0];
			dest[1] = optr[1];
			dest[2] = optr[2];
			dest[3] = optr[3 * WIDTH / 2 + 0];
			dest[4] = optr[3 * WIDTH / 2 + 1];
			dest[5] = optr[3 * WIDTH / 2 + 2];
			dest += 6;
			optr += 3;
		}
	}
}

#define SWAP(x, y) do { tmp = (x); (x) = (y); (y) = tmp; } while (0)

void xc_get_frame(TilemCalc* calc, TilemLCDBuffer* buf)
{
	const byte* restrict src;
	byte* restrict dest;
	int flip_rows, interlace_cols;
	int invert_levels, msb_only, ndl_black;
	int colstart, colcount;
	int p0pos, p0start, p0end, p0size, p1pos, p1start, p1end, p1size;
	int nfloat_start, nfloat_end;
	int imgpos1, imgoffs1, imgsize1;
	int imgpos2, imgoffs2, imgsize2;
	int i, tmp;

	if (TILEM_UNLIKELY(buf->height != HEIGHT
	                   || buf->rowstride != WIDTH * 3)) {
		/* reallocate data buffer */
		tilem_free(buf->data);
		buf->data = tilem_new_atomic(byte, WIDTH * HEIGHT * 3);
		buf->rowstride = WIDTH * 3;
		buf->height = HEIGHT;
		memset(buf->data, 0, WIDTH * HEIGHT * 3);
	}

	buf->format = TILEM_LCD_BUF_SRGB_63;
	buf->width = WIDTH;
	buf->stamp = calc->z80.lastlcdwrite;

	if (!calc->lcd.active || (calc->z80.halted && !calc->poweronhalt)) {
		/* screen is turned off */
		buf->contrast = 0;
		memset(buf->data, 0, buf->height * buf->rowstride);
		return;
	}

	buf->contrast = calc->lcd.contrast;

	ndl_black = (calc->hwregs[LCD_R61] & R61_NDL_BLACK);

	colstart = (calc->hwregs[LCD_R60] & R60_BASE_START_MASK) << 3;
	colcount = ((calc->hwregs[LCD_R60] & R60_BASE_NLINES_MASK) >> 5) + 8;
	if (colstart > WIDTH)
		colstart = WIDTH;
	if (colcount > WIDTH - colstart)
		colcount = WIDTH - colstart;

	nfloat_start = colstart * 3;
	nfloat_end = (WIDTH - (colstart + colcount)) * 3;

	if (calc->hwregs[LCD_R07] & R07_SHOW_BASE) {
		p1pos = 0;
		p1size = colcount;
		if (calc->hwregs[LCD_R61] & R61_SCROLL_ENABLED)
			p1start = (calc->hwregs[LCD_R6A] & R6A_SCROLL_MASK) * 3;
		else
			p1start = 0;

		p0pos = p0start = p0size = 0;
	}
	else {
		/* note: partial image 0 takes precedence over partial
		   image 1 */

		if (calc->hwregs[LCD_R07] & R07_SHOW_PARTIAL0) {
			p0pos = calc->hwregs[LCD_R80] & R80_P0_POS_MASK;
			if (p0pos > WIDTH)
				p0pos -= WIDTH;
			p0start = calc->hwregs[LCD_R81] & R81_P0_START_MASK;
			if (p0start > WIDTH)
				p0start -= WIDTH;
			p0end = calc->hwregs[LCD_R82] & R82_P0_END_MASK;
			if (p0end > WIDTH)
				p0end -= WIDTH;

			p0size = p0end + 1 - p0start;
			if (p0size < 0)
				p0size += WIDTH;

			if (p0pos > colcount)
				p0pos = colcount;
			if (p0pos + p0size > colcount)
				p0size = colcount - p0pos;
		}
		else {
			p0pos = p0start = p0size = 0;
		}

		if (calc->hwregs[LCD_R07] & R07_SHOW_PARTIAL1) {
			p1pos = calc->hwregs[LCD_R83] & R83_P1_POS_MASK;
			if (p1pos > WIDTH)
				p1pos -= WIDTH;
			p1start = calc->hwregs[LCD_R84] & R84_P1_START_MASK;
			if (p1start > WIDTH)
				p1start -= WIDTH;
			p1end = calc->hwregs[LCD_R85] & R85_P1_END_MASK;
			if (p1end > WIDTH)
				p1end -= WIDTH;

			p1size = p1end + 1 - p1start;
			if (p1size < 0)
				p1size += WIDTH;

			if (p1pos > colcount)
				p1pos = colcount;
			if (p1pos + p1size > colcount)
				p1size = colcount - p1pos;
		}
		else {
			p1pos = p1start = p1size = 0;
		}
	}

	flip_rows = (calc->hwregs[LCD_R60] & R60_FLIP_ROWS);
	if (flip_rows) {
		SWAP(nfloat_start, nfloat_end);
		p0pos = WIDTH - (p0pos + p0size);
		p1pos = WIDTH - (p1pos + p1size);
	}

	interlace_cols = (calc->hwregs[LCD_R01] & R01_INTERLACED);
	invert_levels = !(calc->hwregs[LCD_R61] & R61_LEVEL_INVERT);
	msb_only = (calc->hwregs[LCD_R07] & R07_8COLOR);

	src = calc->lcdmem;
	dest = buf->data;

	imgpos1 = p1pos * 3;
	imgoffs1 = p1start * 3;
	imgsize1 = p1size * 3;
	imgpos2 = p0pos * 3;
	imgoffs2 = p0start * 3;
	imgsize2 = p0size * 3;

	for (i = 0; i < HEIGHT; i++) {
		fill_row(dest, src,
		         nfloat_start, nfloat_end,
		         imgpos1, imgoffs1, imgsize1,
		         imgpos2, imgoffs2, imgsize2,
		         flip_rows, interlace_cols,
		         0, invert_levels, msb_only, ndl_black);

		dest += WIDTH * 3;
		src += WIDTH * 3;
	}

	if (nfloat_start || nfloat_end)
		buf->stamp = calc->z80.clock + 1;
}
