#include <stdio.h>
#include <gtk/gtk.h>
#include <string.h>
#include <malloc.h>
#include <gui.h>
#include <glib/gstdio.h>



/* Callback event */
void on_screenshot();
void on_record(GtkWidget* win, GLOBAL_SKIN_INFOS* gsi);
void on_add_frame(GtkWidget* win, GLOBAL_SKIN_INFOS* gsi);
void on_stop();
void on_play();

void screenshot(GLOBAL_SKIN_INFOS *gsi);

static gboolean save_screenshot(GLOBAL_SKIN_INFOS *gsi, const char *filename, const char *format);
