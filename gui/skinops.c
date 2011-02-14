/* Hey EMACS -*- linux-c -*- */
/* $Id$ */

/*
 *   skinedit - a skin editor for the TiEmu emulator
 *   Copyright (C) 2002 Julien BLACHE <jb@tilp.info>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


/*
contra-sh :
   This file is a perfect copy of the tiemu skinops.c file ...
   Thank's to rom's and JB for this wonderful work.
*/



#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <debuginfo.h>
#include "skinops.h"


#include <gtk/gtk.h>
#include <glib/gstdio.h>






/*
	Determine skin type
*/
int skin_get_type(SKIN_INFOS *si, const char *filename)
{
	FILE *fp;
	char str[17];

	fp = fopen(filename, "rb");
  	if (fp == NULL)
    {
      	fprintf(stderr, "Unable to open this file: <%s>\n", filename);
      	return -1;
    }

	memset(str, 0, sizeof(str));
	fread(str, 16, 1, fp);

	if(!strncmp(str, "TiEmu v2.00", 16))
		si->type = SKIN_TYPE_TIEMU;
	else if(!strncmp(str, "TilEm v2.00 ", 16))
		si->type = SKIN_TYPE_TIEMU;
	else if(!strncmp(str, "VTIv2.1 ", 8))
		si->type = SKIN_TYPE_OLD_VTI;
	else if(!strncmp(str, "VTIv2.5 ", 8))
		si->type = SKIN_TYPE_VTI;
	else
  	{
  		fprintf(stderr, "Bad skin format\n");
      	return -1;
  	}

	return 0;
}


/*
	Read TilEm skin informations (header)
*/
int skin_read_header(SKIN_INFOS *si, const char *filename)
{
	FILE *fp;
  	int i;
  	uint32_t endian;
  	uint32_t jpeg_offset;
  	uint32_t length;
	char str[17];

	fp = fopen(filename, "rb");
  	if (fp == NULL)
    {
      	fprintf(stderr, "Unable to open this file: <%s>\n", filename);
      	return -1;
    }
 
	/* signature & offsets */
	fread(str, 16, 1, fp);
  	if ((strncmp(str, "TilEm v2.00", 16))&&(strncmp(str, "TiEmu v2.00", 16)))
  	{
  		fprintf(stderr, "Bad TiEmu skin format\n");
      	return -1;
  	}
  	fread(&endian, 4, 1, fp);
  	fread(&jpeg_offset, 4, 1, fp);

	if (endian != ENDIANNESS_FLAG)
		jpeg_offset = GUINT32_SWAP_LE_BE(jpeg_offset);

	/* Skin name */
  	fread(&length, 4, 1, fp);
	if (endian != ENDIANNESS_FLAG)
		length = GUINT32_SWAP_LE_BE(length);

  	if (length > 0)
    {
      	si->name = (char *)malloc(length + 1);
	    if (si->name == NULL)
			return -1;

	    memset(si->name, 0, length + 1);
	    fread(si->name, length, 1, fp);
    }
	
	/* Skin author */
  	fread(&length, 4, 1, fp);
	if (endian != ENDIANNESS_FLAG)
		length = GUINT32_SWAP_LE_BE(length);

  	if (length > 0)
    {
      	si->author = (char *)malloc(length + 1);
      	if (si->author == NULL)
			return -1;

      	memset(si->author, 0, length + 1);
      	fread(si->author, length, 1, fp);
    }

	/* LCD colors */
  	fread(&si->colortype, 4, 1, fp);
  	fread(&si->lcd_white, 4, 1, fp);
  	fread(&si->lcd_black, 4, 1, fp);

   	/* Calc type */
  	fread(si->calc, 8, 1, fp);

  	/* LCD position */
  	fread(&si->lcd_pos.left, 4, 1, fp);
  	fread(&si->lcd_pos.top, 4, 1, fp);
  	fread(&si->lcd_pos.right, 4, 1, fp);
  	fread(&si->lcd_pos.bottom, 4, 1, fp);

	/* Number of RECT struct to read */
  	fread(&length, 4, 1, fp);
	if (endian != ENDIANNESS_FLAG)
		length = GUINT32_SWAP_LE_BE(length);

  	if (length > SKIN_KEYS)
    		return -1;

  	for (i = 0; i < (int)length; i++)
    {
      	fread(&si->keys_pos[i].left, 4, 1, fp);
      	fread(&si->keys_pos[i].top, 4, 1, fp);
      	fread(&si->keys_pos[i].right, 4, 1, fp);
      	fread(&si->keys_pos[i].bottom, 4, 1, fp);
    }

	if (endian != ENDIANNESS_FLAG)
	{
		si->colortype = GUINT32_SWAP_LE_BE(si->colortype);
		si->lcd_white = GUINT32_SWAP_LE_BE(si->lcd_white);
		si->lcd_black = GUINT32_SWAP_LE_BE(si->lcd_black);
      
		si->lcd_pos.top = GUINT32_SWAP_LE_BE(si->lcd_pos.top);
		si->lcd_pos.left = GUINT32_SWAP_LE_BE(si->lcd_pos.left);
		si->lcd_pos.bottom = GUINT32_SWAP_LE_BE(si->lcd_pos.bottom);
		si->lcd_pos.right = GUINT32_SWAP_LE_BE(si->lcd_pos.right);

		for (i = 0; i < (int)length; i++)
		{
			si->keys_pos[i].top = GUINT32_SWAP_LE_BE(si->keys_pos[i].top);
			si->keys_pos[i].bottom = GUINT32_SWAP_LE_BE(si->keys_pos[i].bottom);
			si->keys_pos[i].left = GUINT32_SWAP_LE_BE(si->keys_pos[i].left);
			si->keys_pos[i].right = GUINT32_SWAP_LE_BE(si->keys_pos[i].right);
		}
	}

	si->jpeg_offset = ftell(fp);
    	
    fclose(fp);
    return 0;
}


/*
	Read skin image (pure jpeg data)
*/
int skin_read_image(SKIN_INFOS *si, const char *filename)
{
	DSKIN_L2_A0("* -> fct : skin_read_image                             *\n");
    FILE *fp = NULL;	
	int i;
	int sw, sh;
    int lw, lh;
	float rw, rh, r;
	double s;
	int lcd_w, lcd_h;

	GdkPixbufLoader *loader;
	GError *error = NULL;
	gboolean result;
	guchar *buf;
	gsize count;
	struct stat st;

	// set lcd size
	if(!strcmp(si->calc, SKIN_TI89) || !strcmp(si->calc, SKIN_TI89T))
	{
		lcd_w = 160;
		lcd_h = 100;
	} 
	else
	{
		lcd_w = 240;
		lcd_h = 128;
	}
  
    	fp = fopen(filename, "rb");
  	if (fp == NULL)
    	{
    		fprintf(stderr, "Unable to open this file: <%s>\n", filename);
    		return -1;
    	}
    	
    // Extract image from skin
	fseek(fp, si->jpeg_offset, SEEK_SET);
	fstat(fileno(fp), &st);
	count = st.st_size - si->jpeg_offset;

	buf = g_malloc(count * sizeof(guchar));
	count = fread(buf, sizeof(guchar), count, fp);
	
	// Feed the pixbuf loader with our jpeg data
	loader = gdk_pixbuf_loader_new();	
	result = gdk_pixbuf_loader_write(loader, buf, count, &error);
	if(result == FALSE)
	{
		fprintf(stderr, "Failed to load pixbuf file: %s\n", filename);
		g_error_free(error);

		return -1;
	}
    
	result = gdk_pixbuf_loader_close(loader, &error);
	if(result == FALSE)
	{
		fprintf(stderr, "Failed to close pixbuf file: %s\n", filename);
		g_error_free(error);

		return -1;
	}
	
    // and get the pixbuf
	si->raw = gdk_pixbuf_loader_get_pixbuf(loader);
	if(si->raw == NULL)
	{
		fprintf(stderr, "Failed to load pixbuf file: %s\n", filename);
		g_error_free(error);

		return -1;
    }

	// Rescale image to a fixed LCD size but keep original image (avoid resolution lost)
    sw = gdk_pixbuf_get_width(si->raw);
    sh = gdk_pixbuf_get_height(si->raw);

	lw = si->lcd_pos.right - si->lcd_pos.left;
	lh = si->lcd_pos.bottom - si->lcd_pos.top;

	rw = (float)lw / lcd_w;
	rh = (float)lh / lcd_h;

	r = (rw < rh) ? rw : rh;
	si->s = s = ceil(10 * r) / 10.0;

	DSKIN_L0_A2("*  image :<%i x %i>                                  *\n", sw, sh);
	DSKIN_L0_A2("*  lcd : <%i x %i>                                   *\n", lw, lh);
	DSKIN_L0_A3("*  ratios : <%2.2f x %2.2f> => %2.1f                       *\n", rw, rh, s);
    
	//if(si->image)
		//g_object_unref(si->image);
    si->image = gdk_pixbuf_scale_simple(si->raw, (int)(sw/s), (int)(sh/s), GDK_INTERP_NEAREST);

    // Get new skin size
    si->width = gdk_pixbuf_get_width(si->image);
    si->height = gdk_pixbuf_get_height(si->image);

	// Rescale all coords
	si->lcd_pos.left = (long)(si->lcd_pos.left / s);
	si->lcd_pos.right = (long)(si->lcd_pos.right / s);
	si->lcd_pos.top = (long)(si->lcd_pos.top / s);
	si->lcd_pos.bottom = (long)(si->lcd_pos.bottom / s);

	for (i = 0; i < SKIN_KEYS; i++) 
	{
      	si->keys_pos[i].left = (long)(si->keys_pos[i].left / s);
      	si->keys_pos[i].top = (long)(si->keys_pos[i].top / s);
      	si->keys_pos[i].right = (long)(long)(si->keys_pos[i].right / s);
      	si->keys_pos[i].bottom = (long)(long)(si->keys_pos[i].bottom / s);
    }

   	return 0;
}

/* Load a skin (TilEm v2.00 only) */
int skin_load(SKIN_INFOS *si, const char *filename)
{
  	int ret = 0;
	DSKIN_L0_A0("**************** fct : skin_load ***********************\n");
  	ret = skin_read_header(si, filename);
	if(ret)
		return ret;

  	ret = skin_read_image(si, filename);
	if(ret)
		return ret;

	if(!ret)
   		printf("*  loading skin:  (%d x %d)                          *\n", si->width, si->height);
	
	DGLOBAL_L0_A0("********************************************************\n");
  
  	return ret;
}

/* Unload skin by freeing allocated memory */
int skin_unload(SKIN_INFOS *si)
{
    if(si->image != NULL)
    {
        g_object_unref(si->image);
        si->image = NULL;
    }

  	free(si->name);
  	free(si->author);

  	memset(si, 0, sizeof(SKIN_INFOS));
  
  	return 0;
}

