CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

PROJECT(Nes_Snd_Emu)

SET(PACKAGE "Nes_Snd_Emu")
SET(PACKAGE_NAME ${PACKAGE})
SET(PACKAGE_VERSION_MAJOR "0")
SET(PACKAGE_VERSION_MINOR "2")
SET(PACKAGE_VERSION_PATCH "0")
SET(VERSION "${PACKAGE_VERSION_MAJOR}.${PACKAGE_VERSION_MINOR}.${PACKAGE_VERSION_PATCH}")
SET(PACKAGE_VERSION ${VERSION})
SET(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")

# Targets
SET(NES_SND_EMU_SOURCES
	emu2413/emu2413.c
	nes_apu/Blip_Buffer.cpp
	nes_apu/gme_custom_dprintf.c
	nes_apu/Multi_Buffer.cpp
	nes_apu/Nes_Apu.cpp
	nes_apu/Nes_Fds_Apu.cpp
	nes_apu/Nes_Fme7_Apu.cpp
	nes_apu/Nes_Mmc5_Apu.cpp
	nes_apu/Nes_Namco_Apu.cpp
	nes_apu/Nes_Oscs.cpp
	nes_apu/Nes_Vrc6_Apu.cpp
	nes_apu/Nes_Vrc7_Apu.cpp
	nes_apu/panning.c
)

SET(NES_SND_EMU_HEADERS
	emu2413/emu2413.h
	nes_apu/blargg_common.h
	nes_apu/blargg_source.h
	nes_apu/Blip_Buffer.h
	nes_apu/Blip_Buffer_impl.h
	nes_apu/Blip_Buffer_impl2.h
	nes_apu/gme_custom_dprintf.h
	nes_apu/Multi_Buffer.h
	nes_apu/Nes_Apu.h
	nes_apu/Nes_Fds_Apu.h
	nes_apu/Nes_Fme7_Apu.h
	nes_apu/Nes_Mmc5_Apu.h
	nes_apu/Nes_Namco_Apu.h
	nes_apu/Nes_Oscs.h
	nes_apu/Nes_Vrc6_Apu.h
	nes_apu/Nes_Vrc7_Apu.h
	nes_apu/panning.h
)

SET(NES_SND_EMU_DOCS
	LICENSE
	README.md)

ADD_LIBRARY(Nes_Snd_Emu SHARED ${NES_SND_EMU_SOURCES} ${NES_SND_EMU_HEADERS} ${NES_SND_EMU_DOCS})
SET_TARGET_PROPERTIES(Nes_Snd_Emu PROPERTIES VERSION ${VERSION})
TARGET_INCLUDE_DIRECTORIES(Nes_Snd_Emu PUBLIC ${PROJECT_SOURCE_DIR} emu2413)
TARGET_COMPILE_DEFINITIONS(Nes_Snd_Emu PUBLIC BLARGG_DYNAMIC PRIVATE BLARGG_BUILD)
TARGET_COMPILE_FEATURES(Nes_Snd_Emu PUBLIC cxx_std_11)
IF(MSVC)
	# windows.h defines min and max, and std::min and std::max aren't defined in <algorithm> without this macro
	TARGET_COMPILE_DEFINITIONS(Nes_Snd_Emu PRIVATE NOMINMAX)
	# stdlib.h on Windows defines min and max preprocessor macros by default as a "language extensions"
	TARGET_COMPILE_DEFINITIONS(Nes_Snd_Emu PRIVATE _CRT_DECLARE_NONSTDC_NAMES=0)
ENDIF()

FIND_PACKAGE(SDL2)
IF(SDL2_FOUND)
	FIND_PATH(SDL2_INCLUDE_DIR SDL.h PATH_SUFFIXES SDL2)

	# Sound_Queue is not LGPL; it can be static
	ADD_LIBRARY(SDL_Sound_Queue STATIC sound_queue/Sound_Queue.cpp sound_queue/Sound_Queue.h)
	TARGET_INCLUDE_DIRECTORIES(SDL_Sound_Queue PRIVATE ${SDL2_INCLUDE_DIR} INTERFACE ${PROJECT_SOURCE_DIR}/sound_queue)
	TARGET_COMPILE_FEATURES(SDL_Sound_Queue PUBLIC cxx_std_11)

	SET(DEMO_SOURCES demo/demo.cpp demo/Simple_Apu.cpp demo/Simple_Apu.h demo/Wave_Writer.cpp demo/Wave_Writer.hpp)
	ADD_EXECUTABLE(demo ${DEMO_SOURCES})
	TARGET_LINK_LIBRARIES(demo PRIVATE Nes_Snd_Emu SDL_Sound_Queue SDL2::SDL2)
	TARGET_COMPILE_FEATURES(demo PUBLIC cxx_std_11)
ENDIF()
