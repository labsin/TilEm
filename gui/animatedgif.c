/*
 * TilEm II
 *
 * Copyright (c) 2010-2011 Thibault Duponchelle
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
#include <gtk/gtk.h>
#include <ticalcs.h>
#include <tilem.h>
#include "gui.h"



void static_screenshot_save(TilemCalcEmulator* emu, char* filename) {
	
	long i= 0;
	int width, height;
	guchar* lcddata;
	int x, y;
	width = emu->calc->hw.lcdwidth;
	height = emu->calc->hw.lcdheight;
	/* Alloc mmem */
	lcddata = g_new(guchar, (width / 8) * height);
		
	/* Get the lcd content using the function 's pointer from Benjamin's core */
	(*emu->calc->hw.get_lcd)(emu->calc, lcddata);


	printf("GIF ENCODER\n");
	FILE* fp;
	fp = fopen(filename, "w");
    	/* Magic number for Gif file format */
    	char magic_number[] = {'G', 'I', 'F', '8', '9', 'a'};
    	/* Size of canvas width on 2 bytes, heigth on 2 bytes, GCT , bg color i (place dans la palette ici c'est blanc) , aspect pixel ratio */
    	char canvas[] = { 96, 0, 64, 0, 0x80, 0x0f, 0x00};
   
    	/* GCT, multiple de 2 */ 
    	char palette[] = { 0xff, 0xff, 0xff, 0x00, 0x00, 0x00};

    
    	/* Extension block introduced by 0x21 ('!'), and an img introduced by 0x2c (',') followed by coordinate corner(0,0), canvas 4 bytes, no local color table */
	static char gif_img[18] = {0x21, 0xf9, 4, 5, 11, 0, 0x0f, 0, 0x2c, 0, 0, 0, 0, 96, 0, 64, 0, 0};
	//static unsigned char example[] = { 0x01, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01};
    
	fwrite(magic_number, 6, 1, fp);
    	fwrite(canvas, 7, 1, fp);
    	fwrite(palette, 6, 1, fp);
    	fwrite(gif_img, 18, 1, fp);
    	
	
	unsigned char q[(96*64)];


	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			if (lcddata[(y * width + x) / 8] & (0x80 >> (x % 8))) {
				//printf("i = %d", i);
				q[i] = 0x01;
				i++;
			} else {
				q[i] = 0x00;
				i++;
			}
		}
	}	
	
	

	printf("toto");
	GifEncode(fp, q , 1, (96*64));
	char trailer[1] = { 0x3b};
	fwrite(trailer, 1, 1,fp);
	//fwrite(end, 1, 1,fp);
	fclose(fp);
}

void static_screenshot_save_with_parameters(TilemCalcEmulator* emu, char* filename) {
	
	int x, y, k;
	long i= 0;
	int width = emu->calc->hw.lcdwidth;
	int height = emu->calc->hw.lcdheight;
	guchar *lcddata = g_new(guchar, (width / 8) * height);
	(*emu->calc->hw.get_lcd)(emu->calc, lcddata);


	printf("GIF ENCODER\n");
	FILE *fp = fopen(filename, "w");

    	/* Magic number for Gif file format */
    	char global_header_magic_number[] = {'G', 'I', 'F', '8', '9', 'a'};
    	/* Size of canvas width on 2 bytes, heigth on 2 bytes */
	char global_header_canvas[] = {96, 0, 64, 0 };
	/* Flag , bg color i (place dans la palette ici c'est blanc) , aspect pixel ratio */
    	char global_header_flag[] = { 0xf7 };
	/* The index in global color table */
	char global_header_background_index[] = {0x00};
	/* Aspect pixel ratio (unknown) */
	char global_header_aspect_pixel_ratio[] = {0x00};
	
   
	char palette_start[] = { 0xff, 0xff, 0xff};
	char padding[] = { 0x0f, 0xf0, 0x00};
	char palette_end[] = { 0x00, 0x00, 0x00};

	char end[1] = {0x00};
	char trailer[1] = { 0x3b};

    
    	/* Extension block introduced by 0x21 ('!'), size before extension_block_terminator, flag byte, delay (10/100) 2 bytes   */
	char extension_block_header[2] = {0x21, 0xf9};
	/* Size before extension_block_terminator */
	char extension_block_size[1] = {4} ;
	/* Flag (unknown) */
	char extension_block_flag[1] = {5} ;
	/* Delay (x/100 sec) on 2 bytes*/
	char extension_block_delay[2] = {10, 0} ;
	/* The index designed by this variable become transparent even if palette gives a black(or something else) color. */ 
	char extension_block_transparent_index[1] = {0x06};
	/* End of extension block */
	char extension_block_terminator[1] = {0x00};
	char img_header[] = { 0x2c, 0, 0, 0, 0, 96, 0, 64, 0, 0};
    
        
	fwrite(global_header_magic_number, 6, 1, fp);
	fwrite(global_header_canvas, 4, 1, fp);
	fwrite(global_header_flag, 1, 1, fp);
	fwrite(global_header_background_index, 1, 1, fp);
	fwrite(global_header_aspect_pixel_ratio, 1, 1, fp);
	fwrite(palette_start, 3, 1, fp);
		for(k = 0; k < 254; k++) {
			fwrite(padding, 3, 1, fp);
		}
	fwrite(palette_end, 3, 1, fp);
    	fwrite(extension_block_header, 2, 1, fp);
    	fwrite(extension_block_size, 1, 1, fp);
    	fwrite(extension_block_flag, 1, 1, fp);
    	fwrite(extension_block_delay, 2, 1, fp);
    	fwrite(extension_block_transparent_index, 1, 1, fp);
    	fwrite(extension_block_terminator, 1, 1, fp);
    	fwrite(img_header, 10, 1, fp);
    	
	
	unsigned char q[(96*64)];


	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			if (lcddata[(y * width + x) / 8] & (0x80 >> (x % 8))) {
				/* BLACK */
				q[i] = 0x01;
				i++;
			} else {
				/* WHITE */
				q[i] = 0x00;
				i++;
			}
		}
	}	
	
	
	GifEncode(fp, q , 1, (96*64));
	fwrite(end, 1, 1,fp);
	fwrite(trailer, 1, 1,fp);
	fclose(fp);
}


/* Create an empty gif and add the first frame */ 
void tilem_animation_start(TilemCalcEmulator* emu) {
	
	/* The 11th byte is a set of flags  : 
	bit 0:    Global Color Table Flag (GCTF)
        bit 1..3: Color Resolution
        bit 4:    Sort Flag to Global Color Table
        bit 5..7: Size of Global Color Table: 2^(1+n)
	It means "use the GCT wich is given after (from the size bit 5..7) and a resolution bit 1..3 
	The Background color is an index in the Global Color Table
	*/
	
	/* Magic number (GIF89a), width pixel 2 bytes, height pixel 2 bytes, GCT follows 256 values * 3 bytes (RGB) , bg color, default aspect ration */ 	
	char gif_header[13] = {'G', 'I', 'F', '8', '9', 'a', 96, 0, 64, 0, 0xf7, 0x00, 0};
    	/* Introduce the block 3bytes, netscape (type of gif : animation), a new block comment */
	char gif_infos[31] = {
        0x21, 0xff, 0x0b, 'N', 'E', 'T', 'S', 'C', 'A', 'P', 'E', '2', '.', '0', 3, 1, 0, 0, 0,
	0x21, 0xfe, 8, 'T', 'i', 'l', 'E', 'm', '2', 0, 0, 0
	};
	
	int width, height;
	guchar* lcddata;
	int k;
	int x, y;
	width = emu->calc->hw.lcdwidth;
	height = emu->calc->hw.lcdheight;
	
	/* Alloc mmem */
	lcddata = g_new(guchar, (width / 8) * height);
		
	/* Get the lcd content using the function 's pointer from Benjamin's core */
	(*emu->calc->hw.get_lcd)(emu->calc, lcddata);





	printf("GIF ENCODER\n");
	FILE* fp;
	fp = fopen("gifencod.gif", "w");
  	if(fp) { 
			
		/* TODO : convert this padding stuff to a real palette */ 
		char palette_start[] = { 0x00, 0x00, 0x00};
		char padding[] = { 0xff, 0xff, 0xff};
		char palette_end[] = { 0xff, 0xff, 0xff};

	    
		/* Extension block introduced by 0x21 ('!'), and an img introduced by 0x2c (',') followed by coordinate corner(0,0), canvas 4 bytes, no local color table */
		static char gif_img[18] = {0x21, 0xf9, 4, 5, 11, 0, 0x0f, 0, 0x2c, 0, 0, 0, 0, 96, 0, 64, 0, 0};
	    
		char end[1] = { 0x00};
		
		fwrite(gif_header, 13, 1, fp);
		fwrite(palette_start, 3, 1, fp);
		for(k = 0; k < 254; k++) {
			fwrite(padding, 3, 1, fp);
		}
		fwrite(palette_end, 3, 1, fp);
		fwrite(gif_infos, 31, 1, fp);
		fwrite(gif_img, 18, 1, fp);
		
		long i= 0;
		
		unsigned char q[(96*64)];


		/* Reduce screen buffer to one byte per pixel */
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				if (lcddata[(y * width + x) / 8] & (0x80 >> (x % 8))) {
					//printf("i = %d", i);
					q[i] = 0x00;
					i++;
				} else {
					q[i] = 0x02;
					i++;
				}
			}
		}	
		
		

		GifEncode(fp, q , 1, (width*height));
		fwrite(end, 1, 1,fp);	/* Write end of the frame */
		fclose(fp);
		emu->gw->ss->isAnimScreenshotRecording = TRUE;
	}
}
    

/* Add a frame to an existing animated gif */
void tilem_animation_add_frame(TilemCalcEmulator* emu) {
	
	printf("GIFENCODER addframe\n");
	int width, height;
	guchar* lcddata;
	int x, y;
	width = emu->calc->hw.lcdwidth;
	height = emu->calc->hw.lcdheight;
	/* Alloc mmem */
	lcddata = g_new(guchar, (width / 8) * height);
		
	/* Get the lcd content using the function 's pointer from Benjamin's core */
	(*emu->calc->hw.get_lcd)(emu->calc, lcddata);


	FILE* fp;
	fp = fopen("gifencod.gif", "a");
	if(fp) {
    	
    		/* Extension block introduced by 0x21 ('!'), and an img introduced by 0x2c (',') followed by coordinate corner(0,0), canvas 4 bytes, no local color table */
		static char gif_img[18] = {0x21, 0xf9, 4, 8, 11, 0, 0x0f, 0, 0x2c, 0, 0, 0, 0, 96, 0, 64, 0, 0};
		char end[1] = { 0x00};
		
		fwrite(gif_img, 18, 1, fp);
		long i= 0;
		
		unsigned char q[(width * height)];

		/* Reduce screen buffer to one byte per pixel */
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				if (lcddata[(y * width + x) / 8] & (0x80 >> (x % 8))) {
					q[i] = 0x00;
					i++;
				} else {
					q[i] = 0x02;
					i++;
				}
			}
		}	
		
		

		GifEncode(fp, q , 1, (width*height));
		fwrite(end, 1, 1,fp);
		fclose(fp);
	}
}

/* Stop recording animations */
void tilem_animation_stop(TilemCalcEmulator* emu) {
	
	
    	char trailer[1] = { 0x3b};
	if(emu->gw->ss->isAnimScreenshotRecording) {
		FILE* fp;
		fp = fopen("gifencod.gif", "a");
		fwrite(trailer, 1, 1,fp);
		fclose(fp);
		
	}

}

/* Add frames to the animations */
gboolean tilem_animation_record(gpointer data) {
	TilemCalcEmulator * emu = (TilemCalcEmulator*) data;
	
	if(emu->gw->ss->isAnimScreenshotRecording) 
		tilem_animation_add_frame(emu);
	return TRUE;

}

