
#include <keylist.h>


/* Load keyboard from file */
void load_keypad_from_file(KEYPAD * kp, char* config_file) {
	GKeyFile * gkf;
	gchar** groups;
	gchar** keys;

	int i;
	int j=0;
	
	gkf = g_key_file_new();
	
	if (! g_key_file_load_from_file(gkf, config_file, G_KEY_FILE_NONE, NULL)) {
		fprintf(stderr, "Could not read config file '%s'\n", config_file);
		exit(EXIT_FAILURE);
	}
	
	/* Get groups (groups are '[foo1]') */ 
	groups = g_key_file_get_groups(gkf, NULL);
	for(j = 0; groups[j]; j++) {
	
		keys = g_key_file_get_keys(gkf,groups[j], NULL, NULL);
		for(i = 0; keys[i]; i++) {
			char* code;
			code = g_key_file_get_string(gkf, groups[j], keys[i], NULL);
			
			kp->kl[i].label = strdup(keys[i]);	
			kp->nb_of_buttons ++ ;
			kp->kl[i].code = atoi(code);
			printf("%d : %s=%d\n", i, kp->kl[i].label, kp->kl[i].code);
			free(code);
		}
		g_strfreev(keys);
	}
	g_strfreev(groups);
	g_key_file_free(gkf);
	print_keypad(kp);
}



/* Load keyboard */
void load_keypad(GLOBAL_SKIN_INFOS* gsi) {

	KEYPAD *kp = malloc(sizeof(KEYPAD)); 	
	//kp->kl = malloc(sizeof(KEY_LIST) * 60);
	kp->nb_of_buttons = 0;
	load_keypad_from_file(kp, "keylist.ini"); 
}

/* Print the KEYPAD */
void print_keypad(KEYPAD* kp) {
	
	int i = 0;	
	printf("Print a keypad struct:\n\n");
	if(kp) {
		for(i = 0; i < kp->nb_of_buttons; i++) 
			printf("%s=%d\n" , kp->kl[i].label, kp->kl[i].code);
	}	
}
