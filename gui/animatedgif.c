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


static void write_global_header(FILE* fout, int width, int height);
static void write_global_footer(FILE* fp);
static void write_extension_block(FILE* fout, word delay);
static void write_image_block_start(FILE *fp, int width, int height);
static void write_image_block_end(FILE *fp);
static void write_comment(FILE* fp);
static void write_image_block(TilemCalcEmulator *emu, FILE *fp, int width, int height);

void static_screenshot_save_with_parameters(TilemCalcEmulator* emu, char* filename, int width, int height) {
	
	
	printf("GIF ENCODER\n");
	FILE *fp = fopen(filename, "w");
	
	write_global_header(fp, width, height);
	write_extension_block(fp, 10);	
	write_image_block_start(fp, width, height);	
	write_image_block(emu, fp, width, height);
	write_image_block_end(fp);
	write_global_footer(fp);
	fclose(fp);
}

static void write_image_block(TilemCalcEmulator * emu, FILE *fp, int width, int height) {

	TilemLCDBuffer * lcdbuf = tilem_lcd_buffer_new();
	byte* buffer = g_new(byte, width * height);
	tilem_lcd_get_frame(emu->calc, lcdbuf);
	tilem_draw_lcd_image_indexed(lcdbuf, buffer, width, height, width, TILEM_SCALE_SMOOTH);

	GifEncode(fp, buffer , 8, (width*height));
}

	
static void write_global_header(FILE* fp, int width, int height) {
	
	/* Magic number for Gif file format */
    	char global_header_magic_number[] = {'G', 'I', 'F', '8', '7', 'a'};
    	/* Size of canvas width on 2 bytes, heigth on 2 bytes */
	char global_header_canvas[] = {96, 0, 64, 0 };

	/* FIXME : allow size superior to 256 */
	global_header_canvas[0] = width; 
	global_header_canvas[2] = height; 

	/* Flag */
	/* The 11th byte is a set of flags  : 
	bit 0:    Global Color Table Flag (GCTF)
        bit 1..3: Color Resolution
        bit 4:    Sort Flag to Global Color Table
        bit 5..7: Size of Global Color Table: 2^(1+n)
	It means "use the GCT wich is given after (from the size bit 5..7) and a resolution bit 1..3 
	The Background color is an index in the Global Color Table
	*/
    	char global_header_flag[] = { 0xf7 };
	/* The index in global color table */
	char global_header_background_index[] = {0x00};
	/* Aspect pixel ratio (unknown) */
	char global_header_aspect_pixel_ratio[] = {0x00};
	
	
	fwrite(global_header_magic_number, 6, 1, fp);
	fwrite(global_header_canvas, 4, 1, fp);
	fwrite(global_header_flag, 1, 1, fp);
	fwrite(global_header_background_index, 1, 1, fp);
	fwrite(global_header_aspect_pixel_ratio, 1, 1, fp);
	
	byte* palette = tilem_color_palette_new1(255, 255, 255, 0, 0, 0, 0.5);
	
	fwrite(palette, 256 * 3, 1, fp);
}

static void write_global_footer(FILE* fp) {

	/* This value means end of gif file */	
	char footer_trailer[1] = { 0x3b};
	
	fwrite(footer_trailer, 1, 1,fp);
}


static void write_extension_block(FILE* fp, word delay) {

	/* Extension block introduced by 0x21 ('!'), size before extension_block_terminator, flag byte, delay (10/100) 2 bytes   */
	char extension_block_header[2] = {0x21, 0xf9};
	/* Size before extension_block_terminator */
	char extension_block_size[1] = { 0x04} ;
	/* Flag (unknown) */
	char extension_block_flag[1] = { 0x00} ;
	/* Delay (x/100 sec) on 2 bytes*/
	char extension_block_delay[2] = {10, 0} ;
	extension_block_delay[0] = delay;
	/* The index designed by this variable become transparent even if palette gives a black(or something else) color. */ 
	char extension_block_transparent_index[1] = {0xff};
	/* End of extension block */
	char extension_block_terminator[1] = {0x00};

	fwrite(extension_block_header, 2, 1, fp);
    	fwrite(extension_block_size, 1, 1, fp);
    	fwrite(extension_block_flag, 1, 1, fp);
    	fwrite(extension_block_delay, 2, 1, fp);
    	fwrite(extension_block_transparent_index, 1, 1, fp);
    	fwrite(extension_block_terminator, 1, 1, fp);

}

static void write_image_block_start(FILE *fp, int width, int height) {

	/* Header */
	char image_block_header[] = { 0x2c};
	/* Left corner x (2 bytes), left corner y (2 bytes), width (2 bytes), height (2 bytes) */
	char image_block_canvas[] = { 0, 0, 0, 0, 96, 0, 64, 0};
	image_block_canvas[4] = width;
	image_block_canvas[6] = height;
	/* Flag */
	char image_block_flag[] = { 0x09};

        fwrite(image_block_header, 1, 1, fp);
    	fwrite(image_block_canvas, 8, 1, fp);
    	fwrite(image_block_flag, 1, 1, fp);

}

static void write_image_block_end(FILE *fp) {
	
 	/* Give an end to the image block */
	char image_block_end[1] = {0x00};

	fwrite(image_block_end, 1, 1,fp);
}

static void write_comment(FILE* fp) {

	char comment[] = {0x21, 0xfe, 8, 'T', 'i', 'l', 'E', 'm', '2', 0, 0, 0};
	fwrite(comment, 12, 1, fp);
}

/* Create an empty gif and add the first frame */ 
void tilem_animation_start(TilemCalcEmulator* emu) {
	
	

    	/* Introduce the block 3bytes, netscape (type of gif : animation), a new block comment */
	char gif_infos[31] = {
        0x21, 0xff, 0x0b, 'N', 'E', 'T', 'S', 'C', 'A', 'P', 'E', '2', '.', '0', 3, 1, 0xff, 0xff, 0x00	};
	
	int width, height;
	guchar* lcddata;
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
			
	    
		/* Extension block introduced by 0x21 ('!'), and an img introduced by 0x2c (',') followed by coordinate corner(0,0), canvas 4 bytes, no local color table */
		static char gif_img[18] = {0x21, 0xf9, 4, 5, 11, 0, 0x0f, 0, 0x2c, 0, 0, 0, 0, 96, 0, 64, 0, 0};
	    
		char end[1] = { 0x00};
		
		write_global_header(fp, 96, 64)	;
		fwrite(gif_infos, 19, 1, fp);
		write_comment(fp);
		fwrite(gif_img, 18, 1, fp);
		
		write_image_block(emu, fp, width, height);
		fwrite(end, 1, 1,fp);	/* Write end of the frame */
		fclose(fp);
		emu->gw->ss->isAnimScreenshotRecording = TRUE;
	}
}
    

/* Add a frame to an existing animated gif */
void tilem_animation_add_frame(TilemCalcEmulator* emu) {
	
	printf("GIFENCODER addframe\n");
		
	int width = emu->calc->hw.lcdwidth;
	int height = emu->calc->hw.lcdheight;

	FILE* fp;
	fp = fopen("gifencod.gif", "a");
	if(fp) {
		write_image_block_start(fp, width, height) ;
		write_image_block(emu, fp, width, height);	
		write_image_block_end(fp);
		fclose(fp);
	}
}

/* Stop recording animations */
void tilem_animation_stop(TilemCalcEmulator* emu) {
	
	if(emu->gw->ss->isAnimScreenshotRecording) {
		FILE* fp;
		fp = fopen("gifencod.gif", "a");
		write_global_footer(fp);
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

