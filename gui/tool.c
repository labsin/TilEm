#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <gui.h>


char choose_rom_popup()
{
	GtkDialog *pPopup;
	GtkWidget *pLabel;
	GtkWidget *pRadio1, *pRadio2, *pRadio3, *pRadio4, *pRadio5, *pRadio6, *pRadio7,*pRadio8, *pRadio9, *pRadio10, *pRadio11, *pRadio12, *pRadio13, *pRadio20;
	gint result; /* to get the choice */

	
	/* Create the dialog */
	pPopup = (GtkDialog*)gtk_dialog_new();
	


	gtk_dialog_add_button(GTK_DIALOG(pPopup),"Valid", 1);
	pLabel = gtk_label_new("Type of rom :\n");
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(pPopup)->vbox),pLabel,FALSE,FALSE,0);
	pRadio1=gtk_radio_button_new_with_label(NULL,"Let TilEm guess");
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(pPopup)->vbox),pRadio1,FALSE,FALSE,0);
	pRadio2 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON (pRadio1), "TI-73");
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(pPopup)->vbox),pRadio2,FALSE,FALSE,0);
	pRadio3 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON (pRadio1), "TI-76");
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(pPopup)->vbox),pRadio3,FALSE,FALSE,0);
	pRadio4 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON (pRadio1), "TI-81");
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(pPopup)->vbox),pRadio4,FALSE,FALSE,0);
	pRadio5 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON (pRadio1), "TI-82");
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(pPopup)->vbox),pRadio5,FALSE,FALSE,0);
	pRadio6 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON (pRadio1), "TI-82");
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(pPopup)->vbox),pRadio6,FALSE,FALSE,0);
	pRadio7 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON (pRadio1), "TI-83");
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(pPopup)->vbox),pRadio7,FALSE,FALSE,0);
	pRadio8 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON (pRadio1), "TI-83+");
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(pPopup)->vbox),pRadio8,FALSE,FALSE,0);
	pRadio9 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON (pRadio1), "TI-83+(SE)");
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(pPopup)->vbox),pRadio9,FALSE,FALSE,0);
	pRadio10 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON (pRadio1), "TI-84+");
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(pPopup)->vbox),pRadio10,FALSE,FALSE,0);
	pRadio11 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON (pRadio1), "TI-84+(SE)");
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(pPopup)->vbox),pRadio11,FALSE,FALSE,0);
	pRadio12 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON (pRadio1), "TI-84+(nSpire)");
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(pPopup)->vbox),pRadio12,FALSE,FALSE,0);
	pRadio13 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON (pRadio1), "TI-85");
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(pPopup)->vbox),pRadio13,FALSE,FALSE,0);
	pRadio20 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON (pRadio1), "TI-86");
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(pPopup)->vbox),pRadio20,FALSE,FALSE,0);
	
	gtk_widget_show_all(GTK_DIALOG(pPopup)->vbox);

	/* Show the msg box */
	result = gtk_dialog_run(GTK_DIALOG(pPopup));
	DEBUGGINGGLOBAL_L0_A0("**************** fct : choose_rom_popup ****************\n");
	DEBUGGINGGLOBAL_L0_A1("*  function choose_rom_popup : Button number : %d       *\n", result);
	
	GSList *pList;
	const gchar *sLabel=NULL;
	int count=0;
	
	/* ######### SEARCH wich radio was selected ######### */
	/* Get the Button's list */
	pList = gtk_radio_button_get_group(GTK_RADIO_BUTTON(pRadio1));
	
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
		    count++;  /* <--- don't remove this */
		}
	}
	
	DEBUGGINGGLOBAL_L0_A1("*  Choosen model : %s                               *\n", sLabel);
	DEBUGGINGGLOBAL_L0_A0("********************************************************\n");
	char * choosen_model;
	strcpy(choosen_model,sLabel);  /* <--- Why another variable ?  Because sLabel is buggy after destroyed the window ... */
	
	/* Destroy the msg box */
	gtk_widget_destroy(GTK_WIDGET(pPopup));

	
	if(strcmp(choosen_model,"TI-86")==0) {
		return '6';
	}else if(strcmp(choosen_model,"TI-85")==0) {
		return '5';
	}else if(strcmp(choosen_model,"TI-84+(nSpire)")==0) {
		return 'n';
	}else if(strcmp(choosen_model,"TI-84+(SE)")==0) {
		return 'z';
	}else if(strcmp(choosen_model,"TI-84+")==0) {
		return '4';
	}else if(strcmp(choosen_model,"TI-83+(SE)")==0) {
		return 's';
	}else if(strcmp(choosen_model,"TI-83+")==0) {
		return 'p';
	}else if(strcmp(choosen_model,"TI-83")==0) {
		return '3';
	}else if(strcmp(choosen_model,"TI-82")==0) {
		return '2';
	}else if(strcmp(choosen_model,"TI-81")==0) {
		return '1';
	}else if(strcmp(choosen_model,"TI-76")==0) {
		return 'f';
	}else if(strcmp(choosen_model,"TI-76")==0) {
		return '7';
	}
	return '0';

}

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
	pBtn=pBtn;

	
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
