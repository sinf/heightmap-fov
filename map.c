#include <FreeImage.h>
#include "map.h"

unsigned char terrain_z[MAP_H][MAP_W] = {{0}};
unsigned char fog_layer[MAP_H][MAP_W] = {{0}};

unsigned char terrain_z_transposed[MAP_W][MAP_H] = {{0}};
unsigned char fog_layer_transposed[MAP_W][MAP_W] = {{0}};

int load_map( const char *filename )
{
	FIBITMAP *fib, *grey;
	FREE_IMAGE_FORMAT filetype;
	int flags = 0;

	filetype = FreeImage_GetFileType( filename, 0 );

	switch( filetype )
	{
		case FIF_JPEG:
			flags = JPEG_GREYSCALE | JPEG_EXIFROTATE | JPEG_ACCURATE;
			break;
		case FIF_PNG:
			flags = PNG_IGNOREGAMMA;
			break;
		case FIF_UNKNOWN:
			return 0;
		default:
			break;
	}

	fib = FreeImage_Load( filetype, filename, flags );
	grey = FreeImage_ConvertToGreyscale( fib );
	FreeImage_Unload( fib );

	int w = FreeImage_GetWidth( grey );
	int h = FreeImage_GetHeight( grey );
	int y, x;

	w = MIN( w, MAP_W );
	h = MIN( h, MAP_H );

	for( y=0; y<h; y++ ) {
		unsigned char *line = FreeImage_GetScanLine( grey, y );
		for( x=0; x<w; x++ ) {
			terrain_z[y][x] = line[x];
		}
	}

	for( x=0; x<w; x++ ) {
		for( y=0; y<h; y++ ) {
			terrain_z_transposed[x][y] = terrain_z[y][x];
		}
	}

	FreeImage_Unload( grey );
	return 1;
}


