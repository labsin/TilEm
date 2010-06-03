#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <gui.h>


void create_config_dat() 
{
	FILE *config_file;
	config_file = g_fopen("config.dat", "w");
		if (config_file) 
		{
			unsigned char magic[11]="TilEm v2.00";
			fwrite(magic,11,1,config_file);
			DCONFIG_FILE_L0_A0("****************** CREATE config.dat *******************\n");
			DCONFIG_FILE_L0_A0("*  config.dat successfully created                     *\n");
			DCONFIG_FILE_L0_A0("********************************************************\n\n");
			fclose(config_file);
		}
	
}

void write_default_skin_for_specific_rom(GLOBAL_SKIN_INFOS *gsi) 
{
	FILE *config_file;
	config_file = g_fopen("config.dat", "a");
		if (config_file) 
		{
			
			fwrite("\n",1,1,config_file);
			fwrite("*",1,1,config_file);
			fwrite(gsi->RomName,strlen(gsi->RomName),1,config_file);
			fwrite("=",1,1,config_file);
			fwrite(gsi->SkinFileName,strlen(gsi->SkinFileName),1,config_file);
			DCONFIG_FILE_L0_A0("****************** WRITE config.dat *******************\n");
			DCONFIG_FILE_L0_A0("*  config.dat successfully modified                     *\n");
			DCONFIG_FILE_L0_A0("********************************************************\n\n");
			fclose(config_file);
		}
	
}

char* get_default_skin_for_specific_rom(GLOBAL_SKIN_INFOS *gsi) 
{
	FILE *config_file;
	char c;
	config_file = g_fopen("config.dat", "a");
		if (config_file) 
		{
			while((c=fgetc(config_file))!='*'){
			}
			if((c=fgetc(config_file))!='*')
				search_string("toto",gsi);
		}
		return "toto";
	
}

gboolean search_string(char* string,GLOBAL_SKIN_INFOS *gsi)
{
	gboolean result;
	char c;
	FILE *config_file;
	config_file = g_fopen("config.dat", "r");
	while((c=fgetc(config_file))!=EOF) {
		if(c=='*')
			result=cmp_string(string,gsi);
	}

	return result;
	
}	

gboolean cmp_string(char* string, GLOBAL_SKIN_INFOS *gsi) {
	int i=0;
	char c;
	FILE *config_file;
	string=string;
	config_file = g_fopen("config.dat", "r");
	while((c=fgetc(config_file))!='=') {
		if(c!=gsi->RomName[i])
			return FALSE;
		i++;
	}
	return TRUE;
}
	
	

