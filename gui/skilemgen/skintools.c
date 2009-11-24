#include "skintools.h"
#include <stdint.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>


SKIN_INFOS set_skins_info() {
	
	SKIN_INFOS skins_info;
	
	skins_info.type=SKIN_TYPE_TIEMU;

	//GdkPixbuf *image;
	
	skins_infos.name=(char*) malloc(14);
	strcpy(skins_infos.name,"Default TI-83");
	
	skins_infos.author=(char*) malloc(14);
	strcpy(skins_infos.author,"Thibault");

	skins_infos.width=0;
	skins_infos.height=0;

	skins_infos.s=65;
	
	skins_infos.colortype=0;
	skins_infos.lcd_white=0xCFE0CC;
	skins_infos.lcd_black=0x22E231;

	strcpy(skins_infos.calc,"TI-89TM");


	

	/* >>>> By default it must be a lcd pos to be reconized by tiemu */
	skins_infos.lcd_pos.left=0xCFE0CC;
	skins_infos.lcd_pos.top=0x222E31;
	skins_infos.lcd_pos.right=0x2D4954;
	skins_infos.lcd_pos.bottom=0x4D543938;
	/* <<<< */
	skins_infos.keys_pos[0].left=0;
	skins_infos.keys_pos[0].top=0;
	skins_infos.keys_pos[0].right=0;
	skins_infos.keys_pos[0].bottom=0;
	skins_infos.keys_pos[1].left=0;
	skins_infos.keys_pos[1].top=0;
	skins_infos.keys_pos[1].right=0;
	skins_infos.keys_pos[1].bottom=0;
	skins_infos.keys_pos[2].left=0;
	skins_infos.keys_pos[2].top=0;
	skins_infos.keys_pos[2].right=0;
	skins_infos.keys_pos[2].bottom=0;
	int i=3;
	for(i;i<80;i++) {
	skins_infos.keys_pos[i].bottom=0;
	skins_infos.keys_pos[i].left=0;
	skins_infos.keys_pos[i].top=0;
	skins_infos.keys_pos[i].right=0;
	skins_infos.keys_pos[i].bottom=0;
	}
		
	

  //long	jpeg_offset;
	return skins_infos;
}




int main(int argc, char* argv[] ) {
	
	FILE *fp = NULL;
	
	char * input_for_read;
	char * jpeg_name;
	jpeg_name=(char*) malloc(7);
	strcpy(jpeg_name,"nothing");
	char * skin_out_name;
	uint32_t jpeg_offset;
	uint32_t endian = ENDIANNESS_FLAG;
	uint32_t length;
	int i;

	unsigned char id[16]="TilEm v2.00";
	
	if(argc>=2) {
		
		printf("\n");
		/*>>>> ONLY READ A SKIN ?*/
		if(strcmp("-read",argv[1])==0) {
			if(argc>=3) {
				input_for_read=(char*) malloc(strlen(argv[2]));
				strcpy(input_for_read,argv[2]);
			} else {
				return 1;
			}
			printf("\nOnly read and test format skin (Says \"Bad skin Format!\" if not reconized)\n\n");
			read_skin_format(input_for_read);
			return 0;
		}
		/* <<<< */
		
		/* NEED HELP OR NOT? */
		if(strcmp("--help",argv[1])==0) {
			printf("You need help\n");
			printf("\nskintools jpeg_name model_name outfile_name\n\n");
		} else {
			jpeg_name=(char*) malloc(strlen(argv[1]));
			strcpy(jpeg_name,argv[1]);
			printf("jpeg name : %s\n",jpeg_name);
		}
	} else {
		printf("\nNot enough parameter!\n");
		printf("\nskintools jpeg_name model_name outfile_name\n\n");
		return 1;
	}


	
	if(argc>=3) {
		strcpy(skins_infos.calc,argv[2]);
		if(((strcmp(CALC_TI73,argv[2])!=0))&&(strcmp(CALC_TI82,argv[2])!=0)&&(strcmp(CALC_TI83,argv[2])!=0)&&(strcmp(CALC_TI83P,argv[2])!=0)
		&&(strcmp(CALC_TI84P,argv[2])!=0)&&(strcmp(CALC_TI85,argv[2])!=0)&&(strcmp(CALC_TI86,argv[2])!=0)&&(strcmp(CALC_TI89,argv[2])!=0)
		&&(strcmp(CALC_TI89T,argv[2])!=0)&&(strcmp(CALC_TI92,argv[2])!=0)&&(strcmp(CALC_TI92P,argv[2])!=0)&&(strcmp(CALC_V200,argv[2])!=0)) {
		printf("The model can only be : \n%s %s %s %s %s %s %s %s %s %s %s %s  \n",CALC_TI73,CALC_TI82,CALC_TI83,CALC_TI83P,CALC_TI84P,CALC_TI85,CALC_TI86,CALC_TI89,CALC_TI89T,CALC_TI92,CALC_TI92P,CALC_V200);
		return -1;
		}
	printf("model name : %s   ",skins_infos.calc);
	} else {
		printf("\nNot enough parameter!\n");
		printf("\nskintools jpeg_name model outfile_name\n\n");
		return -1;
	}
	
	if(argc>=4) {
		skin_out_name=(char*) malloc(65);
		strcpy(skin_out_name,argv[3]);
		printf("outfile name : %s   ",skin_out_name);
	} else {
		skin_out_name=(char*) malloc(11);
		strcpy(skin_out_name,"default.skn");
		printf("\nUse the default output file name : default.skn\n\n");
	}
	
	

	
	
	/* init the skins_infos struct */
	skins_infos=set_skins_info();
	
	FILE *jpeg = NULL;
	unsigned char *jpeg_data = NULL;
	unsigned int jpeg_length;

  /*
   * write the skin to skin_infos.skin_path
   */
	

	     jpeg = fopen(jpeg_name, "rb");
	     if (jpeg == NULL)
	       {
		 fclose(fp);
		 return -1;
	       }

	/* create bin file */
	if((fp = fopen(skin_out_name, "w+b"))) {
		printf("\nopen %s\n",skin_out_name);
		fclose(fp);
	}
	/* open for writing */
	if((fp = fopen(skin_out_name, "r+b"))) {
		printf("open test.skn to write\n");
	}
	
	/* just write "TilEm v2" */
	fwrite(id, 16, 1, fp);
	
	/* just write feedbabe */
	fwrite(&endian, 4, 1, fp);  
	
	/* write the jpeg_offset, reserving 4 bytes */
	fwrite(&jpeg_offset, 4, 1, fp);
		
	/* get the skin name length and write length and name */
	length = strlen(skins_infos.name);
	fwrite(&length, 4, 1, fp);
	fwrite(skins_infos.name, length, 1, fp);
	
	/* get the skin author length and write length and author */
	length = strlen(skins_infos.author);
	fwrite(&length, 4, 1, fp);
	fwrite(skins_infos.author, length, 1, fp);
	
	
	fwrite(&skins_infos.colortype, 4, 1, fp);
	fwrite(&skins_infos.lcd_white, 4, 1, fp);
	fwrite(&skins_infos.lcd_black, 4, 1, fp);
	
	/* And now write the skin author */
	fwrite(&skins_infos.calc, 8, 1, fp);
	

	fwrite(&skins_infos.lcd_pos.left, 4, 1, fp);
	fwrite(&skins_infos.lcd_pos.top, 4, 1, fp);
	fwrite(&skins_infos.lcd_pos.right, 4, 1, fp);
	fwrite(&skins_infos.lcd_pos.bottom, 4, 1, fp);
	
	/* write the number of RECT structs */
	length = SKIN_KEYS; /* SKIN_KEYS */
	fwrite(&length, 4, 1, fp);
	for (i = 0; i < SKIN_KEYS; i++)
	{
		fwrite(&skins_infos.keys_pos[i].left, 4, 1, fp);
		fwrite(&skins_infos.keys_pos[i].top, 4, 1, fp);
		fwrite(&skins_infos.keys_pos[i].right, 4, 1, fp);
		fwrite(&skins_infos.keys_pos[i].bottom, 4, 1, fp);
	}
	
	/* get the current position */
	jpeg_offset = ftell(fp);

	/* go back to the jpeg_offset location */
	fseek(fp, 20, SEEK_SET);
	fwrite(&jpeg_offset, 4, 1, fp);

	/* back to end of file */
	fseek(fp, jpeg_offset, SEEK_SET);
	
	
	jpeg_data = read_image(jpeg, &jpeg_length);


	fwrite(jpeg_data, jpeg_length, 1, fp);
	free(jpeg_data); 
     
	skins_infos.type =SKIN_TYPE_TIEMU ;
	   
	   	   
	fclose(jpeg);  
	fclose(fp);
	
	
	/* read the bin file */
	read_skin_format(skin_out_name);
	skin_get_type(&skins_infos, skin_out_name);
	
	printf("\n%s successfully created !\n \n",skin_out_name);
	
	return 0;
	
}


unsigned char * read_image(FILE *fp, unsigned int *length)
{
  char *buf = NULL;
  char *data = NULL;
  unsigned int l = 0;

  uint32_t endian;
  uint32_t offset;

  *length = 0;

  buf = (char *)malloc(16);
  fread(buf, 16, 1, fp);

  if (strncmp(buf, "TilEm v2.00", 8) == 0)
  {
    fseek(fp, 16, SEEK_SET);
    fread(&endian, 4, 1, fp);

    fread(&offset, 4, 1, fp);

    if (endian != ENDIANNESS_FLAG)
      offset = GUINT32_SWAP_LE_BE(offset);
    fseek(fp, offset, SEEK_SET);
  }
  else
    fseek(fp, 0, SEEK_SET);
  free(buf);
  buf = NULL;

  while (feof(fp) == 0)
    {
      buf = realloc(data, *length + 2048);
      
      if (buf == NULL)
	{
	  if (data != NULL)
	    free(data);
	  
	  return NULL;
	}
      
      data = buf;
      buf = NULL;
      
      l = fread(data+(*length), 1, 2048, fp);
      
      *length += l;
    }

  return (unsigned char *)data;
}



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

	if(!strncmp(str, "TilEm v2.00", 16)) {
		si->type = SKIN_TYPE_TIEMU;
		printf("\nskin_info.type = TilEm_v2 !!!\n");
	} else {
  		fprintf(stderr, "Bad skin format\n");
      	return -1;
  	}

	return 0;
}


int read_skin_format(char * filename) {
	
	int i=0;
	char buffer[150];
	FILE * fp;
	int buffersize=150;
	if((fp=fopen(filename,"r"))) {
		printf("%s is open :\n",filename);
		fread(buffer,1,150,fp);
		
		for(i=0;i<buffersize;i++) {
			printf("%c",buffer[i]);
		}
		fclose(fp);
	}
	printf("\n");
	
	return 0;	
}



int skin_read_header_tilem(SKIN_INFOS *si, const char *filename)
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
  	if (strncmp(str, "TiEmu v2.00", 16))
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

int skin_read_header(SKIN_INFOS *si, const char *filename)
{
	if(skin_get_type(si, filename) == -1)
		return -1;

	if(si->type==SKIN_TYPE_TILEM) {
		return skin_read_header_tilem(si, filename);
	} else {
		return -1;
	}

	return 0;
}








