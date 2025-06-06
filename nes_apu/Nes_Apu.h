// NES 2A03 APU sound chip emulator
#pragma once

#include "Nes_Oscs.h"
#include "Nes_Apu_Base.h"
#include <functional>
#include <climits>

struct apu_state_t;
class Nes_Buffer;

class DLLEXPORT Nes_Apu : public Nes_Apu_Base {
public:
// Basics

	typedef int nes_time_t; // NES CPU clock cycle count
	
	// Sets buffer to generate sound into, or 0 to mute output (reduces
	// emulation accuracy).
	void set_output( Blip_Buffer* ) override;
	
	// All time values are the number of CPU clock cycles relative to the
	// beginning of the current time frame. Before resetting the CPU clock
	// count, call end_frame( last_cpu_time ).
	
	// Writes to register (0x4000-0x4013, and 0x4015 and 0x4017)
	enum { io_addr = 0x4000 };
	enum { io_size = 0x18 };
	void write_register( nes_time_t, uint16_t addr, uint8_t data );
	
	// Reads from status register (0x4015)
	enum { status_addr = 0x4015 };
	uint8_t read_status( nes_time_t );
	
	// Runs all oscillators up to specified time, ends current time frame, then
	// starts a new time frame at time 0. Time frames have no effect on emulation
	// and each can be whatever length is convenient.
	void end_frame( nes_time_t ) override;
	
// Optional

	// Resets internal frame counter, registers, and all oscillators.
	// Uses PAL timing if pal_timing is true, otherwise use NTSC timing.
	// Sets the DMC oscillator's initial DAC value to initial_dmc_dac without
	// any audible click.
	void reset( bool pal_mode = false, uint8_t initial_dmc_dac = 0 );
	
	// Same as set_output(), but for a particular channel
	// 0: Square 1, 1: Square 2, 2: Triangle, 3: Noise, 4: DMC
	enum { osc_count = 5 };
	void set_output( int chan, Blip_Buffer* buf );
	
	// Adjusts frame period
	void set_tempo( double );
	
	// Saves/loads exact emulation state
	void save_state( apu_state_t* out ) const;
	void load_state( apu_state_t const& );
	
	// Sets overall volume (default is 1.0)
	void volume( double ) override;
	
	// Sets treble equalization (see notes.txt)
	void treble_eq( const blip_eq_t& );
	
	// Gets time that APU-generated IRQ will occur if no further register reads
	// or writes occur. If IRQ is already pending, returns irq_waiting. If no
	// IRQ will occur, returns no_irq.
	enum { no_irq = INT_MAX/2 + 1 };
	enum { irq_waiting = 0 };
	nes_time_t earliest_irq( nes_time_t ) const;
	
	// Counts number of DMC reads that would occur if 'run_until( t )' were executed.
	// If last_read is not NULL, set *last_read to the earliest time that
	// 'count_dmc_reads( time )' would result in the same result.
	int count_dmc_reads( nes_time_t t, nes_time_t* last_read = nullptr ) const;
	
	// Time when next DMC memory read will occur
	nes_time_t next_dmc_read_time() const;
	
	// Runs DMC until specified time, so that any DMC memory reads can be
	// accounted for (i.e. inserting CPU wait states).
	void run_until( nes_time_t );
	

// Implementation
public:
	Nes_Apu();

#ifdef _MSC_VER
#pragma warning(push)
	// prevents "warning C4251: 'Nes_Apu::dmc_reader': class 'std::function<int (int)>' needs to have dll-interface to be used by clients of class 'Nes_Apu'"
	// std::function<>, being a templated class inside of the STL, is guaranteed to generate code where needed for clients, because it's all in the header
#pragma warning(disable : 4251)
#endif

	// Memory reader callback used by DMC oscillator to fetch samples.
	// Use std::bind to add custom parameters.
	std::function<int(int)> dmc_reader;

	// IRQ time callback that is invoked when the time of earliest IRQ may have changed.
	// Use std::bind to add custom parameters.
	std::function<void()> irq_notifier;

#ifdef _MSC_VER
#pragma warning(pop)
#endif

	void enable_nonlinear_( double sq, double tnd );
	static float tnd_total_() { return 196.015f; }

	void enable_w4011_( bool enable = true ) { enable_w4011 = enable; }
	
private:
	friend struct Nes_Dmc;
	
	// noncopyable
	Nes_Apu( const Nes_Apu& );
	Nes_Apu& operator = ( const Nes_Apu& );
	
	Nes_Osc*            oscs [osc_count];
#ifdef _MSC_VER
	// These are truly private members, and we don't need the compiler
	// to complain "needs to have dll-interface to be used by clients of class"
#pragma warning(push)
#pragma warning(disable : 4251)
#endif
	Nes_Square          square1;
	Nes_Square          square2;
	Nes_Noise           noise;
	Nes_Triangle        triangle;
	Nes_Dmc             dmc;

	double tempo_;
	nes_time_t last_time; // has been run until this time in current frame
	nes_time_t last_dmc_time;
	nes_time_t earliest_irq_;
	nes_time_t next_irq;
	int frame_period;
	int frame_delay; // cycles until frame counter runs next
	int frame; // current frame (0-3)
	int osc_enables;
	int frame_mode;
	bool irq_flag;
	bool enable_w4011;
	Nes_Square::Synth square_synth; // shared by squares
#ifdef _MSC_VER
#pragma warning(pop)
#endif

	void irq_changed();
	void state_restored();
	void run_until_( nes_time_t );
};

inline void Nes_Apu::set_output( int osc, Blip_Buffer* buf )
{
	assert( (unsigned) osc < osc_count );
	oscs [osc]->output = buf;
}

inline Nes_Apu::nes_time_t Nes_Apu::earliest_irq( nes_time_t ) const
{
	return earliest_irq_;
}

inline int Nes_Apu::count_dmc_reads( nes_time_t time, nes_time_t* last_read ) const
{
	return dmc.count_reads( time, last_read );
}
	
inline Nes_Apu::nes_time_t Nes_Dmc::next_read_time() const
{
	if ( length_counter == 0 )
		return Nes_Apu::no_irq; // not reading
	
	return apu->last_dmc_time + delay + (bits_remain - 1) * period;
}

inline Nes_Apu::nes_time_t Nes_Apu::next_dmc_read_time() const { return dmc.next_read_time(); }
