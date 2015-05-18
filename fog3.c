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

#define MAX_SECTORS 256

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
	int pitch; /* how many bytes to skip to get from row N to row (N+1) */
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
		sec->a = 0;
		sec->b = 0;
		Real t = ( 2.0f*(s+1) - n_sec + 1 ) / n_sec;
		sec->dx1dy = t;
		sec->x1 = eye_x + ry * t;
	}
	#if 0
	__asm__ volatile ( "nop; nop; addl $0, %eax; nop; nop; nop;" );
	#endif
}

void scan_sectors( int n_sec,
	int row, int row_step, int row_end, Real ry,
	FogWorld world )
{
	Sector sectors[MAX_SECTORS];
	char dead_sectors[MAX_SECTORS] = {0};

	row += row_step;
	ry += 1;
	init_sectors( sectors, n_sec, ry, world.eye[0] );

	int pitch = world.pitch;
	unsigned char *row_data_z = world.z + row * pitch;
	unsigned char *row_data_f = world.fog + row * pitch;
	pitch *= row_step;

	do {
		Real ry2 = ry * ry;
		Real prev_x0 = world.eye[0] - ry;

		for( int s=0; s<n_sec; s++ ) {
			if ( dead_sectors[s] )
				continue;

			Sector *sec = sectors + s;
			Real prev_a = sec->a;
			Real prev_b = sec->b;

			int x0 = prev_x0;
			int x1 = (int) sec->x1;
			prev_x0 = sec->x1;

			x0 = MAX( x0, 0 );
			x1 = MIN( x1, world.max_col );

			Real limit = ry * prev_b;

			Real next_rz0 = -10000000;
			Real next_rz = next_rz0;
			int x;

			Real tx;
			tx = 1 + ( x0 + x1 >> 1 );
			tx = tx - world.eye[0];
			if ( tx*tx + ry2 > world.max_dist_sq ) {
				dead_sectors[s] = 1;
				continue;
			}

			for( x=x0; x<=x1; x++ ) {
				Real rz, z;
				z = row_data_z[x];

				/* {rx,ry,rz} is the vector from light to terrain vertex */
				rz = z - world.eye[2];

				if ( limit <= prev_a*rz ) {
					row_data_f[x] = 0;
					if ( rz > next_rz )
						next_rz = rz;
				}
			}

			if ( next_rz != next_rz0 ) {
				sec->a = ry;
				sec->b = next_rz;
			}

			sec->x1 += sec->dx1dy;
		}

		row_data_z += pitch;
		row_data_f += pitch;
		row += row_step;
		ry += 1;
	} while( row != row_end );
}

void calc_fog3( Light li[1] )
{
	Real r = li->radius;
	Real rr = r * r;

	FogWorld world_xy =  {
		.z = terrain_z[0],
		.fog = fog_layer[0],
		.pitch = TERRAIN_W,
		.max_col = TERRAIN_W-1,
		.eye = {li->pos[0], li->pos[1], li->pos[2]},
		.max_dist_sq = rr,
		.max_dist = r,
	};
	FogWorld world_yx =  {
		.z = terrain_z_transposed[0],
		.fog = fog_layer_transposed[0],
		.pitch = TERRAIN_H,
		.max_col = TERRAIN_H-1,
		.eye = {li->pos[1], li->pos[0], li->pos[2]},
		.max_dist_sq = rr,
		.max_dist = r,
	};

	int n_sectors = 256;
	int cell[3];
	float cell_off[2];
	int i;

	n_sectors = MIN( n_sectors, MAX_SECTORS );

	for( i=0; i<2; i++ ) {
		cell[i] = li->pos[i];
		cell_off[i] = li->pos[i] - cell[i];
	}

	if ( !in_map_bounds( cell[0]+1, cell[1]+1 ) )
		return;
	if ( !in_map_bounds( cell[0]-1, cell[1]-1 ) )
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


