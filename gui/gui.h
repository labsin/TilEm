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


/* THANK YOU Julien Solignac AND/OR Benjamin Moody FOR THIS MAGIC "ASCII" ;D
 * 
 * The Keypad...
 *
 * is described by one or more 'kmapsto' structures.  A HW model
 * *should* only need one, but the 84+ has a stupid keyboard which
 * means we allow more than one.
 *
 *            xbegin_btn_w   	    x_jump_btn_w 
 *                           	    |<	      >|
 *                  |      | |      | |      | |      | |      |
 *  ybegin_btn_w  --+------+-+------+-+------+-+------+-+------+-
 *                  |  F1  | |  F2  | |  F3  | |  F4  | |  F5  |
 *  	          --+------+-+------+-+------+-+------+-+------+-
 *                  |      | |      | |      | |      | |      |
 *                  |      | |      | |      |  +----+---+----+
 *  xbegin_btn_rk --+------+-+------+-+------+- +--+ |   | +--+
 *                  | 2nd  | |      | |      |  |  | +---+ |  |
 *                --+------+-+------+-+------+- |  |       |  |
 *            -   --+------+-+------+-+------+- |  | +---+ |  |
 *   y_jump_rk|     |Alpha | |      | |      |  +--+ |   | +--+
 *   	      |   --+------+-+------+-+------+- +----+---+----+
 *            -   --+------+-+------+-+------+-+------+-+------+-
 *                  |      | |      | |      | |      | |      |
 *                --+------+-+------+-+------+-+------+-+------+-
 *                --+------+-+------+-+------+-+------+-+------+-
 *   	            |      | |      | |      | |      | |      |
 *                --+------+-+------+-+------+-+------+-+------+-
 *                  |      | |      | |      | |      | |      |
 *                --+------+-+------+-+------+-+------+-+------+-
 *                --+------+-+------+-+------+-+------+-+------+-
 *                  |      | |      | |      | |      | |      |
 *		    |<	    >|
 *                  x_jump_btn_rk          	   
 *
 *
 *                            td      td
 *              boxleft  left min    max right  boxright
 *                 |       |  |        |  |       |
 * ar_boxtop     --+-------+--+--------+--+-------+-
 *                 |       |  |        |  |       |
 *                 |       |  |   UP   |  |       |
 *                 |       |  |        |  |       |
 * ar_top        --+-------|--+--------+----------+-
 *                 |       |  |        |  |       |
 * ar_lrmin      --+-------+--|-----------+-------+-
 *                 |       |  |        |  |       |
 *                 | LEFT  |  |        |  | RIGHT |
 *                 |       |  |        |  |       |
 * ar_lrmax      --+-------+-----------|--+-------+-
 *                 |       |  |        |  |       |
 * ar_down       --+----------+--------+--|-------+-
 *                 |       |  |        |  |       |
 *                 |       |  |  DOWN  |  |       |
 *                 |       |  |        |  |       |
 * ar_boxbase    --+-------+--+--------+--+-------+-
 *
 */


typedef struct TilemKeyCoord {
	/* window */
	int x_begin_btn_w;
	int y_begin_btn_w;
	int x_size_btn_w;
	int y_size_btn_w;
	int x_jump_btn_w;
	/* "real" keyb */
	int x_begin_btn_rk;
	int y_begin_btn_rk;
	int x_size_btn_rk;
	int y_size_btn_rk;
	int x_jump_btn_rk;
	int y_jump_btn_rk;
	/* arrows */
	int x_begin_btn_ar;
	int y_begin_btn_ar;
	int x_size_btn_ar;
	int y_size_btn_ar;
	int x_jump_btn_ar;
}TilemKeyCoord;


/* List of the keys (a code and a label) */
typedef struct TilemKeyList {
	int code;
	const char* label;
}TilemKeyList;


/* This struct is used to know which key is clicked */
typedef struct TilemKeyMap {
	TilemKeyCoord Calc_Key_Coord;
	TilemKeyList Calc_Key_List[60];	//put 60 for the warning...
}TilemKeyMap;

static const struct TilemKeyCoord x3_coord= {
	22,189,52,11,39,22,231,52,16,39,28,320,320,320,320,6};
	
static const struct TilemKeyCoord test_coord= {
	0,0,52,11,39,22,231,52,16,39,28,320,320,320,320,6};


/* For the TI83 by example */
static const struct TilemKeyList x3_keylist[] = {
	// Window  (5keys at the top of the keyboard)
	{ 0x35, "Y=" },
	{ 0x34, "WINDOW" },
	{ 0x33, "ZOOM" },
	{ 0x32, "TRACE" },
	{ 0x31, "GRAPH" },
	
	// 1rst row (only 3 keys by line)
	{ 0x36, "2nd" },
	{ 0x37, "MODE" },
	{ 0x38, "DEL" },
	
	//2nd row  (only 3 keys by line)
	{ 0x30, "ALPHA" },
	{ 0x28, "X,T,\342\200\212\316\270,\342\200\212<i>n</i>" },
	{ 0x20, "STAT" },
	
	//3rd row 
	{ 0x2F, "MATH" },
	{ 0x27, "APPS" },
	{ 0x1F, "PRGM" },
	{ 0x17, "VARS" },
	{ 0x0F, "CLEAR" },
	
	// 4rth row 
	{ 0x2E, "X<sup>-1</sup>" },
	{ 0x26, "SIN" },
	{ 0x1E, "COS" },
	{ 0x16, "TAN" },
	{ 0x0E, "^" },
	
	//5th row
	{ 0x2D, "X<sup>2</sup>" },
	{ 0x25, "," },
	{ 0x1D, "(" },
	{ 0x15, ")" },
	{ 0x0D, "/" },
	
	// 6th row 
	{ 0x2C, "LOG" },
	{ 0x24, "7" },
	{ 0x1C, "8" },
	{ 0x14, "9" },
	{ 0x0C, "*" },
	
	// 7th row 
	{ 0x2B, "LN" },
	{ 0x23, "4" },
	{ 0x1B, "5" },
	{ 0x13, "6" },
	{ 0x0B, "-" },
	
	// 8th row 
	{ 0x2A, "STO\342\200\211\342\226\266" },
	{ 0x22, "1" },
	{ 0x1A, "2" },
	{ 0x12, "3" },
	{ 0x0A, "+" },
	
	// the last one 
	{ 0x29, "ON" },
	{ 0x21, "0" },
	{ 0x19, "." },
	{ 0x11, "(\342\210\222)" },
	{ 0x09, "ENTER" },
	
	// Arrows 
	{ 0x01, "UP" },
	{ 0x02, "LEFT" },
	{ 0x03, "RIGHT" },
	{ 0x04, "DOWN" }}; 

	
/* Actually this values are not really good even for the 83... used for testing tilem_guess_key_map */
/* Warning : this will be probably modified because it exist model with numpad isn't aligned with the rest of real key */
static const struct TilemKeyMap x2_keymap= {
	{19,186,48,10,29,12,222,48,16,39,28,320,320,320,320,6},

	{
	/* Window  (5keys at the top of the keyboard)*/
	{ 0x35, "Y=" },
	{ 0x34, "WINDOW" },
	{ 0x33, "ZOOM" },
	{ 0x32, "TRACE" },
	{ 0x31, "GRAPH" },
	
	/* 1rst row (only 3 keys by line)*/
	{ 0x36, "2nd2" },
	{ 0x37, "MODE" },
	{ 0x38, "DEL" },
	
	/* 2nd row  (only 3 keys by line)*/
	{ 0x30, "ALPHA" },
	{ 0x28, "X,T,\342\200\212\316\270,\342\200\212<i>n</i>" },
	{ 0x20, "STAT" },
	
	/* 3rd row */
	{ 0x2F, "MATH" },
	{ 0x27, "APPS" },
	{ 0x1F, "PRGM" },
	{ 0x17, "VARS" },
	{ 0x0F, "CLEAR" },
	
	/* 4rth row */
	{ 0x2E, "X<sup>-1</sup>" },
	{ 0x26, "SIN" },
	{ 0x1E, "COS" },
	{ 0x16, "TAN" },
	{ 0x0E, "^" },
	
	/* 5th row */
	{ 0x2D, "X<sup>2</sup>" },
	{ 0x25, "," },
	{ 0x1D, "(" },
	{ 0x15, ")" },
	{ 0x0D, "/" },
	
	/* 6th row */
	{ 0x2C, "LOG" },
	{ 0x24, "7" },
	{ 0x1C, "8" },
	{ 0x14, "9" },
	{ 0x0C, "*" },
	
	/* 7th row */
	{ 0x2B, "LN" },
	{ 0x23, "4" },
	{ 0x1B, "5" },
	{ 0x13, "6" },
	{ 0x0B, "-" },
	
	/* 8th row */
	{ 0x2A, "STO\342\200\211\342\226\266" },
	{ 0x22, "1" },
	{ 0x1A, "2" },
	{ 0x12, "3" },
	{ 0x0A, "+" },
	
	/* the last one */
	{ 0x29, "ON" },
	{ 0x21, "0" },
	{ 0x19, "." },
	{ 0x11, "(\342\210\222)" },
	{ 0x09, "ENTER" },
	
	/* Arrows */
	{ 0x01, "UP" },
	{ 0x02, "LEFT" },
	{ 0x03, "RIGHT" },
	{ 0x04, "DOWN" }}};
	

static const struct TilemKeyMap x3_keymap= {
	{22,189,52,11,39,22,231,52,16,39,28,320,320,320,320,6},
	
	{
	//Window  (5keys at the top of the keyboard)
	{ 0x35, "Y=" },
	{ 0x34, "WINDOW" },
	{ 0x33, "ZOOM" },
	{ 0x32, "TRACE" },
	{ 0x31, "GRAPH" },
	
	// 1rst row (only 3 keys by line)
	{ 0x36, "2nd3" },
	{ 0x37, "MODE" },
	{ 0x38, "DEL" },
	
	// 2nd row  (only 3 keys by line)
	{ 0x30, "ALPHA" },
	{ 0x28, "X,T,\342\200\212\316\270,\342\200\212<i>n</i>" },
	{ 0x20, "STAT" },
	
	// 3rd row 
	{ 0x2F, "MATH" },
	{ 0x27, "APPS" },
	{ 0x1F, "PRGM" },
	{ 0x17, "VARS" },
	{ 0x0F, "CLEAR" },
	
	//4rth row 
	{ 0x2E, "X<sup>-1</sup>" },
	{ 0x26, "SIN" },
	{ 0x1E, "COS" },
	{ 0x16, "TAN" },
	{ 0x0E, "^" },
	
	// 5th row 
	{ 0x2D, "X<sup>2</sup>" },
	{ 0x25, "," },
	{ 0x1D, "(" },
	{ 0x15, ")" },
	{ 0x0D, "/" },
	
	// 6th row 
	{ 0x2C, "LOG" },
	{ 0x24, "7" },
	{ 0x1C, "8" },
	{ 0x14, "9" },
	{ 0x0C, "*" },
	
	// 7th row 
	{ 0x2B, "LN" },
	{ 0x23, "4" },
	{ 0x1B, "5" },
	{ 0x13, "6" },
	{ 0x0B, "-" },
	
	// 8th row 
	{ 0x2A, "STO\342\200\211\342\226\266" },
	{ 0x22, "1" },
	{ 0x1A, "2" },
	{ 0x12, "3" },
	{ 0x0A, "+" },
	
	// the last one 
	{ 0x29, "ON" },
	{ 0x21, "0" },
	{ 0x19, "." },
	{ 0x11, "(\342\210\222)" },
	{ 0x09, "ENTER" },
	
	// Arrows
	{ 0x01, "UP" },
	{ 0x02, "LEFT" },
	{ 0x03, "RIGHT" },
	{ 0x04, "DOWN" }}};


/* etc.... */
	


	
/* Detect and handle a "destroy" event */
void OnDestroy(GtkWidget *pWidget, gpointer pData);	// close the pWindow

/* Detect a keyboard press event */
void keyboard_event();	

/* Detect a mouse event and Get the 'x' and 'y' values (Calc_Key_Map is given as parameter) */
void mouse_event(GtkWidget* pWindow,GdkEvent *event,TilemKeyMap * Calc_Key_Map);	
	
	/* Detect a mouse event and Get the 'x' and 'y' values (Calc_Key_Map is given as parameter) */
void toto(GtkWidget* pWindow,GdkEvent *event,GtkWidget * tata);

/* Create a CalcSkin with an TilemCalcEmulator */
TilemCalcSkin* tilem_guess_skin_set(TilemCalc* calc);

/* Create a KeyMap with an TilemCalcEmulator */
TilemKeyMap* tilem_guess_key_map(TilemCalc* calc);	

/* Set a TilemKeyCoord.To adapt easily personal skins in the future */
void tilem_set_coord(TilemKeyMap *Calc_Key_Map,TilemKeyCoord test_coord);

/* Set a TilemCalcSkin.To adapt easily personal skins in the future */
void tilem_set_skin(TilemCalcSkin * Calc_Skin,TilemCalcSkin * skin_perso);



//static int nmenu_items = sizeof(menu_items) / sizeof(menu_items[0]);
gint button_press (GtkWidget *widget, GdkEvent *event);
void create_menus(GtkWidget *window, GtkItemFactoryEntry *items, int this_items, const char *menuname);


gint show_menu(GtkWidget *widget, GdkEvent *event);





