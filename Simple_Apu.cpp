
// Nes_Snd_Emu 0.1.7. http://www.slack.net/~ant/libs/

#include "Simple_Apu.h"

#include <functional>

/* Copyright (C) 2003-2005 Shay Green. This module is free software; you
can redistribute it and/or modify it under the terms of the GNU Lesser
General Public License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version. This
module is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details. You should have received a copy of the GNU Lesser General
Public License along with this module; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA */

static int null_dmc_reader( int )
{
	return 0x55; // causes dmc sample to be flat
}

Simple_Apu::Simple_Apu()
{
	time = 0;
	frame_length = 29780;
	apu.dmc_reader = &null_dmc_reader;
}

Simple_Apu::~Simple_Apu()
{
}

void Simple_Apu::dmc_reader( int (*f)(void* user_data, int addr), void* p )
{
	assert( f );
	apu.dmc_reader = std::bind(f, p, std::placeholders::_1);
}

blargg_err_t Simple_Apu::sample_rate( long rate )
{
	apu.set_output( &buf );
	buf.clock_rate( 1789773 );
	return buf.set_sample_rate( rate );
}

void Simple_Apu::write_register( int addr, int data )
{
	apu.write_register( clock(), addr, data );
}

int Simple_Apu::read_status()
{
	return apu.read_status( clock() );
}

void Simple_Apu::end_frame()
{
	time = 0;
	frame_length ^= 1;
	apu.end_frame( frame_length );
	buf.end_frame( frame_length );
}

long Simple_Apu::samples_avail() const
{
	return buf.samples_avail();
}

long Simple_Apu::read_samples( sample_t* p, long s )
{
	return buf.read_samples( p, s );
}

/*
void Simple_Apu::save_snapshot( apu_snapshot_t* out ) const
{
	apu.save_snapshot( out );
}

void Simple_Apu::load_snapshot( apu_snapshot_t const& in )
{
	apu.load_snapshot( in );
}
*/
