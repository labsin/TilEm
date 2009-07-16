/*
 * libtilemcore - Graphing calculator emulation library
 *
 * Copyright (C) 2009 Benjamin Moody
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

char tilem_guess_rom_type(FILE* romfile)
{
	const TilemHardware** hwmodels;
	int nmodels;
	unsigned long initpos;
	dword size;
	int i;
	char result = 0;

	initpos = ftell(romfile);

	fseek(romfile, 0L, SEEK_END);
	size = ftell(romfile);

	tilem_get_supported_hardware(&hwmodels, &nmodels);

	for (i = 0; i < nmodels; i++) {
		if (size < hwmodels[i]->romsize
		    || size > (hwmodels[i]->romsize * 3) / 2) {
			continue;
		}

		if (!hwmodels[i]->checkrom) {
			result = hwmodels[i]->model_id;
			break;
		}

		fseek(romfile, 0L, SEEK_SET);
		if ((*hwmodels[i]->checkrom)(romfile)) {
			result = hwmodels[i]->model_id;
			break;
		}
	}

	fseek(romfile, initpos, SEEK_SET);
	return result;
}

int tilem_rom_find_string(const char *str, FILE *romfile, dword limit)
{
	char buf[256];
	int pos = 0;
	int len, i;

	len = strlen(str);

	for (i = 0; i < len-1; i++) {
		buf[pos] = fgetc(romfile);
		pos = (pos+1)%256;
		limit--;
	}

	while (limit > 0 && !feof(romfile) && !ferror(romfile)) {
		buf[pos] = fgetc(romfile);
		pos = (pos+1)%256;
		limit--;

		for (i = 0; i < len; i++) {
			if (str[i] != buf[(pos + 256 - len + i)%256])
				break;
		}
		if (i == len)
			return 1;
	}
	return 0;
}
