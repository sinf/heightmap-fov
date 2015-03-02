#ifndef _MAP_H
#define _MAP_H

#if 0
#define MAP_W 256
#define MAP_H 256
#endif

#define MAP_W 128
#define MAP_H 128

#define TERRAIN_W MAP_W
#define TERRAIN_H MAP_H

#define MIN(x,y) ((x)<(y)?(x):(y))
#define MAX(x,y) ((x)>(y)?(x):(y))
#define CLAMP(x,low,high) MAX(low,MIN(x,high))

extern unsigned char terrain_z[MAP_H][MAP_W];
extern unsigned char terrain_z_transposed[MAP_W][MAP_H];

extern unsigned char fog_layer[MAP_H][MAP_W];
extern unsigned char fog_layer_transposed[MAP_W][MAP_W];

int load_map( const char *filename );

#define get_height(x,y) terrain_z[(y)][(x)]
#define clear_fog(x,y) fog_layer[(y)][(x)]=0
#define in_map_bounds(x,y) ((x) > -1 && (y) > -1 && (x) < MAP_W && (y) < MAP_H)

#endif

