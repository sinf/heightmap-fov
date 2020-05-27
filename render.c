#include <GL/glew.h>
#include <GL/gl.h>
#include "render.h"
#include "map.h"
#include "fog.h"

void init_gl( void )
{
	glDisable( GL_DEPTH_TEST );
	glClearColor( 0.2, 0.2, 0.2, 0.2 );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
}

void render_map( void )
{
	unsigned char fog[MAP_H][MAP_W][4];
	int y, x;
	for( y=0; y<MAP_H;y++ ) {
		for( x=0; x<MAP_W; x++ ) {
			unsigned char *f = fog[y][x];
			f[0] = f[1] = f[2] = 0;
			f[3] = fog_layer[y][x] ? 200 : 0;
		}
	}

	glPixelZoom( (float) WIN_W / MAP_W, - (float) WIN_H / MAP_H );

	glDisable( GL_BLEND );
	glWindowPos2i( 0, WIN_H-1 );
	glDrawPixels( MAP_W, MAP_H, GL_LUMINANCE, GL_UNSIGNED_BYTE, terrain_z );

	glEnable( GL_BLEND );
	glWindowPos2i( 0, WIN_H-1 );
	glDrawPixels( MAP_W, MAP_H, GL_RGBA, GL_UNSIGNED_BYTE, fog );

	//draw_fog_debug();
}

void show_light_pos( struct Light *li )
{
	float x = 2 * li->pos[0] / MAP_W - 1;
	float y = 1 - 2 * li->pos[1] / MAP_H;

	glBegin( GL_LINES );
	glColor3ub( 255, 0, 0 );
	glVertex2f( x, -1 );
	glVertex2f( x, 1 );
	glVertex2f( -1, y );
	glVertex2f( 1, y );
	glEnd();
}

