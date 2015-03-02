#ifndef _FOG_H
#define _FOG_H

typedef struct Light {
	float pos[3];
	float radius; 
} Light;

typedef void (*FogFunction)( Light * );
extern FogFunction the_fog_function;

void calc_fog1( Light *li );
void calc_fog2( Light *li );
void calc_fog3( Light *li );
void calc_fog4( Light *li );

void calc_fog( Light *li );

#endif

