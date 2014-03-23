/*
 * Generate a dithering pattern
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

/* This code implements a simplified version of Robert Ulichney's
   void-and-cluster algorithm (instead of starting from a random
   pattern, we start with a single pixel.)  The full void-and-cluster
   algorithm is currently patented in the US. */

#define WIDTH 32
#define HEIGHT 32
#define SIGMA 1.5

#define IND(xx, yy) (((xx) + WIDTH) % WIDTH + (((yy) + HEIGHT) % HEIGHT) * WIDTH)

static double pixel_weight(const int *order, int x, int y)
{
	int i, j;
	double f = 0, k;

	k = -0.5 / (SIGMA * SIGMA);

	for (j = -((HEIGHT-1)/2); j <= (HEIGHT-1)/2; j++) {
		for (i = -((WIDTH-1)/2); i <= (WIDTH-1)/2; i++) {
			if (i == 0 && j == 0)
				continue;
			if (order[IND(x + i, y + j)] >= 0)
				continue;
			f += exp(((i * i) + (j * j)) * k);
		}
	}
	return f;
}

static int next_pixel(const int *order)
{
	int best, x, y;
	double bestweight, w;

	best = -1;
	bestweight = -1;
	for (y = 0; y < HEIGHT; y++) {
		for (x = 0; x < WIDTH; x++) {
			if (order[IND(x, y)] >= 0)
				continue;

			w = pixel_weight(order, x, y);
			if (w >= bestweight) {
				bestweight = w;
				best = IND(x, y);
			}
		}
	}

	return best;
}

static void dumporder(const int *order)
{
	int x, y;

	for (y = 0; y < HEIGHT; y++) {
		if (y)
			printf(",\n");
		for (x = 0; x < WIDTH; x++) {
			putchar(x ? ',' : ' ');
			if (order[IND(x, y)] >= 0)
				printf("%4d", order[IND(x, y)]);
			else
				printf("    ");
		}
	}
	printf("\n");
}

int main()
{
	int order[WIDTH * HEIGHT];
	int i, n;

	for (i = 0; i < WIDTH * HEIGHT; i++)
		order[i] = -1;

	order[0] = 0;
	n = 1;
	while ((i = next_pixel(order)) >= 0) {
		assert(n < WIDTH * HEIGHT);
		assert(order[i] < 0);
		order[i] = n;
		n++;

		/*dumporder(order);*/
		if (n % 10 == 0) {
			fprintf(stderr, "\r%4d/%4d", n, WIDTH * HEIGHT);
			fflush(stderr);
		}
	}
	assert(n == WIDTH * HEIGHT);

	fputc('\n', stderr);
	dumporder(order);

	return 0;
}
