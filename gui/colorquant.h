/*
 * TilEm II
 *
 * Copyright (c) 2013 Benjamin Moody
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

typedef struct _ColorHistogram ColorHistogram;
typedef struct _ColorKDTree ColorKDTree;

typedef struct _ColorPalette {
	/* List of colors in the palette */
	int ncolors;
	dword colors[256];

	/* Set if palette is grayscale */
	int grayscale;

	/* Size of color cube.  If a color cube is used, it occupies
	   the first (nrvalues * ngvalues * nbvalues) entries in the
	   palette, and is stored in numeric order. */
	int nrvalues;
	int ngvalues;
	int nbvalues;

	/* Hash table mapping colors to palette indices */
	GHashTable *htab;

	/* K-D tree used to find closest color in the palette */
	ColorKDTree *tree;
} ColorPalette;

/* Create a new empty color histogram. */
ColorHistogram * color_histogram_new(void);

/* Free a color histogram. */
void color_histogram_free(ColorHistogram *hist);

/* Add pixels to histogram. */
void color_histogram_add_pixels(ColorHistogram *hist,
                                const byte *pixels, int npixels);

/* mark given colors as "fixed."  (If these colors are used, they will
   be considered higher-priority for palette assignment.) */
void color_histogram_add_fixed_colors(ColorHistogram *hist,
                                      const dword *colors, int ncolors);

/* Get number of distinct pixel colors. */
int color_histogram_num_colors(ColorHistogram *hist);

/* Generate a grayscale palette. */
ColorPalette * color_palette_new_gray(int maxcolors);

/* Generate an RGB palette.  If NRVALUES, NGVALUES, NBVALUES are
   nonzero, palette will include a color cube.  If HIST is non-null,
   palette entries not used for the color cube will be assigned based
   on the histogram. */
ColorPalette * color_palette_new_rgb(int maxcolors, ColorHistogram *hist,
                                     int nrvalues, int ngvalues, int nbvalues);

/* Free a palette. */
void color_palette_free(ColorPalette *pal);

/* Quantize an RGB image using the given color palette.  If palette
   includes a color cube, colors not found in the palette will be
   dithered.  Otherwise each color will be mapped to its nearest
   representative. */
void color_palette_quantize_image(ColorPalette *pal,
                                  byte * restrict buffer,
                                  const byte * restrict image,
                                  int width, int height, int rowstride);
