#include <string.h>
#include "map.h"
#include "fog.h"

void (*the_fog_function)( Light * ) = calc_fog2;

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

			if ( black <= 2 )
				fog_layer[y][x] = 0;
		}
	}
}

void calc_fog( Light *li )
{
	memset( fog_layer, 1, sizeof( fog_layer ) );
	memset( fog_layer_transposed, 1, sizeof( fog_layer ) );
	the_fog_function( li );

	for( int y=0; y<MAP_H; y++ ) {
		for( int x=0; x<MAP_W; x++ ) {
			fog_layer[y][x] &= fog_layer_transposed[x][y];
		}
	}

	if ( 0 )
		noise_reduction_pass();
}

void draw_fog_debug( void )
{
	if ( the_fog_function == calc_fog4 ) {
		extern void draw_fog_debug4( void );
		draw_fog_debug4();
	}
}

