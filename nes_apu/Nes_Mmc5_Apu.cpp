#include "Nes_Mmc5_Apu.h"

/* Copyright (C) 2003-2006 Shay Green. This module is free software; you
 can redistribute it and/or modify it under the terms of the GNU Lesser
 General Public License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version. This
 module is distributed in the hope that it will be useful, but WITHOUT ANY
 WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 details. You should have received a copy of the GNU Lesser General Public
 License along with this module; if not, write to the Free Software Foundation,
 Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA */

#include "blargg_source.h"

void Nes_Mmc5_Apu::set_output( int i, Blip_Buffer* b )
{
	// in: square 1, square 2, PCM
	// out: square 1, square 2, skipped, skipped, PCM
	if ( i > 1 )
		i += 2;
	Nes_Apu::set_output( i, b );
}

void Nes_Mmc5_Apu::set_output( Blip_Buffer* b )
{
	set_output( 0, b );
	set_output( 1, b );
	set_output( 2, b );
}

void Nes_Mmc5_Apu::write_register( blip_time_t time, unsigned addr, int data )
{
	switch ( addr )
	{
		case 0x5015: // channel enables
			data &= 0x03; // enable the square waves only
			// fall through
		case 0x5000: // Square 1
		case 0x5002:
		case 0x5003:
		case 0x5004: // Square 2
		case 0x5006:
		case 0x5007:
		case 0x5011: // DAC
			Nes_Apu::write_register( time, addr - 0x1000, data );
			break;
			
		case 0x5010: // some things write to this for some reason
			break;
			
#ifdef BLARGG_DEBUG_H
		default:
			dprintf( "Unmapped MMC5 APU write: $%04X <- $%02X\n", addr, data );
#endif
	}
}
