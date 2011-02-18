#include <gtk/gtk.h>
#include <stdlib.h>
 
int main(int argc, char *argv[])
{
    GtkWidget *fenetre = NULL;
    GtkWidget *image = NULL;
 
    //init
    gtk_init(&argc, &argv);
 
    //fenetre
    fenetre = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(G_OBJECT(fenetre),"destroy",G_CALLBACK(gtk_main_quit), NULL);
 
    //image
    image = gtk_image_new_from_file("1.gif");
    gtk_container_add(GTK_CONTAINER(fenetre),image);
 
    //affichage + boucle
    gtk_widget_show_all(fenetre);
    gtk_main();
 
    return EXIT_SUCCESS;
}
 

