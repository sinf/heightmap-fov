#include <stdint.h>
#include <math.h>
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
	Real a, b;
	float x0, x1;
	float dx0dy, dx1dy;
	int alive;
} Sector;

static void scan_sectors( Light li[1], Sector sec[], int n_sec, int step_y, int32_t src_cell[3] )
{
	Real radius2 = li->radius * li->radius;
	int y = li->pos[1];
	int end_y = y + step_y * (int)( li->radius + 0.5f );
	end_y = CLAMP( end_y, -1, TERRAIN_H );

	Real ry = 1.0f - ( li->pos[1] - (int) li->pos[1] );

	for( y=li->pos[1]; y != end_y; y+=step_y, ry+=1 )
	{
		Real ry2 = ry * ry;
		int s;
		for( s=0; s<n_sec; s++ ) {
			if ( !sec[s].alive )
				continue;

			int x0 = sec[s].x0;
			int x1 = (int) sec[s].x1; // + 1;
			int x;
			int any_in_range = 0;

			Real min_rz = 1000000;

			x0 = MAX( x0, 0 );
			x1 = MIN( x1, TERRAIN_W-1 );

			for( x=x0; x<=x1; x++ ) {
				Real a, b, rx, rz;

				rx = x - li->pos[0];
				rz = get_height( x, y ) - li->pos[2];

				int in_range = ( rx*rx + ry2 < radius2 );
				any_in_range |= in_range;

				a = ry;
				b = rz;

				if ( a*sec[s].b <= sec[s].a*b ) {
					if ( rz < min_rz ) {
						min_rz = rz;
						sec[s].a = a;
						sec[s].b = b;
					}
					if ( in_range )
						clear_fog( x, y );
				}
			}
			sec[s].alive = any_in_range;

			sec[s].x0 += sec[s].dx0dy;
			sec[s].x1 += sec[s].dx1dy;
		}
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

	float x0, x1;
	float dy;
	Sector sec[n_sectors];

	dy = 1.0f - cell_off[1];
	x0 = li->pos[0] - dy;
	x1 = li->pos[0] + dy;

	float x_step = ( x1 - x0 ) / n_sectors;

	for( i=0; i<n_sectors; i++ ) {
		sec[i].x0 = x0 + i * x_step;
		sec[i].x1 = sec[i].x0 + x_step;
		sec[i].dx1dy = ( sec[i].x1 - li->pos[0] ) / dy;
		sec[i].a = 0;
		sec[i].b = 0;
		sec[i].alive = 1;
	}

	sec[0].dx0dy = -1;
	for( i=1; i<n_sectors; i++ ) {
		sec[i].dx0dy = sec[i-1].dx1dy;
	}

	scan_sectors( li, sec, n_sectors, 1, cell );
}

