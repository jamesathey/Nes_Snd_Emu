// Konami VRC6 sound chip emulator
#pragma once

#include "Nes_Apu_Base.h"

struct vrc6_apu_state_t;

class DLLEXPORT Nes_Vrc6_Apu : public Nes_Apu_Base {
public:
	// See Nes_Apu.h for reference
	void reset();
	void volume( double ) override;
	void treble_eq( blip_eq_t const& );
	void set_output( Blip_Buffer* ) override;
	enum { osc_count = 3 };
	void set_output( int index, Blip_Buffer* );
	void end_frame( blip_time_t ) override;
	void save_state( vrc6_apu_state_t* ) const;
	void load_state( vrc6_apu_state_t const& );
	
	// Oscillator 0 write-only registers are at $9000-$9002
	// Oscillator 1 write-only registers are at $A000-$A002
	// Oscillator 2 write-only registers are at $B000-$B002
	enum { reg_count = 3 };
	enum { base_addr = 0x9000 };
	enum { addr_step = 0x1000 };
	void write_osc( blip_time_t, int osc, int reg, uint8_t data );
	
public:
	Nes_Vrc6_Apu();
private:
	// noncopyable
	Nes_Vrc6_Apu( const Nes_Vrc6_Apu& );
	Nes_Vrc6_Apu& operator = ( const Nes_Vrc6_Apu& );
	
	struct Vrc6_Osc
	{
		uint8_t regs [3];
		Blip_Buffer* output;
		int delay;
		int last_amp;
		int phase;
		int amp; // only used by saw
		
		int period() const
		{
			return (regs [2] & 0x0F) * 0x100 + regs [1] + 1;
		}
	};
	
	Vrc6_Osc oscs [osc_count];
	blip_time_t last_time;
	
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
#endif
	Blip_Synth_Fast saw_synth;
	Blip_Synth_Norm square_synth;
#ifdef _MSC_VER
#pragma warning(pop)
#endif
	
	void run_until( blip_time_t );
	void run_square( Vrc6_Osc& osc, blip_time_t );
	void run_saw( blip_time_t );
};

struct vrc6_apu_state_t
{
	uint8_t regs [3] [3];
	uint8_t saw_amp;
	uint16_t delays [3];
	uint8_t phases [3];
	uint8_t unused;
};

inline void Nes_Vrc6_Apu::set_output( int i, Blip_Buffer* buf )
{
	assert( (unsigned) i < osc_count );
	oscs [i].output = buf;
}

inline void Nes_Vrc6_Apu::volume( double v )
{
	double const factor = 0.0967 * 2;
	saw_synth.volume( factor / 31 * v );
	square_synth.volume( factor * 0.5 / 15 * v );
}

inline void Nes_Vrc6_Apu::treble_eq( blip_eq_t const& eq )
{
	saw_synth.treble_eq( eq );
	square_synth.treble_eq( eq );
}
