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
	printf("skinfilename : %s\n", SkinFileName);
	
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
void set_modelcalcid(char* romname, char id) {
	
	GKeyFile * gkf;
	gkf = g_key_file_new();
	
	if (! g_key_file_load_from_file(gkf, CONFIG_FILE, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, NULL)) {
		fprintf(stderr, "Could not read config file '%s'\n", CONFIG_FILE);
		exit(EXIT_FAILURE);
	}
	
	char idstr[2];
	idstr[0] = id;
	idstr[1] = '\0';
	g_key_file_set_string(gkf, "model", romname, idstr);
	
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

/* search, write, and save config on right click menu */
void add_or_modify_defaultskin(GLOBAL_SKIN_INFOS* gsi) {

	set_defaultskin(gsi->RomName, gsi->SkinFileName);
}

/* search, write, and save config on right click menu */
void add_or_modify_defaultmodel(GLOBAL_SKIN_INFOS* gsi) {
	set_modelcalcid(gsi->RomName, gsi->calc_id);

}


/* Search and return the last directory opened to send a file*/
char* get_sendfile_recentdir() {

	GKeyFile * gkf;
	gkf = g_key_file_new();
	
	if (! g_key_file_load_from_file(gkf, CONFIG_FILE, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, NULL)) {
		fprintf(stderr, "Could not read config file '%s'\n", CONFIG_FILE);
		exit(EXIT_FAILURE);
	}

	char* recentdir = g_key_file_get_string(gkf, "upload", "sendfile_recentdir", NULL);
	printf("send file recent dir : %s\n", recentdir);
	
	g_key_file_free(gkf);
	return recentdir;
}

/* Set the last dir opened to send file */
void set_sendfile_recentdir(char* recentdir) {
	
	GKeyFile * gkf;
	gkf = g_key_file_new();
	
	if (! g_key_file_load_from_file(gkf, CONFIG_FILE, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, NULL)) {
		fprintf(stderr, "Could not read config file '%s'\n", CONFIG_FILE);
		exit(EXIT_FAILURE);
	}
	
	g_key_file_set_string(gkf, "upload", "sendfile_recentdir", recentdir);
	
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
