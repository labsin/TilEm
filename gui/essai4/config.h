#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <glib/gstdio.h>
//#include <gui.h>

#ifndef CONFIG_FILE
#define CONFIG_FILE "config.ini"
#endif

/* Search and return the default skin for this model */
char* get_defaultskin(char* romname);

/* Set a default skin, or add it if not exists */
void set_defaultskin(char* romname, char* skinname);

/* Search the most recent rom */
char* get_recentrom(char* romname);

/* Set a default skin, or add it if not exists */
void set_recentrom(char* romname);

/* get the model */
char get_modelcalcid(char* romname);

/* Set model calc id */
void set_modelcalcid(char* romname, char* id);
