#include <stdio.h>

#define HEATSHRINK_SF_IMPLEMENTATION

#include "heatshrink_sf.h"

int main()
{
	// echo "hello world, how are hello world, you doing" | ./heatshrink/heatshrink -e -w 8 -l 4 | xxd -i
	uint8_t test_data[] = { 
		0xb4, 0x59, 0x6d, 0x96, 0xcb, 0x7c, 0x82, 0xef, 0x6f, 0xb9, 0x5b, 0x2c,
		0x92, 0xc9, 0x05, 0xa2, 0xdf, 0x77, 0x90, 0x58, 0x6e, 0x56, 0x50, 0x38,
		0x85, 0x2e, 0xf3, 0x6f, 0xba, 0xc8, 0x2c, 0x96, 0xfb, 0x4d, 0xba, 0xcf,
		0x0a
	};

	heatshrink_sfh_context ctx;
	uint8_t output_buffer[256] = { 0 }; // Must be at least 1<<windowsize 
	int r = heatshrink_sf_init( &ctx, output_buffer, sizeof( output_buffer ), 8, 4 );
	if( r ) return r;

	ctx.input_buffer = test_data;
	ctx.input_size = sizeof( test_data );
	ctx.input_place = 0;

	ctx.out_buffer = output_buffer;
	ctx.out_buffer_size = sizeof( output_buffer );
	ctx.out_buffer_place = 0;
	r = heatshrink_sf_proceed( &ctx );
	if( r ) return r;

	puts( output_buffer );
	return 0;
}

