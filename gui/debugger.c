#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include <tilem.h>
#include "gui.h"
#include "z80.h"
#include <malloc.h>

void on_debug_destroy(GtkWidget* debug_win, GdkEvent* Event, GLOBAL_SKIN_INFOS* gsi);
void create_debug_window(GLOBAL_SKIN_INFOS* gsi);
GtkWidget* create_debug_table(GLOBAL_SKIN_INFOS* gsi);
void create_debug_button(GtkWidget* debug_table);
void create_register_list(GtkWidget* debug_table, GLOBAL_SKIN_INFOS* gsi); 
static char *rlabel[12] = {"AF'", "AF", "BC'", "BC", "DE'", "DE", "HL'", "HL", "PC", "SP", "IY", "IX"};
void getreg(GLOBAL_SKIN_INFOS* gsi , int ii, char* string);
static void printstate(TilemCalcEmulator* emu);


char* rvalue[12];

/* This function is the first to be called. Start the debugger */
void launch_debugger(GLOBAL_SKIN_INFOS* gsi){

	//printf("%s", gsi->RomName);
	create_debug_window(gsi);
	
}

/* Destroy the debugger, and turn to FALSE the isDebuggerRunning variable (use by other function to know if debuuger is running) */
void on_debug_destroy(GtkWidget* debug_win, GdkEvent* Event, GLOBAL_SKIN_INFOS* gsi)
{
	Event=NULL;	
	gsi->isDebuggerRunning=FALSE;
	gtk_widget_destroy(GTK_WIDGET(debug_win));

}

/* Print register value in terminal (copy paste of Benjamin Moody's work) */
static void printstate(TilemCalcEmulator* emu)
{
	printf("*  PC=%02X:%04X AF=%04X BC=%04X DE=%04X                  *\n"
		"*  HL=%04X IX=%04X IY=%04X SP=%04X                     *\n",
       emu->calc->mempagemap[emu->calc->z80.r.pc.b.h >> 6],
       emu->calc->z80.r.pc.w.l,
       emu->calc->z80.r.af.w.l,
       emu->calc->z80.r.bc.w.l,
       emu->calc->z80.r.de.w.l,
       emu->calc->z80.r.hl.w.l,
       emu->calc->z80.r.ix.w.l,
       emu->calc->z80.r.iy.w.l,
       emu->calc->z80.r.sp.w.l);
}

/* Return the string value of the register.Used by create_register_list */
void getreg(GLOBAL_SKIN_INFOS* gsi , int i, char* string)
{
	//char string[50];
	switch(i)
	{
	case 0:
		snprintf(string, 5, "%04X", gsi->emu->calc->z80.r.af2.d);
		break;
	case 1:
		snprintf(string, 5, "%04X", gsi->emu->calc->z80.r.af.d);
		break;
	case 2:
		snprintf(string, 5, "%04X", gsi->emu->calc->z80.r.bc2.d);
		break;
	case 3:
		snprintf(string, 5, "%04X", gsi->emu->calc->z80.r.bc.d);
		break;
	case 4:
		snprintf(string, 5, "%04X", gsi->emu->calc->z80.r.de2.d);
		break;
	case 5:
		snprintf(string, 5, "%04X", gsi->emu->calc->z80.r.de.d);
		break;
	case 6:
		snprintf(string, 5, "%04X", gsi->emu->calc->z80.r.hl2.d);
		break;
	case 7:
		snprintf(string, 5, "%04X", gsi->emu->calc->z80.r.hl.d);
		break;
	case 8:
		snprintf(string, 5, "%04X", gsi->emu->calc->z80.r.pc.d);
		break;
	case 9:
		snprintf(string, 5, "%04X", gsi->emu->calc->z80.r.sp.d);
		break;
	case 10:
		snprintf(string, 5, "%04X", gsi->emu->calc->z80.r.iy.d);
		break;
	case 11:
		snprintf(string, 5, "%04X", gsi->emu->calc->z80.r.ix.d);
		break;
	default:
		snprintf(string, 5, "%s", "----");
		break;

	}

}


/* Create the debugger window */
void create_debug_window(GLOBAL_SKIN_INFOS* gsi) 
{
	
	GtkWidget* debug_win= gtk_window_new(GTK_WINDOW_TOPLEVEL);
	//gtk_window_set_modal(GTK_WINDOW(debug_win), TRUE);
	gtk_window_set_resizable(GTK_WINDOW(debug_win), FALSE);	
	GtkWidget* debug_table= create_debug_table(gsi);
	
	
	GtkWidget* debug_aspectframe= gtk_aspect_frame_new(NULL,0,0,1,FALSE);
	
	gtk_table_attach_defaults(GTK_TABLE(debug_table),debug_aspectframe, 1 , 6, 1, 7); 
	gtk_container_add(GTK_CONTAINER(debug_win), debug_table);
	gtk_signal_connect(GTK_OBJECT(debug_win), "delete-event", G_CALLBACK(on_debug_destroy), gsi);
	gtk_widget_show_all(debug_win);
}

/* The debugger windows contains a GtkTable.This is the high layout. */
GtkWidget* create_debug_table(GLOBAL_SKIN_INFOS* gsi) 
{
	GtkWidget* debug_table= gtk_table_new(7, 8, FALSE);
	create_debug_button(debug_table);
	create_register_list(debug_table, gsi);
	return debug_table;
}

/* Create navigation button */
void create_debug_button(GtkWidget* debug_table) 
{
	GtkWidget* buttonleft2= gtk_button_new();
	GtkWidget* left2= gtk_image_new_from_file("./pix/rewindleft2.png"); 
	gtk_button_set_image(GTK_BUTTON(buttonleft2), left2);
	gtk_table_attach(GTK_TABLE(debug_table), buttonleft2, 1, 2, 0, 1, 0, 0, 0, 0);
	
	GtkWidget* buttonleft1= gtk_button_new();
	GtkWidget* left1= gtk_image_new_from_file("./pix/rewindleft1.png"); 
	gtk_button_set_image(GTK_BUTTON(buttonleft1), left1);
	gtk_table_attach(GTK_TABLE(debug_table), buttonleft1, 2, 3, 0, 1, 0, 0, 0, 0);
	
	GtkWidget* buttonpause= gtk_button_new();
	GtkWidget* pause= gtk_image_new_from_file("./pix/pause.png"); 
	gtk_button_set_image(GTK_BUTTON(buttonpause), pause);
	gtk_table_attach(GTK_TABLE(debug_table), buttonpause, 3, 4, 0, 1, 0, 0, 0, 0);
	
	GtkWidget* buttonright1= gtk_button_new();
	GtkWidget* right1= gtk_image_new_from_file("./pix/rewindright1.png"); 
	gtk_button_set_image(GTK_BUTTON(buttonright1), right1);
	gtk_table_attach(GTK_TABLE(debug_table), buttonright1, 4, 5, 0, 1, 0, 0, 0, 0);
	
	GtkWidget* buttonright2= gtk_button_new();
	GtkWidget* right2= gtk_image_new_from_file("./pix/rewindright2.png"); 
	gtk_button_set_image(GTK_BUTTON(buttonright2), right2);
	gtk_table_attach(GTK_TABLE(debug_table), buttonright2, 5, 6, 0, 1, 0, 0, 0, 0);

}

/*GtkWidget* create_debug_list(GtkWidget* debug_table)
{
		

}*/

/* Refresh the register value in the GtkEntry */
void refresh_register(GLOBAL_SKIN_INFOS* gsi)
{
	DEBUGGINGDEBUGGER_L0_A0("******************** fct: refresh_register *************\n");
	DEBUGGINGDEBUGGER_L0_A0("*  Refresh register :                                  *\n");
	printstate(gsi->emu);
	DEBUGGINGDEBUGGER_L0_A0("********************************************************\n");

	if(gsi->reg_entry!=NULL)
	{
		char string[50];
		int i=0;
		for(i=0; i<12; i++) 
		{
			getreg(gsi, i, string);
			gtk_entry_set_text(GTK_ENTRY(gsi->reg_entry->reg[i]), string);
		}
	}

}

/* Create another GtkTable which contain a one GtkFrame per register */
void create_register_list(GtkWidget* debug_table, GLOBAL_SKIN_INFOS* gsi) 
{
	GtkWidget* debug_register_frame= gtk_frame_new("Registers");
	GtkWidget* debug_register_table= gtk_table_new(5, 8, FALSE);
	
	gtk_table_attach_defaults(GTK_TABLE(debug_table), debug_register_frame, 0, 1, 0, 7);
	gtk_container_add(GTK_CONTAINER(debug_register_frame), debug_register_table);
	
	char string[50]; /* Useful to get and format (snprintf) the registers */
	int i=0;
	int j=0;
	int n=0;

	/* Init gsi->reg_entry. This is a save of the register widget.It 's easier to refresh it from another function with it. */
	gsi->reg_entry=malloc(sizeof(TilemDebuggerRegister));
	gsi->isDebuggerRunning=TRUE;
	for(i=0; i<12; i++)
		gsi->reg_entry->reg[i]= gtk_entry_new();

	/* Create the register table */
	for(i=0; i<6; i++) {
		for(j=0; j<2; j++) {

			GtkWidget* frame= gtk_frame_new(rlabel[n]);
			GtkWidget* entry= gtk_entry_new();
			gsi->reg_entry->reg[n]=entry;
			gtk_widget_set_size_request(GTK_WIDGET(entry), 40,20);
			//snprintf(string, 5, "%04X", gsi->emu->calc->z80.r.af.d);
			getreg(gsi, n, string);
			gtk_entry_set_text(GTK_ENTRY(entry), string);
			gtk_container_add(GTK_CONTAINER(frame), entry);
			gtk_table_attach(GTK_TABLE(debug_register_table), frame, j, j+1, i, i+1, 0, 0, 0, 0);
			n++;
		}
	}
	DEBUGGINGDEBUGGER_L0_A0("********************* fct: create_register_list ********\n");
	printstate(gsi->emu);
	DEBUGGINGDEBUGGER_L0_A0("********************************************************\n");
}

