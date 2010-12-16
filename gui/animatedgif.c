#include "animatedgif.h"


void screenshot_anim_create(GLOBAL_SKIN_INFOS* gsi) {
	

	int width, height;
	guchar* lcddata;
	int x, y;
	width = gsi->emu->calc->hw.lcdwidth;
	height = gsi->emu->calc->hw.lcdheight;
	/* Alloc mmem */
	lcddata = g_new(guchar, (width / 8) * height);
		
	/* Get the lcd content using the function 's pointer from Benjamin's core */
	(*gsi->emu->calc->hw.get_lcd)(gsi->emu->calc, lcddata);





	printf("GIF ENCODER\n");
	FILE* fp;
	fp = fopen("gifencod.gif", "w");
    	/* Magic number for Gif file format */
    	char magic_number[] = {'G', 'I', 'F', '8', '9', 'a'};
    	/* Size of canvas width on 2 bytes, heigth on 2 bytes, GCT , bg color i (place dans la palette ici c'est blanc) , aspect pixel ratio */
    	char canvas[] = { 96, 0, 64, 0, 0x80, 0x0f, 0x00};
   
    	/* GCT, multiple de 2 */ 
    	char palette[] = { 0xff, 0xff, 0xff, 0x00, 0x00, 0x00};

    
    	/* Extension block introduced by 0x21 ('!'), and an img introduced by 0x2c (',') followed by coordinate corner(0,0), canvas 4 bytes, no local color table */
	static char gif_img[18] = {0x21, 0xf9, 4, 5, 11, 0, 0x0f, 0, 0x2c, 0, 0, 0, 0, 96, 0, 64, 0, 0};
	//static unsigned char example[] = { 0x01, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01};
    
    	char end[1] = { 0xC3};
        
	fwrite(magic_number, 6, 1, fp);
    	fwrite(canvas, 7, 1, fp);
    	fwrite(palette, 6, 1, fp);
    	fwrite(gif_img, 18, 1, fp);
    	
	//fwrite(example, 19, 1, fp);
	long i= 0;
	
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
	
	

	GifEncode(fp, q , 1, (96*64));
	fwrite(end, 1, 1,fp);
	//fclose(fp);
}
    
void screenshot_anim_create_nostatic(GLOBAL_SKIN_INFOS* gsi) {
	
	char gif_header[13] = {'G', 'I', 'F', '8', '9', 'a', 96, 0, 64, 0, 0x80, 0x0f, 0};
    	char gif_infos[31] = {
        0x21, 0xff, 0x0b, 'N', 'E', 'T', 'S', 'C', 'A', 'P', 'E', '2', '.', '0', 3, 1, 0, 0, 0,
	0x21, 0xfe, 8, 'T', 'i', 'l', 'E', 'm', '2', 0, 0, 0
	};
	
	int width, height;
	guchar* lcddata;
	int x, y;
	width = gsi->emu->calc->hw.lcdwidth;
	height = gsi->emu->calc->hw.lcdheight;
	/* Alloc mmem */
	lcddata = g_new(guchar, (width / 8) * height);
		
	/* Get the lcd content using the function 's pointer from Benjamin's core */
	(*gsi->emu->calc->hw.get_lcd)(gsi->emu->calc, lcddata);





	printf("GIF ENCODER\n");
	FILE* fp;
	fp = fopen("gifencod.gif", "w");
    	/* Magic number for Gif file format */
    	//char magic_number[] = {'G', 'I', 'F', '8', '9', 'a'};
    	/* Size of canvas width on 2 bytes, heigth on 2 bytes, GCT , bg color i (place dans la palette ici c'est blanc) , aspect pixel ratio */
    	//char canvas[] = { 96, 0, 64, 0, 0x80, 0x0f, 0x00};
   
    	char palette[] = { 0xff, 0xff, 0xff, 0x00, 0x00, 0x00};

    
    	/* Extension block introduced by 0x21 ('!'), and an img introduced by 0x2c (',') followed by coordinate corner(0,0), canvas 4 bytes, no local color table */
	static char gif_img[18] = {0x21, 0xf9, 4, 5, 11, 0, 0x0f, 0, 0x2c, 0, 0, 0, 0, 96, 0, 64, 0, 0};
	//static unsigned char example[] = { 0x01, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01};
    
    	//char end[1] = { 0xC3};
        
	fwrite(gif_header, 13, 1, fp);
    	fwrite(gif_infos, 31, 1, fp);
    	fwrite(palette, 6, 1, fp);
    	fwrite(gif_img, 18, 1, fp);
    	
	//fwrite(example, 19, 1, fp);
	long i= 0;
	
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
	
	

	GifEncode(fp, q , 1, (96*64));
	//fwrite(end, 1, 1,fp);
	//fclose(fp);
}
    


void screenshot_anim_addframe(GLOBAL_SKIN_INFOS* gsi) {
	

	int width, height;
	guchar* lcddata;
	int x, y;
	width = gsi->emu->calc->hw.lcdwidth;
	height = gsi->emu->calc->hw.lcdheight;
	/* Alloc mmem */
	lcddata = g_new(guchar, (width / 8) * height);
		
	/* Get the lcd content using the function 's pointer from Benjamin's core */
	(*gsi->emu->calc->hw.get_lcd)(gsi->emu->calc, lcddata);


	FILE* fp;
	fp = fopen("gifencod.gif", "a");
    	
	
    	/* Size of canvas width on 2 bytes, heigth on 2 bytes, GCT , bg color i (place dans la palette ici c'est blanc) , aspect pixel ratio */
    	//char canvas[] = { 96, 0, 64, 0, 0x80, 0x0f, 0x00};
   
    	/* GCT, multiple de 2 */ 
    	//char palette[] = { 0xff, 0xff, 0xff, 0x00, 0x00, 0x00};

    
    	/* Extension block introduced by 0x21 ('!'), and an img introduced by 0x2c (',') followed by coordinate corner(0,0), canvas 4 bytes, no local color table */
	static char gif_img[18] = {0x21, 0xf9, 4, 5, 11, 0, 0x0f, 0, 0x2c, 0, 0, 0, 0, 96, 0, 64, 0, 0};
	//static unsigned char example[] = { 0x01, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01};
    
    	char end[1] = { 0xC3};
        
    	//fwrite(canvas, 7, 1, fp);
    	//fwrite(palette, 6, 1, fp);
    	fwrite(gif_img, 18, 1, fp);
    	
	//fwrite(example, 19, 1, fp);
	long i= 0;
	
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
	
	

	GifEncode(fp, q , 1, (96*64));
	fwrite(end, 1, 1,fp);
	//fclose(fp);
}
    

