#include <string.h>
#include "map.h"
#include "fog.h"

void (*the_fog_function)( Light * ) = calc_fog1;

void calc_fog( Light *li )
{
	memset( fog_layer, 1, sizeof( fog_layer ) );
	the_fog_function( li );
}

