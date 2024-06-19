#include <stdio.h>
#include <string.h>

#define HEATSHRINK_SF_FORCE_MAX_WINDOW 9
#define HEATSHRINK_SF_FORCE_MAX_COUNT 5
#define HEATSHRINK_SF_DYNAMIC_STREAM  1
#define HEATSHRINK_SF_ALLOW_IN_BUFFER 0

#define HEATSHRINK_SF_IMPLEMENTATION

#include "heatshrink_sf.h"

int feed_stdin( heatshrink_sfh_context * ctx )
{
	int c = getchar();
	if( c == EOF ) return HSF_END_OF_INPUT;
	return c;
}

int produce_stdout( heatshrink_sfh_context * ctx, uint8_t c )
{
	putchar( c );
	return 0;
}

int main()
{
	heatshrink_sfh_context ctx;
	uint8_t output_buffer[512] = { 0 };
	int r = heatshrink_sf_init( &ctx, output_buffer, sizeof( output_buffer ) );

	ctx.produce = produce_stdout;
	ctx.feed = feed_stdin;


	r = heatshrink_sf_proceed( &ctx );

	if( r )
	{
		fprintf( stderr, "heatshrink_sf_proceed 2 () = %d\n", r );
		return -6;
	}

	return 0;
}

