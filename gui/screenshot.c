/*
 * TilEm II
 *
 * Copyright (c) 2010-2011 Thibault Duponchelle
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <ticalcs.h>
#include <tilem.h>

#include "gui.h"

void on_screenshot();
void on_record(GtkWidget* win, GLOBAL_SKIN_INFOS* gsi);
void on_add_frame(GtkWidget* win, GLOBAL_SKIN_INFOS* gsi);
void on_stop(GtkWidget* win, GLOBAL_SKIN_INFOS* gsi);
void on_play(GLOBAL_SKIN_INFOS* gsi);
void on_destroy_playview(GtkWidget* playwin);

void screenshot(GLOBAL_SKIN_INFOS *gsi);

static gboolean save_screenshot(GLOBAL_SKIN_INFOS *gsi, const char *filename, const char *format);

/* This method is called from click event */
void screenshot(GLOBAL_SKIN_INFOS *gsi) {
	int i;
	FILE * fp;
	char *buffer;
	char * filename;
	filename = (char*) malloc((strlen("screenshot")+8)*sizeof(char));
	strcpy(filename, "screenshot");
	
	for(i=0; i<500; i++) {
		/* Complicated method just to find a free filename (don not save more than 500 screenshots ! I know this is stupid) */
		buffer = (char*) malloc(3);
		sprintf(buffer,"%03d", i);
		strcpy(filename, "screenshot");
		strcat(filename, buffer);
		strcat(filename, ".png");
		//printf("filename : %s\n", filename);

		if((fp = g_fopen(filename,"r"))) {
			fclose(fp);
		} else { 
			save_screenshot(gsi, filename, "png");
			break;
		}
	}
}

/* Destroy the screenshot box */
void on_destroy_screenshot(GtkWidget* screenshotanim_win)   {
	
	gtk_widget_destroy(GTK_WIDGET(screenshotanim_win));
}

/* Create the screenshot menu */
void create_screenshot_window(GLOBAL_SKIN_INFOS* gsi) {
	GtkWidget* screenshotanim_win;
	screenshotanim_win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(screenshotanim_win), "Screenshot");
	gtk_window_set_default_size(GTK_WINDOW(screenshotanim_win), 400,30);
	
	g_signal_connect(GTK_OBJECT(screenshotanim_win), "delete-event", G_CALLBACK(on_destroy_screenshot), NULL);

	GtkWidget* hbox, *vbox, *parent_vbox;
	parent_vbox = gtk_vbox_new (0, 1);
	vbox = gtk_vbox_new(0,1);
	hbox = gtk_hbox_new (0, 1);
	
	
		
	GtkWidget * screenshot_preview = gtk_expander_new("preview");
	gtk_expander_set_expanded(GTK_EXPANDER(screenshot_preview), TRUE);
	gsi->screenshot_preview_image = gtk_image_new_from_file(tilem_config_universal_getter("screenshot", "animation_recent"));
	gtk_container_add(GTK_CONTAINER(screenshot_preview), gsi->screenshot_preview_image);

	gtk_container_add(GTK_CONTAINER(screenshotanim_win), parent_vbox);
	gtk_box_pack_start(GTK_BOX(parent_vbox), hbox, 2, 3, 4);
	gtk_box_pack_start(GTK_BOX(hbox), screenshot_preview, 2, 3, 4);
	gtk_box_pack_end(GTK_BOX(hbox), vbox, 2, 3, 4);

		
	GtkWidget* screenshot_button = gtk_button_new_with_label ("Shoot!");
	GtkWidget* record = gtk_button_new_with_label ("Record");
	//GtkWidget* add_frame = gtk_button_new_with_label ("Add frame (anim)");
	GtkWidget* stop = gtk_button_new_with_label ("Stop");
	GtkWidget* play = gtk_button_new_with_label ("Play (detached)");
	GtkWidget* animation_directory = gtk_button_new_with_label (tilem_config_universal_getter("screenshot", "animation_directory"));
	GtkWidget* screenshot_directory = gtk_button_new_with_label (tilem_config_universal_getter("screenshot", "screenshot_directory"));
	
	gtk_box_pack_start (GTK_BOX (vbox), screenshot_button, 2, 3, 4);
	gtk_widget_show(screenshot_button);
	gtk_box_pack_start (GTK_BOX (vbox), record, 2, 3, 4);
	gtk_widget_show(record);
	//gtk_box_pack_start (GTK_BOX (hbox), add_frame, 2, 3, 4);
	//gtk_widget_show(add_frame);
	gtk_box_pack_start (GTK_BOX (vbox), stop, 2, 3, 4);
	gtk_widget_show(stop);
	gtk_box_pack_start (GTK_BOX (vbox), play, 2, 3, 4);
	gtk_widget_show(play);

	gtk_box_pack_start (GTK_BOX (parent_vbox), animation_directory, 2, 3, 4);
	gtk_widget_show(animation_directory);
	gtk_box_pack_end (GTK_BOX (parent_vbox), screenshot_directory, 2, 3, 4);
	gtk_widget_show(screenshot_directory);
	
	g_signal_connect(GTK_OBJECT(screenshot_button), "clicked", G_CALLBACK(on_screenshot), gsi);
	g_signal_connect(GTK_OBJECT(record), "clicked", G_CALLBACK(on_record), gsi);
	//g_signal_connect(GTK_OBJECT(add_frame), "clicked", G_CALLBACK(on_add_frame), gsi);
	g_signal_connect(GTK_OBJECT(stop), "clicked", G_CALLBACK(on_stop), gsi);
	g_signal_connect(GTK_OBJECT(play), "clicked", G_CALLBACK(on_play), gsi);
    
	gtk_widget_show_all(screenshotanim_win);
}

/* Callback for record button */
void on_record(GtkWidget* win, GLOBAL_SKIN_INFOS* gsi) {
	win = win;
	g_print("record event\n");
	tilem_animation_start(gsi);
}

/* Only used for testing purpose */
void on_add_frame(GtkWidget* win, GLOBAL_SKIN_INFOS* gsi) {
	win = win;
	g_print("record event\n");
	tilem_animation_add_frame(gsi);
}

/* Callback for stop button (stop the recording) */
void on_stop(GtkWidget* win, GLOBAL_SKIN_INFOS* gsi) {
	win = win;
	g_print("stop event\n");
	char* dest = NULL;
	if(gsi->isAnimScreenshotRecording) 
		dest = select_file_for_save(gsi, tilem_config_universal_getter("screenshot", "animation_directory"));
	tilem_animation_stop(gsi) ;
	if(dest) {
		set_animation_recentfile(dest);	
		copy_paste("gifencod.gif", dest);
		char* p =  strrchr(dest, '/');
		printf("%s", p);
		if(p)
			strcpy(p, "\0");
		printf("%s", dest);
		
		set_animation_recentdir(dest);	
	}

	
	if(GTK_IS_WIDGET(gsi->screenshot_preview_image)) {
		GtkWidget * screenshot_preview = gtk_widget_get_parent(GTK_WIDGET(gsi->screenshot_preview_image));
		gtk_object_destroy(GTK_OBJECT(gsi->screenshot_preview_image));
		gsi->screenshot_preview_image = gtk_image_new_from_file(tilem_config_universal_getter("screenshot", "animation_recent"));
		gtk_widget_show(gsi->screenshot_preview_image);
		gtk_container_add(GTK_CONTAINER(screenshot_preview), gsi->screenshot_preview_image);
	}
	
}

/* Callback for screenshot button (take a screenshot) */
void on_screenshot(GtkWidget* win, GLOBAL_SKIN_INFOS* gsi) {
	win = win;
	screenshot(gsi);
	g_print("screenshot event\n");
}


/* Screenshot saver */
static gboolean save_screenshot(GLOBAL_SKIN_INFOS *gsi, const char *filename,
                                const char *format)
{
	int width = gsi->emu->calc->hw.lcdwidth * 2;
	int height = gsi->emu->calc->hw.lcdheight * 2;
	guchar *buffer;
	dword *palette;
	GdkPixbuf *pb;
	gboolean status;
	GError *err = NULL;

	buffer = g_new(guchar, width * height * 3);

	/* FIXME: this palette should be cached for future use.  Also
	   might want the option to save images in skinned colors. */
	palette = tilem_color_palette_new(255, 255, 255, 0, 0, 0, 2.2);

	g_mutex_lock(gsi->emu->lcd_mutex);
	tilem_gray_lcd_draw_image_rgb(gsi->emu->glcd, buffer,
	                              width, height, width * 3, 3,
	                              palette, TILEM_SCALE_SMOOTH);
	g_mutex_unlock(gsi->emu->lcd_mutex);

	tilem_free(palette);

	pb = gdk_pixbuf_new_from_data(buffer, GDK_COLORSPACE_RGB, FALSE, 8,
	                              width, height, width * 3, NULL, NULL);

	status = gdk_pixbuf_save(pb, filename, format, &err, NULL);

	g_object_unref(pb);
	g_free(buffer);

	if (!status) {
		fprintf(stderr, "*** unable to save screenshot: %s\n", err->message);
		g_error_free(err);
	}

	return status;
}

/* Destroy the screenshot box */
void on_destroy_playview(GtkWidget* playwin)   {
	
	gtk_widget_destroy(GTK_WIDGET(playwin));
}

/* Callback for play button (replay the last gif) */
void on_play(GLOBAL_SKIN_INFOS* gsi) {
	g_print("play event : %d\n", gsi->isAnimScreenshotRecording);

	printf("play\n");
	GtkWidget *fenetre = NULL;
	GtkWidget *image = NULL;

	fenetre = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(fenetre),"destroy",G_CALLBACK(on_destroy_playview), NULL);

	image = gtk_image_new_from_file(tilem_config_universal_getter("screenshot", "animation_recent"));
	gtk_container_add(GTK_CONTAINER(fenetre),image);

	gtk_widget_show_all(fenetre);

}
	 
