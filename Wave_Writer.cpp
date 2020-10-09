#include "Wave_Writer.hpp"

#include <assert.h>
#include <stdlib.h>

/* Copyright (C) 2003-2005 by Shay Green. Permission is hereby granted, free
of charge, to any person obtaining a copy of this software and associated
documentation files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use, copy, modify,
merge, publish, distribute, sublicense, and/or sell copies of the Software, and
to permit persons to whom the Software is furnished to do so, subject to the
following conditions: The above copyright notice and this permission notice
shall be included in all copies or substantial portions of the Software. THE
SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

const unsigned header_size = 0x2C;

static void exit_with_error( const char* str ) {
	fprintf( stderr, "Error: %s\n", str );
	exit( EXIT_FAILURE );
}

Wave_Writer::Wave_Writer(uint32_t sample_rate, const char* filename):
	sample_count_(0),
	rate(sample_rate),
	buf_pos(header_size)
{
	stereo(false);
	
	file = fopen( filename, "wb" );
	if ( !file )
		exit_with_error( "Couldn't open WAVE file for writing" );
}

void Wave_Writer::flush()
{
	if ( buf_pos && !fwrite( buf, buf_pos, 1, file ) )
		exit_with_error( "Couldn't write WAVE data" );
	buf_pos = 0;
}

void Wave_Writer::write( const sample_t* in, long remain, int skip )
{
	sample_count_ += remain;
	while ( remain )
	{
		if ( buf_pos >= buf_size )
			flush();
		
		long n = (unsigned long) (buf_size - buf_pos) / sizeof (sample_t);
		if ( n > remain )
			n = remain;
		remain -= n;
		
		// convert to lsb first format
		unsigned char* p = &buf [buf_pos];
		while ( n-- ) {
			int s = *in;
			in += skip;
			*p++ = (unsigned char) s;
			*p++ = (unsigned char) (s >> 8);
		}
		
		buf_pos = p - buf;
		assert( buf_pos <= buf_size );
	}
}

Wave_Writer::~Wave_Writer()
{
	flush();
	
	// generate header
	uint32_t ds = sample_count_ * sizeof (sample_t);
	uint32_t rs = header_size - 8 + ds;
	uint16_t frame_size = chan_count * sizeof (sample_t);
	uint32_t bps = rate * frame_size;
	uint8_t header [header_size] = {
		'R','I','F','F',
		// length of rest of file, unsigned little endian 32 bits
		(uint8_t)(rs & 0xFF),
		(uint8_t)((rs>>8) & 0xFF), 
		(uint8_t)((rs>>16) & 0xFF),
		(uint8_t)((rs>>24) & 0xFF),
		'W','A','V','E',
		'f','m','t',' ',
		0x10,0,0,0,         // size of fmt chunk
		1,0,                // uncompressed format
		chan_count,0,       // channel count
		// sample rate, unsigned little endian 32 bits
		(uint8_t)(rate & 0xFF),
		(uint8_t)((rate>>8) & 0xFF),
		(uint8_t)((rate>>16) & 0xFF),
		(uint8_t)((rate>>24) & 0xFF),
		// bytes per second, unsigned little endian 32 bits
		(uint8_t)(bps & 0xFF),
		(uint8_t)((bps>>8) & 0xFF),
		(uint8_t)((bps>>16) & 0xFF),
		(uint8_t)((bps>>24) & 0xFF),
		// bytes per sample frame, unsigned little endian 16 bits
		(uint8_t)(frame_size & 0xFF),(uint8_t)((frame_size>>8) & 0xFF),
		16,0,               // bits per sample
		'd','a','t','a',
		// size of sample data, unsigned little endian 32 bits
		(uint8_t)(ds & 0xFF),
		(uint8_t)((ds>>8) & 0xFF),
		(uint8_t)((ds>>16) & 0xFF),
		(uint8_t)((ds>>24) & 0xFF),
		// ...              // sample data
	};
	
	// write header
	fseek( file, 0, SEEK_SET );
	fwrite( header, sizeof header, 1, file );
	
	fclose( file );
}
