#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <gui.h>

/* Show the "On about" window */
void on_about(GtkWidget *pBtn, gpointer data);

/* Search wich key is clicked */
static int scan_click(int MAX, double x, double y, GLOBAL_SKIN_INFOS * gsi);

/* Reset calc */
void on_reset(GLOBAL_SKIN_INFOS * gsi);
