# heatshrink-sfh

Single-file-header version of [heatshrink](https://github.com/atomicobject/heatshrink/)
decompression algorithm, rewritten for embedded and application-specific environments, able
to be dynamically or statically configured for streaming or in-place decompression.

Designed to be extremely small as well.  Typically within about 500 bytes of code. In fact
a complete demo in tests/heatshrink_test_size.c compiles to 670 bytes of code, has no
dependencies (`-nostdlib`) and runs in Linux on x86_64.  And it only requires about 40 bytes
of RAM for the decode struct, and another 32-byte buffer to write into.

## General notes about heatshrink

1. It uses a configurable window/max history count.
2. There is no header, you MUST match corresponding window/max history in decode with encode.
3. Compression does not have an "end of stream" so you can stream or just stop when you know you are done.
4. You MUST still have a buffer that corresponds to the `1<<MAX_WINDOW_SIZE`, even if streaming out because the compression uses history.

## Options

```c
// Example values.
#define HEATSHRINK_SF_FORCE_MAX_WINDOW 8  // hard-code the window 
#define HEATSHRINK_SF_FORCE_MAX_COUNT 4   // hard-code max count
#define HEATSHRINK_SF_DYNAMIC_STREAM  0   // Allow streaming functions "feed" "produce"
#define HEATSHRINK_SF_ALLOW_IN_BUFFER 1   // Read from an input buffer `input_buffer`
#define HEATSHRINK_SF_OPAQUE void * opaque; // Throw extra opaque's in the context struct to pass to functions.
#define HEATSHRINK_SF_STATIC_FEED feed_static // Hard-code a function for acquiring data.
#define HEATSHRINK_SF_STATIC_PRODUCE produce_static // Hard-code a function for receiving decompressed data.

#define HEATSHRINK_SF_IMPLEMENTATION
#include "heatshrink_sf.h"
```

## Example

```c
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
```

## Quick comparison

| File | Initial Size | gzip -9 | heatshrink -w 8 -l 4 |
| x86_64 gcc 11 | 928584 | 337702 (36%) | 478246 (51%) |
| BadApple-mod.mid | 13040 | 1009 (7.7%) | 2394 (18%) |
| spacestations.txt (tle) | 5378 | 1797 (33%) | 2800 (52%) |

As you can see, it's not as good as gzip, but in some cases it still can compress rather normal things by around 50%!

## Background

Heatshrink, itself, is like gzip but without all the fancy tables.  It uses 
[LZSS](https://en.wikipedia.org/wiki/Lempel%E2%80%93Ziv%E2%80%93Storer%E2%80%93Szymanski)
compression, but does not leverage huffman tables which are common in more sophisticated
compression algorithms, so compression is not as good as deflate, but it's still pretty
impressive what a simple algorithm can use.

