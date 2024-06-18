#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define HEATSHRINK_SF_FORCE_MAX_WINDOW 9
#define HEATSHRINK_SF_FORCE_MAX_COUNT 5
#define HEATSHRINK_SF_DYNAMIC_STREAM  0
#define HEATSHRINK_SF_ALLOW_IN_BUFFER 0
#define HEATSHRINK_SF_STATIC_FEED feed_stdin
#define HEATSHRINK_SF_STATIC_PRODUCE produce_stdout 

#define HEATSHRINK_SF_IMPLEMENTATION

struct heatshrink_sfh_context_s;

static inline int feed_stdin( struct heatshrink_sfh_context_s * ctx );
static inline int produce_stdout( struct heatshrink_sfh_context_s * ctx, uint8_t c );

#include "heatshrink_sf.h"

static inline int feed_stdin( heatshrink_sfh_context * ctx )
{
	int c = getchar();
	if( c == EOF ) return HSF_END_OF_INPUT;
	return c;
}

static inline int produce_stdout( heatshrink_sfh_context * ctx, uint8_t c )
{
	putchar( c );
	return 0;
}


int main()
{
	heatshrink_sfh_context ctx;
	uint8_t output_buffer[512] = { 0 };
	int r = heatshrink_sf_init( &ctx, output_buffer, sizeof( output_buffer ) );

	r = heatshrink_sf_proceed( &ctx );

	if( r )
	{
		fprintf( stderr, "heatshrink_sf_proceed 2 () = %d\n", r );
		return -6;
	}

	return 0;
}

