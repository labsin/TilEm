#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <gui.h>


void on_about(GtkWidget *pBtn, gpointer data);

/* Create the SKIN file selector */
void SkinSelection(GLOBAL_SKIN_INFOS *gsi);

/* Get the skin file selected */
void GetSkinSelected(GLOBAL_SKIN_INFOS *gsi);

void on_valid(GtkWidget *pBtn, GLOBAL_SKIN_INFOS *gsi);