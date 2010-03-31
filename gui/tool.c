#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <gui.h>

static const char * list[] = { "TI-73", "TI-76", "TI-81", "TI-83", "TI-83+", "TI-83+(SE)", "TI-84+", "TI-84+(SE)", "TI-84+(nSpire)", "TI-85", "TI-86" };
static const int NB_RADIO_BUTTON=11;
char choose_rom_popup()
{
	GtkDialog *pPopup;
	GtkWidget *pLabel;
	GtkWidget *pRadio1, *pRadio ;/*, *pRadio3, *pRadio4, *pRadio5, *pRadio6, *pRadio7,*pRadio8, *pRadio9, *pRadio10, *pRadio11, *pRadio12, *pRadio13;*/
	int i=0; /* counter for the radio button creation */
	gint result; /* to get the choice */

	
	/* Create the dialog */
	pPopup = (GtkDialog*)gtk_dialog_new();
	


	gtk_dialog_add_button(GTK_DIALOG(pPopup),"Valid", 1);
	pLabel = gtk_label_new("Type of rom :\n");
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(pPopup)->vbox),pLabel,FALSE,FALSE,0);
	pRadio1=gtk_radio_button_new_with_label(NULL,"Let TilEm guess");
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(pPopup)->vbox),pRadio1,FALSE,FALSE,0);
	



	
	/* Create the other radio button with the pRadio1 group */
	for(i=0; i<NB_RADIO_BUTTON;i++) {
	pRadio = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON (pRadio1), list[i]);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(pPopup)->vbox),pRadio,FALSE,FALSE,0);
	}
	
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
	//choosen_model=malloc(strlen(sLabel)*sizeof(char)+1);
	
	/* Apply the patch of Mischa POSLAWSKY <shiar@shiar.org> */
	choosen_model = strdup(sLabel); /* <--- Why another variable ?  Because sLabel is buggy after destroyed the window ... */
	/* end patch */
	
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
	}else if(strcmp(choosen_model,"TI-73")==0) {
		return '7';
	}
	return '0';

}

