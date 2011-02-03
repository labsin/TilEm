#include <stdio.h>
#include <gtk/gtk.h>

void on_destroy();
GtkWidget* create_screenshotanim_popup_window();
GtkWidget* add_buttons(GtkWidget* screenshotanim_win) ;
GtkWidget* create_screenshot_window();
void on_screenshot();
void on_record();
void on_stop();
void on_play();

void on_destroy() {
	gtk_main_quit();
}


int main(int argc, char* argv[]) {
	
	GtkWidget* screenshotanim_win;

	gtk_init(&argc, &argv);
	screenshotanim_win = create_screenshot_window();
	gtk_main();
	return 0;
}




GtkWidget* create_screenshot_window() {
	GtkWidget* screenshotanim_win;
	screenshotanim_win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(screenshotanim_win), "Screenshot");
	gtk_window_set_default_size(GTK_WINDOW(screenshotanim_win), 400,30);
	
	gtk_signal_connect(GTK_OBJECT(screenshotanim_win), "delete-event", G_CALLBACK(on_destroy), NULL);

	GtkWidget* box;
	box = gtk_hbox_new (0, 1);
	
	gtk_container_add(GTK_CONTAINER(screenshotanim_win), box);

		
	GtkWidget* screenshot = gtk_button_new_with_label ("Take Screenshot");
	GtkWidget* record = gtk_button_new_with_label ("Record (anim)");
	GtkWidget* stop = gtk_button_new_with_label ("Stop (anim)");
	GtkWidget* play = gtk_button_new_with_label ("Play (anim)");
	
	gtk_box_pack_start (GTK_BOX (box), screenshot, 2, 3, 4);
	gtk_widget_show(screenshot);
	gtk_box_pack_start (GTK_BOX (box), record, 2, 3, 4);
	gtk_widget_show(record);
	gtk_box_pack_start (GTK_BOX (box), stop, 2, 3, 4);
	gtk_widget_show(stop);
	gtk_box_pack_start (GTK_BOX (box), play, 2, 3, 4);
	gtk_widget_show(play);
	
	g_signal_connect(GTK_OBJECT(screenshot), "clicked", G_CALLBACK(on_screenshot), NULL);
	g_signal_connect(GTK_OBJECT(record), "clicked", G_CALLBACK(on_record), NULL);
	g_signal_connect(GTK_OBJECT(stop), "clicked", G_CALLBACK(on_stop), NULL);
	g_signal_connect(GTK_OBJECT(play), "clicked", G_CALLBACK(on_play), NULL);
    
	gtk_widget_show_all(screenshotanim_win);
	return screenshotanim_win;
}

void on_record() {
	g_print("record event\n");
}

void on_stop() {
	g_print("stop event\n");
}

void on_play() {
	g_print("play event\n");
}

void on_screenshot() {
	g_print("screenshot event\n");
}
