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

/* Scale the input buffer, multiply by F * INCOUNT, and add to the
   output buffer (which must be an exact multiple of the size of the
   input buffer.) */
static inline void add_scale1d_exact(const byte * /*restrict*/ in, int incount,
				     unsigned int * /*restrict*/ out,
				     int outcount, int f)
{
	int i, j;

	for (i = 0; i < incount; i++) {
		for (j = 0; j < outcount / incount; j++) {
			*out += *in * f * incount;
			out++;
		}
		in++;
	}
}

/* Scale a 1-dimensional buffer, multiply by F * INCOUNT, and add to
   the output buffer. */
static inline void add_scale1d_smooth(const byte * /*restrict*/ in, int incount,
				      unsigned int * /*restrict*/ out,
				      int outcount, int f)
{
	int in_rem, out_rem;
	byte inv;
	unsigned int outv;
	int i;

	in_rem = outcount;
	out_rem = incount;
	inv = *in;
	outv = 0;
	i = outcount;
	while (i > 0) {
		if (in_rem < out_rem) {
			out_rem -= in_rem;
			outv += in_rem * inv * f;
			inv = *++in;
			in_rem = outcount;
		}
		else {
			in_rem -= out_rem;
			outv += out_rem * inv * f;
			*out += outv;
			outv = 0;
			out++;
			out_rem = incount;
			i--;
		}
	}
}

/* Scale a 2-dimensional buffer, multiply by INWIDTH * INHEIGHT, and
   store in the output buffer. */
static void scale2d_smooth(const byte * /*restrict*/ in,
			   int inwidth, int inheight, int inrowstride,
			   unsigned int * /*restrict*/ out,
			   int outwidth, int outheight, int outrowstride)
{
	int in_rem, out_rem;
	int i;

	memset(out, 0, outrowstride * outheight * sizeof(int));

	in_rem = outheight;
	out_rem = inheight;
	i = outheight;
	while (i > 0) {
		if (in_rem < out_rem) {
			if (in_rem) {
				if (inwidth % outwidth)
					add_scale1d_smooth(in, inwidth, out,
							   outwidth, in_rem);
				else
					add_scale1d_exact(in, inwidth, out,
							  outwidth, in_rem);
			}
			out_rem -= in_rem;
			in += inrowstride;
			in_rem = outheight;
		}
		else {
			in_rem -= out_rem;
			if (inwidth % outwidth)
				add_scale1d_smooth(in, inwidth, out, outwidth,
						   out_rem);
			else
				add_scale1d_exact(in, inwidth, out, outwidth,
						  in_rem);
			out += outrowstride;
			out_rem = inheight;
			i--;
		}
	}
}

/* Quickly scale a 1-dimensional buffer and store in the output
   buffer. */
static inline void scale1d_fast(const byte * /*restrict*/ in, int incount,
				byte * /*restrict*/ out, int outcount)
{
	int i, e;

	e = outcount / 2;
	i = outcount;
	while (i > 0) {
		if (e >= 0) {
			*out = *in;
			out++;
			e -= incount;
			i--;
		}
		else {
			e += outcount;
			in++;
		}
	}
}

/* Quickly scale a 2-dimensional buffer and store in the output
   buffer. */
static void scale2d_fast(const byte * /*restrict*/ in,
			 int inwidth, int inheight, int inrowstride,
			 byte * /*restrict*/ out,
			 int outwidth, int outheight, int outrowstride)
{
	int i, e;

	e = outheight / 2;
	i = outheight;
	while (i > 0) {
		if (e >= 0) {
			scale1d_fast(in, inwidth, out, outwidth);
			out += outrowstride;
			e -= inheight;
			i--;
		}
		else {
			e += outheight;
			in += inrowstride;
		}
	}
}

#define GETSCALEBUF(ttt, www, hhh) \
	((ttt *) alloc_scalebuf(glcd, (www) * (hhh) * sizeof(ttt)))

static void* alloc_scalebuf(TilemGrayLCD *glcd, int size)
{
	if (size > glcd->scalebufsize) {
		glcd->scalebufsize = size;
		tilem_free(glcd->scalebuf);
		glcd->scalebuf = tilem_malloc_atomic(size);
	}

	return glcd->scalebuf;
}

void tilem_gray_lcd_draw_image_indexed(TilemGrayLCD *glcd,
				       byte * /*restrict*/ buffer,
				       int imgwidth, int imgheight,
				       int rowstride, int scaletype)
{
	int dwidth = glcd->calc->hw.lcdwidth;
	int dheight = glcd->calc->hw.lcdheight;
	int i, j;
	unsigned int *ibuf;

	if (scaletype == TILEM_SCALE_FAST
	    || (imgwidth % dwidth == 0 && imgheight % dheight == 0)) {
		scale2d_fast(glcd->levelbuf, dwidth, dheight, glcd->bwidth * 8,
			     buffer, imgwidth, imgheight, rowstride);
	}
	else {
		ibuf = GETSCALEBUF(unsigned int, imgwidth, imgheight);

		scale2d_smooth(glcd->levelbuf, dwidth, dheight,
			       glcd->bwidth * 8,
			       ibuf, imgwidth, imgheight, imgwidth);

		for (i = 0; i < imgheight; i++) {
			for (j = 0; j < imgwidth; j++)
				buffer[j] = ibuf[j] / (dwidth * dheight);
			ibuf += imgwidth;
			buffer += rowstride;
		}
	}
}

void tilem_gray_lcd_draw_image_rgb(TilemGrayLCD *glcd, byte * /*restrict*/ buffer,
				   int imgwidth, int imgheight, int rowstride,
				   int pixbytes, const dword * /*restrict*/ palette,
				   int scaletype)
{
	int dwidth =  glcd->calc->hw.lcdwidth;
	int dheight = glcd->calc->hw.lcdheight;
	int i, j, v;
	unsigned int *ibuf;
	byte *bbuf;

	if (scaletype == TILEM_SCALE_FAST
	    || (imgwidth % dwidth == 0 && imgheight % dheight == 0)) {
		bbuf = GETSCALEBUF(byte, imgwidth, imgheight);

		scale2d_fast(glcd->levelbuf, dwidth, dheight, glcd->bwidth * 8,
			     bbuf, imgwidth, imgheight, imgwidth);

		for (i = 0; i < imgheight; i++) {
			for (j = 0; j < imgwidth; j++) {
				v = bbuf[j];
				buffer[j * pixbytes] = palette[v] >> 16;
				buffer[j * pixbytes + 1] = palette[v] >> 8;
				buffer[j * pixbytes + 2] = palette[v];
			}
			bbuf += imgwidth;
			buffer += rowstride;
		}
	}
	else {
		ibuf = GETSCALEBUF(unsigned int, imgwidth, imgheight);

		scale2d_smooth(glcd->levelbuf, dwidth, dheight, glcd->bwidth * 8,
			       ibuf, imgwidth, imgheight, imgwidth);

		for (i = 0; i < imgheight; i++) {
			for (j = 0; j < imgwidth; j++) {
				v = ibuf[j] / (dwidth * dheight);
				buffer[j * pixbytes] = palette[v] >> 16;
				buffer[j * pixbytes + 1] = palette[v] >> 8;
				buffer[j * pixbytes + 2] = palette[v];
			}
			ibuf += imgwidth;
			buffer += rowstride;
		}
	}
}
