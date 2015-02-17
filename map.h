#ifndef _MAP_H
#define _MAP_H

#if 0
#define MAP_W 256
#define MAP_H 256
#endif

#define MAP_W 128
#define MAP_H 128

#define MIN(x,y) ((x)<(y)?(x):(y))
#define MAX(x,y) ((x)>(y)?(x):(y))
#define CLAMP(x,low,high) MAX(low,MIN(x,high))

extern unsigned char terrain_z[MAP_H][MAP_W];
extern unsigned char fog_layer[MAP_H][MAP_W];

int load_map( const char *filename );

#endif

