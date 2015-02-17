#include <glew.h>
#include <gl.h>
#include <SDL.h>
#include <FreeImage.h>
#include "map.h"
#include "fog.h"
#include "render.h"

static Light light = {
	.pos = {MAP_W/2,MAP_H/2,128},
	.radius = 80,
};

static void set_light_xy( int x, int y )
{
	light.pos[0] = (float) MAP_W * x / WIN_W;
	light.pos[1] = (float) MAP_H * y / WIN_H;
}

static int process_events( void )
{
	float lz = 1, lz2 = 10;
	SDL_Event e;
	while( SDL_PollEvent( &e ) ) {
		switch( e.type ) {
			case SDL_QUIT:
				return 0;
			case SDL_KEYDOWN:
				switch( e.key.keysym.sym ) {
					case SDLK_ESCAPE:
						return 0;
					case SDLK_UP:
						light.pos[2] += lz;
						break;
					case SDLK_DOWN:
						light.pos[2] -= lz;
						break;
					case SDLK_RIGHT:
						light.pos[2] += lz2;
						break;
					case SDLK_LEFT:
						light.pos[2] -= lz2;
						break;
					default:
						break;
				}
				break;
			case SDL_MOUSEMOTION:
				if ( SDL_GetModState() & KMOD_CTRL )
					set_light_xy( e.motion.x, e.motion.y );
				break;
			case SDL_MOUSEBUTTONDOWN:
				set_light_xy( e.button.x, e.button.y );
				break;
			default:
				break;
		}
	}
	return 1;
}

int main( int argc, char **argv )
{
	char *img_fn = "107uafq.png";

	if ( argc > 1 ) {
		img_fn = argv[1];
	}

	FreeImage_Initialise( 0 );
	if ( !load_map( img_fn ) ) {
		printf( "Failed to load image\n" );
		return -1;
	}

	SDL_Init( SDL_INIT_VIDEO );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	SDL_SetVideoMode( WIN_W, WIN_H, 0, SDL_OPENGL );
	glewInit();
	init_gl();

	for( ;; ) {
		if ( !process_events() )
			break;

		calc_fog( &light );
		render_map();
		show_light_pos( &light );
		SDL_GL_SwapBuffers();
		SDL_Delay( 10 );
	}

	SDL_Quit();
	return 0;
}


