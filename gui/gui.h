#include <tilem.h>
#include <z80.h>
#include <skinops.h>
#include <scancodes.h>
#include <debuginfo.h>
//#include <tilemdb.h>
#define TI73   "TI73"
#define TI82   "TI82"
#define TI83   "TI83"
#define TI83P  "TI83+"
#define TI85   "TI85"
#define TI86   "TI86"
#define X_FRINGE 2
#define Y_FRINGE 1
 
/* A global boolean to say "save the state" */
int SAVE_STATE;

typedef struct _TilemDebuggerRegister {
	GtkWidget* reg[12];
	
} TilemDebuggerRegister;

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
	GtkWidget *pFrame;
	GtkWidget *pAf;
	gchar* SkinFileName;
	GtkFileSelection *FileSelected;
	GtkWidget *pRadio;
	int view;
	char calc_id;
	char *RomName;
	char RomType;
	TilemCalcEmulator *emu;
	TilemDebuggerRegister *reg_entry;
	gboolean isDebuggerRunning;
}GLOBAL_SKIN_INFOS;

static const char rcstr[] =
	"style \"tilem-key-default\" {\n"
	"  font_name = \"Sans 7\"\n"
	"  GtkButton::inner-border = { 0, 0, 0, 0 }\n"
	"  GtkButton::focus-padding = 0\n"
	"}\n"
	"widget \"*.tilem-key\" style \"tilem-key-default\"\n"
	"style \"tilem-lcd-default\" {\n"
	"  bg[NORMAL] = { 0.74, 0.74, 0.70 }\n"
	"  fg[NORMAL] = { 0.0, 0.0, 0.0 }\n"
	"}\n"
	"widget \"*.tilem-lcd\" style \"tilem-lcd-default\"\n";

static const struct KEY_LIST x4_keylist[] = {
	
	{ 0x35, "Y=" },
	{ 0x34, "WINDOW" },
	{ 0x33, "ZOOM" },
	{ 0x32, "TRACE" },
	{ 0x31, "GRAPH" },
	
	{ 0x36, "2nd" },
	{ 0x37, "MODE" },
	{ 0x38, "DEL" },
	
	{ 0x02, "LEFT" },
	{0x03, "RIGHT" },
	{ 0x04, "TOP" },
	{ 0x01, "BOTTOM" },
	
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
	{ 0x24, "7" },
	{ 0x1C, "8" },
	{ 0x14, "9" },
	{ 0x0C, "*" },
	
	/* 7th row   */
	{ 0x2B, "LN" },
	{ 0x23, "4" },
	{ 0x1B, "5" },
	{ 0x13, "6" },
	{ 0x0B, "-" },
	
	/* 8th row   */
	{ 0x2A, "STO->" },
	{ 0x22, "1" },
	{ 0x1A, "2" },
	{ 0x12, "3" },
	{ 0x0A, "+" },
	
	/* the last one   */
	{ 0x29, "ON" },	/* ## ON ## */
	{ 0x21, "0" },
	{ 0x25, "." },
	{ 0x11, "(-)" }, 
	{ 0x09, "ENTER" },
	
	/* the last one   */
	{ 0x29, "ON" },	/* ## ON ## */
	{ 0x21, "0" },
	{ 0x21, "." },
	{ 0x29, "(-)" }, 
	{ 0x09, "ENTER" },
	
	/* the last one   */
	{ 0x29, "ON" },	/* ## ON ## */
	{ 0x21, "0" },
	{ 0x21, "." },
	{ 0x29, "(-)" }, 
	{ 0x09, "ENTER" },
	
	/* the last one   */
	{ 0x29, "ON" },	/* ## ON ## */
	{ 0x21, "0" },
	{ 0x21, "." },
	{ 0x29, "(-)" }, 
	{ 0x09, "ENTER" },
	
	/* the last one   */
	{ 0x29, "ON" },	/* ## ON ## */
	{ 0x21, "0" },
	{ 0x21, "." },
	{ 0x29, "(-)" }, 
	{ 0x09, "ENTER" },
	
	};
	
	static const struct KEY_LIST x3_keylist[] = {
	
	{ 0x35, "Y=" },
	{ 0x34, "WINDOW" },
	{ 0x33, "ZOOM" },
	{ 0x32, "TRACE" },
	{ 0x31, "GRAPH" },
	
	{ 0x36, "2nd" },
	{ 0x37, "MODE" },
	{ 0x38, "DEL" },
	
	{ 0x02, "LEFT" },
	{0x03, "RIGHT" },
	{ 0x04, "TOP" },
	{ 0x01, "BOTTOM" },
	
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
	{ 0x24, "7" },
	{ 0x1C, "8" },
	{ 0x14, "9" },
	{ 0x0C, "*" },
	
	/* 7th row   */
	{ 0x2B, "LN" },
	{ 0x23, "4" },
	{ 0x1B, "5" },
	{ 0x13, "6" },
	{ 0x0B, "-" },
	
	/* 8th row   */
	{ 0x2A, "STO->" },
	{ 0x22, "1" },
	{ 0x1A, "2" },
	{ 0x12, "3" },
	{ 0x0A, "+" },
	
	/* the last one   */
	{ 0x29, "ON" },	/* ## ON ## */
	{ 0x21, "0" },
	{ 0x25, "." },
	{ 0x11, "(-)" }, 
	{ 0x09, "ENTER" },
	
	/* the last one   */
	{ 0x29, "ON" },	/* ## ON ## */
	{ 0x21, "0" },
	{ 0x21, "." },
	{ 0x29, "(-)" }, 
	{ 0x09, "ENTER" },
	
	/* the last one   */
	{ 0x29, "ON" },	/* ## ON ## */
	{ 0x21, "0" },
	{ 0x21, "." },
	{ 0x29, "(-)" }, 
	{ 0x09, "ENTER" },
	
	/* the last one   */
	{ 0x29, "ON" },	/* ## ON ## */
	{ 0x21, "0" },
	{ 0x21, "." },
	{ 0x29, "(-)" }, 
	{ 0x09, "ENTER" },
	
	/* the last one   */
	{ 0x29, "ON" },	/* ## ON ## */
	{ 0x21, "0" },
	{ 0x21, "." },
	{ 0x29, "(-)" }, 
	{ 0x09, "ENTER" },
	
	};
	
/* core's forcebreak value */
static volatile int sforcebreak = 0;
	

/* ###### event.c ##### */

/* Detect and handle a "destroy" event */
void on_destroy(); /* close the pWindow */

/* The window about in the right_click_menu */
void on_about(GtkWidget *pBtn);

/* Detect a keyboard press event */
void keyboard_event();	

/* Detect a mouse event and Get the 'x' and 'y' values (Calc_Key_Map is given as parameter) */
gboolean mouse_press_event(GtkWidget* pWindow,GdkEvent *event,GLOBAL_SKIN_INFOS * gsi);
gboolean mouse_release_event(GtkWidget* pWindow,GdkEvent *event,GLOBAL_SKIN_INFOS * gsi) ;
	


/* ###### skin.c ##### */
	
/* Create the SKIN file selector */
void SkinSelection(GLOBAL_SKIN_INFOS *gsi);

/* Get the skin file selected */
void GetSkinSelected(GLOBAL_SKIN_INFOS *gsi);

/* Choose automatically wich skin tilem must load */
void choose_skin_filename(TilemCalc* calc,GLOBAL_SKIN_INFOS *gsi);



/* ###### screen.c ##### */

/* Create Screen (skin and lcd) */
GtkWidget* draw_screen(GLOBAL_SKIN_INFOS * gsi) ;

/* Redraw_screen when modify the skin */
GLOBAL_SKIN_INFOS* redraw_screen(GtkWidget *pWindow,GLOBAL_SKIN_INFOS * gsi) ;

/* Create the lcd area */
GtkWidget * create_draw_area(GLOBAL_SKIN_INFOS * gsi);

/* Repaint another skin */
gboolean screen_repaint(GtkWidget* w G_GNUC_UNUSED,GdkEventExpose* ev G_GNUC_UNUSED,GLOBAL_SKIN_INFOS * gsi);

/* update the screen.Repaint the drawing_area widget */
gboolean screen_update(gpointer data);

/* refresh the lcd content */
void update_lcdimage(TilemCalcEmulator *emu);

/* ##### event.c ##### */

/* Switch view to lcd only or skin + lcd */
void switch_view(GLOBAL_SKIN_INFOS * gsi) ;

/* Switch borderless. */
void switch_borderless(GLOBAL_SKIN_INFOS* gsi); 

/* Create the right click menu */
void create_menus(GtkWidget *window,GdkEvent *event,GtkItemFactoryEntry *items, int this_items, const char *menuname,gpointer* gsi);

/* Adapt the style */
void screen_restyle(GtkWidget* w, GtkStyle* oldstyle G_GNUC_UNUSED,GLOBAL_SKIN_INFOS * gsi);

/* Resize screen */
void screen_resize(GtkWidget* w G_GNUC_UNUSED,GtkAllocation* alloc, GLOBAL_SKIN_INFOS * gsi);



/* ##### tool.c ##### */

/* Generic popup (could be use for all kind of msg) */
void popup_error(char* msg, GLOBAL_SKIN_INFOS * gsi);

/* The popup to choose what kind of rom you are trying to load  (at startup)*/
char choose_rom_popup();

/* like on_destroy but save state */
void quit_with_save();

/* Dialog mesg */
void show_about();

/* ##### config.c ##### */

/* Create the config.dat file  (normally only the first launch */
void create_config_dat();

void write_default_skin_for_specific_rom();

gboolean search_string(char* string,GLOBAL_SKIN_INFOS *gsi);

gboolean cmp_string(char* string, GLOBAL_SKIN_INFOS *gsi);

/* ##### debugger.c ##### */
void launch_debugger(GLOBAL_SKIN_INFOS *gsi);

void refresh_register(GLOBAL_SKIN_INFOS* gsi);








