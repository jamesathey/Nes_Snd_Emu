/* Copyright (C) 2003-2008 Shay Green
   Copyright (C) 2020-2021 James Athey
   
   This module is free software; you can redistribute it and/or modify it under
   the terms of the GNU Lesser General Public License as published by the Free
   Software Foundation; either version 2.1 of the License, or (at your option)
   any later version. This module is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
   General Public License for more details. You should have received a copy of
   the GNU Lesser General Public License along with this module; if not, write
   to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301 USA */

#include "Nes_Mmc5_Apu.h"
#include "blargg_source.h"

int const amp_range = 15;

Nes_Mmc5_Apu::Nes_Mmc5_Apu() :
	tempo_(1.0),
	pcm_mode(WRITE_MODE),
	irq_enable(false),
	irq_flag(false),
	square1(&square_synth),
	square2(&square_synth)
{

	oscs[0] = &square1;
	oscs[1] = &square2;
	//oscs[2] = &pcm;

	set_output(nullptr);
	volume(1.0);
	reset();
}


void Nes_Mmc5_Apu::treble_eq(const blip_eq_t& eq)
{
	square_synth.treble_eq(eq);
	//pcm.synth.treble_eq(eq);
}


void Nes_Mmc5_Apu::volume(double v)
{
	v *= 1.0 / 1.11; // TODO: merge into values below
	square_synth.volume(0.125 / amp_range * v); // was 0.1128   1.108
	// pcm.synth.volume(0.450 / 2048 * v); // was 0.42545  1.058
}


void Nes_Mmc5_Apu::set_output(int osc, Blip_Buffer* buf)
{
	assert((unsigned)osc < osc_count);
	oscs[osc]->output = buf;
}

void Nes_Mmc5_Apu::set_output( Blip_Buffer* b )
{
	set_output( 0, b );
	set_output( 1, b );
	//set_output( 2, b );
}

void Nes_Mmc5_Apu::set_tempo(double t)
{
	tempo_ = t;
	if (t != 1.0)
		frame_period = (int)(frame_period / tempo_) & ~1; // must be even
	else
		frame_period = 7458;
}


void Nes_Mmc5_Apu::reset()
{
	set_tempo(tempo_);

	square1.reset();
	square2.reset();
	//pcm.reset();

	last_time = 0;
	osc_enables = 0;
	irq_flag = false;
	frame_delay = 1;
	write_register(0, 0x5015, 0);

	write_register(0, 0x5000, 0x10);
	write_register(0, 0x5002, 0);
	write_register(0, 0x5003, 0);
	write_register(0, 0x5004, 0x10);
	write_register(0, 0x5006, 0);
	write_register(0, 0x5007, 0);
	write_register(0, 0x5010, 0); // PCM write-mode, IRQ off
	write_register(0, 0x5011, 0xFF); // power-on value experimentally measured to be 0xFF
}

void Nes_Mmc5_Apu::run_until_(blip_time_t end_time)
{
	assert(end_time >= last_time);

	if (end_time == last_time)
		return;

	while (true)
	{
		// earlier of next frame time or end time
		blip_time_t time = last_time + frame_delay;
		if (time > end_time)
			time = end_time;
		frame_delay -= time - last_time;

		// run oscs to present
		square1.run(last_time, time);
		square2.run(last_time, time);
		last_time = time;

		if (time == end_time)
			break; // no more frames to run

		// take frame-specific actions
		frame_delay = frame_period;
		switch (frame++)
		{
		case 0:
		case 2:
			// clock length and sweep on frames 0 and 2
			square1.clock_length(0x20);
			square2.clock_length(0x20);

			square1.clock_sweep(-1);
			square2.clock_sweep(0);
			break;

		case 1:
			break;

		case 3:
			frame = 0;
			break;
		}

		// clock envelopes and linear counter every frame
		square1.clock_envelope();
		square2.clock_envelope();
	}
}

template<class T>
inline void zero_apu_osc(T* osc, blip_time_t time)
{
	Blip_Buffer* output = osc->output;
	int last_amp = osc->last_amp;
	osc->last_amp = 0;
	if (output && last_amp)
		osc->synth.offset(time, -last_amp, output);
}


void Nes_Mmc5_Apu::end_frame(blip_time_t end_time)
{
	if (end_time > last_time)
		run_until_(end_time);

	// make times relative to new frame
	last_time -= end_time;
	assert(last_time >= 0);
}

// registers

static const unsigned char length_table[0x20] = {
	0x0A, 0xFE, 0x14, 0x02, 0x28, 0x04, 0x50, 0x06,
	0xA0, 0x08, 0x3C, 0x0A, 0x0E, 0x0C, 0x1A, 0x0E,
	0x0C, 0x10, 0x18, 0x12, 0x30, 0x14, 0x60, 0x16,
	0xC0, 0x18, 0x48, 0x1A, 0x10, 0x1C, 0x20, 0x1E
};

void Nes_Mmc5_Apu::write_register(blip_time_t time, uint16_t addr, uint8_t data)
{
	assert(addr > 0x20); // addr must be actual address (i.e. 0x40xx)
	assert((unsigned)data <= 0xFF);

	// Ignore addresses outside range
	if (addr < regs_addr || addr >= (regs_addr + regs_size))
		return;

	run_until_(time);

	switch (addr)
	{
	case 0x5000: // Square 1
	case 0x5002:
	case 0x5003:
	case 0x5004: // Square 2
	case 0x5006:
	case 0x5007:
	{
		// Write to channel
		int osc_index = (addr - regs_addr) >> 2;
		Nes_Osc* osc = oscs[osc_index];

		int reg = addr & 3;
		osc->regs[reg] = data;
		osc->reg_written[reg] = true;

		if (reg == 3)
		{
			// load length counter
			if ((osc_enables >> osc_index) & 1)
				osc->length_counter = length_table[(data >> 3) & 0x1F];

			// reset square phase
			((Nes_Square*)osc)->phase = Nes_Square::phase_range - 1;
		}
		break;
	}
	case 0x5010:
		pcm_mode = (data & 0x1) != 0;
		irq_enable = (data & 0x80) != 0;
		break;
	case 0x5011: // DAC
		// TODO
		break;

	case 0x5015:
		// Channel enables
		if (!(data & 0x1))
			oscs[0]->length_counter = 0;
		if (!(data & 0x2))
			oscs[1]->length_counter = 0;

		osc_enables = data;
		break;

#ifdef BLARGG_DEBUG_H
	default:
		dprintf("Unmapped MMC5 APU write: $%04X <- $%02X\n", addr, data);
#endif
	}
}


uint8_t Nes_Mmc5_Apu::read_status(blip_time_t time)
{
	run_until_(time - 1);
	uint8_t result = square1.length_counter != 0 ? 1 : 0;
	if (square2.length_counter != 0)
		result |= 0x2;

	run_until_(time);

	return result;
}


uint8_t Nes_Mmc5_Apu::read_irq_status(blip_time_t time)
{
	run_until_(time);
	uint8_t result = pcm_mode;
	if (irq_flag && irq_enable)
		result |= 0x80;
	irq_flag = false;
	return result;
}
