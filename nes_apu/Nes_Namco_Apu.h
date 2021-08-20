// Namco 106 sound chip emulator
#pragma once

#include "Blip_Buffer.h"

struct namco_state_t;

class DLLEXPORT Nes_Namco_Apu {
public:
	// See Nes_Apu.h for reference.
	void volume( double );
	void treble_eq( const blip_eq_t& );
	void set_output( Blip_Buffer* );
	enum { osc_count = 8 };
	void set_output( int index, Blip_Buffer* );
	void reset();
	void end_frame( blip_time_t );
	
	// Read/write data register is at 0x4800
	enum { data_reg_addr = 0x4800 };
	void write_data( blip_time_t, uint8_t );
	uint8_t read_data();
	
	// Write-only address register is at 0xF800
	enum { addr_reg_addr = 0xF800 };
	void write_addr( uint8_t );
	
	// to do: implement save/restore
	void save_state( namco_state_t* out ) const;
	void load_state( namco_state_t const& );
	
public:
	Nes_Namco_Apu();
private:
	// noncopyable
	Nes_Namco_Apu( const Nes_Namco_Apu& );
	Nes_Namco_Apu& operator = ( const Nes_Namco_Apu& );
	
	struct Namco_Osc {
		int delay;
		Blip_Buffer* output;
		short last_amp;
		short wave_pos;
	};
	
	Namco_Osc oscs [osc_count];
	
	blip_time_t last_time;
	int addr_reg;
	
	enum { reg_count = 0x80 };
	uint8_t reg [reg_count];
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
#endif
	Blip_Synth_Norm synth;
#ifdef _MSC_VER
#pragma warning(pop)
#endif
	
	uint8_t& access();
	void run_until( blip_time_t );
};
/*
struct namco_state_t
{
	uint8_t regs [0x80];
	uint8_t addr;
	uint8_t unused;
	uint8_t positions [8];
	uint32_t delays [8];
};
*/

inline uint8_t& Nes_Namco_Apu::access()
{
	int addr = addr_reg & 0x7F;
	if ( addr_reg & 0x80 )
		addr_reg = (addr + 1) | 0x80;
	return reg [addr];
}

inline void Nes_Namco_Apu::volume( double v ) { synth.volume( 0.10 / osc_count / 15 * v ); }

inline void Nes_Namco_Apu::treble_eq( const blip_eq_t& eq ) { synth.treble_eq( eq ); }

inline void Nes_Namco_Apu::write_addr( uint8_t v ) { addr_reg = v; }

inline uint8_t Nes_Namco_Apu::read_data() { return access(); }

inline void Nes_Namco_Apu::set_output( int i, Blip_Buffer* buf )
{
	assert( (unsigned) i < osc_count );
	oscs [i].output = buf;
}

inline void Nes_Namco_Apu::write_data( blip_time_t time, uint8_t data )
{
	run_until( time );
	access() = data;
}
