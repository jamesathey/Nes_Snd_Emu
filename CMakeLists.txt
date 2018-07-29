CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(Nes_Snd_Emu)

SET(PACKAGE "Nes_Snd_Emu")
SET(PACKAGE_NAME ${PACKAGE})
SET(PACKAGE_VERSION_MAJOR "0")
SET(PACKAGE_VERSION_MINOR "1")
SET(PACKAGE_VERSION_PATCH "7")
SET(VERSION "${PACKAGE_VERSION_MAJOR}.${PACKAGE_VERSION_MINOR}.${PACKAGE_VERSION_PATCH}")
SET(PACKAGE_VERSION ${VERSION})
SET(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")

FIND_PACKAGE(Boost REQUIRED)

ADD_DEFINITIONS("-DBLARGG_HAVE_CONFIG_H")
ADD_DEFINITIONS("-DBLARGG_BUILD")

# Targets
SET(NES_SND_EMU_SOURCES
	nes_apu/apu_snapshot.cpp
	nes_apu/Blip_Buffer.cpp
	nes_apu/Multi_Buffer.cpp
	nes_apu/Nes_Apu.cpp
	nes_apu/Nes_Namco.cpp
	nes_apu/Nes_Oscs.cpp
	nes_apu/Nes_Vrc6.cpp
	nes_apu/Nonlinear_Buffer.cpp
)

SET(NES_SND_EMU_HEADERS
	nes_apu/apu_snapshot.h
	nes_apu/blargg_common.h
	nes_apu/blargg_source.h
	nes_apu/Blip_Buffer.h
	nes_apu/Blip_Synth.h
	nes_apu/Multi_Buffer.h
	nes_apu/Nes_Apu.h
	nes_apu/Nes_Namco.h
	nes_apu/Nes_Oscs.h
	nes_apu/Nes_Vrc6.h
	nes_apu/Nonlinear_Buffer.h
	blargg_config.h
)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})

ADD_LIBRARY(Nes_Snd_Emu SHARED ${NES_SND_EMU_SOURCES} ${NES_SND_EMU_HEADERS})
SET_TARGET_PROPERTIES(Nes_Snd_Emu PROPERTIES VERSION ${VERSION})
