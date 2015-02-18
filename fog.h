#ifndef _FOG_H
#define _FOG_H

typedef struct Light {
	float pos[3];
	float radius; 
} Light;

extern void (*the_fog_function)( Light * );
void calc_fog1( Light *li );
void calc_fog2( Light *li );

void calc_fog( Light *li );

#endif

