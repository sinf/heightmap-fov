#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>
#include "map.h"
#include "fog.h"

/*
cur_occl = init occlusion shape to not occlude anything
next_occl = uninitialized
for each row (from light origin to max.distance):
	for each column:
		tile = map[row][column]
		if tile is not occluded:
			next_occl.add_occluder(tile)
			clear fog at tile
	swap cur_occl and next_occl

occlusion data:
 - is a vector representation of what can be seen
 - needs a function that gives an occlusion plane for each (x,y) position
 - occlusion data = a set of planes & their start/end coordinates
*/

typedef struct FogMap {
	unsigned char *f, *z;
	int pitch;
} FogMap;

static int map_addr( FogMap *m, int x, int y )
{
	return y * m->pitch + x;
}

#define MAX_SHADOWS 1024
typedef struct {
	int n;
	float x[MAX_SHADOWS][2]; // start,end x
	float y[MAX_SHADOWS];
	float z[MAX_SHADOWS];
} Vision;

static void clear( Vision *v )
{
	v->n = 0;
}
static void swap( Vision **old, Vision **new )
{
	Vision *temp = *old;
	*old = *new;
	*new = temp;
	clear( temp );
}

static float cross_z( float ax, float ay, float bx, float by )
{
	return ax*by - bx*ay;
}

static void travel_sector( Light *li, FogMap *map )
{
	int y0 = li->pos[1] + 1;
	int y1 = li->pos[1] + li->radius;
	int y;
	Vision all_vis[2];
	Vision *old_vis = all_vis;
	Vision *new_vis = all_vis+1;

	y0 = MAX( y0, 0 );
	y1 = MIN( y1, MAP_H-1 );

	clear( old_vis );
	clear( new_vis );

	for( y=y0; y<=y1; y++ ) {

		int r = y - li->pos[1];
		int x0 = li->pos[0] - r;
		int x1 = li->pos[0] + r;
		int x;
		int s = 0;

		x0 = MAX( x0, 0 );
		x1 = MIN( x1, MAP_W-1 );

		for( x=x0; x<=x1; x++ ) {
			int z = map->z[map_addr(map,x,y)];

			float p[3] =  {
				x + 0.5f - li->pos[0],
				y + 0.5f - li->pos[1],
				z - li->pos[2]
			};

			if ( s < old_vis->n ) {
				s += cross_z(
					old_vis->x[s][1], old_vis->y[s],
					p[0], p[1] ) > 0;
			}

			if ( s == old_vis->n || cross_z(
			old_vis->y[s], old_vis->z[s], p[1], p[2] ) > 0 ) {
				map->f[map_addr(map,x,y)] = 0;
			}
		}

		swap( &old_vis, &new_vis );
	}
}

void calc_fog4( Light *li )
{
	FogMap m = {.f=fog_layer[0], .z=terrain_z[0], .pitch=MAP_W};
	travel_sector( li, &m );
}

#include <GL/gl.h>
void draw_fog_debug4( void )
{
}

