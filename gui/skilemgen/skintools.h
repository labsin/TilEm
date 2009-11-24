#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


#include <gtk/gtk.h>
#include <glib/gstdio.h>



#define SKIN_KEYS 80

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

#define CALC_TI73	"TI-73"
#define CALC_TI82	"TI-82"
#define CALC_TI83	"TI-83"
#define CALC_TI83P	"TI-83+"
#define CALC_TI84P	"TI-84+"
#define CALC_TI85	"TI-85"
#define CALC_TI86	"TI-86"
#define CALC_TI89	"TI-89"
#define CALC_TI89T	"TI-89TM"
#define CALC_TI92	"TI-92"
#define CALC_TI92P	"TI-92+"
#define CALC_V200	"V200PLT"


#define SKIN_TYPE_TIEMU   10
#define SKIN_TYPE_TILEM 20
#define ENDIANNESS_FLAG 0xfeedbabe


typedef struct
{
	uint32_t left;
	uint32_t top;
	uint32_t right;
	uint32_t bottom;
} RECT;

/* the original skinInfos struct from skinedit
/*struct skinInfos
{
  char *jpeg_path;
  char *skin_path;
  int type;
  int changed;

  GdkPixbuf *img_orig;

  unsigned int width;
  unsigned int height;
  char calc[9];
  unsigned int keymap;
  uint32_t colortype;
  uint32_t lcd_black;
  uint32_t lcd_white;
  char *name;
  char *author;
  struct RECT lcd_pos;
  struct RECT keys_pos[SKIN_KEYS];
};*/

typedef struct
{
	  int type;

	  //GdkPixbuf *image;

	  unsigned int width;
	  unsigned int height;

	  //GdkPixbuf *raw;	// raw jpeg image
	  double	s;		// scaling factor

	  char calc[9];
	   uint32_t colortype;

	   uint32_t lcd_black;
	   uint32_t lcd_white;

	  char *name;
	  char *author;

	  RECT lcd_pos;
	  RECT keys_pos[4];

	  long jpeg_offset;

} SKIN_INFOS;



SKIN_INFOS set_skins_info();

SKIN_INFOS skins_infos;

RECT keys_pos[]=
	{
	{1,2,3,4},
	{2,2,4,4},
	{3,3,3,3},
	{1,1,1,1}
	};

int read_skin_format(char * filename) ;
int skin_get_type(SKIN_INFOS *si, const char *filename);

int skin_read_header_tilem(SKIN_INFOS *si, const char *filename);
unsigned char * read_image(FILE *fp, unsigned int *length);
	
