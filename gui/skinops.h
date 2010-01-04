/* 
From Roms(?) :
   Most of these definitions and code comes from the JB's SkinEdit
   which is based on TiEmu skin code. TiEmu skin code is also based on
   VTi's skin code.
   
contra-sh :
   This file is a perfect copy of the tiemu skinops.h file ...
   Thank's to rom's and JB for this wonderful work.

*/



#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdint.h>
#include <gdk-pixbuf/gdk-pixbuf.h>


/***************/
/* Definitions */
/***************/

#define LCD_COLORTYPE_LOW    0
#define LCD_COLORTYPE_HIGH   1
#define LCD_COLORTYPE_CUSTOM 2

#define LCD_HI_WHITE 0xb0ccae
#define LCD_HI_BLACK 0x8a6f53

#define LCD_LOW_WHITE 0xcfe0cc
#define LCD_LOW_BLACK 0x222e31

#define MAX_COLORS (256 - 16)		// we need to keep 16 colors for grayscales
#define SKIN_KEYS  80

#define SKIN_TI73   "TI-73"
#define SKIN_TI82   "TI-82"
#define SKIN_TI83   "TI-83"
#define SKIN_TI83P  "TI-83+"
#define SKIN_TI85   "TI-85"
#define SKIN_TI86   "TI-86"
#define SKIN_TI89   "TI-89"
#define SKIN_TI92   "TI-92"
#define SKIN_TI92P  "TI-92+"
#define SKIN_V200   "V200PLT"
#define SKIN_TI89T  "TI-89TM"

#define SKIN_TYPE_TIEMU   10
#define SKIN_TYPE_VTI     2
#define SKIN_TYPE_OLD_VTI 1
#define SKIN_TYPE_NEW     0

#define ENDIANNESS_FLAG 0xfeedbabe
#define TIEMU_SKIN_ID   "TiEmu v2.00"

/*********/
/* Types */
/*********/


typedef struct
{
  uint32_t left;
  uint32_t top;
  uint32_t right;
  uint32_t bottom;
} RECT;


typedef struct
{
  int type;

  GdkPixbuf *image;

  int width;
  int height;

  GdkPixbuf *raw;	// raw jpeg image
  double	s;		// scaling factor

  char calc[9];
  uint32_t colortype;

  uint32_t lcd_black;
  uint32_t lcd_white;

  char *name;
  char *author;

  RECT lcd_pos;
  RECT keys_pos[SKIN_KEYS];

  long	jpeg_offset;

} SKIN_INFOS;

extern SKIN_INFOS skin_infos;

/*************/
/* Functions */
/*************/

int skin_load(SKIN_INFOS *infos, const char *filename);
int skin_unload(SKIN_INFOS *infos);

int skin_read_header(SKIN_INFOS *infos, const char* filename);
int skin_read_image (SKIN_INFOS *infos, const char* filename);




