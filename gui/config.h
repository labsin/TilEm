/* 
contra-sh :
	This file is based on skinops.h which is based on Julien Blache and Romain Lievins 's work.
*/

#include <stdint.h>

/***************/
/* Definitions */
/***************/

#define ENDIANNESS_FLAG 0xfeedbabe
#define TILEM_CONFIG_ID   "TilEm v2.00"

/*********/
/* Types */
/*********/
typedef struct
{
	char* romname;
	char* defaultskinname; /* The associated skin */
} SKIN_ASSOCIATION;
/* NB :
 * A skin is associated with one rom (not one model!).By example a ti83 rom could be associated to ti82stats.skn or ti83.skn depending its rom version... 
 * You can define different skin for one kind of model. Wonderful isn't it?
 *
 * */

typedef struct
{
  SKIN_ASSOCIATION sa[60];
} CONFIG_INFOS;

extern CONFIG_INFOS config_infos;

/*************/
/* Functions */
/*************/

/* Load a config.dat file (in fact this function just calls config_read) */
int config_unload(CONFIG_INFOS *infos);

/* Read a config.dat file (but do not search a defaultskinname inside !!!) */
int config_read(CONFIG_INFOS *infos, const char* filename);




