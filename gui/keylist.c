#include <keylist.h>


/* Load keyboard from file */
void load_keypad_from_file(GLOBAL_SKIN_INFOS * gsi, char* config_file) {
	GKeyFile * gkf;
	gchar** keys;

	int i;
	gsi->kp->nb_of_buttons = 0;
	
	gkf = g_key_file_new();
	
	if (! g_key_file_load_from_file(gkf, config_file, G_KEY_FILE_NONE, NULL)) {
		fprintf(stderr, "Could not read config file '%s'\n", config_file);
		exit(EXIT_FAILURE);
	}
	
	/* Get keys from group called "ti82" or "ti83" or another name. It require core must be launched firstly */	
	keys = g_key_file_get_keys(gkf, gsi->emu->calc->hw.name, NULL, NULL);
	
	for(i = 0; keys && keys[i]; i++) {
		char* code;
		code = g_key_file_get_string(gkf, gsi->emu->calc->hw.name, keys[i], NULL);
		
		gsi->kp->kl[i].label = strdup(keys[i]);	
		gsi->kp->nb_of_buttons ++ ;
		gsi->kp->kl[i].code = atoi(code);
		printf("%d : %s=%d\n", i, gsi->kp->kl[i].label, gsi->kp->kl[i].code);
		free(code);
	}

	g_strfreev(keys);
	g_key_file_free(gkf);
	print_keypad(gsi->kp);
}



/* Load keyboard */
void load_keypad(GLOBAL_SKIN_INFOS* gsi) {

	gsi->kp = malloc(sizeof(KEYPAD)); 	
	gsi->kp->nb_of_buttons = 0;
	load_keypad_from_file(gsi, "keylist.ini"); 
}

/* Print the KEYPAD (for debug) */
void print_keypad(KEYPAD* kp) {
	
	int i = 0;	
	printf("Print a keypad struct:\n\n");
	if(kp) {
		for(i = 0; i < kp->nb_of_buttons; i++) 
			printf("%s=%d\n" , kp->kl[i].label, kp->kl[i].code);
	}	
}
