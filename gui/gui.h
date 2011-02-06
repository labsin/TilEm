#include <tilem.h>
#include <z80.h>
#include <skinops.h>
#include <config.h>
#include <romconfig.h>
#include <scancodes.h>
#include <debuginfo.h>
#include <tilemdb.h>

#include <ticalcs.h>
#include <ticables.h>
#include <tifiles.h>
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

	GMutex* calc_mutex;
	TilemCalc* calc;

	GMutex* lcd_mutex;
	TilemGrayLCD* glcd;

	byte* lcd_image_buf;
	int lcd_image_width;
	int lcd_image_height;
	GdkRgbCmap* lcd_cmap;

	GtkWidget* lcdwin;
} TilemCalcEmulator;

/* Internal data structure for gui */
typedef struct GLOBAL_SKIN_INFOS {
	SKIN_INFOS *si;
	CONFIG_INFOS *ci;
	ROMCONFIG_INFOS *rci;
	GtkWidget *pWindow;
	GtkWidget *pLayout;
	GtkWidget *pFrame;
	GtkWidget *pAf;
	gchar* FileSelected;
	GtkFileChooser *pFileChooser;
	gint FileChooserResult;
	gchar* SkinFileName;
	GtkWidget *pRadio;
	int view;
	char calc_id;
	char *RomName;
	char *SavName;
	char RomType;
	char *FileToLoad; 
	char *MacroName;
	TilemCalcEmulator *emu;
	TilemDebuggerRegister *reg_entry;
	gboolean isDebuggerRunning;
	GtkWidget* stack_treeview;	
	FILE * macro_file;
	FILE * animation_file;
	gboolean isMacroRecording;
	gboolean isMacroPlaying;
	gboolean isStartingSkinless;

	int mouse_key;		/* Key currently pressed by mouse button */
}GLOBAL_SKIN_INFOS;


/* ###### event.c ##### */

/* Detect and handle a "destroy" event */
void on_destroy(); /* close the pWindow */

/* Save state of current rom */
void save_state(GLOBAL_SKIN_INFOS * gsi);

/* The window about in the right_click_menu */
void on_about(GtkWidget *pBtn);

/* Detect a keyboard press event */
void keyboard_event();	

/* Button-press event */
gboolean mouse_press_event(GtkWidget* w, GdkEventButton *event, gpointer data);

/* Pointer-motion event */
gboolean pointer_motion_event(GtkWidget* w, GdkEventMotion *event, gpointer data);

/* Button-release event */
gboolean mouse_release_event(GtkWidget* w, GdkEventButton *event, gpointer data);

/* Load a file from PC to TI */
void load_file(GLOBAL_SKIN_INFOS *gsi);

/* Load the file designed by filename */
void load_file_from_file(GLOBAL_SKIN_INFOS *gsi, char* filename) ;

/* Take a screenshot i*/
void screenshot(GLOBAL_SKIN_INFOS *gsi);




/* ###### skin.c ##### */
	
/* Create the SKIN file selector */
void skin_selection(GLOBAL_SKIN_INFOS *gsi);

/* Choose automatically wich skin tilem must load */
void choose_skin_filename(TilemCalc* calc,GLOBAL_SKIN_INFOS *gsi);




/* ###### screen.c ##### */

/* Create Screen (skin and lcd) */
GtkWidget* draw_screen(GLOBAL_SKIN_INFOS * gsi) ;

/* Redraw_screen when modify the skin */
void redraw_screen(GLOBAL_SKIN_INFOS *gsi);

/* Create the lcd area */
GtkWidget * create_draw_area(GLOBAL_SKIN_INFOS * gsi);

/* Repaint another skin */
gboolean screen_repaint(GtkWidget* w G_GNUC_UNUSED,GdkEventExpose* ev G_GNUC_UNUSED,GLOBAL_SKIN_INFOS * gsi);

/* update the screen.Repaint the drawing_area widget */
gboolean screen_update(gpointer data);

/* refresh the lcd content */
void update_lcdimage(TilemCalcEmulator *emu);

/* Display the lcd image into the terminal */
void display_lcdimage_into_terminal(GLOBAL_SKIN_INFOS* gsi);



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
char choose_rom_popup(GtkWidget *parent_window, const char *filename,
                      char default_model);

/* like on_destroy but save state */
void quit_with_save();

/* Dialog mesg */
void show_about();

/* File chooser */
char * select_file(GLOBAL_SKIN_INFOS *gsi, char* basedir);

/* File chooser with a different folder */
void select_file_with_basedir(GLOBAL_SKIN_INFOS *gsi, char* basedir);

/* Get the skin file selected */
void get_selected_file(GLOBAL_SKIN_INFOS *gsi);




/* ##### config.c ##### */

/* Create the config.dat file  (normally only the first launch */
void create_config_dat(GLOBAL_SKIN_INFOS* gsi);

/* Just load the config_file by reading it and save into CONFIG_INFOS */
void config_load(CONFIG_INFOS *infos);

/* Called by event.c in the right click menu */
void write_default_skin_for_this_rom();

/* Write the config.dat (modification only) */
void write_config_file(GLOBAL_SKIN_INFOS *gsi);

/* Search for the romname in the CONFIG_INFOS struct and answer by true or false */
gboolean is_this_rom_in_config_infos(char* romname,GLOBAL_SKIN_INFOS *gsi);

/* Get the name of the skin to use with this rom */
void search_defaultskin_in_config_infos(char* romname,GLOBAL_SKIN_INFOS *gsi);

/* search, write, and save config on right click menu */
void add_or_modify_defaultskin(GLOBAL_SKIN_INFOS* gsi);




/* ##### romconfig.c ##### */

/* Create the config.dat file  (normally only the first launch */
void create_romconfig_dat(GLOBAL_SKIN_INFOS* gsi);

/* Just load the config_file by reading it and save into CONFIG_INFOS */
void romconfig_load(ROMCONFIG_INFOS *infos);

/* Write the config.dat (modification only) */
void write_romconfig_file(GLOBAL_SKIN_INFOS *gsi);

/* Search for the romname in the CONFIG_INFOS struct and answer by true or false */
gboolean is_this_rom_in_romconfig_infos(char* romname,GLOBAL_SKIN_INFOS *gsi);

/* Get the name of the skin to use with this rom */
void search_defaultmodel_in_romconfig_infos(char* romname,GLOBAL_SKIN_INFOS *gsi);

/* search, write, and save config on right click menu */
void add_or_modify_defaultmodel(GLOBAL_SKIN_INFOS* gsi);




/* ##### debugger.c ##### */

/* Main function for the debugger. */
void launch_debugger(GLOBAL_SKIN_INFOS *gsi);

/* Refresh on click the register in the debugger */
void refresh_register(GLOBAL_SKIN_INFOS* gsi);

/* Refresh on click the stack in the debugger */
void refresh_stack(GLOBAL_SKIN_INFOS* gsi);

/* Print state register in terminal */
void printstate(TilemCalcEmulator* emu);




/* ##### link.c ##### */

/* Init libtis, create ch/cbl, attach cable, and send file to TI */
void send_file(TilemCalcEmulator* emu, CalcHandle* ch, const char* filename);

/* Init libtis, create ch/cbl, attach cable */ 
CableHandle* internal_link_handle_new(TilemCalcEmulator* emu);

/* test if the calc is ready */
int is_ready(CalcHandle* h);

/* Simply print the error */
void print_lc_error(int errnum);

/* Get calc model from calc_id */
int get_calc_model(TilemCalc* calc);

/* Simply emulate a click on key (use to prepare link -> come into receive mode) */
void run_with_key(TilemCalc* calc, int key);



/* ##### macro.c ##### */

/* Create the macro_file */
void create_or_replace_macro_file(GLOBAL_SKIN_INFOS* gsi) ;

/* Recording macro */
void add_event_in_macro_file(GLOBAL_SKIN_INFOS* gsi, char* string) ;

/* Add a load file */
void add_load_file_in_macro_file(GLOBAL_SKIN_INFOS* gsi, int length, char* filename) ;

/* Not used ...? */
void save_macro_file(GLOBAL_SKIN_INFOS* gsi) ;

/* Play it ! And play it again ! */
void play_macro(GLOBAL_SKIN_INFOS* gsi) ;

/* Play it ! And play it again ! */
void play_macro_from_file(GLOBAL_SKIN_INFOS* gsi) ;

/* Play it ! And play it again ! */
int play_macro_default(GLOBAL_SKIN_INFOS* gsi, char* macro_name) ;

/* Turn on the recording */
void start_record_macro(GLOBAL_SKIN_INFOS* gsi) ;

/* Turn off the recording */
void stop_record_macro(GLOBAL_SKIN_INFOS* gsi) ;

/* Run slowly to play macro */
void run_with_key_slowly(TilemCalc* calc, int key);



/* ##### args.c ##### */

/* Help (usage: how to use in command line) */
void help(char *name, int ret) ;

/* Command line argument handling */
int getargs(int argc, char* argv[], GLOBAL_SKIN_INFOS* gsi) ;

/* Create the SavName */
void create_savname(GLOBAL_SKIN_INFOS* gsi) ;



/* ##### animatedgif.c ##### */

/* Create a animated screenshot */
void screenshot_anim_create_nostatic(GLOBAL_SKIN_INFOS* gsi) ;

/* Add a frame to animation */
void screenshot_anim_addframe(GLOBAL_SKIN_INFOS* gsi) ;



/* ##### gifencod.c ##### */

/* Encode gif data */
void GifEncode(FILE *fout, unsigned char *pixels, int depth, int siz);



/* ##### screenshot.c ##### */

/* create the screenshot popup */
void create_screenshot_window(GLOBAL_SKIN_INFOS* gsi);
