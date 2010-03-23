#include <stdlib.h>
#include <gtk/gtk.h>

void OnDestroy(GtkWidget *pWidget, gpointer pData);
gboolean mouse_press_event(GtkWidget* pWindow,GdkEvent *event,gpointer pData) ;
gboolean mouse_release_event(GtkWidget* pWindow,GdkEvent *event,gpointer pData) ;

int main(int argc,char **argv)
{
	/* Declaration du widget */
	GtkWidget *pWindow;

	gtk_init(&argc,&argv);

	/* Creation de la fenetre */
	pWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	/* Definition de la position */
	gtk_window_set_position(GTK_WINDOW(pWindow), GTK_WIN_POS_CENTER);
	/* Definition de la taille de la fenetre */
	gtk_window_set_default_size(GTK_WINDOW(pWindow), 200, 100);


	/* Connexion du signal "destroy" */
	g_signal_connect(G_OBJECT(pWindow), "destroy", G_CALLBACK(OnDestroy), NULL);
	gtk_widget_add_events(pWindow, GDK_BUTTON_PRESS_MASK);	
	gtk_signal_connect(GTK_OBJECT(pWindow), "button-press-event", G_CALLBACK(mouse_press_event),NULL);
	gtk_widget_add_events(pWindow, GDK_BUTTON_RELEASE_MASK);	
	gtk_signal_connect(GTK_OBJECT(pWindow), "button-release-event", G_CALLBACK(mouse_release_event), NULL); 
	
	/* Affichage de la fenetre */
	gtk_widget_show_all(pWindow);

	/* Demarrage de la boucle evenementielle */
	gtk_main();

	return EXIT_SUCCESS;
}

void OnDestroy(GtkWidget *pWidget, gpointer pData)
{
    /* Arret de la boucle evenementielle */
    gtk_main_quit();
}

gboolean mouse_press_event(GtkWidget* pWindow,GdkEvent *event,gpointer pData) {
	printf("PRESS\n");
	return FALSE;
}

gboolean mouse_release_event(GtkWidget* pWindow,GdkEvent *event,gpointer pData) {
	printf("RELEASE\n");
	return FALSE;
}

