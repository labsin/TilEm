#include <stdio.h>
#include <gtk/gtk.h>

void on_destroy();
void create_debug_window();
GtkWidget* create_debug_table();
void create_debug_button(GtkWidget* debug_table);
void create_registry_list(GtkWidget* debug_table); 
static char *rlabel[12] = {"AF'", "AF", "HL'", "HL", "DE'", "DE", "BC'", "BC", "PC", "SP", "IY", "IX"};


int main(int argc, char* argv[]) {

	gtk_init(&argc, &argv);
	create_debug_window();
	gtk_main();

	return 0;
}

void on_destroy() {
	gtk_main_quit();
}

void create_debug_window() 
{
	
	GtkWidget* debug_win= gtk_window_new(GTK_WINDOW_TOPLEVEL);
	//gtk_window_set_default_size(GTK_WINDOW(debug_win), 300,300);
	gtk_window_set_resizable(GTK_WINDOW(debug_win), FALSE);	
	GtkWidget* debug_table= create_debug_table();
	
	
	GtkWidget* debug_aspectframe= gtk_aspect_frame_new(NULL,0,0,1,FALSE);
	//gtk_container_add(GTK_CONTAINER(debug_aspectframe), debug_table);
	//gtk_container_add(GTK_CONTAINER(debug_win), debug_aspectframe);	
	
	gtk_table_attach_defaults(GTK_TABLE(debug_table),debug_aspectframe, 1 , 6, 1, 7); 
	gtk_container_add(GTK_CONTAINER(debug_win), debug_table);
	gtk_signal_connect(GTK_OBJECT(debug_win), "delete-event", G_CALLBACK(on_destroy), NULL);
	gtk_widget_show_all(debug_win);
}

GtkWidget* create_debug_table() 
{
	GtkWidget* debug_table= gtk_table_new(7, 8, FALSE);
	GtkWidget* button1= gtk_button_new_with_label("toto");
	GtkWidget* label= gtk_label_new("je suis un très très très long label pour tester");
	//gtk_table_attach(GTK_TABLE(debug_table),label, 1 , 6, 1, 7, 0, 0, 0, 0); 
	create_debug_button(debug_table);
	create_registry_list(debug_table);
	return debug_table;
}

void create_debug_button(GtkWidget* debug_table) 
{
	GtkWidget* buttonleft2= gtk_button_new();
	GtkWidget* left2= gtk_image_new_from_file("./pixs/rewindleft2.png"); 
	gtk_button_set_image(GTK_BUTTON(buttonleft2), left2);
	gtk_table_attach(GTK_TABLE(debug_table), buttonleft2, 1, 2, 0, 1, 0, 0, 0, 0);
	
	GtkWidget* buttonleft1= gtk_button_new();
	GtkWidget* left1= gtk_image_new_from_file("./pixs/rewindleft1.png"); 
	gtk_button_set_image(GTK_BUTTON(buttonleft1), left1);
	gtk_table_attach(GTK_TABLE(debug_table), buttonleft1, 2, 3, 0, 1, 0, 0, 0, 0);
	
	GtkWidget* buttonpause= gtk_button_new();
	GtkWidget* pause= gtk_image_new_from_file("./pixs/pause.png"); 
	gtk_button_set_image(GTK_BUTTON(buttonpause), pause);
	gtk_table_attach(GTK_TABLE(debug_table), buttonpause, 3, 4, 0, 1, 0, 0, 0, 0);
	
	GtkWidget* buttonright1= gtk_button_new();
	GtkWidget* right1= gtk_image_new_from_file("./pixs/rewindright1.png"); 
	gtk_button_set_image(GTK_BUTTON(buttonright1), right1);
	gtk_table_attach(GTK_TABLE(debug_table), buttonright1, 4, 5, 0, 1, 0, 0, 0, 0);
	
	GtkWidget* buttonright2= gtk_button_new();
	GtkWidget* right2= gtk_image_new_from_file("./pixs/rewindright2.png"); 
	gtk_button_set_image(GTK_BUTTON(buttonright2), right2);
	gtk_table_attach(GTK_TABLE(debug_table), buttonright2, 5, 6, 0, 1, 0, 0, 0, 0);

}

GtkWidget* create_debug_list(GtkWidget* debug_table)
{
		

}

void create_registry_list(GtkWidget* debug_table) 
{
	GtkWidget* debug_registry_frame= gtk_frame_new("Registers");
	GtkWidget* debug_registry_alignment= gtk_alignment_new(0, 1, 0, 1);
	GtkWidget* debug_registry_table= gtk_table_new(5, 8, FALSE);
	gtk_container_add(GTK_CONTAINER(debug_registry_alignment), debug_registry_table);
	gtk_container_add(GTK_CONTAINER(debug_registry_frame), debug_registry_alignment);
	gtk_table_attach_defaults(GTK_TABLE(debug_table), debug_registry_frame, 0, 1, 0, 7);
	
	char string[50]; /* Useful to get and format (snprintf) the registers */
	int i=0;
	int j=0;
	int n=0;
	
	/* Create the register table */
	for(i=0; i<6; i++) {
		for(j=0; j<2; j++) {

			GtkWidget* frame= gtk_frame_new(rlabel[n]);
			GtkWidget* entry= gtk_entry_new();
			gtk_widget_set_size_request(GTK_WIDGET(entry), 40,20);
			//snprintf(string, 4, "%04X", getreg(n) );
			gtk_entry_set_text(GTK_ENTRY(entry), "0000");
			gtk_container_add(GTK_CONTAINER(frame), entry);
			gtk_table_attach(GTK_TABLE(debug_registry_table), frame, j, j+1, i, i+1, 0, 0, 0, 0);
			n++;
		}
	}

	

	gtk_container_add(GTK_CONTAINER(debug_registry_frame), debug_registry_table);	
	



}

