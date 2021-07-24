// NES MMC5 sound chip emulator

#pragma once

#include "blargg_common.h"
#include "Nes_Oscs.h"
#include "Blip_Buffer.h"

struct apu_state_t;
class Nes_Buffer;

class DLLEXPORT Nes_Mmc5_Apu {
public:
	void set_output(Blip_Buffer*);

	enum { regs_addr = 0x5000 };
	enum { regs_size = 0x16 };
	
	void write_register(blip_time_t, uint16_t addr, uint8_t data);

	// All time values are the number of CPU clock cycles relative to the
	// beginning of the current time frame. Before resetting the CPU clock
	// count, call end_frame( last_cpu_time ).

	enum { status_addr = 0x5015 };
	// Read the status register ($5015). The time is the number of CPU clock
	// cycles relative to the beginning of the current time frame.
	uint8_t read_status( blip_time_t );

	// Read the IRQ status register ($5010). The time is the number of CPU clock
	// cycles relative to the beginning of the current time frame.
	// Reading this register acknowledges the PCM IRQ (if raised) and clears
	// the IRQ flag.
	uint8_t read_irq_status(blip_time_t);

	// Runs all oscillators up to specified time, ends current time frame, then
	// starts a new time frame at time 0. Time frames have no effect on emulation
	// and each can be whatever length is convenient.
	void end_frame(blip_time_t);

	// Resets internal frame counter, registers, and all oscillators.
	void reset();

	// Same as set_output(), but for a particular channel
	// 0: Square 1, 1: Square 2, 2: PCM
	enum { osc_count = 3 };
	void set_output(int chan, Blip_Buffer* buf);

	// Saves/loads exact emulation state
	void save_state(apu_state_t* out) const;
	void load_state(apu_state_t const&);

	// Sets overall volume (default is 1.0)
	void volume(double);

	// Sets treble equalization (see notes.txt)
	void treble_eq(const blip_eq_t&);

	// Implementation
public:
	Nes_Mmc5_Apu();

private:
	// noncopyable
	Nes_Mmc5_Apu(const Nes_Mmc5_Apu&);
	Nes_Mmc5_Apu& operator = (const Nes_Mmc5_Apu&);

#ifdef _MSC_VER
	// These are truly private members, and we don't need the compiler
	// to complain "needs to have dll-interface to be used by clients of class"
#pragma warning(push)
#pragma warning(disable : 4251)
#endif
	Nes_Square square1;
	Nes_Square square2;
	// Nes_Mmc5_Pcm pcm;

	double tempo_;
	blip_time_t last_time; // has been run until this time in current frame
	int frame_period;
	int frame_delay; // cycles until frame counter runs next
	bool square1_enabled;
	bool square2_enabled;
	enum PcmMode { WRITE_MODE, READ_MODE };
	bool pcm_mode;
	bool irq_enable;
	bool irq_flag;
	Nes_Square::Synth square_synth; // shared by squares
#ifdef _MSC_VER
#pragma warning(pop)
#endif

	void set_tempo(double t);
	void state_restored();
	void run_until_(blip_time_t);
};
