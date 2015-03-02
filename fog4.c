#include <stdlib.h>
#include <stdint.h>
#include "map.h"
#include "fog.h"

typedef struct {
	unsigned char *z;
	unsigned char *f;
	int pitch;
	int min_x, max_x;
	int eye_x;
	int eye_y;
	int eye_z;
} FogWorld;

static void cast_vision( FogWorld world, int row_end )
{
	unsigned char *z_map = world.z;
	unsigned char *f_map = world.f;
	int row;
	int x0, x1;

	x0 = x1 = world.eye_x;
	z_map += world.pitch * world.eye_y;
	f_map += world.pitch * world.eye_y;

	for( row=0; row<row_end; row++ )
	{
		int x;
		for( x=x0; x<=x1; x++ ) {
			f_map[x] = 0;
		}

		x0 -= 1;
		x1 += 1;
		x0 = MAX( x0, world.min_x );
		x1 = MIN( x1, world.max_x );
		z_map += world.pitch;
		f_map += world.pitch;
	}
}

void calc_fog4( Light *li )
{
	FogWorld fw = {
		.z = terrain_z[0],
		.f = fog_layer[0],
		.pitch = TERRAIN_W,
		.min_x = 0,
		.max_x = TERRAIN_W-1,
		.eye_x = li->pos[0],
		.eye_y = li->pos[1],
		.eye_z = li->pos[2],
	};

	int y0 = li->pos[1];
	int y1 = li->pos[1] + li->radius;
	y1 = MIN( TERRAIN_H-1, y1 );

	if ( !in_map_bounds( (int) li->pos[0], (int) li->pos[1] ) )
		return;

	cast_vision( fw, y1 - y0 + 1 );
}

