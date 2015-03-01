#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>
#include "fog.h"
#include "map.h"

#if 0
typedef int32_t Real;
typedef int64_t DReal;
#else
typedef float Real;
typedef float DReal;
#endif

typedef struct {
	// right edge of the scanline
	// (left edge is always the right edge of the previous sector)
	Real x1, dx1dy;
	Real a, b;
	// {a,b}: vector to the last occluder, projected to YZ plane
	// a=y, b=z
} Sector;

typedef struct {
	unsigned char *z;
	unsigned char *fog;
	int shl_row, shl_col;
	int max_col;
	/* cell index is computed as follows:
		i = ( row << shl_row ) + ( col << shl_col )
	*/
	Real eye[3];
	Real max_dist_sq;
	Real max_dist;
} FogWorld;

static void init_sectors( Sector sectors[], int n_sec, Real ry, Real eye_x )
{
	/* initialize sectors */
	for( int s=0; s<n_sec; s++ ) {
		Sector *sec = sectors + s;
		sec->a = sec->b = 0;
		Real t = ( 2.0f*(s+1) - n_sec + 1 ) / n_sec;
		sec->dx1dy = t;
		sec->x1 = eye_x + ry * t;
	}

	//__asm__ volatile ( "nop; nop; addl $0, %eax; nop; nop; nop;" );
}

void scan_sectors_1( int n_sec,
	int row, int row_step, int row_end, Real ry,
	FogWorld world )
{
	Sector sectors[n_sec];
	init_sectors( sectors, n_sec, ry, world.eye[0] );

	while( row != row_end ) {
		Real ry2 = ry * ry;
		Real prev_x0 = world.eye[0] - ry;
		int s;
		for( s=0; s<n_sec; s++ ) {
			Sector *sec = sectors + s;

			const Real prev_a = sec->a;
			const Real prev_b = sec->b;

			Real min_rz = 1000000;

			int x0 = prev_x0;
			int x1 = (int) sec->x1; // + 1;
			prev_x0 = x1;
			x0 = MAX( x0, 0 );
			x1 = MIN( x1, TERRAIN_W-1 );

			int x;
			Real limit = ry * prev_b;

			for( x=x0; x<=x1; x++ ) {
				const int cell_index =
					( row << world.shl_row )
					+ ( x << world.shl_col );

				Real rx, rz, z;
				z = world.z[cell_index];

				/* {rx,ry,rz} is the vector from light to terrain vertex */
				rx = x - world.eye[0];
				rz = z - world.eye[2];

				if ( limit <= prev_a*rz ) {
					if ( rz < min_rz ) {
						min_rz = rz;
						sec->a = ry;
						sec->b = rz;
					}
					int in_range = ( rx*rx + ry2 < world.max_dist_sq );
					if ( in_range )
						world.fog[cell_index] = 0;
				}
			}
			sec->x1 += sec->dx1dy;
		}
		row += row_step;
		ry += 1;
	}
}

void calc_fog3( Light li[1] )
{
	Real r = li->radius;
	Real rr = r * r;

	FogWorld world_xy =  {
		.z = terrain_z[0],
		.fog = fog_layer[0],
		.shl_row = 7,
		.shl_col = 0,
		.max_col = TERRAIN_W-1,
		.eye = {li->pos[0], li->pos[1], li->pos[2]},
		.max_dist_sq = rr,
		.max_dist = r,
	};
	FogWorld world_yx =  {
		.z = terrain_z[0],
		.fog = fog_layer[0],
		.shl_row = 0,
		.shl_col = 7,
		.max_col = TERRAIN_H-1,
		.eye = {li->pos[1], li->pos[0], li->pos[2]},
		.max_dist_sq = rr,
		.max_dist = r,
	};

	const int n_sectors = 128;
	int cell[3];
	float cell_off[2];
	int i;

	for( i=0; i<2; i++ ) {
		cell[i] = li->pos[i];
		cell_off[i] = li->pos[i] - cell[i];
	}

	if ( !in_map_bounds( cell[0], cell[1] ) )
		return;

	cell[2] = li->pos[2];

	int y0 = cell[1] - r - 1;
	int y1 = cell[1] + r + 1;
	y0 = MAX( y0, -1 );
	y1 = MIN( y1, TERRAIN_H );
	
	int x0 = cell[0] - r - 1;
	int x1 = cell[0] + r + 1;
	x0 = MAX( x0, -1 );
	x1 = MIN( x1, TERRAIN_W );

	assert( y0 <= y1 );
	assert( x0 <= x1 );

	int q = 0xF;

	#if 1
	#define scan_sectors scan_sectors_1
	#endif

	if ( q & 1 )
	scan_sectors( n_sectors,
		cell[1], 1, y1,
		1.0f - cell_off[1],
		world_xy );

	if ( q & 2 )
	scan_sectors( n_sectors,
		cell[1], -1, y0,
		cell_off[1],
		world_xy );
	
	if ( q & 4 )
	scan_sectors( n_sectors,
		cell[0], 1, x1,
		1.0f - cell_off[0],
		world_yx );

	if ( q & 8 )
	scan_sectors( n_sectors,
		cell[0], -1, x0,
		cell_off[0],
		world_yx );
}


