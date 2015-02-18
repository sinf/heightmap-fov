#include <string.h>
#include "map.h"
#include "fog.h"

void (*the_fog_function)( Light * ) = calc_fog1;

static void noise_reduction_pass( void )
{
	int y, x;
	for( y=1; y<MAP_H-1; y++ ) {
		for( x=1; x<MAP_W-1; x++ ) {
			int f[9], *fp=f;
			int black = 0;
			for( int dx=-1; dx<=1; dx++ ) {
				for( int dy=-1; dy<=1; dy++ ) {
					int fog = fog_layer[y+dy][x+dx];
					*fp++ = fog;
					black += fog != 0;
				}
			}

			if ( black < 5 )
				fog_layer[y][x] = 0;
		}
	}
}

void calc_fog( Light *li )
{
	memset( fog_layer, 1, sizeof( fog_layer ) );
	the_fog_function( li );

	if ( 1 ) {
		for( int i=0; i<1; i++ )
			noise_reduction_pass();
	}
}

