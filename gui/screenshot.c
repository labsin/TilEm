#include <screenshot.h>

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
	
	gtk_signal_connect(GTK_OBJECT(screenshotanim_win), "delete-event", G_CALLBACK(on_destroy_screenshot), NULL);

	GtkWidget* box;
	box = gtk_hbox_new (0, 1);
	
	gtk_container_add(GTK_CONTAINER(screenshotanim_win), box);

		
	GtkWidget* screenshot_button = gtk_button_new_with_label ("Shoot!");
	GtkWidget* record = gtk_button_new_with_label ("Record");
	//GtkWidget* add_frame = gtk_button_new_with_label ("Add frame (anim)");
	GtkWidget* stop = gtk_button_new_with_label ("Stop");
	GtkWidget* play = gtk_button_new_with_label ("Play");
	
	gtk_box_pack_start (GTK_BOX (box), screenshot_button, 2, 3, 4);
	gtk_widget_show(screenshot_button);
	gtk_box_pack_start (GTK_BOX (box), record, 2, 3, 4);
	gtk_widget_show(record);
	//gtk_box_pack_start (GTK_BOX (box), add_frame, 2, 3, 4);
	//gtk_widget_show(add_frame);
	gtk_box_pack_start (GTK_BOX (box), stop, 2, 3, 4);
	gtk_widget_show(stop);
	gtk_box_pack_start (GTK_BOX (box), play, 2, 3, 4);
	gtk_widget_show(play);
	
	g_signal_connect(GTK_OBJECT(screenshot_button), "clicked", G_CALLBACK(on_screenshot), gsi);
	g_signal_connect(GTK_OBJECT(record), "clicked", G_CALLBACK(on_record), gsi);
	//g_signal_connect(GTK_OBJECT(add_frame), "clicked", G_CALLBACK(on_add_frame), gsi);
	g_signal_connect(GTK_OBJECT(stop), "clicked", G_CALLBACK(on_stop), gsi);
	g_signal_connect(GTK_OBJECT(play), "clicked", G_CALLBACK(on_play), gsi);
    
	gtk_widget_show_all(screenshotanim_win);
}

void on_record(GtkWidget* win, GLOBAL_SKIN_INFOS* gsi) {
	win = win;
	g_print("record event\n");
	screenshot_anim_create_nostatic(gsi);
}

void on_add_frame(GtkWidget* win, GLOBAL_SKIN_INFOS* gsi) {
	win = win;
	g_print("record event\n");
	screenshot_anim_addframe(gsi);
}

void on_stop(GtkWidget* win, GLOBAL_SKIN_INFOS* gsi) {
	win = win;
	g_print("stop event\n");
	stop_anim_screenshot(gsi) ;
}


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

void on_play(GLOBAL_SKIN_INFOS* gsi) {
	g_print("play event : %d\n", gsi->isAnimScreenshotRecording);

	printf("play\n");
	GtkWidget *fenetre = NULL;
	GtkWidget *image = NULL;

	fenetre = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(fenetre),"destroy",G_CALLBACK(on_destroy_playview), NULL);

	image = gtk_image_new_from_file("gifencod.gif");
	gtk_container_add(GTK_CONTAINER(fenetre),image);

	gtk_widget_show_all(fenetre);

}
	 

