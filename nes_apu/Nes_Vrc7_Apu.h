// Konami VRC7 sound chip emulator
#pragma once

#include <system_error>
#include "Blip_Buffer.h"

struct vrc7_snapshot_t;

class DLLEXPORT Nes_Vrc7_Apu {
public:
	std::error_condition init();

	// See Nes_Apu.h for reference
	void reset();
	void volume( double );
	void treble_eq( blip_eq_t const& );
	void set_output( Blip_Buffer* );
	enum { osc_count = 6 };
	void set_output( int index, Blip_Buffer* );
	void end_frame( blip_time_t );
	void save_snapshot( vrc7_snapshot_t* ) const;
	void load_snapshot( vrc7_snapshot_t const& );

	void write_reg( uint8_t reg );
	void write_data( blip_time_t, uint8_t data );

public:
	Nes_Vrc7_Apu();
	~Nes_Vrc7_Apu();
private:
	// noncopyable
	Nes_Vrc7_Apu( const Nes_Vrc7_Apu& );
	Nes_Vrc7_Apu& operator = ( const Nes_Vrc7_Apu& );

	struct Vrc7_Osc
	{
		uint8_t regs [3];
		Blip_Buffer* output;
		int last_amp;
	};

	Vrc7_Osc oscs [osc_count];
	uint8_t kon;
	uint8_t inst [8];
	void* opll;
	int addr;
	blip_time_t next_time;
	struct {
		Blip_Buffer* output;
		int last_amp;
	} mono;

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
#endif
	Blip_Synth_Fast synth;
#ifdef _MSC_VER
#pragma warning(pop)
#endif

	void run_until( blip_time_t );
	void output_changed();
};

struct vrc7_snapshot_t
{
	uint8_t latch;
	uint8_t inst [8];
	uint8_t regs [6] [3];
	uint8_t delay;
};

inline void Nes_Vrc7_Apu::set_output( int i, Blip_Buffer* buf )
{
	assert( (unsigned) i < osc_count );
	oscs [i].output = buf;
	output_changed();
}

// DB2LIN_AMP_BITS == 11, * 2
inline void Nes_Vrc7_Apu::volume( double v ) { synth.volume( 1.0 / 3 / 4096 * v ); }

inline void Nes_Vrc7_Apu::treble_eq( blip_eq_t const& eq ) { synth.treble_eq( eq ); }
