#ifndef _RENDER_H
#define _RENDER_H

#define WIN_W 800
#define WIN_H 800

void init_gl( void );
void render_map( void );

struct Light;
void show_light_pos( struct Light *li );

#endif

