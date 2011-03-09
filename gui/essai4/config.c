#include "config.h" 




/* Search and return the default skin for this model */
char* get_defaultskin(char* romname) {

	GKeyFile * gkf;
	gkf = g_key_file_new();
	
	if (! g_key_file_load_from_file(gkf, CONFIG_FILE, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, NULL)) {
		fprintf(stderr, "Could not read config file '%s'\n", CONFIG_FILE);
		exit(EXIT_FAILURE);
	}

	char* SkinFileName = g_key_file_get_string(gkf, "skin", romname, NULL);

	
	g_key_file_free(gkf);
	return SkinFileName;
}

/* Set a default skin, or add it if not exists */
void set_defaultskin(char* romname, char* skinname) {
	
	GKeyFile * gkf;
	gkf = g_key_file_new();
	
	if (! g_key_file_load_from_file(gkf, CONFIG_FILE, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, NULL)) {
		fprintf(stderr, "Could not read config file '%s'\n", CONFIG_FILE);
		exit(EXIT_FAILURE);
	}
	
	g_key_file_set_string(gkf, "skin", romname, skinname);
	
	/* Save the config */
	FILE *file;
        char *data;

        if (!(file = fopen (CONFIG_FILE, "w")))
        {
            fprintf(stderr, "Could not open file: %s", CONFIG_FILE);
            return;
        }
        data = g_key_file_to_data (gkf, NULL, NULL);
        fputs (data, file);
        fclose (file);
	g_key_file_free(gkf);
        g_free (data); 
}

/* Search the most recent rom */
char* get_recentrom(char* romname) {

	GKeyFile * gkf;
	gkf = g_key_file_new();
	
	if (! g_key_file_load_from_file(gkf, CONFIG_FILE, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, NULL)) {
		fprintf(stderr, "Could not read config file '%s'\n", CONFIG_FILE);
		exit(EXIT_FAILURE);
	}

	char* recentrom = g_key_file_get_string(gkf, "skin", romname, NULL);

	
	g_key_file_free(gkf);
	return recentrom;
}




/* Set a default skin, or add it if not exists */
void set_recentrom(char* romname) {
	
	GKeyFile * gkf;
	gchar** keys;
	
	gkf = g_key_file_new();
	
	if (! g_key_file_load_from_file(gkf, CONFIG_FILE, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, NULL)) {
		fprintf(stderr, "Could not read config file '%s'\n", CONFIG_FILE);
		exit(EXIT_FAILURE);
	}
	char* temp;
	char* lastrom;
	int i = 0;	
		
	keys = g_key_file_get_keys(gkf, "recent", NULL, NULL);
	
	/* Shift from top to bottom */	
	for(i = 9; i > 0; i--) {	
		printf("%s\n", g_key_file_get_string(gkf, "recent", keys[(i-1)], NULL));
		g_key_file_set_string(gkf, "recent", keys[i], g_key_file_get_string(gkf, "recent", keys[(i-1)], NULL));
		
	}
	
	/* write the most recent rom */
	g_key_file_set_string(gkf, "recent", keys[0], romname);
	
	/* Save the config */
	FILE *file;
        char *data;

        if (!(file = fopen (CONFIG_FILE, "w")))
        {
            fprintf(stderr, "Could not open file: %s", CONFIG_FILE);
            return;
        }
        data = g_key_file_to_data (gkf, NULL, NULL);
        fputs (data, file);
        fclose (file);
	
	g_strfreev(keys);
	g_key_file_free(gkf);
        g_free (data); 
}

char get_modelcalcid(char* romname) {
	GKeyFile * gkf;
	gkf = g_key_file_new();
	
	if (! g_key_file_load_from_file(gkf, CONFIG_FILE, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, NULL)) {
		fprintf(stderr, "Could not read config file '%s'\n", CONFIG_FILE);
		exit(EXIT_FAILURE);
	}

	char* pcalc_id = g_key_file_get_string(gkf, "model", romname, NULL);
	if(pcalc_id == NULL) {
		printf("Not found :\n");
		return '0';
	}
	
	g_key_file_free(gkf);
	//printf("calc_id : %s\n", pcalc_id);
	return pcalc_id[0];
}

/* Set model calc id */
void set_modelcalcid(char* romname, char* id) {
	
	GKeyFile * gkf;
	gkf = g_key_file_new();
	
	if (! g_key_file_load_from_file(gkf, CONFIG_FILE, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, NULL)) {
		fprintf(stderr, "Could not read config file '%s'\n", CONFIG_FILE);
		exit(EXIT_FAILURE);
	}
	
	g_key_file_set_string(gkf, "model", romname, id);
	
	/* Save the config */
	FILE *file;
        char *data;

        if (!(file = fopen (CONFIG_FILE, "w")))
        {
            fprintf(stderr, "Could not open file: %s", CONFIG_FILE);
            return;
        }
        data = g_key_file_to_data (gkf, NULL, NULL);
        fputs (data, file);
        fclose (file);
	g_key_file_free(gkf);
        g_free (data); 
}

int main(int argc, char* argv[]) {


	char* rom0 = "/home/tib/roms/ti83_110.rom";
	char* rom1 = "/home/tib/roms/ti83p_112.rom";
	char* romerror = "/home/tib/roms/ti83p_112sdfgsdfgsdfg.rom";
	char* skin0 = "/home/tib/Code/tilem190/trunk/gui/skn/ti86.skn";
	
	set_defaultskin(rom0, skin0);

	printf("%s = %s\n", rom0, get_defaultskin(rom0));
	printf("%s = %s\n", rom1, get_defaultskin(rom1));
	printf("%s = %s\n", romerror, get_defaultskin(romerror));
	
	printf("%s = %s\n", rom1, get_defaultskin(rom1));
	set_recentrom("test2");
	printf("calc id : %c\n", get_modelcalcid("ti83_110.rom"));
	set_modelcalcid("ti83_110.rom", "4");
	printf("calc id : %c\n", get_modelcalcid("ti83_110.rom"));
	
	return 0;

}

