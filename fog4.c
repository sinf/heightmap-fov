#include <stdlib.h>
#include <stdint.h>
#include "map.h"
#include "fog.h"

typedef struct {
	unsigned char *z;
	unsigned char *f;
	int pitch;
	int min_x, max_x;
	float eye_x;
	float eye_y;
	float eye_z;
} FogWorld;

#define MAX_R 512

static void cast_vision( FogWorld world, int row_end )
{
	unsigned char *z_map = world.z;
	unsigned char *f_map = world.f;
	int row;
	int x0, x1;

	x0 = x1 = world.eye_x;
	z_map += world.pitch * (int) world.eye_y;
	f_map += world.pitch * (int) world.eye_y;

	// yz vector to occluder
	float
		occ_yz[2][MAX_R][2],
		(*cur_occ)[2] = &occ_yz[0][0],
		(*prev_occ)[2] = &occ_yz[1][0];

	for( int i=0; i<MAX_R; i++ ) {
		prev_occ[i][0] = 1;
		prev_occ[i][1] = -1000000;
	}

	row_end = MIN( row_end, MAX_R-1 );

	for( row=0; row<row_end; row++ )
	{
		int x;
		int src_x = 0;
		int row_len = x1 - x0 + 1;

		for( x=0; x<row_len; x++ ) {
			int world_x = x0 + x;
			float z = z_map[world_x] - world.eye_z;

			src_x = x * ( row_len - 2 ) / (float) row_len + 0.5f;
			if ( src_x < 0 || src_x > row_len )
				continue;

			/* (current y, current z) x (occluder y, occluder z) */
			if ( row*prev_occ[src_x][1] - prev_occ[src_x][0]*z < 0 ) {
				f_map[world_x] = 0;
				cur_occ[x][0] = row;
				cur_occ[x][1] = z;
			} else {
				cur_occ[x][0] = prev_occ[src_x][0];
				cur_occ[x][1] = prev_occ[src_x][1];
			}

			/*
			row_len / ( row_len + 2 ) = src_x / x
			src_x = x * row_len / ( row_len + 2 )
			*/
			#if 0
			tx += dtx;
			if ( tx >= 1.0f ) {
				tx = 0;
				src_x += 1;
			}

			if ( src_x >= row_len )
				break;
			#endif
		}

		x0 -= 1;
		x1 += 1;
		x0 = MAX( x0, world.min_x );
		x1 = MIN( x1, world.max_x );
		z_map += world.pitch;
		f_map += world.pitch;

		void *temp_occ = cur_occ;
		cur_occ = prev_occ;
		prev_occ = temp_occ;
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

	int cell_x = li->pos[0];
	int cell_y = li->pos[1];

	if ( !in_map_bounds( cell_x, cell_y ) )
		return;

	cast_vision( fw, y1 - y0 + 1 ); //, li->pos[0] - cell_x );
}

