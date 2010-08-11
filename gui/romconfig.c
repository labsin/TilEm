#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <gui.h>



/*
	Read TilEm config informations 
*/
int romconfig_read(ROMCONFIG_INFOS *rci, const char *filename)
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
	      		rci->ma[i].romname = (char *)malloc(length);
		    	if (rci->ma[i].romname == NULL)
				return -1;
	
			memset(rci->ma[i].romname, 0, length);
		    	fread(rci->ma[i].romname, length, 1, fp);
		}
		DCONFIG_FILE_L0_A2("rom_name :%s (lgth=%d)", rci->ma[i].romname, length);
	
	      	rci->ma[i].defaultmodel='0';
	      	fread(&rci->ma[i].defaultmodel, 1, 1, fp);

		DCONFIG_FILE_L0_A1(" ---> default_model : %c \n", rci->ma[i].defaultmodel);
	    	
	}	
    	
    fclose(fp);
    return 0;
}

void romconfig_load(ROMCONFIG_INFOS *rci)
{
	romconfig_read(rci, "romconfig.dat");
	DCONFIG_FILE_L0_A0("********** fct : config_load *******************\n");
	DCONFIG_FILE_L0_A0("*  romconfig.dat successfully readed                      *\n");
	DCONFIG_FILE_L0_A0("********************************************************\n\n");
	
}
/* Unload skin by freeing allocated memory */
int romconfig_unload(ROMCONFIG_INFOS *rci)
{
	int i=0;

	for(i=0; i< 60; i++)
	{
		free(rci->ma[i].romname);
  		rci->ma[i].defaultmodel='0';
	}
  	memset(rci, 0, sizeof(ROMCONFIG_INFOS));
  
  	return 0;
}


/* First call of tilem : create the config file */
void create_romconfig_dat(GLOBAL_SKIN_INFOS *gsi) 
{
	int i;
	//FILE *config_file;
	//unsigned char id[16] = TILEM_CONFIG_ID;
	//uint32_t endian= ENDIANNESS_FLAG;
	//CONFIG_INFOS* ci;
	//memset(ci, 0, sizeof(CONFIG_INFOS));
	/*config_file = g_fopen("romconfig.dat", "wb");
		if (config_file) 
		{
			fwrite(id,16,1,config_file);
			fwrite(&endian,4,1, config_file);
			//fwrite(ci,sizeof(CONFIG_INFOS),1,config_file);
			DCONFIG_FILE_L0_A0("****************** CREATE romconfig.dat *******************\n");
			DCONFIG_FILE_L0_A0("*  romconfig.dat successfully created                     *\n");
			DCONFIG_FILE_L0_A0("********************************************************\n\n");
			fclose(config_file);
		}
	*/
	for(i=0; i<60; i++)
		gsi->rci->ma[i].defaultmodel='0';

	write_romconfig_file(gsi);
	romconfig_read(gsi->rci, "romconfig.dat");
	
}
/* This function is executed when stopping the emulator 
 * It simply mave which skin must be useb with wich rom 
 */
void write_romconfig_file(GLOBAL_SKIN_INFOS *gsi) {

			
	
	FILE *config_file;
	unsigned char id[16] = TILEM_CONFIG_ID;
	uint32_t endian= ENDIANNESS_FLAG;
	uint32_t length_romname; 
	int i;

	config_file = g_fopen("romconfig.dat", "w");
	
	if (config_file) 
	{	
		fwrite(id,16,1,config_file);
		fwrite(&endian,4,1, config_file);
		
		for(i=0; i<60;i++) {
			if((gsi->rci->ma[i].romname==NULL)||(gsi->rci->ma[i].defaultmodel=='0'))
			{
				length_romname = 0;	
			} else {
				length_romname = strlen(gsi->rci->ma[i].romname) +1;
			}
				
			fwrite(&length_romname, 4, 1, config_file);
			fwrite(gsi->rci->ma[i].romname, length_romname, 1, config_file);
			fwrite(&gsi->rci->ma[i].defaultmodel, 1, 1, config_file);

		}
		//fwrite(gsi->ci,sizeof(ROMCONFIG_INFOS),1,config_file);


		DCONFIG_FILE_L0_A0("****************** WRITE romconfig.dat *******************\n");
		DCONFIG_FILE_L0_A0("*  romconfig.dat successfully modified                     *\n");
		DCONFIG_FILE_L0_A0("********************************************************\n\n");
		fclose(config_file);
	}
}




/* Search in the ROMCONFIG_INFOS to see if the skin is saved */
gboolean is_this_rom_in_romconfig_infos(char* romname,GLOBAL_SKIN_INFOS *gsi)
{	
	int i=0;	
	for(i=0; i<60; i++)
	{
		if(gsi->rci->ma[i].romname!=NULL)
			if(strcmp(romname, gsi->rci->ma[i].romname)==0)
				return TRUE; 
	}	
	
	return FALSE;
}
	
/* Get the skin name for this rom */
void search_defaultmodel_in_romconfig_infos(char* romname,GLOBAL_SKIN_INFOS *gsi)
{
	int i=0;	
	for(i=0; i<60; i++)
	{	
		if(gsi->rci->ma[i].romname!=NULL)
			if(strcmp(romname, gsi->rci->ma[i].romname)==0) {
				gsi->calc_id = gsi->rci->ma[i].defaultmodel;
				DCONFIG_FILE_L0_A0("* search_defaultmodel_in_romconfig_infos (romconfig.dat)*\n");
				DCONFIG_FILE_L0_A1("*  gsi->ci->sa[i].defaultmodel = %c *\n",gsi->rci->ma[i].defaultmodel);
				DCONFIG_FILE_L0_A1("*  gsi->calc_id (used to choose which to load) = %c *\n", gsi->calc_id);
				DCONFIG_FILE_L0_A0("********************************************************\n\n");
			}
	}	
}	

void add_or_modify_defaultmodel(GLOBAL_SKIN_INFOS* gsi) 
{
	int i;
	DCONFIG_FILE_L0_A0("********** fct : add_or_modify_defaultskin ***********\n");
	printf("*  RomName : %s   ", gsi->RomName);
	printf("gsi->calc_id : %c            \n", gsi->calc_id);
	DCONFIG_FILE_L0_A0("********************************************************\n\n");
	
	if(is_this_rom_in_romconfig_infos(gsi->RomName, gsi))
	{

		for(i=0; i<60; i++)
			if(strcmp(gsi->RomName, gsi->rci->ma[i].romname)==0)
		       	{
				gsi->rci->ma[i].defaultmodel= gsi->calc_id;
				write_romconfig_file(gsi);
				break;
			}
	} else {
		for(i=0; i<60; i++)
			if((gsi->rci->ma[i].romname==NULL)&&(gsi->rci->ma[i].defaultmodel=='0')) 
			{
				gsi->rci->ma[i].romname= gsi->RomName;
				gsi->rci->ma[i].defaultmodel= gsi->calc_id;
				write_romconfig_file(gsi);
				romconfig_read(gsi->rci, "romconfig.dat");
				break;
			}
	}
	romconfig_read(gsi->rci, "romconfig.dat");
}
