/*
 * libtilemcore - Graphing calculator emulation library
 *
 * Copyright (C) 2010 Benjamin Moody
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
#include "tilem.h"
#include "graylcd.h"

/* Read screen contents and update pixels that have changed */
static void tmr_screen_update(TilemCalc *calc, void *data)
{
	TilemGrayLCD *glcd = data;
	byte *np, *op, d;
	int i, j, n;
	dword delta;

	glcd->t++;

	if (calc->z80.lastlcdwrite == glcd->lcdupdatetime)
		return;
	glcd->lcdupdatetime = calc->z80.lastlcdwrite;

	(*calc->hw.get_lcd)(calc, glcd->newbits);

	np = glcd->newbits;
	op = glcd->oldbits;
	glcd->oldbits = np;
	glcd->newbits = op;
	n = 0;

	for (i = 0; i < glcd->bwidth * glcd->height; i++) {
		d = *np ^ *op;
		for (j = 0; j < 8; j++) {
			if (d & (0x80 >> j)) {
				delta = glcd->t - glcd->tchange[n];
				glcd->tchange[n] = glcd->t;

				if (*op & (0x80 >> j)) {
					glcd->curpixels[n].ndark += delta;
					glcd->curpixels[n].ndarkseg++;
				}
				else {
					glcd->curpixels[n].nlight += delta;
					glcd->curpixels[n].nlightseg++;
				}
			}
			n++;
		}

		np++;
		op++;
	}
}

TilemGrayLCD* tilem_gray_lcd_new(TilemCalc *calc, int windowsize, int sampleint)
{
	TilemGrayLCD *glcd = tilem_new(TilemGrayLCD, 1);
	int nbytes, npixels;

	glcd->bwidth = (calc->hw.lcdwidth + 7) / 8;
	glcd->height = calc->hw.lcdheight;
	nbytes = glcd->bwidth * glcd->height;
	npixels = nbytes * 8;

	glcd->oldbits = tilem_new_atomic(byte, nbytes);
	glcd->newbits = tilem_new_atomic(byte, nbytes);
	glcd->tchange = tilem_new_atomic(dword, npixels);
	glcd->tframestart = tilem_new_atomic(dword, windowsize);
	glcd->curpixels = tilem_new_atomic(TilemGrayLCDPixel, npixels);
	glcd->framebasepixels = tilem_new_atomic(TilemGrayLCDPixel,
						 npixels * windowsize);
	glcd->levelbuf = tilem_new_atomic(byte, npixels);

	memset(glcd->oldbits, 0, nbytes);
	memset(glcd->tchange, 0, npixels * sizeof(dword));
	memset(glcd->tframestart, 0, windowsize * sizeof(dword));
	memset(glcd->curpixels, 0, npixels * sizeof(TilemGrayLCDPixel));
	memset(glcd->framebasepixels, 0, (npixels * windowsize
					  * sizeof(TilemGrayLCDPixel)));

	glcd->calc = calc;
	glcd->timer_id = tilem_z80_add_timer(calc, sampleint / 2, sampleint, 1,
					     &tmr_screen_update, glcd);

	glcd->lcdupdatetime = calc->z80.lastlcdwrite - 1;
	glcd->t = 0;
	glcd->windowsize = windowsize;
	glcd->sampleint = sampleint;
	glcd->framenum = 0;

	glcd->scalebufsize = 0;
	glcd->scalebuf = NULL;

	return glcd;
}

void tilem_gray_lcd_free(TilemGrayLCD *glcd)
{
	tilem_z80_remove_timer(glcd->calc, glcd->timer_id);

	tilem_free(glcd->oldbits);
	tilem_free(glcd->newbits);
	tilem_free(glcd->tchange);
	tilem_free(glcd->tframestart);
	tilem_free(glcd->curpixels);
	tilem_free(glcd->framebasepixels);
	tilem_free(glcd->levelbuf);
	tilem_free(glcd->scalebuf);
	tilem_free(glcd);
}

/* Update levelbuf with the current, exact pixel values from LCD
   memory */
static void update_levels_mono(TilemGrayLCD* glcd, int min, int max)
{
	int i, j;
	byte *bp, *op;

	(*glcd->calc->hw.get_lcd)(glcd->calc, glcd->oldbits);
	bp = glcd->oldbits;
	op = glcd->levelbuf;

	for (i = 0; i < glcd->bwidth * glcd->height; i++) {
		for (j = 0; j < 8; j++) {
			if (*bp & (0x80 >> j))
				*op = max;
			else
				*op = min;
			op++;
		}
		bp++;
	}
}

/* Update levelbuf with values based on the accumulated grayscale
   data */
static void update_levels_gray(TilemGrayLCD *glcd, int min, int max)
{
	int i, j, n;
	unsigned int current, delta, fd, fl;
	word ndark, nlight, ndarkseg, nlightseg;
	byte *bp, *op;
	TilemGrayLCDPixel *pix, *basepix;
	int d = max - min;
	dword tbase, tlimit;

	(*glcd->calc->hw.get_lcd)(glcd->calc, glcd->oldbits);
	bp = glcd->oldbits;
	op = glcd->levelbuf;
	pix = glcd->curpixels;
	basepix = glcd->framebasepixels + (glcd->framenum * glcd->height
					   * glcd->bwidth * 8);
	n = 0;

	tbase = glcd->tframestart[glcd->framenum];
	glcd->tframestart[glcd->framenum] = glcd->t;
	tlimit = glcd->t - tbase;

	for (i = 0; i < glcd->bwidth * glcd->height; i++) {
		for (j = 0; j < 8; j++) {
			current = *bp & (0x80 >> j);

			ndark = pix[n].ndark - basepix[n].ndark;
			nlight = pix[n].nlight - basepix[n].nlight;
			ndarkseg = pix[n].ndarkseg - basepix[n].ndarkseg;
			nlightseg = pix[n].nlightseg - basepix[n].nlightseg;

			if (glcd->tchange[n] - tbase > tlimit) {
				glcd->tchange[n] = tbase;
			}

			delta = glcd->t - glcd->tchange[n];

			if (current) {
				if (delta * ndarkseg >= ndark) {
					ndark += delta;
					ndarkseg++;
				}
			}
			else {
				if (delta * nlightseg >= nlight) {
					nlight += delta;
					nlightseg++;
				}
			}

			fd = ndark * nlightseg;
			fl = nlight * ndarkseg;

			if (fd + fl == 0)
				*op = (ndark ? max : min);
			else
				*op = min + ((d * fd) / (fd + fl));

			n++;
			op++;
		}
		bp++;
	}

	memcpy(basepix, pix, (glcd->height * glcd->bwidth * 8
			      * sizeof(TilemGrayLCDPixel)));
}

void tilem_gray_lcd_next_frame(TilemGrayLCD *glcd, int mono)
{
	int min, max;

	if (!glcd->calc->lcd.active
	    || (glcd->calc->z80.halted && !glcd->calc->poweronhalt)) {
		min = max = 0;
	}
	else if (glcd->calc->lcd.contrast < 32) {
		min = 0;
		max = glcd->calc->lcd.contrast * 8;
	}
	else {
		min = (glcd->calc->lcd.contrast - 32) * 8;
		max = 255;
	}

	if (mono)
		update_levels_mono(glcd, min, max);
	else
		update_levels_gray(glcd, min, max);

	glcd->framenum = (glcd->framenum + 1) % glcd->windowsize;
}
