#pragma once

#include "Blip_Buffer.h"

class DLLEXPORT Nes_Apu_Base
{
public:
	// Basics
	typedef int nes_time_t; // NES CPU clock cycle count
	
	// Sets buffer to generate sound into, or 0 to mute output (reduces
	// emulation accuracy).
	virtual void set_output( Blip_Buffer* ) = 0;
	
	// Runs all oscillators up to specified time, ends current time frame, then
	// starts a new time frame at time 0. Time frames have no effect on emulation
	// and each can be whatever length is convenient.
	virtual void end_frame( nes_time_t ) = 0;
	
	// Sets overall volume (default is 1.0)
	virtual void volume( double ) = 0;

protected:
	Nes_Apu_Base() = default;
};
