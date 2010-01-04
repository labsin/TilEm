#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <gui.h>

void choose_rom_type(GLOBAL_SKIN_INFOS * gsi) {
	
	GtkWidget *pRomTypeWindow;
	GtkWidget *pVBox;
	GtkWidget *pRadio1, *pRadio2, *pRadio3;
	GtkWidget *pValid;
	GtkWidget *pLabel;

	
	pRomTypeWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(pRomTypeWindow), "Rom type Selection");
	gtk_window_set_default_size(GTK_WINDOW(pRomTypeWindow), 320, 200);
	
	/* Create the box and the radio button */
	pVBox = gtk_vbox_new(TRUE, 0);
	gtk_container_add(GTK_CONTAINER(pRomTypeWindow),pVBox);
	pLabel = gtk_label_new("Type of rom :");
	gtk_box_pack_start(GTK_BOX(pVBox), pLabel, FALSE, FALSE, 0);
	pRadio1=gtk_radio_button_new_with_label(NULL,"TI-82");
	gtk_box_pack_start(GTK_BOX(pVBox),pRadio1,FALSE,FALSE,0);
	pRadio2 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON (pRadio1), "TI-83 (the best one)");
	gtk_box_pack_start(GTK_BOX(pVBox),pRadio2,FALSE,FALSE,0);
	pRadio3 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON (pRadio1), "Let TilEm choose for you (could not work)");
	gtk_box_pack_start(GTK_BOX(pVBox),pRadio3,FALSE,FALSE,0);
	
	/* Tha valid button */
	pValid=gtk_button_new_from_stock(GTK_STOCK_OK);
	gtk_box_pack_start(GTK_BOX (pVBox), pValid, FALSE, FALSE, 0);
	
	gtk_widget_show_all(pRomTypeWindow);
	/* Save pRadio */
	gsi->pRadio=pRadio1;
	
	/* Connect the signals */
	gtk_signal_connect(GTK_OBJECT(pRomTypeWindow),"destroy",G_CALLBACK(gtk_widget_destroy), NULL);
	gtk_signal_connect(GTK_OBJECT(pValid),"clicked",G_CALLBACK(on_valid),gsi);
	gtk_signal_connect_object(GTK_OBJECT(pValid),"clicked",G_CALLBACK(gtk_widget_destroy),pRomTypeWindow);

	
}

void on_valid(GtkWidget *pBtn,GLOBAL_SKIN_INFOS *gsi) {
	
	GtkWidget *pInfo;
	GtkWidget *pWindow;
	GSList *pList;
	const gchar *sLabel=NULL;
	int count=0;

	
	/* Get the Button's list */
	pList = gtk_radio_button_get_group(GTK_RADIO_BUTTON(gsi->pRadio));
	
	while(pList)
	{

		/* Is this button selected? */
		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pList->data)))
		{
		    /* YES */
		    sLabel = gtk_button_get_label(GTK_BUTTON(pList->data));
		    /* To break the while */
		    pList = NULL;
		}
		else
		{
		    /* NO */
		    pList = g_slist_next(pList);
		    count++;
		}
		
	}
	
	if(count==0) {
		/* curiously (in fact not really) the radion button are in the like in a stack LIFO */
		/* So 1 is for tilem guess rom type and 3 is for TI-82 ... */
	    pWindow = gtk_widget_get_toplevel(GTK_WIDGET(gsi->pRadio));

	    pInfo = gtk_message_dialog_new (GTK_WINDOW(pWindow),
		GTK_DIALOG_MODAL,
		GTK_MESSAGE_INFO,
		GTK_BUTTONS_OK,
		"%s", sLabel);
		gsi->RomType='0';
	
		
		gtk_dialog_run(GTK_DIALOG(pInfo));
		gtk_widget_destroy(pInfo);
	}

		

			
}
