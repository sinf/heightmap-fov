#ifndef _FOG_H
#define _FOG_H

typedef struct Light {
	float pos[3];
	float radius; 
} Light;

void calc_fog( Light *li );

#endif

