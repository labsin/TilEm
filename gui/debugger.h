#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <gtk/gtk.h>
#include <tilem.h>
#include "gui.h"
#include "z80.h"
#include <malloc.h>
#include "tilemdb.h"
#include <glib.h>

//typedef unsigned short  gushort;

/* Central dasm list */
enum
{
	COL_OFFSET_DASM = 0,
	COL_OP_DASM,
	COL_ARGS_DASM,
  	NUM_COLS_DASM
};

/* Stack list */
enum
{
	COL_OFFSET_STK = 0,
	COL_VALUE_STK,
  	NUM_COLS_STK
};

/* Memory list */
enum
{
	COL_OFFSET_MEM = 0,
	COL_HEXA_MEM = 1,
	COL_ASCII_MEM = 9,
  	NUM_COLS_MEM = 10
};

/* Disasm struct */
TilemDisasm *dasm;

/* Register values */
char* rvalue[12];

/* List of label for create_register_list */
static char *rlabel[12] = {"AF'", "AF", "BC'", "BC", "DE'", "DE", "HL'", "HL", "IY", "IX", "SP", "PC" };

/* close debug window */
static void on_debug_destroy(GtkWidget* debug_win, GdkEvent* Event, GLOBAL_SKIN_INFOS* gsi);

/* create top level(*) debug window. (not really top level for gtk) */
static void create_debug_window(GLOBAL_SKIN_INFOS* gsi);

/* Create global GtkTable */
static GtkWidget* create_debug_table(GLOBAL_SKIN_INFOS* gsi);

/* Create navigation buttons */
static void create_debug_button(GtkWidget* debug_table);

/* Create register area (GtkFrame + entry) */
static void create_register_list(GtkWidget* debug_table, GLOBAL_SKIN_INFOS* gsi); 

/* Get the register values */
static void getreg(GLOBAL_SKIN_INFOS* gsi , int i, char* string);

/* Get the register values decimal) */
gushort getreg_int(GLOBAL_SKIN_INFOS* gsi , int i);

/* Print the register values in terminal */
static void printstate(TilemCalcEmulator* emu);

/* Create the dasm list */
static void create_dasm_list(GtkWidget* debug_dasmscroll, GLOBAL_SKIN_INFOS* gsi); 

/* Fill the GtkList used by dasm_list */
static GtkTreeModel* fill_dasm_list(void);

/* Create the stack list */
static void create_stack_list(GtkWidget* debug_stackscroll, GLOBAL_SKIN_INFOS* gsi); 

/* Fill the GtkList used by dasm_list */
static GtkTreeModel* fill_stk_list(GLOBAL_SKIN_INFOS* gsi);

/* Create the memory list */
static void create_memory_list(GtkWidget* debug_memoryscroll, GLOBAL_SKIN_INFOS* gsi); 

/* Fill the GtkList used by memory list */
static GtkTreeModel* fill_memory_list(void);

