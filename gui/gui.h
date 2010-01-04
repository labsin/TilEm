#include <tilem.h>
#include <z80.h>
#include <skinops.h>

#include <scancodes.h>
#define TI73   "TI73"
#define TI82   "TI82"
#define TI83   "TI83"
#define TI83P  "TI83+"
#define TI85   "TI85"
#define TI86   "TI86"
#define X_FRINGE 2
#define Y_FRINGE 1


static const char* bpstring[7] = {0, "read", "exec", "write", "in", "out", "op"};


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


/* List of the keys (a code and a label) */
typedef struct KEY_LIST {
	int code;
	const char* label;
}KEY_LIST;

/* List of the keys (a code and a label) */
typedef struct GLOBAL_SKIN_INFOS {
	KEY_LIST kl;
	SKIN_INFOS *si;
	GtkWidget *pWindow;
	GtkWidget *pLayout;
	gchar* SkinFileName;
	GtkFileSelection *FileSelected;
	GtkWidget *pRadio;
	FILE *romfile;
	char *romname;
	char calc_id;
	char RomType;
	TilemCalcEmulator *emu;
}GLOBAL_SKIN_INFOS;

static const struct KEY_LIST x3_keylist[] = {
	/* Window  (5keys at the top of the keyboard)  */
	{ 0x35, "Y=" },
	{ 0x34, "WINDOW" },
	{ 0x33, "ZOOM" },
	{ 0x32, "TRACE" },
	{ 0x31, "GRAPH" },
	
	/* 1rst row (only 3 keys by line)  */
	{ 0x36, "2nd3" },
	{ 0x37, "MODE" },
	{ 0x38, "DEL" },
	
	/* 2nd row  (only 3 keys by line)  */
	{ 0x30, "ALPHA" },
	{ 0x28, "X,T,\342\200\212\316\270,\342\200\212<i>n</i>" },
	{ 0x20, "STAT" },
	
	/* 3rd row  */
	{ 0x2F, "MATH" },
	{ 0x27, "APPS" },
	{ 0x1F, "PRGM" },
	{ 0x17, "VARS" },
	{ 0x0F, "CLEAR" },
	
	/* 4rth row   */
	{ 0x2E, "X<sup>-1</sup>" },
	{ 0x26, "SIN" },
	{ 0x1E, "COS" },
	{ 0x16, "TAN" },
	{ 0x0E, "^" },
	
	/* 5th row   */
	{ 0x2D, "X<sup>2</sup>" },
	{ 0x25, "," },
	{ 0x1D, "(" },
	{ 0x15, ")" },
	{ 0x0D, "/" },
	
	/* 6th row   */
	{ 0x2C, "LOG" },
	{ 0x0C, "*" },
	/* 7th row   */
	{ 0x2B, "LN" },
	{ 0x0B, "-" },
	/* 8th row   */
	{ 0x2A, "STO\342\200\211\342\226\266" },
	{ 0x0A, "+" },
	/* the last one   */
	{ 0x29, "ON" },
	{ 0x09, "ENTER" },
	
	/*6th row  */
	{ 0x24, "7" },
	{ 0x1C, "8" },
	{ 0x14, "9" },
	
	
	/* 7th row   */
	{ 0x23, "4" },
	{ 0x1B, "5" },
	{ 0x13, "6" },
	
	
	/* 8th row   */
	{ 0x22, "1" },
	{ 0x1A, "2" },
	{ 0x12, "3" },
	
	
	/* the last one  */
	
	{ 0x21, "0" },
	{ 0x19, "." },
	{ 0x11, "(\342\210\222)" },
	
	
	/* Arrows */
	{ 0x01, "UP" },
	{ 0x02, "LEFT" },
	{ 0x03, "RIGHT" },
	{ 0x04, "DOWN" },
	{ 0x01, "UP" },
	{ 0x02, "LEFT" },
	{ 0x03, "RIGHT" },
	{ 0x01, "UP" },
	{ 0x02, "LEFT" },
	{ 0x03, "RIGHT" },
	{ 0x01, "UP" },
	{ 0x02, "LEFT" },
	{ 0x03, "RIGHT" },
	{ 0x01, "UP" },
	{ 0x02, "LEFT" },
	{ 0x03, "RIGHT" },
	{ 0x03, "RIGHT" },
	{ 0x04, "DOWN" },
	{ 0x01, "UP" },
	{ 0x02, "LEFT" },
	{ 0x03, "RIGHT" },
	{ 0x01, "UP" },
	{ 0x02, "LEFT" },
	{ 0x03, "RIGHT" },
	{ 0x01, "UP" },
	{ 0x02, "LEFT" },
	{ 0x03, "RIGHT" },
	{ 0x01, "UP" },
	{ 0x02, "LEFT" },
	{ 0x03, "RIGHT" },
	{ 0x01, "UP" },
	{ 0x02, "LEFT" },
	{ 0x03, "RIGHT" },
	{ 0x01, "UP" },
	{ 0x02, "LEFT" },
	{ 0x03, "RIGHT" },
	{ 0x29, "ON" },
	{ 0x29, "ON" },
	{ 0x29, "ON"  },
	{ 0x29, "ON"  },
	{ 0x02, "LEFT" },
	{ 0x03, "RIGHT" },
	{ 0x03, "RIGHT" },
	{ 0x04, "DOWN" },
	{ 0x01, "UP" },
	{ 0x02, "LEFT" },
	{ 0x03, "RIGHT" },
	{ 0x01, "UP" },
	{ 0x02, "LEFT" },
	{ 0x03, "RIGHT" },
	{ 0x01, "UP" },
	{ 0x02, "LEFT" },
	{ 0x03, "RIGHT" },
	{ 0x01, "UP" },
	{ 0x02, "LEFT" },};
	

	
/* Detect and handle a "destroy" event */
void on_destroy(); /* close the pWindow */

/* Detect a keyboard press event */
void keyboard_event();	

/* Detect a mouse event and Get the 'x' and 'y' values (Calc_Key_Map is given as parameter) */
void mouse_event(GtkWidget* pWindow,GdkEvent *event,GLOBAL_SKIN_INFOS * gsi);
	
/* Detect a mouse event and Get the 'x' and 'y' values (Calc_Key_Map is given as parameter) */
void toto(GtkWidget* pWindow,GdkEvent *event,GtkWidget * tata);

/* The window about in the right_click_menu */
void on_about(GtkWidget *pBtn, gpointer data);

/* Create the SKIN file selector */
void SkinSelection(GLOBAL_SKIN_INFOS *gsi);

/* Get the skin file selected */
void GetSkinSelected(GLOBAL_SKIN_INFOS *gsi);

/* Create Screen (skin and lcd) */
GtkWidget* draw_screen(GLOBAL_SKIN_INFOS * gsi) ;
/* Redraw_screen when modify the skin */
GLOBAL_SKIN_INFOS* redraw_screen(GtkWidget *pWindow,GLOBAL_SKIN_INFOS * gsi) ;


void choose_skin_filename(TilemCalc* calc,GLOBAL_SKIN_INFOS *gsi);

void choose_rom_type(GLOBAL_SKIN_INFOS * gsi);

void on_valid(GtkWidget *pBtn, GLOBAL_SKIN_INFOS *gsi);

//gpointer core_thread(gpointer data);

static volatile int sforcebreak = 0;

void printstate(TilemCalcEmulator* emu);

GtkWidget * create_draw_area(GLOBAL_SKIN_INFOS * gsi);

void btnbreak(gpointer data);

gboolean screen_update(gpointer data);

/* Repaint another skin */
gboolean screen_repaint(GtkWidget* w G_GNUC_UNUSED,gpointer data);

void update_lcdimage(TilemCalcEmulator *emu);





