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
	char defaultmodel; /* The associated skin */
} MODEL_ASSOCIATION;
/* NB :
 * A model is associated with one rom. 
 * Warning : If you don't choose the right model, the rom will not be emulate (or you're very lucky).
 * Simply erase romconfig.dat to reset the config...
 *
 * */

typedef struct
{
  MODEL_ASSOCIATION ma[60];
} ROMCONFIG_INFOS;

extern ROMCONFIG_INFOS romconfig_infos;

/*************/
/* Functions */
/*************/


/* Load a romconfig.dat file (in fact this function just calls romconfig_read) */
int romconfig_unload(ROMCONFIG_INFOS *infos);

/* Read a romconfig.dat file (but do not search a defaultmodel inside !!!) */
int romconfig_read(ROMCONFIG_INFOS *infos, const char* filename);




