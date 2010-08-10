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

typedef struct
{
  MODEL_ASSOCIATION ma[60];
} ROMCONFIG_INFOS;

extern ROMCONFIG_INFOS romconfig_infos;

/*************/
/* Functions */
/*************/

int romconfig_unload(ROMCONFIG_INFOS *infos);

int romconfig_read(ROMCONFIG_INFOS *infos, const char* filename);
int romconfig_print(ROMCONFIG_INFOS *infos, const char* filename);




