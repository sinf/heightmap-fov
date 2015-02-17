#include <math.h>
#include <string.h>
#include "map.h"
#include "fog.h"

typedef struct {
	int x, y;
	float z;
	float z_inc;
	// sa,sb: direction vectors from light to tile corners
	// sa,sb are sorted counter clockwise
	float sa[2];
	float sb[2];
	// sc: from light to tile centre
	float sc[2];
	float sh[2];
	int fogged;
} FogTile;

static void find_corners( float *a, float *b, const float off[2], int sector_id )
{
	int q = sector_id & 1;
	q ^= sector_id >> 1;

	if ( 1 ) {
		// make sure a,b are counter-clockwise 
		float *a0 = a;
		a = b;
		b = a0;
	}

	a[0] = off[0] + q;
	a[1] = off[1] + !q;

	b[0] = off[0] + !q;
	b[1] = off[1] + q;
}

static void find_shadow( Light *li, FogTile *t, int sector_id )
{
	float nw[2] = {t->x, t->y};
	float a[2], b[2];
	int i;

	find_corners( a, b, nw, sector_id );

	for( i=0; i<2; i++ ) {
		t->sa[i] = a[i] - li->pos[i];
		t->sb[i] = b[i] - li->pos[i];
	}
}

static float cross_z( float a[2], float b[2] )
{
	return a[0]*b[1] - b[0]*a[1];
}

static void test_occlusion( Light *li, FogTile *cur, FogTile *prev )
{
	if ( prev->z > li->pos[2] ) {
		float a = cross_z( prev->sa, cur->sc );
		float b = cross_z( cur->sc, prev->sb );
		if ( a >= 0 && b >= 0 ) {

			for( int i=0; i<2; i++ ) {
				cur->sa[i] = prev->sa[i];
				cur->sb[i] = prev->sb[i];
			}

			cur->fogged = 1;
			cur->z = prev->z;
		}
	}
}

static void clear_sector( Light *li, int sector_id,
	int x0, int x_end, int x_inc,
	int y0, int y_end, int y_inc, float rr )
{
	#define MAX_R 512
	FogTile fog_temp[2][MAX_R] = {0};
	FogTile *prev_row = fog_temp[0];
	FogTile *cur_row = fog_temp[1];
	int y, x, rx;

	if ( x0 == x_end || y0 == y_end )
		return;
	
	for( x=0; x<MAX_R; x++ ) {
		FogTile *t = prev_row + x;
		t->x = x0 + x;
		t->y = y0;
		t->fogged = 0;
		t->z = -10000;
		t->z_inc = 0;
	}

	for( y=y0; y!=y_end; y+=y_inc ) {

		for( rx=0, x=x0; x!=x_end; rx++, x+=x_inc ) {

			float cx = x + 0.5f;
			float cy = y + 0.5f;
			float dx = cx - li->pos[0];
			float dy = cy - li->pos[1];
			float d = dx*dx + dy*dy;

			if ( d > rr )
				break;

			FogTile *cur = cur_row + rx;

			cur->x = x;
			cur->y = y;
			cur->fogged = 0;
			cur->z = terrain_z[y][x];
			cur->sc[0] = dx;
			cur->sc[1] = dy;

			find_shadow( li, cur, sector_id );

			if ( y != y0 )
				test_occlusion( li, cur, prev_row+rx );

			if ( rx > 0 ) {
				if ( y != y0 )
					test_occlusion( li, cur, prev_row+rx-1 );
				if ( 1 )
					test_occlusion( li, cur, cur_row+rx-1 );
			}

			fog_layer[y][x] = cur->fogged;
		}

		FogTile *temp = cur_row;
		cur_row = prev_row;
		prev_row = temp;
	}
}

static void get_limits_pos( int xi, float x, float r, int out[3], int max_x )
{
	int x_end = x + r;
	x_end += 1;
	x_end = MIN( max_x, x_end );
	out[0] = CLAMP( xi, -1, x_end );
	out[1] = x_end;
	out[2] = 1;
}
static void get_limits_neg( int xi, float x, float r, int out[3], int max_x )
{
	int x_end = x - r;
	xi -= 1; // prevent processing the same column twice
	x_end -= 1;
	x_end = MAX( -1, x_end );
	out[0] = CLAMP( xi, x_end, max_x-1 );
	out[1] = x_end;
	out[2] = -1;
}

void calc_fog( Light *li )
{
	memset( fog_layer, 1, sizeof( fog_layer ) );

	int lx[2][3], ly[2][3];
	float r = li->radius;
	float rr = r * r;
	float xf = li->pos[0];
	float yf = li->pos[1];
	int xi = xf;
	int yi = yf;

	get_limits_neg( xi, xf, r, lx[0], MAP_W );
	get_limits_neg( yi, yf, r, ly[0], MAP_W );
	get_limits_pos( xi, xf, r, lx[1], MAP_W );
	get_limits_pos( yi, yf, r, ly[1], MAP_W );

	for( int sector=0; sector<4; sector++ ) {
		int *x = lx[sector & 1];
		int *y = ly[sector >> 1];
		clear_sector( li, sector, x[0], x[1], x[2], y[0], y[1], y[2], rr );
	}
}

