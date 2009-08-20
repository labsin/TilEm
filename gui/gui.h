#include <tilem.h>

typedef struct _TilemCalcEmulator {
	GMutex* run_mutex;
	gboolean exiting;
	gboolean forcebreak;

	GMutex* calc_mutex;
	TilemCalc* calc;

	GMutex* lcd_mutex;
	guchar* lcdlevel;
	guchar* lcdimage;
	GdkColor lcdfg, lcdbg;
	GdkPixbuf* lcdpb;
	GdkPixbuf* lcdscaledpb;

	GtkWidget* lcdwin;
} TilemCalcEmulator;

/* this struct is for the Skin's set */
typedef struct _TilemCalcSkin {
	char * top;
	char * left;
	char * right;
	char * bot;
} TilemCalcSkin;




/* Create a CalcSkin with an TilemCalcEmulator */
TilemCalcSkin* tilem_guess_skin_set(TilemCalc* calc);

/* Detect and handle a "destroy" event */
void OnDestroy(GtkWidget *pWidget, gpointer pData);	// close the pWindow

/* Detect a keyboard press event */
void keyboard_event();	

/* Detect a mouse event */
void mouse_event();	

