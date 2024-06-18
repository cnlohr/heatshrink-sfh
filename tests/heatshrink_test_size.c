#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

#include <asm/unistd.h>      // compile without -m32 for 64 bit call numbers

#define HEATSHRINK_SF_FORCE_MAX_WINDOW 5
#define HEATSHRINK_SF_FORCE_MAX_COUNT 4
#define HEATSHRINK_SF_DYNAMIC_STREAM  0
#define HEATSHRINK_SF_ALLOW_IN_BUFFER 1
#define HEATSHRINK_SF_STATIC_PRODUCE produce_stdout
#define HEATSHRINK_SF_IMPLEMENTATION

struct heatshrink_sfh_context_s;
static inline int produce_stdout( struct heatshrink_sfh_context_s * ctx, uint8_t c );

struct heatshrink_sfh_context_s;


#include "heatshrink_sf.h"

static inline int produce_stdout( struct heatshrink_sfh_context_s * ctx, uint8_t c )
{
	//return write( 0, &c, 1 ) == 1;
	int ret;
    asm volatile
    (
        "syscall"
        : "=a" (ret)
        //                 EDI      RSI       RDX
        : "0"(__NR_write), "D"(/*fd*/0), "S"(&c), "d"(1)
        : "rcx", "r11", "memory"
    );
	return ret == 1;
}

// echo "hello world, how are hello world, you doing" | ./heatshrink/heatshrink -e -w 5 -l 4 | xxd -i
const uint8_t test_data[] = { 
	0xb4, 0x59, 0x6d, 0x96, 0xcb, 0x7c, 0x82, 0xef, 0x6f, 0xb9, 0x5b, 0x2c,
	0x92, 0xc9, 0x05, 0xa2, 0xdf, 0x77, 0x90, 0x58, 0x6e, 0x56, 0x51, 0xc5,
	0x4b, 0xbc, 0xdb, 0xee, 0xb2, 0x0b, 0x25, 0xbe, 0xd3, 0x6e, 0xb3, 0xc2,
	0x80
};

heatshrink_sfh_context ctx;
uint8_t output_buffer[32] = { 0 };

int start()
{
	int r = heatshrink_sf_init( &ctx, output_buffer, sizeof( output_buffer ) );
	ctx.input_buffer = test_data;
	ctx.input_place = 0;
	ctx.input_size = sizeof( test_data );
	r = heatshrink_sf_proceed( &ctx );

    asm volatile
    (
        "syscall"
        : "=a" (r)
        //                 EDI  
        : "0"(__NR_exit), "D"(/*fd*/0), "S"(0), "d"(0)
        : "memory"
    );
}

