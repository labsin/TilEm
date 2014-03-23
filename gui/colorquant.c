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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <glib.h>
#include <tilem.h>

#include "colorquant.h"

#ifndef HAVE_LROUND
# define lround(x) ((long) floor((x) + 0.5))
#endif

struct _ColorHistogram {
	GHashTable *pixel_colors;
	dword *fixed_colors;
	int n_fixed_colors;
};

typedef struct {
	dword color;
	int priority;
} ColorListCell;

typedef struct {
	int maxsize;
	int size;
	ColorListCell *c;
	ColorListCell *maxcell;
} ColorList, ColorHeap;

struct _ColorKDTree {
	dword color;
	dword value;
	ColorKDTree *child[2];
};

/**************** Hash tables ****************/

static GHashTable * color_hash_table_new()
{
	return g_hash_table_new(&g_direct_hash, &g_direct_equal);
}

static dword color_hash_table_get(GHashTable *table, dword c)
{
	gpointer p;
	p = g_hash_table_lookup(table, TILEM_DWORD_TO_PTR(c));
	return p ? TILEM_PTR_TO_DWORD(p) : 0;
}

static void color_hash_table_put(GHashTable *table, dword c, dword v)
{
	g_hash_table_insert(table, TILEM_DWORD_TO_PTR(c),
	                    TILEM_DWORD_TO_PTR(v));
}

/**************** Lists and heaps ****************/

/* Create a ColorList (or ColorHeap) able to hold up to MAXSIZE
   colors. */
static ColorList * color_list_new(int maxsize)
{
	ColorList *list = g_slice_new0(ColorList);
	list->c = g_new(ColorListCell, maxsize);
	list->maxsize = maxsize;
	return list;
}

/* Free a ColorList. */
static void color_list_free(ColorList *list)
{
	g_return_if_fail(list != NULL);
	g_free(list->c);
	g_slice_free(ColorList, list);
}

/* Append a color to a ColorList. */
static void color_list_append(ColorList *list, dword color, int priority)
{
	g_return_if_fail(list != NULL);
	g_return_if_fail(list->size < list->maxsize);

	list->c[list->size].color = color;
	list->c[list->size].priority = priority;
	list->size++;
}

#define HPARENT(idx) (((idx) - 1) / 2)
#define HCHILD0(idx) ((idx) * 2 + 1)
#define HCHILD1(idx) ((idx) * 2 + 2)

static inline int hprio(ColorHeap *heap, int i)
{
	if (i < 0)
		return G_MININT;
	else if (i >= heap->size)
		return G_MAXINT;
	else
		return heap->c[i].priority;
}

static inline void hswap(ColorList *heap, int i, int j)
{
	ColorListCell tmp;

	tmp = heap->c[i];
	heap->c[i] = heap->c[j];
	heap->c[j] = tmp;
}

/* Add a color to the ColorHeap, dropping the element with least
   priority when the heap grows too large. */
static void color_heap_add(ColorHeap *heap, dword color, int priority)
{
	int i;

	g_return_if_fail(heap != NULL);

	if (heap->size < heap->maxsize) {
		i = heap->size;
		heap->c[i].color = color;
		heap->c[i].priority = priority;
		heap->size++;

		while (hprio(heap, HPARENT(i)) > hprio(heap, i)) {
			hswap(heap, i, HPARENT(i));
			i = HPARENT(i);
		}
	}
	else if (priority > heap->c[0].priority) {
		heap->c[0].color = color;
		heap->c[0].priority = priority;

		i = 0;
		while (hprio(heap, i) > hprio(heap, HCHILD0(i))
		       || hprio(heap, i) > hprio(heap, HCHILD1(i))) {
			if (hprio(heap, HCHILD0(i)) < hprio(heap, HCHILD1(i)))
				i = HCHILD0(i);
			else
				i = HCHILD1(i);
			hswap(heap, i, HPARENT(i));
		}
	}
}

/* Remove element with least priority from the heap. */
static void color_heap_pop(ColorHeap *heap)
{
	int i;

	g_return_if_fail(heap != NULL);
	g_return_if_fail(heap->size > 0);

	heap->c[0] = heap->c[heap->size - 1];
	heap->size--;

	i = 0;
	while (hprio(heap, i) > hprio(heap, HCHILD0(i))
	       || hprio(heap, i) > hprio(heap, HCHILD1(i))) {
		if (hprio(heap, HCHILD0(i)) < hprio(heap, HCHILD1(i)))
			i = HCHILD0(i);
		else
			i = HCHILD1(i);
		hswap(heap, i, HPARENT(i));
	}
}

/* Convert heap (in place) into a sorted list. */
static void color_heap_sort(ColorHeap *heap)
{
	int n = heap->size;
	ColorListCell tmp;
	while (heap->size > 0) {
		tmp = heap->c[0];
		color_heap_pop(heap);
		heap->c[heap->size] = tmp;
	}
	heap->size = n;
}

/**************** K-D trees ****************/

/* Search for a given color in a ColorKDTree. */
static ColorKDTree * color_kdtree_find(ColorKDTree *tree, dword color)
{
	int axis;
	dword mask;

	while (tree) {
		for (axis = 0; axis < 24 && tree; axis += 8) {
			if (tree->color == color)
				return tree;

			mask = (0xff << axis);
			if ((tree->color & mask) >= (color & mask))
				tree = tree->child[0];
			else
				tree = tree->child[1];
		}
	}
	return NULL;
}

/* Generate a new balanced ColorKDTree from a list of colors. */
static ColorKDTree * color_kdtree_new_balanced(int axis, int ncolors,
                                               const dword *colors)
{
	ColorKDTree *tree;
	ColorHeap *heap;
	dword colors0[256], colors1[256];
	int i, nc0, nc1;

	g_return_val_if_fail(ncolors <= 256, NULL);

	if (ncolors == 0)
		return NULL;
	if (ncolors == 1) {
		tree = g_slice_new0(ColorKDTree);
		tree->color = colors[0];
		return tree;
	}

	/* find median of the list */

	heap = color_list_new((ncolors + 1) / 2);

	for (i = 0; i < ncolors; i++)
		color_heap_add(heap, colors[i], (colors[i] >> axis) & 0xff);

	/* divide into two sub-lists */

	nc0 = nc1 = 0;
	for (i = 0; i < ncolors; i++) {
		if (colors[i] == heap->c[0].color)
			continue;
			
		if (heap->c[0].priority >= (int) ((colors[i] >> axis) & 0xff))
			colors0[nc0++] = colors[i];
		else
			colors1[nc1++] = colors[i];
	}

	tree = g_slice_new0(ColorKDTree);
	tree->color = heap->c[0].color;
	color_list_free(heap);

	axis = (axis + 8) % 24;
	tree->child[0] = color_kdtree_new_balanced(axis, nc0, colors0);
	tree->child[1] = color_kdtree_new_balanced(axis, nc1, colors1);
	return tree;
}

/* Compute squared Euclidean distance between two colors */
static int color_distance(dword c1, dword c2)
{
	int i, x, y, d2;

	d2 = 0;
	for (i = 0; i < 24; i += 8) {
		x = (c1 >> i) & 0xff;
		y = (c2 >> i) & 0xff;
		d2 += (x - y) * (x - y);
	}
	return d2;
}

/* Search through a ColorKDTree for the closest color */
static void find_closest(ColorKDTree *tree, int axis, dword color,
                         ColorKDTree ** restrict best,
                         int * restrict best_distance)
{
	int d2, x, y, ch;

	if (!tree)
		return;

	if (tree->color == color) {
		*best = tree;
		*best_distance = 0;
		return;
	}

	x = (tree->color >> axis) & 0xff;
	y = (color >> axis) & 0xff;
	if (x >= y)
		ch = 0;
	else
		ch = 1;

	axis = (axis + 8) % 24;

	/* Search through "near" subtree first */
	find_closest(tree->child[ch], axis, color, best, best_distance);

	/* Search through "far" subtree only if the splitting plane is
	   closer than the best match found so far */
	if ((x - y) * (x - y) >= *best_distance)
		return;

	d2 = color_distance(color, tree->color);
	if (d2 < *best_distance) {
		*best_distance = d2;
		*best = tree;
	}

	find_closest(tree->child[!ch], axis, color, best, best_distance);
}

/* Find the color in the ColorKDTree that is closest (in Euclidean
   distance in RGB space) to the given color. */
static ColorKDTree * color_kdtree_find_closest(ColorKDTree *tree, dword color,
                                               int *distance)
{
	ColorKDTree *best = NULL;
	int best_distance = G_MAXINT;

	find_closest(tree, 0, color, &best, &best_distance);
	if (distance)
		*distance = best_distance;
	return best;
}

/* Free a ColorKDTree. */
static void color_kdtree_free(ColorKDTree *tree)
{
	if (tree) {
		color_kdtree_free(tree->child[0]);
		color_kdtree_free(tree->child[1]);
		g_slice_free(ColorKDTree, tree);
	}
}

/**************** ColorHistogram ****************/

ColorHistogram * color_histogram_new()
{
	ColorHistogram *hist = g_slice_new0(ColorHistogram);
	hist->pixel_colors = color_hash_table_new();
	return hist;
}

void color_histogram_free(ColorHistogram *hist)
{
	g_return_if_fail(hist != NULL);
	g_hash_table_destroy(hist->pixel_colors);
	g_free(hist->fixed_colors);
	g_slice_free(ColorHistogram, hist);
}

void color_histogram_add_pixels(ColorHistogram *hist,
                                const byte *pixels, int npixels)
{
	int i;
	dword c, n;

	g_return_if_fail(hist != NULL);
	g_return_if_fail(pixels != NULL);

	for (i = 0; i < npixels; i++) {
		c = (pixels[3 * i] << 16
		     | pixels[3 * i + 1] << 8
		     | pixels[3 * i + 2]);

		n = color_hash_table_get(hist->pixel_colors, c);
		if (n + 1 != 0)
			color_hash_table_put(hist->pixel_colors, c, n + 1);
	}
}

void color_histogram_add_fixed_colors(ColorHistogram *hist,
                                      const dword *colors, int ncolors)
{
	int i;

	g_return_if_fail(hist != NULL);
	g_return_if_fail(colors != NULL);

	hist->fixed_colors = g_renew(dword, hist->fixed_colors,
	                             hist->n_fixed_colors + ncolors);
	for (i = 0; i < ncolors; i++)
		hist->fixed_colors[hist->n_fixed_colors + i] = colors[i];
	hist->n_fixed_colors += ncolors;
}

int color_histogram_num_colors(ColorHistogram *hist)
{
	g_return_val_if_fail(hist != NULL, 0);
	return g_hash_table_size(hist->pixel_colors);
}

/**************** ColorPalette ****************/

/* Add colors from the input tree to a ColorHeap, sorting by
   popularity.  The resulting heap will contain the N most popular
   colors, starting with the least popular of these. */
static void fill_popularity_heap(gpointer k, gpointer v, gpointer data)
{
	dword color = TILEM_PTR_TO_DWORD(k);
	dword popularity = TILEM_PTR_TO_DWORD(v);
	ColorHeap *heap = data;
	color_heap_add(heap, color, popularity);
}

/* Make a list of the N most popular colors in the hash table. */
static ColorList * get_popularity_list(GHashTable *table, int maxcount)
{
	ColorList *list = color_list_new(maxcount);
	g_hash_table_foreach(table, &fill_popularity_heap, list);
	color_heap_sort(list);
	return list;
}

struct dlinfo {
	ColorList *list;
	ColorKDTree *reftree;
};

/* Add colors from the input tree to an unsorted ColorList, setting
   priority of each color to the distance between that color and the
   given tree of reference colors. */
static void fill_distance_list(gpointer k, G_GNUC_UNUSED gpointer v, gpointer data)
{
	dword color = TILEM_PTR_TO_DWORD(k);
	struct dlinfo *dli = data;
	int distance;
	color_kdtree_find_closest(dli->reftree, color, &distance);
	color_list_append(dli->list, color, distance);
	if (!dli->list->maxcell || distance > dli->list->maxcell->priority)
		dli->list->maxcell = &dli->list->c[dli->list->size - 1];
}

/* Make a list of all colors in the hash table and their distances to
   the given reference tree.  Set maxcell to the most distant. */
static ColorList * get_distance_list(GHashTable *table, ColorKDTree *reftree)
{
	struct dlinfo dli;
	dli.list = color_list_new(g_hash_table_size(table));
	dli.reftree = reftree;
	g_hash_table_foreach(table, &fill_distance_list, &dli);
	return dli.list;
}

/* Update distance list after a color has been added to the reference
   tree. */
static void update_distance_list(ColorList *list, dword newcolor)
{
	int i, distance, maxdistance = 0;

	list->maxcell = NULL;

	for (i = 0; i < list->size; i++) {
		if (list->c[i].color == newcolor)
			list->c[i].priority = 0;
		if (list->c[i].priority == 0)
			continue;

		distance = color_distance(list->c[i].color, newcolor);
		if (list->c[i].priority > distance)
			list->c[i].priority = distance;

		if (!list->maxcell || list->c[i].priority > maxdistance) {
			list->maxcell = &list->c[i];
			maxdistance = list->c[i].priority;
		}
	}
}

/* Add a color to the palette if it hasn't been added already. */
static int add_color(ColorPalette *pal, dword color)
{
	if (color_hash_table_get(pal->htab, color))
		return 0;

	g_return_val_if_fail(pal->ncolors < 256, 1);
	pal->colors[pal->ncolors] = color;
	color_hash_table_put(pal->htab, color, pal->ncolors + 1);
	pal->ncolors++;
	return 1;
}

/* Generate a balanced K-D tree for the palette. */
static void gen_tree(ColorPalette *pal)
{
	int i;
	ColorKDTree *t;

	color_kdtree_free(pal->tree);
	pal->tree = color_kdtree_new_balanced(0, pal->ncolors, pal->colors);
	for (i = 0; i < pal->ncolors; i++) {
		t = color_kdtree_find(pal->tree, pal->colors[i]);
		if (t)
			t->value = i;
	}
}

ColorPalette * color_palette_new_gray(int maxcolors)
{
	ColorPalette *pal;
	int i, v;

	g_return_val_if_fail(maxcolors >= 2, NULL);
	g_return_val_if_fail(maxcolors <= 256, NULL);

	pal = g_slice_new0(ColorPalette);
	pal->htab = color_hash_table_new();
	pal->grayscale = 1;
	for (i = 0; i < maxcolors; i++) {
		v = (i * 255 + (maxcolors - 1) / 2) / (maxcolors - 1);
		add_color(pal, v * 0x010101);
	}
	return pal;
}

ColorPalette * color_palette_new_rgb(int maxcolors, ColorHistogram *hist,
                                     int nrvalues, int ngvalues, int nbvalues)
{
	ColorPalette *pal;
	ColorList *poplist, *distlist;
	int n, i, j, k;
	byte rval[256], gval[256], bval[256];
	dword c;

	g_return_val_if_fail(maxcolors > 0, NULL);
	g_return_val_if_fail(maxcolors <= 256, NULL);
	g_return_val_if_fail(nrvalues != 1, NULL);
	g_return_val_if_fail(ngvalues != 1, NULL);
	g_return_val_if_fail(nbvalues != 1, NULL);

	n = nrvalues * ngvalues * nbvalues;
	g_return_val_if_fail(n >= 0, NULL);
	g_return_val_if_fail(n <= maxcolors, NULL);
	if (n == 0)
		nrvalues = ngvalues = nbvalues = 0;

	pal = g_slice_new0(ColorPalette);
	pal->htab = color_hash_table_new();

	/* generate color cube */

	pal->nrvalues = nrvalues;
	pal->ngvalues = ngvalues;
	pal->nbvalues = nbvalues;

	for (i = 0; i < nrvalues; i++)
		rval[i] = (i * 255 + (nrvalues - 1) / 2) / (nrvalues - 1);
	for (i = 0; i < ngvalues; i++)
		gval[i] = (i * 255 + (ngvalues - 1) / 2) / (ngvalues - 1);
	for (i = 0; i < nbvalues; i++)
		bval[i] = (i * 255 + (nbvalues - 1) / 2) / (nbvalues - 1);

	for (i = 0; i < nrvalues; i++) {
		for (j = 0; j < ngvalues; j++) {
			for (k = 0; k < nbvalues; k++) {
				c = (rval[i] << 16 | gval[j] << 8 | bval[k]);
				add_color(pal, c);
			}
		}
	}

	if (!hist)
		return pal;

	/* add fixed colors */

	for (i = 0; i < hist->n_fixed_colors; i++) {
		if (pal->ncolors >= maxcolors)
			break;
		c = hist->fixed_colors[i];
		if (color_hash_table_get(hist->pixel_colors, c))
			add_color(pal, c);
	}

	if (pal->ncolors >= maxcolors)
		return pal;

	/* find most popular colors */

	poplist = get_popularity_list(hist->pixel_colors, maxcolors);

	/* if using a color cube, fill remaining entries with most
	   popular colors */
	if (nrvalues * ngvalues * nbvalues > 0) {
		i = 0;
		while (pal->ncolors < maxcolors && i < poplist->size) {
			add_color(pal, poplist->c[i].color);
			i++;
		}
	}
	/* otherwise, use modified diversity algorithm - alternate
	   between picking most-popular and least-well-represented
	   color */
	else {
		gen_tree(pal);
		distlist = get_distance_list(hist->pixel_colors, pal->tree);
		i = 0;

		while (i < poplist->size && pal->ncolors < maxcolors) {
			while (i < poplist->size) {
				c = poplist->c[i++].color;
				if (add_color(pal, c)) {
					update_distance_list(distlist, c);
					break;
				}
			}

			if (pal->ncolors >= maxcolors)
				break;

			if (distlist->maxcell) {
				c = distlist->maxcell->color;
				add_color(pal, c);
				update_distance_list(distlist, c);
			}
			else {
				break;
			}
		}

		color_list_free(distlist);
		gen_tree(pal);
	}

	color_list_free(poplist);

	return pal;
}

void color_palette_free(ColorPalette *pal)
{
	g_return_if_fail(pal != NULL);
	color_kdtree_free(pal->tree);
	if (pal->htab)
		g_hash_table_destroy(pal->htab);
	g_slice_free(ColorPalette, pal);
}

/**************** Quantization ****************/

#define DITHER_WIDTH 32
#define DITHER_HEIGHT 32
#define DITHER_MAX (DITHER_WIDTH * DITHER_HEIGHT)

static const gushort dither_pattern[DITHER_WIDTH * DITHER_HEIGHT] = {
    0, 604, 317, 775, 669, 879, 399, 923, 762, 987, 481, 258, 138,1009,  36, 857, 249, 978, 797, 513, 753, 137, 679, 827, 459, 543, 172, 361, 594, 868, 190, 417,
  241, 860, 159, 964, 266,  32, 619, 100, 438, 181, 728, 351, 787, 657, 441, 591, 683, 113, 423, 184,1005, 397, 550, 339, 125, 893, 736, 963, 490, 106, 665, 782,
  527, 458, 730, 369, 578, 474,1023, 329, 552, 666,   7, 931, 522,  93, 310, 202, 936, 332, 878, 634, 281,  49, 779, 948, 254, 628,  16, 312, 221, 910, 374,1008,
  124, 654,  65, 900, 135, 828, 206, 780, 916, 248, 846, 426, 175, 973, 826, 735, 472, 557,  61, 732, 495, 861, 613,  77, 500, 841, 414, 689, 809, 614,  54, 296,
  930, 803, 234, 446, 712, 306, 642,  66, 493, 134, 602, 293, 703, 572, 387,  22, 158, 814, 247, 968, 151, 437, 212, 716, 359, 164,1018, 534, 144, 435, 756, 566,
  411, 324, 584, 982,  18, 535, 947, 418, 744, 349, 965, 776,  55, 232, 903, 647,1020, 415, 702, 357, 580, 912, 322, 972, 551, 765, 278,  64, 886, 255, 970, 201,
  664, 877, 150, 771, 358, 870, 166, 264, 660, 865,  96, 529, 456, 843, 338, 488, 213, 600, 101, 850,  11, 670, 121, 820,  39, 639, 919, 377, 608, 476, 714,  23,
  520,  83, 482, 678, 246, 626, 505, 813,  27, 565, 209, 366, 992, 131, 752,  70, 800, 314, 945, 477, 263, 778, 510, 409, 237, 462, 132, 734, 839,  97, 343, 824,
  914, 273,1015, 416,  63, 960, 120, 398,1010, 323, 905, 688, 609, 277, 559, 675, 915, 526, 179, 650, 386, 980, 183, 874, 696, 989, 315, 523, 207, 995, 593, 165,
  370, 629, 785, 163, 842, 724, 294, 605, 711, 467, 127, 819,  14, 873, 432, 156, 384,  35, 832, 738,  80, 538, 630, 345,  85, 555, 807,   6, 681, 287, 434, 725,
  214,  30, 563, 337, 469, 553, 927, 192,  78, 788, 240, 401, 509, 205, 942, 761, 270,1000, 442, 236, 891, 297,  25, 740, 894, 167, 403, 925, 471, 859,  62, 959,
  501, 872, 697, 944, 229,   5, 368, 853, 525, 966, 641, 737,1004, 330, 635,  92, 541, 620, 710, 123, 586, 793,1016, 439, 256, 606, 768, 228, 633, 146, 548, 792,
   98, 420, 265, 111, 829, 623, 759, 259, 424,  38, 300, 147, 564,  53, 795, 463, 858, 196, 325, 953, 487, 364, 143, 680, 506, 955,  51, 336,1013, 393, 691, 313,
  631,1002, 770, 514, 400, 986, 140, 682, 883, 592, 922, 378, 848, 707, 235, 354, 979,   4, 413, 806,  59, 643, 224, 847,  94, 383, 720, 562, 104, 834, 233, 898,
  174, 346,  41, 659, 188, 568, 321, 484, 107, 225, 773, 497,  88, 444, 924, 141, 651, 741, 536, 896, 275, 760, 921, 556, 302, 801, 182, 913, 449, 748,  19, 473,
  723, 545, 928, 286, 869, 729,  47, 799,1014, 352, 624, 185, 984, 288, 598, 825, 498, 260, 168, 603, 116, 475, 390,  15, 998, 625, 486, 261, 655, 326, 569, 956,
  402, 119, 816, 451,  99, 376, 909, 540, 421, 700,  13, 876, 547, 690,  40, 406,  91, 866, 950, 355, 721, 977, 194, 698, 425, 133, 867,  44, 988, 162, 855, 257,
  774, 617, 227, 692, 976, 607, 195, 268, 114, 932, 304, 751, 380, 215, 791,1022, 307, 673, 450,  28, 837, 309, 588, 887, 274, 733, 542, 371, 777, 431, 677,  60,
  333, 997, 528,  10, 318, 478, 833, 658, 781, 573, 171, 480, 103, 901, 521, 149, 585, 764, 204, 531, 652,  79, 494, 157, 817,  72, 940, 198, 596, 108, 517, 884,
  483, 153, 373, 897, 742, 130, 952,  34, 454, 353, 967, 831, 621, 283, 722, 455, 935,  67, 379, 990, 262, 789, 958, 362, 601, 457, 335, 667, 838, 290, 962, 219,
   43, 701, 836, 579, 218, 405, 558, 299, 906, 222, 686,  58, 410, 981,  21, 356, 252, 804, 615, 885, 118, 429, 684,  26, 911, 715, 239,1011,   3, 396, 644, 747,
  888, 427, 282,  71,1019, 672, 811,  95, 727, 590, 139, 516, 758, 211, 577, 895, 671, 170, 464, 327, 726, 216, 511, 316, 180, 805, 110, 537, 470, 786, 122, 554,
  993, 193, 636, 755, 466, 340, 189, 485, 854, 375,1012, 289, 882, 115, 818, 503,  74,1007, 567,   8, 821, 595, 863, 999, 570, 412, 638, 852, 173, 920, 243, 341,
   86, 802, 533, 136, 862,  42, 961, 632, 253,   2, 739, 461, 637, 388, 705, 301, 404, 766, 267, 907, 391, 160,  73, 749, 269,  45, 949, 291, 363, 708, 610, 465,
  685, 292, 382, 939, 245, 575, 717, 407, 926, 544, 177, 941,  52, 231, 983, 148, 871, 653, 187, 508, 706, 975, 468, 367, 674, 499, 763,  75, 549, 969,  29, 849,
  507, 971,  12, 695, 443, 815, 161, 320, 102, 794, 687, 331, 576, 783, 491, 599,  31, 448, 957,  89, 311, 622, 230, 830, 109, 892, 226, 430, 822, 155, 408, 217,
  618, 169, 772, 546, 328,  68,1006, 524, 875, 453, 244, 845, 422, 105, 889, 271, 754, 342, 581, 798, 890,  24, 532, 934, 186, 589, 718,1021, 648, 272, 784, 904,
   90, 447, 881, 208, 917, 656, 743, 203, 611,  46, 649, 142,1003, 709, 372, 197, 937, 676, 145, 250, 433, 757, 348, 661, 440, 308,   9, 347, 112, 479, 713, 334,
  991, 276, 663, 392, 126, 489, 284, 419, 974, 344, 929, 530, 303,  17, 640, 835, 512,  56,1017, 539, 699, 178, 994,  69, 796, 946, 504, 851, 571, 933,  37, 587,
  492, 750,  48, 808, 612, 943,  20, 790, 129, 694, 220, 745, 460, 918, 561, 128, 428, 295, 823, 389,  87, 856, 285, 518, 154, 616, 242, 731, 176, 395, 238, 840,
  350, 199, 560,1001, 251, 360, 844, 574, 502, 864, 394,  81, 810, 191, 280, 951, 769, 662, 200, 908, 646, 452, 597, 746, 385, 880,  82, 445, 996, 645, 767, 117,
  693, 902, 436,  84, 515, 152, 704, 210, 305,  57, 627, 899, 582, 381, 719, 496,  76, 365, 583,   1, 319, 938, 223,  33, 985, 279, 668, 812,  50, 298, 519, 954
};

/* Generate a table for converting sRGB values to linear RGB */
static gpointer init_srgb_conv_table(gpointer data)
{
	int i;
	int *rgbtab = data;
	double r, lr;

	for (i = 0; i < 256; i++) {
		r = i / 255.0;
		lr = (r < 0.04045
		      ? r / 12.92
		      : pow((r + 0.055) / 1.055, 2.4));
		rgbtab[i] = lround(lr * 65536);
	}
	return NULL;
}

static G_GNUC_CONST const int * get_srgb_conv_table()
{
	static GOnce once = G_ONCE_INIT;
	static int rgbtab[256];
	g_once(&once, &init_srgb_conv_table, rgbtab);
	return rgbtab;
}

/* Convert sRGB color to (scaled) CIE luminance */
static inline int srgb_to_y(dword c)
{
	const int *rgbtab = get_srgb_conv_table();
	return (rgbtab[(c >> 16) & 0xff] * 2126
	        + rgbtab[(c >> 8) & 0xff] * 7152
	        + rgbtab[c & 0xff] * 722);
}

static inline int dither_coord(guint value, const guint *levels,
                               int nlevels, guint dither)
{
	int i;

	for (i = 0; i < nlevels - 1; i++)
		if (levels[i + 1] > value)
			break;
	if (i >= nlevels - 1)
		return i;

	value += ((dither * (levels[i + 1] - levels[i]))
	          / (DITHER_WIDTH * DITHER_HEIGHT));

	if (value >= levels[i + 1])
		return i + 1;
	else
		return i;
}

static inline unsigned int quantize_pixel(dword color, ColorPalette *pal,
                                          int vpos, int hpos,
                                          const guint *rlevels,
                                          const guint *glevels,
                                          const guint *blevels)
{
	guint i, rv, gv, bv, ri, gi, bi, dither, y, a, b, c;
	ColorKDTree *t;
	const int *rgbtab;

	if (pal->nrvalues) {
		i = color_hash_table_get(pal->htab, color);
		if (i)
			return i - 1;

		hpos &= DITHER_WIDTH - 1;
		vpos &= DITHER_HEIGHT - 1;
		dither = dither_pattern[vpos * DITHER_WIDTH + hpos];

		rgbtab = get_srgb_conv_table();
		rv = rgbtab[(color >> 16) & 0xff];
		gv = rgbtab[(color >> 8) & 0xff];
		bv = rgbtab[color & 0xff];
		ri = dither_coord(rv, rlevels, pal->nrvalues, dither);
		gi = dither_coord(gv, glevels, pal->ngvalues,
		                  DITHER_MAX - dither - 1);
		bi = dither_coord(bv, blevels, pal->nbvalues, dither);

		return (ri * pal->ngvalues * pal->nbvalues
		        + gi * pal->nbvalues
		        + bi);
	}
	else if (pal->grayscale) {
		y = srgb_to_y(color);
		a = 0;
		b = pal->ncolors - 1;
		while (a + 1 < b) {
			c = (a + b) / 2;
			if (y == rlevels[c])
				return c;
			else if (y > rlevels[c])
				a = c;
			else
				b = c;
		}
		if (y - rlevels[a] < rlevels[a + 1] - y)
			return a;
		else
			return a + 1;
	}
	else {
		i = color_hash_table_get(pal->htab, color);
		if (i)
			return i - 1;

		t = color_kdtree_find_closest(pal->tree, color, NULL);
		g_return_val_if_fail(t != NULL, 0);
		color_hash_table_put(pal->htab, color, t->value + 1);
		return t->value;
	}
}

void color_palette_quantize_image(ColorPalette *pal,
                                  byte * restrict buffer,
                                  const byte * restrict image,
                                  int width, int height, int rowstride)
{
	const int *rgbtab = get_srgb_conv_table();
	guint rlevels[256], glevels[256], blevels[256];
	int i, j;
	dword c;

	g_return_if_fail(pal != NULL);
	g_return_if_fail(image != NULL);
	g_return_if_fail(buffer != NULL);

	/* convert color cube to linear R, G, B coordinates */

	for (i = 0; i < pal->nrvalues; i++) {
		c = pal->colors[i * pal->ngvalues * pal->nbvalues];
		rlevels[i] = rgbtab[(c >> 16) & 0xff];
	}
	for (i = 0; i < pal->ngvalues; i++) {
		c = pal->colors[i * pal->nbvalues];
		glevels[i] = rgbtab[(c >> 8) & 0xff];
	}
	for (i = 0; i < pal->nbvalues; i++) {
		c = pal->colors[i];
		blevels[i] = rgbtab[c & 0xff];
	}

	/* grayscale: convert palette to linear luminance */

	if (pal->grayscale)
		for (i = 0; i < pal->ncolors; i++)
			rlevels[i] = srgb_to_y(pal->colors[i]);

	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			c = (image[3 * j] << 16
			     | image[3 * j + 1] << 8
			     | image[3 * j + 2]);
			*buffer = quantize_pixel(c, pal, i, j, rlevels,
			                         glevels, blevels);
			buffer++;
		}
		image += rowstride;
	}
}
