#include <stdint.h>
#include <math.h>
#include <assert.h>
#include "fog.h"
#include "map.h"

#if 0
typedef int32_t Real;
typedef int64_t DReal;
#else
typedef double Real;
typedef double DReal;
#endif

typedef struct {
	// {a,b}: vector to the last occluder, projected to YZ plane
	// a=y, b=z
	Real a, b;
	// scanline limits
	float x0, x1;
	float dx0dy, dx1dy;
	int alive;
} Sector;

static void scan_sectors(
	Sector sectors[], int n_sec,
	int y, int step_y, int end_y,
	Real ry, Real light_x, Real light_z, Real rr,
	unsigned char *z_map, unsigned char *fog_map,
	int map_s, int map_t )
{
	while( y != end_y ) {
		Real ry2 = ry * ry;
		int s;
		for( s=0; s<n_sec; s++ ) {
			Sector *sec = sectors + s;

			if ( !sec->alive )
				continue;

			const Real prev_a = sec->a;
			const Real prev_b = sec->b;

			Real min_rz = 1000000;
			int x0 = sec->x0;
			int x1 = (int) sec->x1; // + 1;
			x0 = MAX( x0, 0 );
			x1 = MIN( x1, TERRAIN_W-1 );

			int any_in_range = 0;
			int x;

			any_in_range |= x1 <= x0;

			for( x=x0; x<=x1; x++ ) {
				int cell_index = ( y << map_s ) + ( x << map_t );
				Real a, b, rx, rz, z;

				z = z_map[cell_index];
				//z = get_height( x, y );

				/* {rx,ry,rz} is the vector from light to terrain vertex */
				rx = x - light_x;
				rz = z - light_z;

				int in_range = ( rx*rx + ry2 < rr );
				any_in_range |= in_range;

				/* {a,b} is {rx,ry,rz} projected to the YZ plane */
				a = ry;
				b = rz;

				if ( a*prev_b <= prev_a*b ) {
					if ( rz < min_rz ) {
						min_rz = rz;
						sec->a = a;
						sec->b = b;
					}
					if ( in_range ) {
						fog_map[cell_index] = 0;
						//clear_fog( x, y );
					}
				}
			}

			sec->alive = any_in_range;

			sec->x0 += sec->dx0dy;
			sec->x1 += sec->dx1dy;
		}
		y += step_y;
		ry += 1;
	}
}

void make_sectors( Sector sectors[], int count, float a, float a_inc, float off_y, float tx, int swap )
{
	int i;
	float u, v;
	if ( swap ) {
		u = sin( a );
		v = cos( a );
	} else {
		u = cos( a );
		v = sin( a );
	}
	float prev_dx1dy = u;
	float prev_x0 = v * off_y + tx;
	for( i=0; i<count; i++ ) {
		Sector *sec = sectors + i;
		a += a_inc;
		u = cos( a );
		v = sin( a );
		if ( swap ) {
			float tmp = v;
			v = u;
			u = tmp;
		}
		sec->a = 1;
		sec->b = -10000000;
		sec->dx0dy = prev_dx1dy;
		sec->dx1dy = prev_dx1dy = u;
		sec->x0 = prev_x0;
		sec->x1 = prev_x0 = v * off_y + tx;
		sec->alive = 1;
	}
}

void calc_fog3( Light li[1] )
{
	const int n_sectors = 256;
	int32_t cell[3];
	float cell_off[2];
	int i;

	for( i=0; i<2; i++ ) {
		cell[i] = li->pos[i];
		cell_off[i] = li->pos[i] - cell[i];
	}

	cell[2] = li->pos[2];

	Sector sec[4][n_sectors];
	float a_inc = M_PI_2 / n_sectors;

	make_sectors( sec[0], n_sectors,
		M_PI - M_PI_4, -a_inc,
		1.0f - cell_off[1], li->pos[0], 0 );
	make_sectors( sec[1], n_sectors,
		M_PI + M_PI_4, a_inc,
		cell_off[1], li->pos[0], 0 );
	make_sectors( sec[2], n_sectors,
		-M_PI_4, a_inc,
		1.0f - cell_off[0], li->pos[1], 1 );
	make_sectors( sec[3], n_sectors,
		M_PI + M_PI_4, -a_inc,
		cell_off[0], li->pos[1], 1 );

	unsigned char *z_map = terrain_z[0];
	unsigned char *fog_map = fog_layer[0];

	Real rr = li->radius * li->radius;
	int r = li->radius;
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

	int log2_pitch = 7;
	int q = 0xF;

	if ( q & 1 )
	scan_sectors( sec[0], n_sectors,
		cell[1], 1, y1,
		1.0f - cell_off[1], li->pos[0], li->pos[2], rr,
		z_map, fog_map, log2_pitch, 0 );

	if ( q & 2 )
	scan_sectors( sec[1], n_sectors,
		cell[1], -1, y0,
		cell_off[1], li->pos[0], li->pos[2], rr,
		z_map, fog_map, log2_pitch, 0 );
	
	if ( q & 4 )
	scan_sectors( sec[2], n_sectors,
		cell[0], 1, x1,
		1.0f - cell_off[0], li->pos[1], li->pos[2], rr,
		z_map, fog_map, 0, log2_pitch );

	if ( q & 8 )
	scan_sectors( sec[3], n_sectors,
		cell[0], -1, x0,
		cell_off[0], li->pos[1], li->pos[2], rr,
		z_map, fog_map, 0, log2_pitch );
}


