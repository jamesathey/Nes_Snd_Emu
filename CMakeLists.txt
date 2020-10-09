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
	nes_apu/blargg_common.cpp
	nes_apu/blargg_errors.cpp
	nes_apu/Blip_Buffer.cpp
	nes_apu/emu2413.c
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
	nes_apu/2413tone.h
	nes_apu/blargg_common.h
	nes_apu/blargg_errors.h
	nes_apu/blargg_source.h
	nes_apu/Blip_Buffer.h
	nes_apu/Blip_Buffer_impl.h
	nes_apu/Blip_Buffer_impl2.h
	nes_apu/emu2413.h
	nes_apu/emutypes.h
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
	nes_apu/vrc7tone.h
	blargg_config.h
)

SET(NES_SND_EMU_DOCS
	LICENSE
	README.md)

ADD_LIBRARY(Nes_Snd_Emu SHARED ${NES_SND_EMU_SOURCES} ${NES_SND_EMU_HEADERS} ${NES_SND_EMU_DOCS})
SET_TARGET_PROPERTIES(Nes_Snd_Emu PROPERTIES VERSION ${VERSION})
TARGET_INCLUDE_DIRECTORIES(Nes_Snd_Emu PUBLIC ${PROJECT_SOURCE_DIR})
TARGET_COMPILE_DEFINITIONS(Nes_Snd_Emu PUBLIC BLARGG_DYNAMIC PRIVATE BLARGG_BUILD)
IF(MSVC)
	# windows.h defines min and max, and std::min and std::max aren't defined in <algorithm> without this macro
	TARGET_COMPILE_DEFINITIONS(Nes_Snd_Emu PRIVATE NOMINMAX)
ENDIF()

SET_PROPERTY(TARGET Nes_Snd_Emu PROPERTY CXX_STANDARD 11)
SET_PROPERTY(TARGET Nes_Snd_Emu PROPERTY CXX_STANDARD_REQUIRED ON)

FIND_PACKAGE(SDL2)
IF(SDL2_FOUND)
	FIND_PATH(SDL2_INCLUDE_DIR SDL.h PATH_SUFFIXES SDL2)
	# Sound_Queue is not LGPL; it can be static
	ADD_LIBRARY(SDL_Sound_Queue STATIC Sound_Queue.cpp Sound_Queue.h)
	TARGET_INCLUDE_DIRECTORIES(SDL_Sound_Queue PRIVATE ${SDL2_INCLUDE_DIR})
	SET_PROPERTY(TARGET SDL_Sound_Queue PROPERTY CXX_STANDARD 11)
	SET_PROPERTY(TARGET SDL_Sound_Queue PROPERTY CXX_STANDARD_REQUIRED ON)
ENDIF()

SET(DEMO_SOURCES demo.cpp Simple_Apu.cpp Simple_Apu.h Wave_Writer.cpp Wave_Writer.hpp)
ADD_EXECUTABLE(demo ${DEMO_SOURCES})
TARGET_LINK_LIBRARIES(demo PRIVATE Nes_Snd_Emu)
SET_PROPERTY(TARGET demo PROPERTY CXX_STANDARD 11)
SET_PROPERTY(TARGET demo PROPERTY CXX_STANDARD_REQUIRED ON)
