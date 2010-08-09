#include "skin.h"


/* choose_skin_filename is used to give the name of the default skin file name to load when the emulator starts */
void choose_skin_filename(TilemCalc* calc,GLOBAL_SKIN_INFOS *gsi) {
	DSKIN_L0_A0("**************** fct : choose_skin_filename ************\n");
	
	if(strcmp(calc->hw.name,"ti73")==0) {
		  gsi->SkinFileName=(gchar*)malloc(15);
		  strcpy(gsi->SkinFileName,"./skn/ti73.skn");
	}else if(strcmp(calc->hw.name,"ti76")==0) {
		  gsi->SkinFileName=(gchar*)malloc(16);
		  strcpy(gsi->SkinFileName,"./skn/ti73.skn"); /* no ti76skin for the moment */
	}else if(strcmp(calc->hw.name,"ti81")==0) {
		  gsi->SkinFileName=(gchar*)malloc(16);
		  strcpy(gsi->SkinFileName,"./skn/ti81.skn");
	}else if(strcmp(calc->hw.name,"ti82")==0) {
		  gsi->SkinFileName=(gchar*)malloc(15);
		  strcpy(gsi->SkinFileName,"./skn/ti82.skn");
	}else if(strcmp(calc->hw.name,"ti83")==0) {
		gsi->SkinFileName=(gchar*)malloc(41);
		strcpy(gsi->SkinFileName,"./skn/ti83.skn");
	}else if(strcmp(calc->hw.name,"ti83p")==0) {
		gsi->SkinFileName=(gchar*)malloc(16);
		strcpy(gsi->SkinFileName,"./skn/ti83plus.skn");
	}else if(strcmp(calc->hw.name,"ti84p")==0) {
		gsi->SkinFileName=(gchar*)malloc(16);
		strcpy(gsi->SkinFileName,"./skn/ti84plus.skn");
	}else if(strcmp(calc->hw.name,"ti84pse")==0) {
		gsi->SkinFileName=(gchar*)malloc(16);
		strcpy(gsi->SkinFileName,"./skn/ti84plus.skn");
	}else if(strcmp(calc->hw.name,"ti84pns")==0) {
		gsi->SkinFileName=(gchar*)malloc(16);
		strcpy(gsi->SkinFileName,"./skn/ti84plus.skn");
	}else if(strcmp(calc->hw.name,"ti85")==0) {
		gsi->SkinFileName=(gchar*)malloc(16);
		strcpy(gsi->SkinFileName,"./skn/ti82.skn");
	}else if(strcmp(calc->hw.name,"ti86")==0) {
		gsi->SkinFileName=(gchar*)malloc(16);
		strcpy(gsi->SkinFileName,"./skn/ti82.skn");
	} else {
		gsi->SkinFileName=(gchar*)malloc(16);
		strcpy(gsi->SkinFileName,"./skn/ti83plus.skn");
	}
		
	DSKIN_L0_A1("*  calc->hw.name == %s                             *\n",calc->hw.name);
	DSKIN_L0_A0("********************************************************\n");
	//gsi->kl=x3_keylist;
}

/* GtkFileSelection */
/* TODO : Replace deprecated GtkFileSelection by GtkFileChooser */
void SkinSelection(GLOBAL_SKIN_INFOS *gsi) {

	if(gsi->view==1) 
	{
		DGLOBAL_L2_A0("Use >>Switch view<< before !\n");
		popup_error("Use >>Switch view<< before !\n", gsi);
	} else {
		GtkFileSelection * file_selection;
		file_selection=(GtkFileSelection*)gtk_file_selection_new("SkinLoad");
		gtk_file_selection_set_filename(GTK_FILE_SELECTION(file_selection), "./skn/");
		gtk_window_set_default_size(GTK_WINDOW(file_selection), 500, 405);
		gtk_widget_show(GTK_WIDGET(file_selection));
		printf("\nSKINSELECTION\n");
		gsi->FileSelected=file_selection;

		/* ######## SIGNALS ######## */
		/* Connect the signal to get the filename (when OK button is clicked) */
		gtk_signal_connect_object(GTK_OBJECT(file_selection->ok_button),"clicked",G_CALLBACK(GetSkinSelected),(gpointer)gsi);
		/* Connect the signal to close the widget when OK is clicked (gtk_widget_destroy is already define in the Gtk library)*/
		gtk_signal_connect_object(GTK_OBJECT(file_selection->ok_button),"clicked",G_CALLBACK(gtk_widget_destroy),file_selection);
		/* Connect the signal to close the window when CANCEL is clicked */
		gtk_signal_connect_object(GTK_OBJECT(file_selection->cancel_button),"clicked",G_CALLBACK(gtk_widget_destroy),file_selection);
	}
}

void GetSkinSelected(GLOBAL_SKIN_INFOS *gsi) {

	/* Just get the file wich was selected */
	gsi->SkinFileName=(gchar*)gtk_file_selection_get_filename(gsi->FileSelected);
	printf("gsi->si->name : %s gsi->si->type  %d\n",gsi->si->name,gsi->si->type);
	printf("file to load : %s\n",gsi->SkinFileName);
	/* redraw the skin into the Window (here gsi->pWindow) */
	
	
	/* Only while test */
	
	redraw_screen(gsi->pWindow,gsi);
	
}
