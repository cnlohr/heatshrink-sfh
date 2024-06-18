#ifndef _HEATSHRINK_SF_H
#define _HEATSHRINK_SF_H

/*
  Heatshink single-file header decoder.

  Copyright 2024 <>< cnlohr

  This file may be licensed under the ISC, MIT new BSD or in public domain as
  the user desires.

  Heatshrink compression and other heatshrink files are under the ISC License,
  Copyright (c) 2013-2015, Scott Vokes <vokes.s@gmail.com>

optionally, and required:
#define HEATSHRINK_SF_STATIC

// Example values.
#define HEATSHRINK_SF_FORCE_MAX_WINDOW 8
#define HEATSHRINK_SF_FORCE_MAX_COUNT 4
#define HEATSHRINK_SF_DYNAMIC_STREAM  0
#define HEATSHRINK_SF_ALLOW_IN_BUFFER 1
#define HEATSHRINK_SF_OPAQUE void * opaque;
#define HEATSHRINK_SF_STATIC_FEED feed_static
#define HEATSHRINK_SF_STATIC_PRODUCE produce_static

#define HEATSHRINK_SF_IMPLEMENTATION
#include "heatshrink_sf.h"

*/


#include <stdint.h>
#include <string.h>

#ifdef HEATSHRINK_SF_STATIC
#define HSF_DECORATOR static
#else
#ifndef HSF_DECORATOR
#define HSF_DECORATOR
#endif
#endif

// heatshrink common defines
#define HEATSHRINK_MIN_WINDOW_BITS 4
#define HEATSHRINK_MAX_WINDOW_BITS 15
#define HEATSHRINK_MIN_LOOKAHEAD_BITS 3
#define HEATSHRINK_LITERAL_MARKER 0x01
#define HEATSHRINK_BACKREF_MARKER 0x00

#define HSF_END_OF_INPUT          -2
#define HSF_END_OF_INPUT_ERROR    -4
#define HSF_OUTPUT_OVERFLOW       -3


#ifndef HEATSHRINK_SF_DYNAMIC_STREAM
#if !defined( HEATSHRINK_SF_STATIC_FEED ) && !defined( HEATSHRINK_SF_STATIC_PRODUCE )
#define HEATSHRINK_SF_DYNAMIC_STREAM 1
#endif
#endif

#ifndef HEATSHRINK_SF_ALLOW_IN_BUFFER
#define HEATSHRINK_SF_ALLOW_IN_BUFFER 1
#endif

#if ( defined( HEATSHRINK_SF_FORCE_MAX_WINDOW ) && !defined( HEATSHRINK_SF_FORCE_MAX_COUNT ) ) || ( defined( HEATSHRINK_SF_FORCE_MAX_COUNT ) && !defined( HEATSHRINK_SF_FORCE_MAX_WINDOW ) )
#error If defining a MAX_WINDOW must also define a MAX_COUNT
#endif

#ifdef HEATSHRINK_SF_FORCE_MAX_COUNT
#if HEATSHRINK_SF_FORCE_MAX_COUNT < HEATSHRINK_MIN_LOOKAHEAD_BITS
#error INVALID HEATSHRINK_SF_FORCE_MAX_COUNT
#endif
#if HEATSHRINK_SF_FORCE_MAX_WINDOW > HEATSHRINK_MAX_WINDOW_BITS || HEATSHRINK_SF_FORCE_MAX_WINDOW < HEATSHRINK_MIN_WINDOW_BITS
#error INVALID HEATSHRINK_SF_FORCE_MAX_WINDOW
#endif

#define HSF_MAX_WINDOW(s) HEATSHRINK_SF_FORCE_MAX_WINDOW
#define HSF_MAX_COUNT(s) HEATSHRINK_SF_FORCE_MAX_COUNT

#else
#define HSF_MAX_WINDOW(s) s->pow2_window_size
#define HSF_MAX_COUNT(s) s->pow2_backref_len
#endif

#ifndef HEATSHRINK_SF_OPAQUE
#define HEATSHRINK_SF_OPAQUE
#endif


/* States for the polling state machine. */
typedef enum {
    HSF_TAG_BIT = 0,
    HSF_LITERAL,
    HSF_INDEX_MSB,
    HSF_INDEX_LSB,
    HSF_COUNT_MSB,
    HSF_COUNT_LSB,
    HSF_BACKREF,
} heatshrink_sf_state;


typedef struct heatshrink_sfh_context_s
{
	uint8_t state;
	uint8_t current_byte;
	uint8_t bit_index;
	uint8_t next_bits_pull;

#ifndef HEATSHRINK_SF_FORCE_MAX_WINDOW
	uint8_t pow2_window_size;
	uint8_t pow2_backref_len;
	uint16_t reserved;
#endif

	uint16_t backref_index;
	uint16_t backref_count;

	// For streaming data in.
#if HEATSHRINK_SF_DYNAMIC_STREAM
	int (*feed)( struct heatshrink_sfh_context_s * ctx ); //Set to null if not in use.
#endif
#if HEATSHRINK_SF_ALLOW_IN_BUFFER
	const uint8_t * input_buffer;
	int input_place;
	int input_size; 
#endif

	// For streaming data out.
#ifdef HEATSHRINK_SF_DYNAMIC_STREAM
	int (*produce)( struct heatshrink_sfh_context_s * ctx, uint8_t c ); //Set to null if not in use.
#endif
	uint8_t * out_buffer;
	int out_buffer_size;
	int out_buffer_place;
	HEATSHRINK_SF_OPAQUE
} heatshrink_sfh_context;

#ifdef HEATSHRINK_SF_FORCE_MAX_WINDOW
int HSF_DECORATOR heatshrink_sf_init( heatshrink_sfh_context * ctx, uint8_t * out_buffer, int out_buffer_size );
#else
int HSF_DECORATOR heatshrink_sf_init( heatshrink_sfh_context * ctx, uint8_t * out_buffer, int out_buffer_size, int pow2windowsize, int pow2backreflen );
#endif

int HSF_DECORATOR heatshrink_sf_proceed( heatshrink_sfh_context * ctx );

#endif

#ifdef HEATSHRINK_SF_IMPLEMENTATION

static inline int hsf_get_bits( heatshrink_sfh_context * ctx, int nrbits )
{
	int bit_index = ctx->bit_index;
	int current_byte = ctx->current_byte;
	int next_byte = 0;
	int mask = (1<<nrbits) - 1;
	if( bit_index - nrbits < 0 )
	{

#ifdef HEATSHRINK_SF_STATIC_FEED
		int t = HEATSHRINK_SF_STATIC_FEED( ctx );
		if( t < 0 )
			return t;
		next_byte = t;
		if( 1 );
#elif HEATSHRINK_SF_DYNAMIC_STREAM
		if( ctx->feed )
		{
			int t = ctx->feed( ctx );
			if( t < 0 )
				return t;
			next_byte = t;
		}
#else // Not static feed or dynamic stream
		if( 0 );
#endif
#if HEATSHRINK_SF_ALLOW_IN_BUFFER
		else if( ctx->input_buffer && ctx->input_place < ctx->input_size )
			next_byte = ctx->input_buffer[ctx->input_place++];
		else
			return ( nrbits == 1 ) ? HSF_END_OF_INPUT : HSF_END_OF_INPUT_ERROR;
#else
		else return HSF_END_OF_INPUT_ERROR;
#endif

		int wordpair = ( current_byte << 8 ) | next_byte;

		int next_index = bit_index - nrbits + 8;
		int r = ( wordpair >> next_index ) & mask;
		ctx->current_byte = next_byte;
		ctx->bit_index = next_index;
		return r;
	}
	int next_index = bit_index - nrbits;
	int r = ( current_byte >> next_index ) & mask;
	ctx->bit_index = next_index;
	return r;
}

static int hsf_emit_byte( heatshrink_sfh_context * ctx, uint8_t byteout )
{
	int obp = ctx->out_buffer_place;
	ctx->out_buffer[obp++] = byteout;
	if( obp == ctx->out_buffer_size )
		obp = 0;
	ctx->out_buffer_place = obp;

#ifdef HEATSHRINK_SF_STATIC_PRODUCE
	return HEATSHRINK_SF_STATIC_PRODUCE( ctx, byteout );
#elif defined( HEATSHRINK_SF_DYNAMIC_STREAM )
	if( ctx->produce )
	{
		return ctx->produce( ctx, byteout );
	}
	else
#endif
	if( obp == 0 ) 
	{
		return HSF_OUTPUT_OVERFLOW;
	}
	return 0;
}

#ifdef HEATSHRINK_SF_FORCE_MAX_WINDOW
int HSF_DECORATOR heatshrink_sf_init( heatshrink_sfh_context * ctx, uint8_t * out_buffer, int out_buffer_size )
#else
int HSF_DECORATOR heatshrink_sf_init( heatshrink_sfh_context * ctx, uint8_t * out_buffer, int out_buffer_size, int pow2windowsize, int pow2backreflen )
#endif
{
	memset( ctx, 0, sizeof( heatshrink_sfh_context ) );
	ctx->out_buffer = out_buffer;
	ctx->out_buffer_size = out_buffer_size;
	ctx->state = HSF_TAG_BIT;
	ctx->next_bits_pull = 1;
#ifndef HEATSHRINK_SF_FORCE_MAX_WINDOW
	ctx->pow2_window_size = pow2windowsize;
	ctx->pow2_backref_len = pow2backreflen;
	if( pow2windowsize > HEATSHRINK_MAX_WINDOW_BITS || pow2windowsize < HEATSHRINK_MIN_WINDOW_BITS || pow2backreflen < HEATSHRINK_MIN_LOOKAHEAD_BITS ) return -1;
#endif

	return 0;
}

int HSF_DECORATOR heatshrink_sf_proceed( heatshrink_sfh_context * ctx )
{
	int temp = 0;

	for(;;)
	{
		int state = ctx->state;
		int bits = 0;
		int next_bits_pull = ctx->next_bits_pull;
		if( next_bits_pull )
			bits = hsf_get_bits( ctx, next_bits_pull );

		if( bits < 0 )
			return (bits == HSF_END_OF_INPUT) ? 0 : bits;

		switch( state )
		{
		case HSF_TAG_BIT: 
			if( bits != HEATSHRINK_BACKREF_MARKER )
			{
				state = HSF_LITERAL;
				next_bits_pull = 8;
			}
			else
			{
				if( HSF_MAX_WINDOW( ctx ) > 8 )
				{
					state = HSF_INDEX_MSB;
					next_bits_pull = HSF_MAX_WINDOW( ctx ) - 8;
				}
				else
				{
					state = HSF_INDEX_LSB;
					next_bits_pull = HSF_MAX_WINDOW( ctx );
					ctx->backref_index = 0;
				}
			}
			break;
		case HSF_LITERAL:
			temp = hsf_emit_byte( ctx, bits );
			if( temp < 0 ) return temp;
			state = HSF_TAG_BIT;
			next_bits_pull = 1;
			break;
		case HSF_INDEX_MSB:
			ctx->backref_index = bits << 8;
			next_bits_pull = 8;
			state = HSF_INDEX_LSB;
			break;
		case HSF_INDEX_LSB:
			ctx->backref_index = ctx->backref_index + bits + 1;
			temp = HSF_MAX_COUNT( ctx );
			if( temp > 8 )
			{
				state = HSF_COUNT_MSB;
				next_bits_pull = temp - 8;
			}
			else
			{
				state = HSF_COUNT_LSB;
				next_bits_pull = temp;
				ctx->backref_count = 0;
			}
			break;
		case HSF_COUNT_MSB:
			ctx->backref_count = bits << 8;
			state = HSF_INDEX_LSB;
			next_bits_pull = 8;
			break;
		case HSF_COUNT_LSB:
			ctx->backref_count = ctx->backref_count + bits;
			state = HSF_BACKREF;
			next_bits_pull = 0;
			break;
		case HSF_BACKREF:
			do
			{
				temp = ctx->out_buffer_place - ctx->backref_index; 
				if( temp < 0 )
					temp += ctx->out_buffer_size;
				if( temp < 0 )
					return -9;
				temp = hsf_emit_byte( ctx, ctx->out_buffer[temp] );
				if( temp < 0 ) return temp;
			}
			while( ctx->backref_count-- != 0 );
			state = HSF_TAG_BIT;
			next_bits_pull = 1;
			break;
		}
		ctx->next_bits_pull = next_bits_pull;
		ctx->state = state;
	}
	return -1;
}

#endif

