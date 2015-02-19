#include <math.h>
#include "map.h"
#include "fog.h"

typedef struct {
	int cell_start[2];
	int cell_end[2];
	int cell_inc[2];
	float cell_off[2];
} RayConfig;

void raycast( const RayConfig rc[1], float org[3], float dir[2], float ray_len_max )
{
	const int *cell_end = rc->cell_end;
	const int *cell_inc = rc->cell_inc;
	int cell[2], i;
	float abs_dir[2];
	float t[2], t_inc[2];

	float lz = -10000; // lowest Z coordinate that can be seen
	float lz_slope = 0;

	for( i=0; i<2; i++ ) {
		float ad = fabs( dir[i] );
		float iad = 1.0f / ad;
		abs_dir[i] = ad;
		cell[i] = rc->cell_start[i];
		t_inc[i] = iad;
		t[i] = rc->cell_off[i] * iad;
	}

	//fog_layer[cell[1]][cell[0]] = 0;

	for( ;; ) {
		float ray_len;

		if ( t[0] < t[1] ) {
			cell[0] += cell_inc[0];
			if ( cell[0] == cell_end[0] )
				break;
			ray_len = t[0];
			t[0] += t_inc[0];
			lz += lz_slope * abs_dir[0];
		} else {
			cell[1] += cell_inc[1];
			if ( cell[1] == cell_end[1] )
				break;
			ray_len = t[1];
			t[1] += t_inc[1];
			lz += lz_slope * abs_dir[1];
		}

		unsigned char *fog = &fog_layer[cell[1]][cell[0]];
		float z = terrain_z[cell[1]][cell[0]];

		if ( z > lz ) {
			lz = z;
			lz_slope = ( z - org[2] ) / ray_len;
			*fog = 0;
		}

		if ( ray_len > ray_len_max )
			break;
	}
}

static RayConfig get_ray_config( Light *li, int q )
{
	RayConfig rc;
	float *org = li->pos;
	int i;

	/* quadrant indices represented by q:
	  ^
	1 | 0
	--+-->
	2 | 3
	*/
	const signed char dir_is_neg[4][2] = {
		{0,0},
		{1,0},
		{1,1},
		{0,1}
	};
	int dim[2] = {MAP_W, MAP_H};

	for( i=0; i<2; i++ ) {
		int c = org[i];
		rc.cell_start[i] = c;
		if ( dir_is_neg[q][i] ) {
			rc.cell_end[i] = -1;
			rc.cell_inc[i] = -1;
			//rc.cell_off[i] = c - org[i]; // negative offset
			rc.cell_off[i] = org[i] - c; // positive offset
		} else {
			rc.cell_end[i] = dim[i];
			rc.cell_inc[i] = 1;
			rc.cell_off[i] = c + 1 - org[i];
		}
	}

	return rc;
}

static int in_map_bounds( int x, int y )
{
	return x > -1 && y > -1 && x < MAP_W && y < MAP_H;
}

static void shoot_rays_quadrant( Light *li, int q, int num_rays )
{
	RayConfig rc = get_ray_config( li, q );
	float a_inc = M_PI_2 / num_rays;
	float a = a_inc * 0.5f + q * M_PI;
	int i;
	float dir[2];
	float inc[2];

	if ( !in_map_bounds( rc.cell_start[0], rc.cell_start[1] ) )
		return;
	
	dir[0] = cos( a );
	dir[1] = sin( a );

	inc[0] = cos( a_inc );
	inc[1] = sin( a_inc );

	for( i=0; i<num_rays; i++ ) {
		raycast( &rc, li->pos, dir, li->radius );
		float old_c = dir[0];
		float old_s = dir[1];
		dir[0] = inc[0]*old_c - inc[1]*old_s;
		dir[1] = inc[1]*old_c + inc[0]*old_s;
	}
}

void calc_fog2( Light *li )
{
	int q, n=10000;
	for( q=0; q<4; q++ )
		shoot_rays_quadrant( li, q, n );
}

