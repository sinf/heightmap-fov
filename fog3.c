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
	// scanline limits
	Real x0, x1;
	Real dx0dy, dx1dy;
	// {a,b}: vector to the last occluder, projected to YZ plane
	// a=y, b=z
	Real a, b;
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
	Real prev_dxdy = -1;
	Real prev_x = eye_x - ry;
	for( int s=0; s<n_sec; s++ ) {
		Sector *sec = sectors + s;

		sec->a = sec->b = 0;
		//sec->alive = 1;

		Real t = ( 2.0f*(s+1) - n_sec + 1 ) / n_sec;

		sec->dx0dy = prev_dxdy;
		sec->dx1dy = prev_dxdy = t;

		sec->x0 = prev_x;
		sec->x1 = eye_x + ry * t;

		assert( sec->x1 >= sec->x0 );

		prev_x = sec->x1;
	}
}

void scan_sectors_1( int n_sec,
	int row, int row_step, int row_end, Real ry,
	FogWorld world )
{
	Sector sectors[n_sec];
	init_sectors( sectors, n_sec, ry, world.eye[0] );

	while( row != row_end ) {
		Real ry2 = ry * ry;
		int s;
		for( s=0; s<n_sec; s++ ) {
			Sector *sec = sectors + s;

			const Real prev_a = sec->a;
			const Real prev_b = sec->b;

			Real min_rz = 1000000;

			int x0 = sec->x0;
			int x1 = (int) sec->x1; // + 1;
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

			sec->x0 += sec->dx0dy;
			sec->x1 += sec->dx1dy;
		}
		row += row_step;
		ry += 1;
	}
}

void scan_sectors( int n_sec,
	int row, int row_step, int row_end, Real ry,
	FogWorld world )
{
	Sector sectors[n_sec];
	init_sectors( sectors, n_sec, ry, world.eye[0] );

	// ry: y (=row) coordinate relative to eye
	// rx,ry,rz: vector to terrain vertex relative to eye position

	while( row != row_end ) {

		int col0 = sectors[0].x0;
		int col1 = sectors[n_sec-1].x1;
		col0 = MAX( col0, 0 );
		col1 = MIN( col1, world.max_col );

		int cell_index = ( row << world.shl_row ) + ( col0 << world.shl_col );

		int col = col0;

		const Real ry2 = ry * ry;
		Real rx = col0 - world.eye[0];
		Real rx2 = rx * rx;

		Real radius_sq = rx2 + ry2;
		Sector *sec = sectors;

		if ( col1 < col0 )
			goto next_row;

		do {
			assert( sec < sectors + n_sec );

			const Real
				prev_a = sec->a,
				prev_b = sec->b;

			const Real limit = ry * prev_b;

			int sec_end = sec->x1;
			sec_end = MIN( col1, sec_end );

			Real min_rz = 10000000;

			for( ;; ) {
				Real rz = world.z[cell_index] - world.eye[2];

				if ( limit <= prev_a*rz ) {
					if ( rz < min_rz ) {
						min_rz = rz;
						sec->a = ry;
						sec->b = rz;
					}
					if ( radius_sq < world.max_dist_sq )
						world.fog[cell_index] = 0;
				}

				if ( col < sec_end ) {
					col += 1;
					cell_index += 1 << world.shl_col;
					radius_sq += rx + rx + 1;
					rx += 1;
					continue;
				}

				break;
			}
			sec += 1;
		} while( col < col1 );

		next_row:;

		for( int s=0; s<n_sec; s++ ) {
			Sector *sec = sectors + s;
			sec->x0 += sec->dx0dy;
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
	int32_t cell[3];
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


