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

typedef struct
{
  SKIN_ASSOCIATION sa[60];
} CONFIG_INFOS;

extern CONFIG_INFOS config_infos;

/*************/
/* Functions */
/*************/

int config_unload(CONFIG_INFOS *infos);

int config_read(CONFIG_INFOS *infos, const char* filename);
int config_print(CONFIG_INFOS *infos, const char* filename);




