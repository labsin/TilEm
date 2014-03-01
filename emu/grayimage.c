/*
 * libtilemcore - Graphing calculator emulation library
 *
 * Copyright (C) 2010-2011 Benjamin Moody
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

/* Scale the input buffer, multiply by F * INCOUNT, and add to the
   output buffer (which must be an exact multiple of the size of the
   input buffer.) */
static inline void add_scale1d_exact(const byte * restrict in, int incount,
				     unsigned int * restrict out, int outcount,
                                     int f, int channels)
{
	int i, j, k;

	for (i = 0; i < incount; i++) {
		for (j = 0; j < outcount / incount; j++) {
			for (k = 0; k < channels; k++)
				out[k] += in[k] * f * incount;
			out += channels;
		}
		in += channels;
	}
}

/* Scale a 1-dimensional buffer, multiply by F * INCOUNT, and add to
   the output buffer. */
static inline void add_scale1d_smooth(const byte * restrict in, int incount,
				      unsigned int * restrict out, int outcount,
                                      int f, int channels)
{
	int in_rem, out_rem;
	unsigned int outv[3];
	int i, k;

	in_rem = outcount;
	out_rem = incount;
	outv[0] = outv[1] = outv[2] = 0;
	i = outcount;
	while (i > 0) {
		if (in_rem < out_rem) {
			out_rem -= in_rem;
			for (k = 0; k < channels; k++)
				outv[k] += in_rem * in[k] * f;
			in += channels;
			in_rem = outcount;
		}
		else {
			in_rem -= out_rem;
			for (k = 0; k < channels; k++) {
				outv[k] += out_rem * in[k] * f;
				out[k] += outv[k];
				outv[k] = 0;
			}
			out += channels;
			out_rem = incount;
			i--;
		}
	}
}

/* Scale a 2-dimensional buffer, multiply by INWIDTH * INHEIGHT, and
   store in the output buffer. */
static void scale2d_smooth(const byte * restrict in,
			   int inwidth, int inheight, int inrowstride,
			   unsigned int * restrict out,
                           int outwidth, int outheight, int outrowstride,
                           int channels)
{
	int in_rem, out_rem;
	int i;

	memset(out, 0, outrowstride * sizeof(int));

	in_rem = outheight;
	out_rem = inheight;
	i = outheight;
	while (i > 0) {
		if (in_rem < out_rem) {
			if (in_rem) {
				if (outwidth % inwidth)
					add_scale1d_smooth(in, inwidth, out,
					                   outwidth, in_rem,
					                   channels);
				else
					add_scale1d_exact(in, inwidth, out,
					                  outwidth, in_rem,
					                  channels);
			}
			out_rem -= in_rem;
			in += inrowstride;
			in_rem = outheight;
		}
		else {
			in_rem -= out_rem;
			if (outwidth % inwidth)
				add_scale1d_smooth(in, inwidth, out, outwidth,
				                   out_rem, channels);
			else
				add_scale1d_exact(in, inwidth, out, outwidth,
				                  out_rem, channels);
			out += outrowstride;
			out_rem = inheight;
			i--;
			if (i > 0)
				memset(out, 0, outrowstride * sizeof(int));
		}
	}
}

/* Quickly scale a 1-dimensional, 1-channel buffer and store in the output
   buffer. */
static inline void scale1d_fast(const byte * restrict in, int incount,
				byte * restrict out, int outcount)
{
	int i, e;

	e = outcount - incount / 2;
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

/* Quickly scale a 1-dimensional, 3-channel buffer, multiply by the
   given fixed-point factor, and store in the output buffer. */
static inline void scale1d_fast_rgb(const byte * restrict in, int incount,
                                    byte * restrict out, int outcount,
                                    int outpixbytes, int factor)
{
	int i, e;

	e = outcount - incount / 2;
	i = outcount;
	while (i > 0) {
		if (e >= 0) {
			out[0] = (in[0] * factor) >> 8;
			out[1] = (in[1] * factor) >> 8;
			out[2] = (in[2] * factor) >> 8;
			out += outpixbytes;
			e -= incount;
			i--;
		}
		else {
			e += outcount;
			in += 3;
		}
	}
}


/* Quickly scale a 2-dimensional buffer and store in the output
   buffer. */
static void scale2d_fast(const byte * restrict in,
                         int inwidth, int inheight, int inrowstride,
                         byte * restrict out,
                         int outwidth, int outheight, int outrowstride,
                         int outpixbytes, int rgbfact)
{
	int i, e;

	e = outheight - inheight / 2;
	i = outheight;
	while (i > 0) {
		if (e >= 0) {
			if (rgbfact)
				scale1d_fast_rgb(in, inwidth, out, outwidth,
				                 outpixbytes, rgbfact);
			else
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

/* Determine range of linear pixel values corresponding to a given
   contrast level. */
static void get_contrast_settings(unsigned int contrast,
                                  int *cbase, int *cfact)
{
	if (contrast < 32) {
		*cbase = 0;
		*cfact = contrast * 8;
	}
	else {
		*cbase = (contrast - 32) * 8;
		*cfact = 255 - *cbase;
	}
}

#define GETSCALEBUF(ttt, www, hhh) \
	((ttt *) alloc_scalebuf(buf, (www) * (hhh) * sizeof(ttt)))

static void* alloc_scalebuf(TilemLCDBuffer *buf, unsigned int size)
{
	if (TILEM_UNLIKELY(size > buf->tmpbufsize)) {
		buf->tmpbufsize = size;
		tilem_free(buf->tmpbuf);
		buf->tmpbuf = tilem_malloc_atomic(size);
	}

	return buf->tmpbuf;
}

void tilem_draw_lcd_image_indexed(TilemLCDBuffer * restrict buf,
                                  byte * restrict buffer,
                                  int imgwidth, int imgheight,
                                  int rowstride, int scaletype)
{
	int dwidth = buf->width;
	int dheight = buf->height;
	int i, j, v;
	unsigned int * restrict ibuf;
	int cbase, cfact;
	byte cindex[129];

	if (buf->format != TILEM_LCD_BUF_BLACK_128) {
		fprintf(stderr, "INTERNAL ERROR: cannot draw indexed image from RGB\n");
		return;
	}

	if (dwidth == 0 || dheight == 0 || buf->contrast == 0) {
		for (i = 0; i < imgheight; i++) {
			for (j = 0; j < imgwidth; j++)
				buffer[j] = 0;
			buffer += rowstride;
		}
		return;
	}

	get_contrast_settings(buf->contrast, &cbase, &cfact);

	for (i = 0; i <= 128; i++)
		cindex[i] = ((i * cfact) >> 7) + cbase;

	if (scaletype == TILEM_SCALE_FAST
	    || (imgwidth % dwidth == 0 && imgheight % dheight == 0)) {
		scale2d_fast(buf->data, dwidth, dheight, buf->rowstride,
		             buffer, imgwidth, imgheight, rowstride, 1, 0);

		for (i = 0; i < imgwidth * imgheight; i++)
			buffer[i] = cindex[buffer[i]];
	}
	else {
		ibuf = GETSCALEBUF(unsigned int, imgwidth, imgheight);

		scale2d_smooth(buf->data, dwidth, dheight, buf->rowstride,
		               ibuf, imgwidth, imgheight, imgwidth, 1);

		for (i = 0; i < imgheight; i++) {
			for (j = 0; j < imgwidth; j++) {
				v = ibuf[j] / (dwidth * dheight);
				buffer[j] = cindex[v];
			}
			ibuf += imgwidth;
			buffer += rowstride;
		}
	}
}

static void get_cpalette(TilemLCDBuffer * restrict buf,
                         dword *restrict cpalette,
                         const dword *restrict palette)
{
	int i, v, cbase, cfact;
	get_contrast_settings(buf->contrast, &cbase, &cfact);
	for (i = 0; i <= 128; i++) {
		v = ((i * cfact) >> 7) + cbase;
		cpalette[i] = palette[v];
	}
}

static void draw_rgb_from_gray_fast(TilemLCDBuffer * restrict buf,
                                    byte * restrict buffer,
                                    int imgwidth, int imgheight,
                                    int rowstride, int pixbytes,
                                    const dword * restrict palette)
{
	int dwidth = buf->width;
	int dheight = buf->height;
	int padbytes = rowstride - (imgwidth * pixbytes);
	int i, j, v;
	dword cpalette[129];
	byte * restrict bbuf;

	get_cpalette(buf, cpalette, palette);

	bbuf = GETSCALEBUF(byte, imgwidth, imgheight);

	scale2d_fast(buf->data, dwidth, dheight, buf->rowstride,
	             bbuf, imgwidth, imgheight, imgwidth, 1, 0);

	for (i = 0; i < imgheight; i++) {
		for (j = 0; j < imgwidth; j++) {
			v = bbuf[j];
			buffer[0] = cpalette[v] >> 16;
			buffer[1] = cpalette[v] >> 8;
			buffer[2] = cpalette[v];
			buffer += pixbytes;
		}
		bbuf += imgwidth;
		buffer += padbytes;
	}
}

static void draw_rgb_from_rgb_fast(TilemLCDBuffer * restrict buf,
                                   byte * restrict buffer,
                                   int imgwidth, int imgheight,
                                   int rowstride, int pixbytes)
{
	int dwidth = buf->width;
	int dheight = buf->height;
	int factor = buf->contrast * 32;

	if (factor >= (63335 / 63))
		factor = (65535 / 63);

	scale2d_fast(buf->data, dwidth, dheight, buf->rowstride,
	             buffer, imgwidth, imgheight, rowstride,
	             pixbytes, factor);
}

static void draw_rgb_from_gray_smooth(TilemLCDBuffer * restrict buf,
                                      byte * restrict buffer,
                                      int imgwidth, int imgheight,
                                      int rowstride, int pixbytes,
                                      const dword * restrict palette)
{
	int dwidth = buf->width;
	int dheight = buf->height;
	int padbytes = rowstride - (imgwidth * pixbytes);
	int i, j, v;
	dword cpalette[129];
	unsigned int * restrict ibuf;

	get_cpalette(buf, cpalette, palette);

	ibuf = GETSCALEBUF(unsigned int, imgwidth, imgheight);

	scale2d_smooth(buf->data, dwidth, dheight, buf->rowstride,
	               ibuf, imgwidth, imgheight, imgwidth, 1);

	for (i = 0; i < imgheight; i++) {
		for (j = 0; j < imgwidth; j++) {
			v = (ibuf[j] / (dwidth * dheight));
			buffer[0] = cpalette[v] >> 16;
			buffer[1] = cpalette[v] >> 8;
			buffer[2] = cpalette[v];
			buffer += pixbytes;
		}
		ibuf += imgwidth;
		buffer += padbytes;
	}
}

static void draw_rgb_from_rgb_smooth(TilemLCDBuffer * restrict buf,
                                     byte * restrict buffer,
                                     int imgwidth, int imgheight,
                                     int rowstride, int pixbytes)
{
	int dwidth = buf->width;
	int dheight = buf->height;
	int padbytes = rowstride - (imgwidth * pixbytes);
	int i, j, k, v;
	unsigned int * restrict ibuf;
	int factor = buf->contrast * 32;

	if (factor >= (63335 / 63))
		factor = (65535 / 63);

	ibuf = GETSCALEBUF(unsigned int, imgwidth * 3, imgheight);

	/* FIXME: should do scaling in linear space, not sRGB */

	scale2d_smooth(buf->data, dwidth, dheight, buf->rowstride,
	               ibuf, imgwidth, imgheight, imgwidth * 3, 3);

	for (i = 0; i < imgheight; i++) {
		for (j = 0; j < imgwidth; j++) {
			for (k = 0; k < 3; k++) {
				v = (ibuf[k] / (dwidth * dheight));
				buffer[k] = (v * factor) >> 8;
			}
			ibuf += 3;
			buffer += pixbytes;
		}
		buffer += padbytes;
	}
}

void tilem_draw_lcd_image_rgb(TilemLCDBuffer * restrict buf,
                              byte * restrict buffer,
                              int imgwidth, int imgheight, int rowstride,
                              int pixbytes, const dword * restrict palette,
                              int scaletype)
{
	int dwidth = buf->width;
	int dheight = buf->height;
	int i, j;
	int padbytes = rowstride - (imgwidth * pixbytes);
	dword blankcolor;

	if (dwidth == 0 || dheight == 0 || buf->contrast == 0) {
		if (buf->format == TILEM_LCD_BUF_BLACK_128)
			blankcolor = palette[0];
		else
			blankcolor = 0;

		for (i = 0; i < imgheight; i++) {
			for (j = 0; j < imgwidth; j++) {
				buffer[0] = blankcolor >> 16;
				buffer[1] = blankcolor >> 8;
				buffer[2] = blankcolor;
				buffer += pixbytes;
			}
			buffer += padbytes;
		}
		return;
	}

	if (scaletype == TILEM_SCALE_FAST
	    || (imgwidth % dwidth == 0 && imgheight % dheight == 0)) {

		if (buf->format == TILEM_LCD_BUF_BLACK_128)
			draw_rgb_from_gray_fast(buf, buffer,
			                        imgwidth, imgheight,
			                        rowstride, pixbytes,
			                        palette);
		else
			draw_rgb_from_rgb_fast(buf, buffer,
			                       imgwidth, imgheight,
			                       rowstride, pixbytes);
	}
	else {
		if (buf->format == TILEM_LCD_BUF_BLACK_128)
			draw_rgb_from_gray_smooth(buf, buffer,
			                          imgwidth, imgheight,
			                          rowstride, pixbytes,
			                          palette);
		else
			draw_rgb_from_rgb_smooth(buf, buffer,
			                         imgwidth, imgheight,
			                         rowstride, pixbytes);
	}
}
