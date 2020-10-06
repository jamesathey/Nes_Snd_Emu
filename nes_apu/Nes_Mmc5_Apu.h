// NES MMC5 sound chip emulator

#ifndef NES_MMC5_APU_H
#define NES_MMC5_APU_H

#include "blargg_common.h"
#include "Nes_Apu.h"

class DLLEXPORT Nes_Mmc5_Apu : public Nes_Apu {
public:
	enum { regs_addr = 0x5000 };
	enum { regs_size = 0x16 };
	
	enum { osc_count  = 3 };
	void write_register( blip_time_t, unsigned addr, int data );
	void set_output( Blip_Buffer* );
	void set_output( int index, Blip_Buffer* );
	
	enum { exram_size = 1024 };
	unsigned char exram [exram_size];
};

#endif
