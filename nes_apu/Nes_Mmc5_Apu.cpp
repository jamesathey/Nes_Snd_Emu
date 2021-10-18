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

int const amp_range = 15;

Nes_Mmc5_Apu::Nes_Mmc5_Apu() :
	tempo_(1.0),
	pcm_mode(WRITE_MODE),
	square1(&square_synth, 0),
	square2(&square_synth, 0),
	pcm(this)
{
	set_output(nullptr);
	volume(1.0);
	reset();
}


void Nes_Mmc5_Apu::treble_eq(const blip_eq_t& eq)
{
	square_synth.treble_eq(eq);
	pcm.synth.treble_eq(eq);
}


void Nes_Mmc5_Apu::volume(double v)
{
	v *= 1.0 / 1.11; // TODO: merge into values below
	square_synth.volume(0.125 / amp_range * v); // was 0.1128   1.108
	// TODO this PCM volume scale value is almost certainly wrong - it's probably not identical to DMC volume
	pcm.synth.volume(0.450 / 2048 * v); // was 0.42545  1.058
}


void Nes_Mmc5_Apu::set_output(int osc, Blip_Buffer* buf)
{
	assert((unsigned)osc < osc_count);
	switch (osc) {
	case 0:
		square1.output = buf;
		break;
	case 1:
		square2.output = buf;
		break;
	case 2:
		pcm.output = buf;
		break;
	}
}


void Nes_Mmc5_Apu::set_output( Blip_Buffer* b )
{
	set_output( 0, b );
	set_output( 1, b );
	set_output( 2, b );
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
	pcm.reset();

	last_time = 0;
	square1_enabled = false;
	square2_enabled = false;
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

		// MMC5 clocks the envelopes and length counters of the pulse channels every frame at a fixed 240 Hz
		square1.clock_length(0x20);
		square2.clock_length(0x20);
		//square1.clock_sweep(-1);
		//square2.clock_sweep(0);
		square1.clock_envelope();
		square2.clock_envelope();
	}
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
	// Ignore addresses outside range
	if (addr < regs_addr || addr >= (regs_addr + regs_size))
		return;

	run_until_(time);

	switch (addr)
	{
	case 0x5000: // Square 1
		square1.regs[0] = data;
		square1.reg_written[0] = true;
		break;

	case 0x5002:
		square1.regs[2] = data;
		square1.reg_written[2] = true;
		break;

	case 0x5003:
		square1.regs[3] = data;
		square1.reg_written[3] = true;
		// load length counter
		if (square1_enabled)
			square1.length_counter = length_table[data >> 3];

		// reset square phase
		square1.phase = Nes_Square::phase_range - 1;
		break;

	case 0x5004: // Square 2
		square2.regs[0] = data;
		square2.reg_written[0] = true;
		break;

	case 0x5006:
		square2.regs[2] = data;
		square2.reg_written[2] = true;
		break;

	case 0x5007:
		square2.regs[3] = data;
		square2.reg_written[3] = true;
		// load length counter
		if (square2_enabled)
			square2.length_counter = length_table[data >> 3];

		// reset square phase
		square2.phase = Nes_Square::phase_range - 1;
		break;

	case 0x5010:
		pcm_mode = (data & 0x1) != 0;
		pcm.irq_enabled = (data & 0x80) != 0;
		break;

	case 0x5011: // DAC
		if (pcm_mode == READ_MODE)
			// writes are ignored in read mode
			break;
		pcm.write_dac(time, data);
		break;

	case 0x5015:
		// Channel enables
		square1_enabled = (data & 0x1) != 0;
		square2_enabled = (data & 0x2) != 0;
		if (!square1_enabled)
			square1.length_counter = 0;
		if (!square2_enabled)
			square2.length_counter = 0;
		break;
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
	if (pcm.irq_flag && pcm.irq_enabled)
		result |= 0x80;
	// reading $5010 acknowledges the IRQ and clears the flag
	pcm.update_irq(false);
	return result;
}


Nes_Mmc5_Pcm::Nes_Mmc5_Pcm(Nes_Mmc5_Apu* a) :
	apu(a),
	output(nullptr)
{
	reset();
}


void Nes_Mmc5_Pcm::reset()
{
	last_amp = 0;
	irq_flag = false;
	irq_enabled = false;
}


void Nes_Mmc5_Pcm::write_dac(blip_time_t time, uint8_t data)
{
	if (data == 0) {
		// If you try to assign a value of $00, the DAC is not changed; an IRQ is generated instead.
		update_irq(irq_enabled);
	}
	else {
		// FIXME 16 is a very imprecise gain value, and it's linear
		int in = (int)data * 16;
		int delta = in - last_amp;
		last_amp = in;
		if (output && delta) {
			output->set_modified();
			synth.offset(time, delta, output);
		}
	}
}


void Nes_Mmc5_Pcm::update_irq(bool newIrq)
{
	bool old_irq = irq_flag;
	irq_flag = newIrq;
	if (old_irq != irq_flag && apu->irq_notifier)
		apu->irq_notifier(irq_flag);
}
