#include <stdio.h>
#include <string.h>

#define HEATSHRINK_SF_IMPLEMENTATION
#define HEATSHRINK_SF_OPAQUE void * fin; void * fout;

#include "heatshrink_sf.h"


int feed_file( heatshrink_sfh_context * ctx )
{
	int c = 0;
	if( fread( &c, 1, 1, ctx->fin ) == 1 )
		return c;
	return HSF_END_OF_INPUT;
}

int produce_file( heatshrink_sfh_context * ctx, uint8_t c )
{
	return fwrite( &c, 1, 1, ctx->fout ) != 1;
}

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
	uint8_t output_buffer[512] = { 0 };
	int r = heatshrink_sf_init( &ctx, output_buffer, sizeof( output_buffer ), 8, 4 );
	if( r )
	{
		fprintf( stderr, "heatshrink_sf_init() = %d\n", r );
		return -5;
	}

	ctx.input_buffer = test_data;
	ctx.input_size = sizeof( test_data );
	ctx.input_place = 0;

	ctx.out_buffer = output_buffer;
	ctx.out_buffer_size = sizeof( output_buffer );
	ctx.out_buffer_place = 0;
	r = heatshrink_sf_proceed( &ctx );

	if( r )
	{
		fprintf( stderr, "heatshrink_sf_proceed() = %d\n", r );
		return -6;
	}

	printf( "R: %d\n", r );
	puts( output_buffer );

	if( strcmp( "hello world, how are hello world, you doing\n", output_buffer ) != 0 )
	{
		fprintf( stderr, "Strings don't match\n" );
		return -7;
	}

	r = heatshrink_sf_init( &ctx, output_buffer, sizeof( output_buffer ), 9, 5 );	

	FILE * fin = ctx.fin = fopen( "gcc.hs", "rb" );
	if( !fin )
	{
		fprintf( stderr, "Error: can't open gcc.hs\n" );
		return -8;
	}
	FILE * fout = ctx.fout = fopen( "gcc.check", "wb" );
	if( !fout )
	{
		fprintf( stderr, "Error: can't write to gcc.check\n" );
		return -9;
	}
	ctx.produce = produce_file;
	ctx.feed = feed_file;


	r = heatshrink_sf_proceed( &ctx );

	if( r )
	{
		fprintf( stderr, "heatshrink_sf_proceed 2 () = %d\n", r );
		return -6;
	}

	return 0;
}

