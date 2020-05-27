#ifndef _MAIN_H
#define _MAIN_H

#define MIN(x,y) ((x)<(y)?(x):(y))
#define MAX(x,y) ((x)>(y)?(x):(y))
#define CLAMP(x,low,high) MAX(low,MIN(x,high))

#define MAP_W 128
#define MAP_H 128

extern unsigned char terrain_z[MAP_H][MAP_W];
extern unsigned char terrain_z_transposed[MAP_W][MAP_H];
extern unsigned char fog_layer[MAP_H][MAP_W];
extern unsigned char fog_layer_transposed[MAP_W][MAP_W];

typedef struct Light {
	float pos[3];
	float radius; 
} Light;

typedef void (*FogFunction)( Light * );
extern FogFunction the_fog_function;


#endif

