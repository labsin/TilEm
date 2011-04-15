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
#include "files.h"

static void on_screenshot();
static void on_record(GtkWidget* win, GLOBAL_SKIN_INFOS* gsi);
static void on_stop(GtkWidget* win, GLOBAL_SKIN_INFOS* gsi);
static void on_play(GtkWidget* win,GLOBAL_SKIN_INFOS* gsi);
static void on_playfrom(GtkWidget* win,GLOBAL_SKIN_INFOS* gsi);
static void on_destroy_playview(GtkWidget* playwin);
static void on_change_screenshot_directory(GtkWidget * win, GLOBAL_SKIN_INFOS* gsi);
static void on_change_animation_directory(GtkWidget * win, GLOBAL_SKIN_INFOS* gsi);
static gboolean save_screenshot(GLOBAL_SKIN_INFOS *gsi, const char *filename, const char *format);
char* find_free_filename(const char* directory, const char* filename, const char* extension);
static void change_review_image(GLOBAL_SKIN_INFOS * gsi, char * new_image);
static void start_spinner(GLOBAL_SKIN_INFOS * gsi);
static void stop_spinner(GLOBAL_SKIN_INFOS * gsi);
static void delete_spinner_and_put_logo(GLOBAL_SKIN_INFOS * gsi);

/* UNUSED */
void on_add_frame(GtkWidget* win, GLOBAL_SKIN_INFOS* gsi);

/* This method is called from click event */
void screenshot(GLOBAL_SKIN_INFOS *gsi) {

	char basename[11] = "screenshot";
	char* folder = tilem_config_universal_getter("screenshot", "screenshot_directory");
	printf("folder : %s\n", folder);
	
	/* Look for the next free filename (don't erase previous screenshots) */
	char* filename = find_free_filename(folder, basename, ".png\0");

	if(filename)	
		save_screenshot(gsi, filename, "png");
	 
	set_screenshot_recentfile(filename);
	change_review_image(gsi, filename);
}


char* find_free_filename(const char* folder, const char* basename, const char* extension) {
	
	char *buffer;
	FILE * fp;
	int i;
	char* filename;

	/* Complicated method just to find a free filename (do not save more than 999 screenshots ! I know this is stupid) */
	for(i=0; i<999; i++) {
		buffer = (char*) malloc(3);
		if(folder) {
			filename = (char*) malloc((strlen(folder) + strlen(basename) + 9)*sizeof(char));
			strcpy(filename, folder);
			strcat(filename, "/");
			strcat(filename, basename);
			printf("temp : %s\n", filename);
		} else {
			filename = (char*) malloc((strlen(basename)+9)*sizeof(char) );
			strcpy(filename, basename);
		}
		printf("temp2 : %s\n", filename);
		sprintf(buffer,"%03d", i);
		strcat(filename, buffer);
		strcat(filename, extension);
		
		if((fp = g_fopen(filename,"r"))) {
			fclose(fp);
		} else {
			printf("filename : %s\n", filename);
			return filename;
			break;
		}
	}
	return NULL;
}


/* Replace the current logo/review image by the recent shot/animation */
static void change_review_image(GLOBAL_SKIN_INFOS * gsi, char * new_image) {

	/* Test if the widget exists (should exists), if not don't try to change the image */
	if(GTK_IS_WIDGET(gsi->screenshot_preview_image)) {
		GtkWidget * screenshot_preview = gtk_widget_get_parent(GTK_WIDGET(gsi->screenshot_preview_image));
		if(GTK_IS_SPINNER(gsi->screenshot_preview_image)) {
			printf("is spinner\n");
			gtk_spinner_stop(GTK_SPINNER(gsi->screenshot_preview_image));
		}
		gtk_object_destroy(GTK_OBJECT(gsi->screenshot_preview_image));
		gsi->screenshot_preview_image = gtk_image_new_from_file(new_image);
		gtk_widget_show(gsi->screenshot_preview_image);
		gtk_layout_put(GTK_LAYOUT(screenshot_preview), gsi->screenshot_preview_image, 10, 10);
		gtk_widget_show(screenshot_preview);
	}

}


/* Replace the current logo/review image by the recent shot/animation */
static void start_spinner(GLOBAL_SKIN_INFOS * gsi) {

	/* Test if the widget exists (should exists), if not don't try to change the image */
	if(GTK_IS_WIDGET(gsi->screenshot_preview_image)) {

		GtkWidget * layout = gtk_widget_get_parent(GTK_WIDGET(gsi->screenshot_preview_image));
		gtk_object_destroy(GTK_OBJECT(gsi->screenshot_preview_image));
		
		gsi->screenshot_preview_image = gtk_spinner_new();
		gtk_spinner_start(GTK_SPINNER(gsi->screenshot_preview_image));	
		
		gtk_widget_show(gsi->screenshot_preview_image);
		gtk_layout_put(GTK_LAYOUT(layout), gsi->screenshot_preview_image, 100, 60);
		gtk_widget_show(layout);
	}
}

static void stop_spinner(GLOBAL_SKIN_INFOS * gsi) {
	
		if(GTK_IS_SPINNER(gsi->screenshot_preview_image)) {
			printf("is spinner\n");
			gtk_spinner_stop(GTK_SPINNER(gsi->screenshot_preview_image));
			gtk_widget_hide(GTK_WIDGET(gsi->screenshot_preview_image));
		}
}

static void delete_spinner_and_put_logo(GLOBAL_SKIN_INFOS * gsi) {
	
		if(GTK_IS_SPINNER(gsi->screenshot_preview_image)) {
			printf("is spinner\n");
			gtk_spinner_stop(GTK_SPINNER(gsi->screenshot_preview_image));
			GtkWidget * screenshot_preview = gtk_widget_get_parent(GTK_WIDGET(gsi->screenshot_preview_image));
			gtk_object_destroy(GTK_OBJECT(gsi->screenshot_preview_image));
			char* tilem_logo = get_shared_file_path("pixs", "tilem.png", NULL);
			if(tilem_logo)
				gsi->screenshot_preview_image = gtk_image_new_from_file(tilem_logo);
			gsi->screenshot_preview_image = gtk_image_new_from_file(tilem_logo);
			gtk_widget_show(gsi->screenshot_preview_image);
			gtk_layout_put(GTK_LAYOUT(screenshot_preview), gsi->screenshot_preview_image, 10, 10);
			gtk_widget_show(screenshot_preview);
			
		}
}




/* Destroy the screenshot box */
static void on_destroy_screenshot(GtkWidget* screenshotanim_win)   {
	
	gtk_widget_destroy(GTK_WIDGET(screenshotanim_win));
}

/* Create the screenshot menu */
void create_screenshot_window(GLOBAL_SKIN_INFOS* gsi) {
	GtkWidget* screenshotanim_win;
	screenshotanim_win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(screenshotanim_win), "Screenshot");
	gtk_window_set_default_size(GTK_WINDOW(screenshotanim_win) , 450, 300);
	
	g_signal_connect(GTK_OBJECT(screenshotanim_win), "delete-event", G_CALLBACK(on_destroy_screenshot), NULL);

	GtkWidget* hbox, *vbox, *parent_vbox;
	parent_vbox = gtk_vbox_new (0, 1);
	vbox = gtk_vbox_new(0,1);
	hbox = gtk_hbox_new (0, 1);
	gtk_box_set_homogeneous(GTK_BOX(hbox), TRUE);	
	
	
		
	GtkWidget * screenshot_preview = gtk_expander_new("preview");
	gtk_expander_set_expanded(GTK_EXPANDER(screenshot_preview), TRUE);
	GtkWidget* layout = gtk_layout_new(NULL,NULL);
	gtk_layout_set_size(GTK_LAYOUT(layout), 200, 100); 
	
	/* Print the nice logo (from old tilem) 
	   Maybe try to load the most recent gif/screenshot could be a good idea... (as it was done before)
	   But I think it's good like this, because we don't really need to see last session screenshot */ 
	char* tilem_logo = get_shared_file_path("pixs", "tilem.png", NULL);
	if(tilem_logo)
		gsi->screenshot_preview_image = gtk_image_new_from_file(tilem_logo);
	else 
		gsi->screenshot_preview_image = gtk_image_new();

	gtk_layout_put(GTK_LAYOUT(layout), gsi->screenshot_preview_image, 10, 10);
	gtk_container_add(GTK_CONTAINER(screenshot_preview), layout);

	gtk_container_add(GTK_CONTAINER(screenshotanim_win), parent_vbox);
	gtk_box_pack_start(GTK_BOX(parent_vbox), hbox, 2, 3, 4);
	gtk_box_pack_start(GTK_BOX(hbox), screenshot_preview, 2, 3, 4);
	gtk_box_pack_end(GTK_BOX(hbox), vbox, 2, 3, 4);

		
	GtkWidget* screenshot_button = gtk_button_new_with_label ("Shoot!");
	GtkWidget* record = gtk_button_new_with_label ("Record");
	//GtkWidget* add_frame = gtk_button_new_with_label ("Add frame (anim)");
	GtkWidget* stop = gtk_button_new_with_label ("Stop");
	GtkWidget* play = gtk_button_new_with_label ("Replay (detached)");
	GtkWidget* playfrom = gtk_button_new_with_label ("Replay (browse)");



	/* >>>> SOUTH */	
	GtkWidget * config_expander = gtk_expander_new("config");
	gtk_expander_set_expanded(GTK_EXPANDER(config_expander), TRUE);
	
	GtkWidget* vboxc0, *hboxc1, *hboxc2; 
	vboxc0 = gtk_vbox_new(0,1);
	hboxc1 = gtk_hbox_new (0, 1);
	hboxc2 = gtk_hbox_new (0, 1);
	
	gtk_container_add(GTK_CONTAINER(config_expander), vboxc0);
	gtk_box_pack_start(GTK_BOX(vboxc0), hboxc1, 2, 3, 4);
	gtk_box_pack_end(GTK_BOX(vboxc0), hboxc2, 2, 3, 4);

	/* Labels */	
	GtkWidget * screenshot_dir_label = gtk_label_new("Screenshot folder :");
	GtkWidget * animation_dir_label = gtk_label_new("Animations folder :");

	/* GtkFileChooserButton */
	gsi->folder_chooser_screenshot = gtk_file_chooser_button_new("Screenshot", GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(gsi->folder_chooser_screenshot), tilem_config_universal_getter("screenshot", "screenshot_directory"));	
	gsi->folder_chooser_animation = gtk_file_chooser_button_new("Animation", GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(gsi->folder_chooser_animation), tilem_config_universal_getter("screenshot", "animation_directory"));	

	gtk_box_pack_start (GTK_BOX (hboxc1), screenshot_dir_label, 2, 3, 4);
	gtk_box_pack_end (GTK_BOX (hboxc1), gsi->folder_chooser_screenshot, 2, 3, 4);
	gtk_box_pack_start (GTK_BOX (hboxc2), animation_dir_label, 2, 3, 4);
	gtk_box_pack_end (GTK_BOX (hboxc2), gsi->folder_chooser_animation, 2, 3, 4);
	gtk_widget_show(screenshot_dir_label);
	gtk_widget_show(animation_dir_label);
	gtk_widget_show(gsi->folder_chooser_animation);
	gtk_widget_show(gsi->folder_chooser_screenshot);
	/* <<<< */	
	
	gtk_box_pack_start (GTK_BOX (vbox), screenshot_button, FALSE, 3, 4);
	gtk_widget_show(screenshot_button);
	gtk_box_pack_start (GTK_BOX (vbox), record, FALSE, 3, 4);
	gtk_widget_show(record);
	//gtk_box_pack_start (GTK_BOX (hbox), add_frame, 2, 3, 4);
	//gtk_widget_show(add_frame);
	gtk_box_pack_start (GTK_BOX (vbox), stop, FALSE, 3, 4);
	gtk_widget_show(stop);
	gtk_box_pack_start (GTK_BOX (vbox), play, FALSE, 3, 4);
	gtk_widget_show(play);
	gtk_box_pack_start (GTK_BOX (vbox), playfrom, FALSE, 3, 4);
	gtk_widget_show(playfrom);

	gtk_box_pack_end (GTK_BOX (parent_vbox), config_expander, FALSE, 3, 4);
	
	g_signal_connect(GTK_OBJECT(screenshot_button), "clicked", G_CALLBACK(on_screenshot), gsi);
	g_signal_connect(GTK_OBJECT(record), "clicked", G_CALLBACK(on_record), gsi);
	//g_signal_connect(GTK_OBJECT(add_frame), "clicked", G_CALLBACK(on_add_frame), gsi);
	g_signal_connect(GTK_OBJECT(stop), "clicked", G_CALLBACK(on_stop), gsi);
	g_signal_connect(GTK_OBJECT(play), "clicked", G_CALLBACK(on_play), gsi);
	g_signal_connect(GTK_OBJECT(playfrom), "clicked", G_CALLBACK(on_playfrom), gsi);
	g_signal_connect(GTK_OBJECT(gsi->folder_chooser_screenshot), "selection-changed", G_CALLBACK(on_change_screenshot_directory), gsi);
	g_signal_connect(GTK_OBJECT(gsi->folder_chooser_animation), "selection-changed", G_CALLBACK(on_change_animation_directory), gsi);
	gtk_widget_show_all(screenshotanim_win);
}

/* Callback for record button */
static void on_record(GtkWidget* win, GLOBAL_SKIN_INFOS* gsi) {
	win = win;
	g_print("record event\n");
	start_spinner(gsi);
	tilem_animation_start(gsi);
}

/* Only used for testing purpose */
/* NEVER USED  (but it works) xD */
void on_add_frame(GtkWidget* win, GLOBAL_SKIN_INFOS* gsi) {
	win = win;
	g_print("record event\n");
	tilem_animation_add_frame(gsi);
}

/* Callback for stop button (stop the recording) */
static void on_stop(GtkWidget* win, GLOBAL_SKIN_INFOS* gsi) {
	win = win;
	g_print("stop event\n");
	char* dest = NULL;

	tilem_animation_stop(gsi) ;
	
	if(gsi->isAnimScreenshotRecording) {
		gsi->isAnimScreenshotRecording = FALSE;
		stop_spinner(gsi);
		dest = select_file_for_save(gsi, tilem_config_universal_getter("screenshot", "animation_directory"));
	}

	if(dest) {
		set_animation_recentfile(dest);	
		copy_paste("gifencod.gif", dest);
		change_review_image(gsi, dest);
		char* p =  strrchr(dest, '/');
		printf("%s", p);
		if(p)
			strcpy(p, "\0");
		printf("%s", dest);
		
		set_animation_recentdir(dest);	
	}
	delete_spinner_and_put_logo(gsi);
	
}

/* Callback for screenshot button (take a screenshot) */
static void on_screenshot(GtkWidget* win, GLOBAL_SKIN_INFOS* gsi) {
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
static void on_destroy_playview(GtkWidget* playwin)   {
	
	gtk_widget_destroy(GTK_WIDGET(playwin));
}

/* Callback for play button (replay the last gif) */
static void on_play(GtkWidget * win, GLOBAL_SKIN_INFOS* gsi) {
	win = win;
	gsi = gsi;

	printf("play\n");
	GtkWidget *fenetre = NULL;
	GtkWidget *image = NULL;

	fenetre = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(fenetre),"destroy",G_CALLBACK(on_destroy_playview), NULL);

	image = gtk_image_new_from_file(tilem_config_universal_getter("screenshot", "animation_recent"));
	gtk_container_add(GTK_CONTAINER(fenetre),image);

	gtk_widget_show_all(fenetre);

}

/* Callback for play button (replay the last gif) */
static void on_playfrom(GtkWidget * win, GLOBAL_SKIN_INFOS* gsi) {
	win = win;
	char* src = NULL;
	src = select_file_for_save(gsi, tilem_config_universal_getter("screenshot", "animation_directory"));
	if(src) {
		set_animation_recentfile(src);	
		change_review_image(gsi, src);
		char* p =  strrchr(src, '/');
		printf("%s", p);
		if(p)
			strcpy(p, "\0");
		printf("%s", src);
		
		set_animation_recentdir(src);	
	}


}
	 

static void on_change_screenshot_directory(GtkWidget * win, GLOBAL_SKIN_INFOS* gsi) {
	win = win;
	char* folder = NULL;
	folder = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(gsi->folder_chooser_screenshot)); 
	if(folder) 
		set_screenshot_recentdir(folder);
	
}

	
static void on_change_animation_directory(GtkWidget * win, GLOBAL_SKIN_INFOS* gsi) {
	win = win;
	char* folder = NULL;
	folder = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(gsi->folder_chooser_animation)); 
	if(folder)
		set_animation_recentdir(folder);
	
}
