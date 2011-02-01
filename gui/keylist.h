#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <gui.h>

/* load the keyboard from file ... */
void load_keypad_from_file(KEYPAD * kp, char* config_file) ;

/* Print the keypad */
void print_keypad(KEYPAD* kp) ;
