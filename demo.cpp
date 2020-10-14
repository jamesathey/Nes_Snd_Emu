// Use Simple_Apu to play random tones. Write output to sound file "out.wav".

#include "Simple_Apu.h"

#include <stdlib.h>

#ifdef SDL_INIT_AUDIO
#include "SDL.h"
#endif

const long sample_rate = 44100;
static Simple_Apu apu;

enum {
	A, As, B, C, Cs, D, Ds, E, F, Fs, G, Gs
};

#define P(pitch, octave) (octave * 12) + pitch

uint8_t periodTableLo[] = {
	0xf1, 0x7f, 0x13, 0xad, 0x4d, 0xf3, 0x9d, 0x4c, 0x00, 0xb8, 0x74, 0x34,
	0xf8, 0xbf, 0x89, 0x56, 0x26, 0xf9, 0xce, 0xa6, 0x80, 0x5c, 0x3a, 0x1a,
	0xfb, 0xdf, 0xc4, 0xab, 0x93, 0x7c, 0x67, 0x52, 0x3f, 0x2d, 0x1c, 0x0c,
	0xfd, 0xef, 0xe1, 0xd5, 0xc9, 0xbd, 0xb3, 0xa9, 0x9f, 0x96, 0x8e, 0x86,
	0x7e, 0x77, 0x70, 0x6a, 0x64, 0x5e, 0x59, 0x54, 0x4f, 0x4b, 0x46, 0x42,
	0x3f, 0x3b, 0x38, 0x34, 0x31, 0x2f, 0x2c, 0x29, 0x27, 0x25, 0x23, 0x21,
	0x1f, 0x1d, 0x1b, 0x1a, 0x18, 0x17, 0x15, 0x14
};

uint8_t periodTableHi[] = {
	0x07, 0x07, 0x07, 0x06, 0x06, 0x05, 0x05, 0x05, 0x05, 0x04, 0x04, 0x04,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

int pitches[] = {
	P(C,2), P(E,2), P(G,2), P(C,3), P(E,3), P(G,2), P(C,3), P(E,3),
	P(C,2), P(E,2), P(G,2), P(C,3), P(E,3), P(G,2), P(C,3), P(E,3),

	P(C,2), P(D,2), P(A,3), P(D,3), P(F,3), P(A,3), P(D,3), P(F,3),
	P(C,2), P(D,2), P(A,3), P(D,3), P(F,3), P(A,3), P(D,3), P(F,3),

	P(B,2), P(D,2), P(G,2), P(D,3), P(F,3), P(G,2), P(D,3), P(F,3),
	P(B,2), P(D,2), P(G,2), P(D,3), P(F,3), P(G,2), P(D,3), P(F,3),

	P(C,2), P(E,2), P(G,2), P(C,3), P(E,3), P(G,2), P(C,3), P(E,3),
	P(C,2), P(E,2), P(G,2), P(C,3), P(E,3), P(G,2), P(C,3), P(E,3),
};

int triPitches[] = {
	P(C,3),
	P(C,3),

	P(C,3),
	P(C,3),

	P(B,3),
	P(B,3),

	P(C,3),
	P(C,3),
};

// "emulate" 1/60 second of sound
static bool emulate_frame()
{
	// Decay current tone
	static int volume = 0;
	static int pitchIdx = -1;
	static int triPitchIdx = 0;
	apu.write_register(0x4000, 0xb0 | volume);
	// The rate of decay sets the tempo
	// Decay twice as much every 3rd frame
	if (volume % 3 == 0)
		volume -= 2;
	else
		volume--;
	if (volume < 0)
	{
		volume = 15;
		pitchIdx++;

		if (pitchIdx >= (sizeof(pitches) / sizeof(int)))
			return false;

		// Start the next note
		apu.write_register(Nes_Apu::status_addr, 0x05);
		apu.write_register(0x4002, periodTableLo[pitches[pitchIdx]]);
		apu.write_register(0x4003, periodTableHi[pitches[pitchIdx]]);

		if (pitchIdx % 8 == 0) {
			apu.write_register(0x4008, 0xFF);
			apu.write_register(0x400A, periodTableLo[triPitches[triPitchIdx]]);
			apu.write_register(0x400B, periodTableHi[triPitches[triPitchIdx]]);
			triPitchIdx++;
		}
		else if (pitchIdx % 8 == 7) {
			apu.write_register(0x4008, 0x80);
		}
	}

	// Generate 1/60 second of sound into APU's sample buffer
	apu.end_frame();
	return true;
}

static int read_dmc( void*, int addr )
{
	// call your memory read function here
	//return read_memory( addr );
	return 0;
}

static void init_sound();
static void play_samples( const blip_sample_t*, long count );
static void cleanup_sound();

int main( int argc, char** argv )
{
	init_sound();
	
	// Set sample rate and check for out of memory error
	if ( apu.sample_rate( sample_rate ) )
		return EXIT_FAILURE;
	
	// Set function for APU to read memory with (required for DMC samples to play properly)
	apu.dmc_reader( read_dmc, NULL );
	
	// Generate a few seconds of sound
	while(emulate_frame()) // Simulate emulation of 1/60 second frame
	{
		// Samples from the frame can now be read out of the apu, or
		// allowed to accumulate and read out later. Use samples_avail()
		// to find out how many samples are currently in the buffer.
		
		int const buf_size = 2048;
		static blip_sample_t buf [buf_size];
		
		// Play whatever samples are available
		long count = apu.read_samples( buf, buf_size );
		play_samples( buf, count );
	}
	
	cleanup_sound();
	
	return 0;
}


// Sound output handling (either to SDL or wave file)

#ifdef SDL_INIT_AUDIO

	#include "Sound_Queue.h"

	static Sound_Queue* sound_queue;
	
	static void init_sound()
	{
		if ( SDL_Init( SDL_INIT_AUDIO ) < 0 )
			exit( EXIT_FAILURE );
		
		atexit( SDL_Quit );
		
		sound_queue = new Sound_Queue;
		if ( !sound_queue )
			exit( EXIT_FAILURE );
		
		if ( sound_queue->init( sample_rate ) )
			exit( EXIT_FAILURE );
	}

	static void cleanup_sound()
	{
		delete sound_queue;
	}
	
	static void play_samples( const blip_sample_t* samples, long count )
	{
		sound_queue->write( samples, count );
	}
	
#else

	#include "Wave_Writer.hpp"
	
	static void init_sound()    { }
	static void cleanup_sound() { }

	static void play_samples( const blip_sample_t* samples, long count )
	{
		// write samples to sound file
		static Wave_Writer wave( sample_rate );
		wave.write( samples, count );
	}
	
#endif

