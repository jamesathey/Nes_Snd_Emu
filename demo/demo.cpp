/*
	Copyright (c) 2003-2005 Shay Green
	Copyright (c) 2020 James Athey

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
	DEALINGS IN THE SOFTWARE.
*/

#define SDL_MAIN_HANDLED
#include "SDL.h"

#include "Simple_Apu.h"
#include "Wave_Writer.hpp"
#include "Sound_Queue.h"

#include <cstdio>
#include <memory>

enum Scale {
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

// pitches for the first square wave channel to play, 16 per measure
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

// pitches for the triangle channel to play, 2 per measure
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

// "emulate" 1/60 second of sound. Returns false when there are no more notes to play.
bool emulate_frame(Simple_Apu& apu)
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
			// play 1 triangle note for every 8 square note, aka, once every two beats
			apu.write_register(0x4008, 0xFF);
			apu.write_register(0x400A, periodTableLo[triPitches[triPitchIdx]]);
			apu.write_register(0x400B, periodTableHi[triPitches[triPitchIdx]]);
			triPitchIdx++;
		}
		else if (pitchIdx % 8 == 7) {
			// silence the triangle channel before the last 16th note
			apu.write_register(0x4008, 0x80);
		}
	}

	// Generate 1/60 second of sound into APU's sample buffer
	apu.end_frame();
	return true;
}


int main(int argc, char** argv)
{
	bool liveMode = argc == 1;
	if (argc > 2 || (argc == 2 && strcmp(argv[1], "-w") != 0)) {
		fprintf(stderr, "Usage: %s    (plays live audio)\n", argv[0]);
		fprintf(stderr, "       %s -w (writes to out.wav in the current working directory)\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	SDL_SetMainReady();
	if (SDL_Init(SDL_INIT_AUDIO) < 0)
		exit(EXIT_FAILURE);
	atexit(SDL_Quit);

	const long sample_rate = 44100;

	std::unique_ptr<Sound_Queue> sound_queue;
	std::unique_ptr<Wave_Writer> wave_writer;
	if (liveMode) {
		sound_queue = std::make_unique<Sound_Queue>();
		if (sound_queue->init(sample_rate))
			exit(EXIT_FAILURE);
	}
	else {
		wave_writer = std::make_unique<Wave_Writer>(sample_rate);
	}

	Simple_Apu apu;
	// Set sample rate and check for out of memory error
	if (apu.sample_rate(sample_rate))
		return EXIT_FAILURE;
	
	blip_sample_t buf[2048];

	// Generate sound until we run out of notes
	while(emulate_frame(apu)) // Simulate emulation of 1/60 second frame
	{
		// Samples from the frame can now be read out of the apu, or
		// allowed to accumulate and read out later. Use samples_avail()
		// to find out how many samples are currently in the buffer.
		
		// Fetch whatever samples are available
		long count = apu.read_samples(buf, sizeof(buf) / sizeof(blip_sample_t));

		if (liveMode)
			sound_queue->write(buf, count);
		else
			wave_writer->write(buf, count);
	}
	
	return 0;
}
