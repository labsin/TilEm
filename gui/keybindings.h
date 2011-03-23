
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <string.h>
#include <stdlib.h>
#include <gui.h>

#include <gdk/gdkkeysyms.h>
#include <scancodes.h>


#ifndef KEYBINDINGS_FILE
#define KEYBINDINGS_FILE "keybindings.ini"
#endif


/* A table to translating string values to gdkkeysyms */
typedef struct GdkKeySyms {
	char* keysymstr;
	unsigned int keysym;
} GdkKeySyms;

GdkKeySyms gks[] = {
	{"GDK_A", GDK_A},
	{"GDK_B", GDK_B},
	{"GDK_C", GDK_C},
	{"GDK_D", GDK_D},
	{"GDK_Escape", GDK_Escape},
	{"GDK_BackSpace", GDK_BackSpace},
	{"GDK_KP_Left",GDK_KP_Left},
	{"GDK_KP_Up" , GDK_KP_Up },
	{"GDK_KP_Right", GDK_KP_Right},
	{"GDK_KP_Down", GDK_KP_Down},
	{"GDK_KP_Multiply", GDK_KP_Multiply},
	{"GDK_KP_Add", GDK_KP_Add},
	{"GDK_KP_Subtract", GDK_KP_Subtract},
	{"GDK_KP_Divide", GDK_KP_Divide},
	{"GDK_KP_0", GDK_KP_0},
	{"GDK_KP_1", GDK_KP_1},
	{"GDK_KP_2", GDK_KP_2},
	{"GDK_KP_3", GDK_KP_3},
	{"GDK_KP_4", GDK_KP_4},
	{"GDK_KP_5", GDK_KP_5},
	{"GDK_KP_6", GDK_KP_6},
	{"GDK_KP_7", GDK_KP_7},
	{"GDK_KP_8", GDK_KP_8},
	{"GDK_KP_9", GDK_KP_9},
	{"GDK_0", GDK_0},
	{"GDK_1", GDK_1},
	{"GDK_2", GDK_2},
	{"GDK_3", GDK_3},
	{"GDK_4", GDK_4},
	{"GDK_5", GDK_5},
	{"GDK_6", GDK_6},
	{"GDK_7", GDK_7},
	{"GDK_8", GDK_8},
	{"GDK_9", GDK_9},
	{"GDK_A", GDK_A},
	{"GDK_B", GDK_B},
	{"GDK_C", GDK_C},
	{"GDK_D", GDK_D},
	{"GDK_E", GDK_E},
	{"GDK_F", GDK_F},
	{"GDK_G", GDK_G},
	{"GDK_H", GDK_H},
	{"GDK_I", GDK_I},
	{"GDK_J", GDK_J},
	{"GDK_K", GDK_K},
	{"GDK_L", GDK_L},
	{"GDK_M", GDK_M},
	{"GDK_N", GDK_N},
	{"GDK_O", GDK_O},
	{"GDK_P", GDK_P},
	{"GDK_Q", GDK_Q},
	{"GDK_R", GDK_R},
	{"GDK_S", GDK_S},
	{"GDK_T", GDK_T},
	{"GDK_U", GDK_U},
	{"GDK_V", GDK_V},
	{"GDK_W", GDK_W},
	{"GDK_X", GDK_X},
	{"GDK_Y", GDK_Y},
	{"GDK_Z", GDK_Z},
	{"GDK_a", GDK_a},
	{"GDK_b", GDK_b},
	{"GDK_c", GDK_c},
	{"GDK_d", GDK_d},
	{"GDK_e", GDK_e},
	{"GDK_f", GDK_f},
	{"GDK_g", GDK_g},
	{"GDK_h", GDK_h},
	{"GDK_i", GDK_i},
	{"GDK_j", GDK_j},
	{"GDK_k", GDK_k},
	{"GDK_l", GDK_l},
	{"GDK_m", GDK_m},
	{"GDK_n", GDK_n},
	{"GDK_o", GDK_o},
	{"GDK_p", GDK_p},
	{"GDK_q", GDK_q},
	{"GDK_r", GDK_r},
	{"GDK_s", GDK_s},
	{"GDK_t", GDK_t},
	{"GDK_u", GDK_u},
	{"GDK_v", GDK_v},
	{"GDK_w", GDK_w},
	{"GDK_x", GDK_x},
	{"GDK_y", GDK_y},
	{"GDK_z", GDK_z}
};

/* A table for translating string values to scancode */
typedef struct TilemKeySyms {
	char* scancodestr;
	int scancode;
} TilemKeySyms;


TilemKeySyms tks[] = {
	{"TILEM_KEY_ALPHA", TILEM_KEY_ALPHA},
	{"TILEM_KEY_MODE", TILEM_KEY_MODE},
	{"TILEM_KEY_MATH", TILEM_KEY_MATH},
	{"TILEM_KEY_ENTER", TILEM_KEY_ENTER},
	{"TILEM_KEY_DOWN", TILEM_KEY_DOWN},
	{"TILEM_KEY_LEFT", TILEM_KEY_LEFT},
	{"TILEM_KEY_RIGHT", TILEM_KEY_RIGHT},
	{"TILEM_KEY_UP", TILEM_KEY_UP},
	{"TILEM_KEY_ADD", TILEM_KEY_ADD},
	{"TILEM_KEY_SUB", TILEM_KEY_SUB},
	{"TILEM_KEY_MUL", TILEM_KEY_MUL},
	{"TILEM_KEY_DIV", TILEM_KEY_DIV},
	{"TILEM_KEY_POWER", TILEM_KEY_POWER},
	{"TILEM_KEY_CLEAR", TILEM_KEY_CLEAR},
	{"TILEM_KEY_CHS", TILEM_KEY_CHS},
	{"TILEM_KEY_3", TILEM_KEY_3},
	{"TILEM_KEY_6", TILEM_KEY_6},
	{"TILEM_KEY_9", TILEM_KEY_9},
	{"TILEM_KEY_RPAREN", TILEM_KEY_RPAREN},
	{"TILEM_KEY_TAN", TILEM_KEY_TAN },
	{"TILEM_KEY_VARS", TILEM_KEY_VARS},
	{"TILEM_KEY_DECPNT", TILEM_KEY_DECPNT},
	{"TILEM_KEY_2", TILEM_KEY_2},
	{"TILEM_KEY_5", TILEM_KEY_5},
	{"TILEM_KEY_8", TILEM_KEY_8},
	{"TILEM_KEY_LPAREN", TILEM_KEY_LPAREN},
	{"TILEM_KEY_COS", TILEM_KEY_COS},
	{"TILEM_KEY_PRGM", TILEM_KEY_PRGM},
	{"TILEM_KEY_STAT", TILEM_KEY_STAT},
	{"TILEM_KEY_0", TILEM_KEY_0},
	{"TILEM_KEY_1", TILEM_KEY_1},
	{"TILEM_KEY_4", TILEM_KEY_4},
	{"TILEM_KEY_7", TILEM_KEY_7},
	{"TILEM_KEY_COMMA", TILEM_KEY_COMMA},
	{"TILEM_KEY_SIN", TILEM_KEY_SIN},
	{"TILEM_KEY_MATRIX", TILEM_KEY_MATRIX},
	{"TILEM_KEY_GRAPHVAR", TILEM_KEY_GRAPHVAR},
	{"TILEM_KEY_ON", TILEM_KEY_ON},
	{"TILEM_KEY_STORE", TILEM_KEY_STORE},
	{"TILEM_KEY_LN", TILEM_KEY_LN},
	{"TILEM_KEY_LOG", TILEM_KEY_LOG},
	{"TILEM_KEY_SQUARE", TILEM_KEY_SQUARE},
	{"TILEM_KEY_RECIP", TILEM_KEY_RECIP},
	{"TILEM_KEY_GRAPH", TILEM_KEY_GRAPH},
	{"TILEM_KEY_TRACE", TILEM_KEY_TRACE},
	{"TILEM_KEY_ZOOM", TILEM_KEY_ZOOM},
	{"TILEM_KEY_WINDOW", TILEM_KEY_WINDOW},
	{"TILEM_KEY_YEQU", TILEM_KEY_YEQU},
	{"TILEM_KEY_2ND", TILEM_KEY_2ND},
	{"TILEM_KEY_DEL", TILEM_KEY_DEL}

};


/* Function to translate a string value to a gdksyms */
unsigned int tilem_get_gdkkeysyms(GdkKeySyms *gks, char* gdkkeysymsstr);

/* Function to translate a string value to a tilem scancode */
unsigned int tilem_get_tilemkeysyms(TilemKeySyms *tks, char* tilemkeysymsstr);

/* Search the associated code into keybindings config file */
char* tilem_ksc_get_scancode(char* gdkkeysymstr, const char* model);

/* Get all keys */
static gchar** tilem_ksc_get_all_keys(const char* model);
