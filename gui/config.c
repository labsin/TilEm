#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <gui.h>



/*
	Read TilEm config informations 
*/
int config_read(CONFIG_INFOS *ci, const char *filename)
{
	FILE *fp;
  	int i;
  	uint32_t endian;
  	uint32_t length;
	char str[17];

	fp = fopen(filename, "rb");
  	if (fp == NULL)
    	{
      		fprintf(stderr, "Unable to open this file: <%s>\n", filename);
      		return -1;
    	}
 
	/* signature & offsets */
	memset(str, 0, sizeof(str));	
	fread(str, 16, 1, fp);
  	if (strncmp(str, "TilEm v2.00", 16))
  	{
  		fprintf(stderr, "Bad TilEm config format\n");
      	return -1;
  	}
  	fread(&endian, 4, 1, fp);

	if (endian != ENDIANNESS_FLAG)
		printf(">>>>> endian!= ENDIANNESS_FLAG");
	
	/* You can store 60 associations */
  	for (i = 0; i < 60; i++)
	{
		/* Rom name */
	  	fread(&length, 4, 1, fp);
		if (endian != ENDIANNESS_FLAG)
			length = GUINT32_SWAP_LE_BE(length);
	
	  	if (length > 0)
	    	{
	      		ci->sa[i].romname = (char *)malloc(length + 1);
		    	if (ci->sa[i].romname == NULL)
				return -1;
	
			memset(ci->sa[i].romname, 0, length + 1);
		    	fread(ci->sa[i].romname, length, 1, fp);
		}
	
		/* Default associated skin */
	  	fread(&length, 4, 1, fp);
		if (endian != ENDIANNESS_FLAG)
			length = GUINT32_SWAP_LE_BE(length);
	
	  	if (length > 0)
	   	{
	      		ci->sa[i].defaultskinname = (char *)malloc(length + 1);
	      		if (ci->sa[i].defaultskinname == NULL)
				return -1;
	
	      		memset(ci->sa[i].defaultskinname, 0, length + 1);
	      		fread(ci->sa[i].defaultskinname, length, 1, fp);
	    	}
	}	
    	
    fclose(fp);
    return 0;
}


/*
	Read TilEm config informations + print into terminal 
	The same function as config_read but print it.
*/
int config_print(CONFIG_INFOS *ci, const char *filename)
{
	FILE *fp;
  	int i;
  	uint32_t endian;
  	uint32_t length;
	char str[17];


	fp = fopen(filename, "rb");
  	if (fp == NULL)
    	{
      		fprintf(stderr, "Unable to open this file: <%s>\n", filename);
      		return -1;
    	}
 
	/* signature & offsets */
	memset(str, 0, sizeof(str));	
	fread(str, 16, 1, fp);
	printf("%s\n", str);
  	if (strncmp(str, "TilEm v2.00", 16))
  	{
  		fprintf(stderr, "Bad TilEm config format\n");
      	return -1;
  	}
  	fread(&endian, 4, 1, fp);

	if (endian != ENDIANNESS_FLAG)
		printf(">>>>> endian!= ENDIANNESS_FLAG\n");
	
	/* You can store 60 associations */
  	for (i = 0; i < 60; i++)
	{
		/* Rom name */
	  	fread(&length, 4, 1, fp);
		if (endian != ENDIANNESS_FLAG)
			length = GUINT32_SWAP_LE_BE(length);
	
	  	if (length > 0)
	    	{
	      		ci->sa[i].romname = (char *)malloc(length + 1);
		    	if (ci->sa[i].romname == NULL)
				return -1;
	
			memset(ci->sa[i].romname, 0, length + 1);
		    	fread(ci->sa[i].romname, length, 1, fp);
		}

		DCONFIG_FILE_L0_A2("rom_name :%s (lgth=%d)", ci->sa[i].romname, length);
	
		/* Default associated skin */
	  	fread(&length, 4, 1, fp);
		if (endian != ENDIANNESS_FLAG)
			length = GUINT32_SWAP_LE_BE(length);
	
	  	if (length > 0)
	   	{
	      		ci->sa[i].defaultskinname = (char *)malloc(length + 1);
	      		if (ci->sa[i].defaultskinname == NULL)
				return -1;
	
	      		memset(ci->sa[i].defaultskinname, 0, length + 1);
	      		fread(ci->sa[i].defaultskinname, length, 1, fp);
		}
		DCONFIG_FILE_L0_A2(" ---> default_skin : %s (lgth=%d)\n", ci->sa[i].defaultskinname, length);
	    	
	}	
    	
    fclose(fp);
    return 0;
}


void config_load(CONFIG_INFOS *ci)
{
	config_read(ci, "config.dat");
	DCONFIG_FILE_L0_A0("********** fct : config_load *******************\n");
	DCONFIG_FILE_L0_A0("*  config.dat successfully readed                      *\n");
	DCONFIG_FILE_L0_A0("********************************************************\n\n");
	
}
/* Unload skin by freeing allocated memory */
int config_unload(CONFIG_INFOS *ci)
{
	int i=0;

	for(i=0; i< 60; i++)
	{
		free(ci->sa[i].romname);
  		free(ci->sa[i].defaultskinname);
	}
  	memset(ci, 0, sizeof(CONFIG_INFOS));
  
  	return 0;
}


/* First call of tilem : create the config file */
void create_config_dat(GLOBAL_SKIN_INFOS *gsi) 
{
	//FILE *config_file;
	//unsigned char id[16] = TILEM_CONFIG_ID;
	//uint32_t endian= ENDIANNESS_FLAG;
	//CONFIG_INFOS* ci;
	//memset(ci, 0, sizeof(CONFIG_INFOS));
	/*config_file = g_fopen("config.dat", "wb");
		if (config_file) 
		{
			fwrite(id,16,1,config_file);
			fwrite(&endian,4,1, config_file);
			//fwrite(ci,sizeof(CONFIG_INFOS),1,config_file);
			DCONFIG_FILE_L0_A0("****************** CREATE config.dat *******************\n");
			DCONFIG_FILE_L0_A0("*  config.dat successfully created                     *\n");
			DCONFIG_FILE_L0_A0("********************************************************\n\n");
			fclose(config_file);
		}
	*/
	write_config_file(gsi);
	config_print(gsi->ci, "config.dat");
	
}
/* This function is executed when stopping the emulator 
 * It simply save which skin must be useb with wich rom 
 */
void write_config_file(GLOBAL_SKIN_INFOS *gsi) {

			
	
	FILE *config_file;
	unsigned char id[16] = TILEM_CONFIG_ID;
	uint32_t endian= ENDIANNESS_FLAG;
	uint32_t length_romname; 
	uint32_t length_defaultskinname; 
	int i;

	config_file = g_fopen("config.dat", "w");
	
	if (config_file) 
	{	
		fwrite(id,16,1,config_file);
		fwrite(&endian,4,1, config_file);
		
		for(i=0; i<60;i++) {
			if((gsi->ci->sa[i].romname==NULL)||(gsi->ci->sa[i].defaultskinname==NULL))
			{
				length_romname = 0;	
				length_defaultskinname = 0;	
			} else {
				length_romname = strlen(gsi->ci->sa[i].romname);
				length_defaultskinname = strlen(gsi->ci->sa[i].defaultskinname);
			}
		fwrite(&length_romname, 4, 1, config_file);
		fwrite(gsi->ci->sa[i].romname, length_romname, 1, config_file);
		fwrite(&length_defaultskinname, 4, 1, config_file);
		fwrite(gsi->ci->sa[i].defaultskinname, length_defaultskinname, 1, config_file);

		}
		fwrite(gsi->ci,sizeof(CONFIG_INFOS),1,config_file);


		DCONFIG_FILE_L0_A0("****************** WRITE config.dat *******************\n");
		DCONFIG_FILE_L0_A0("*  config.dat successfully modified                     *\n");
		DCONFIG_FILE_L0_A0("********************************************************\n\n");
		fclose(config_file);
	}
}




/* Search in the CONFIG_INFOS to see if the skin is saved */
gboolean is_this_rom_in_config_infos(char* romname,GLOBAL_SKIN_INFOS *gsi)
{	
	int i=0;	
	for(i=0; i<60; i++)
	{
		if(gsi->ci->sa[i].romname!=NULL)
			if(strcmp(romname, gsi->ci->sa[i].romname)==0)
				return TRUE; 
	}	
	
	return FALSE;
}
	
/* Get the skin name for this rom */
void search_defaultskin_in_config_infos(char* romname,GLOBAL_SKIN_INFOS *gsi)
{
	int i=0;	
	for(i=0; i<60; i++)
	{	
		if(gsi->ci->sa[i].romname!=NULL)
			if(strcmp(romname, gsi->ci->sa[i].romname)==0)
			{
				//gsi->SkinFileName=(gchar*)malloc(sizeof("/home/tib/ti84plus.skn")+1);
				//strcpy(gsi->SkinFileName,"/home/tib/ti84plus.skn");
				printf("gsi->SkinFileName : %s \n",gsi->SkinFileName);
				//char* p;
				
				gsi->SkinFileName=(gchar*)malloc(sizeof(gsi->ci->sa[i].defaultskinname)+1);		
				strcpy(gsi->SkinFileName,gsi->ci->sa[i].defaultskinname);
				printf("gsi->SkinFileName : %s \n",gsi->SkinFileName);
				//strrchr(gsi->SkinFileName,".");
				//printf("### P ###  : %s \n",p);
			}
	}	
}	

int add_or_modify_defaultskin(GLOBAL_SKIN_INFOS* gsi) 
{
	int i;
	DCONFIG_FILE_L0_A0("********** fct : add_or_modify_defaultskin ***********\n");
	printf("*  RomName : %s   ", gsi->RomName);
	printf("SkinFileName : %s            \n", gsi->SkinFileName);
	DCONFIG_FILE_L0_A0("********************************************************\n\n");
	
	if(is_this_rom_in_config_infos(gsi->RomName, gsi))
	{

		for(i=0; i<60; i++)
			if(strcmp(gsi->RomName, gsi->ci->sa[i].romname)==0)
		       	{
				gsi->ci->sa[i].defaultskinname= gsi->SkinFileName;
				write_config_file(gsi);
				return 0;
			}
	} else {
		for(i=0; i<60; i++)
			if((gsi->ci->sa[i].romname==NULL)&&(gsi->ci->sa[i].defaultskinname==NULL)) 
			{
				gsi->ci->sa[i].romname= gsi->RomName;
				gsi->ci->sa[i].defaultskinname= gsi->SkinFileName;
				write_config_file(gsi);
				config_print(gsi->ci, "config.dat");
				return 0;
			}
	}
	config_print(gsi->ci, "config.dat");
	return 0;
}
