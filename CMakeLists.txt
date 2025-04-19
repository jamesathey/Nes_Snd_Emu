CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

PROJECT(Nes_Snd_Emu)

SET(TARGET_NAME Nes_Snd_Emu)

SET(PACKAGE "Nes_Snd_Emu")
SET(PACKAGE_NAME ${PACKAGE})
SET(PACKAGE_VERSION_MAJOR "0")
SET(PACKAGE_VERSION_MINOR "3")
SET(PACKAGE_VERSION_PATCH "0")
SET(VERSION "${PACKAGE_VERSION_MAJOR}.${PACKAGE_VERSION_MINOR}.${PACKAGE_VERSION_PATCH}")
SET(PACKAGE_VERSION ${VERSION})
SET(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")

# Targets
SET(NES_SND_EMU_SOURCES
	emu2413/emu2413.c
	nes_apu/Blip_Buffer.cpp
	nes_apu/Multi_Buffer.cpp
	nes_apu/Nes_Apu.cpp
	nes_apu/Nes_Fds_Apu.cpp
	nes_apu/Nes_Fme7_Apu.cpp
	nes_apu/Nes_Mmc5_Apu.cpp
	nes_apu/Nes_Namco_Apu.cpp
	nes_apu/Nes_Oscs.cpp
	nes_apu/Nes_Vrc6_Apu.cpp
	nes_apu/Nes_Vrc7_Apu.cpp
)

SET(NES_SND_EMU_HEADERS
	nes_apu/Blip_Buffer.h
	nes_apu/Blip_Buffer_impl.h
	nes_apu/Blip_Buffer_impl2.h
	nes_apu/dllexport.h
	nes_apu/Multi_Buffer.h
	nes_apu/Nes_Apu.h
	nes_apu/Nes_Apu_Base.h
	nes_apu/Nes_Fds_Apu.h
	nes_apu/Nes_Fme7_Apu.h
	nes_apu/Nes_Mmc5_Apu.h
	nes_apu/Nes_Namco_Apu.h
	nes_apu/Nes_Oscs.h
	nes_apu/Nes_Vrc6_Apu.h
	nes_apu/Nes_Vrc7_Apu.h
)

SET(NES_SND_EMU_PRIVATE_HEADERS
	emu2413/emu2413.h
)

SET(NES_SND_EMU_DOCS
	LICENSE
	README.md)

ADD_LIBRARY(Nes_Snd_Emu SHARED ${NES_SND_EMU_SOURCES} ${NES_SND_EMU_HEADERS} ${NES_SND_EMU_PRIVATE_HEADERS} ${NES_SND_EMU_DOCS})
SET_TARGET_PROPERTIES(Nes_Snd_Emu PROPERTIES VERSION ${VERSION})
TARGET_INCLUDE_DIRECTORIES(Nes_Snd_Emu
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/nes_apu>
		$<INSTALL_INTERFACE:nes_apu>
	PRIVATE
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/emu2413>)

TARGET_COMPILE_DEFINITIONS(Nes_Snd_Emu PUBLIC NES_SND_DYNAMIC PRIVATE NES_SND_BUILD)
TARGET_COMPILE_FEATURES(Nes_Snd_Emu PUBLIC cxx_std_11)
IF(MSVC)
	# windows.h defines min and max, and std::min and std::max aren't defined in <algorithm> without this macro
	TARGET_COMPILE_DEFINITIONS(Nes_Snd_Emu PRIVATE NOMINMAX)
	# stdlib.h on Windows defines min and max preprocessor macros by default as a "language extensions"
	TARGET_COMPILE_DEFINITIONS(Nes_Snd_Emu PRIVATE _CRT_DECLARE_NONSTDC_NAMES=0)
ENDIF()

set(NES_SND_EMU_BUILD_DEMO "OFF" CACHE BOOL "Build demo executable")

FIND_PACKAGE(SDL2 CONFIG)
IF(SDL2_FOUND)
	FIND_PATH(SDL2_INCLUDE_DIR SDL.h PATH_SUFFIXES SDL2)

	# Sound_Queue is not LGPL; it can be static
	ADD_LIBRARY(SDL_Sound_Queue STATIC sound_queue/Sound_Queue.cpp sound_queue/Sound_Queue.h)
	TARGET_LINK_LIBRARIES(SDL_Sound_Queue PRIVATE SDL2::SDL2)
	TARGET_INCLUDE_DIRECTORIES(SDL_Sound_Queue INTERFACE ${PROJECT_SOURCE_DIR}/sound_queue)
	TARGET_COMPILE_FEATURES(SDL_Sound_Queue PUBLIC cxx_std_11)

	IF(NES_SND_EMU_BUILD_DEMO)
		SET(DEMO_SOURCES demo/demo.cpp demo/Simple_Apu.cpp demo/Simple_Apu.h demo/Wave_Writer.cpp demo/Wave_Writer.hpp)
		ADD_EXECUTABLE(demo ${DEMO_SOURCES})
		TARGET_LINK_LIBRARIES(demo PRIVATE Nes_Snd_Emu SDL_Sound_Queue)
		TARGET_COMPILE_FEATURES(demo PUBLIC cxx_std_11)
	ENDIF()
ENDIF()


set_target_properties(${TARGET_NAME}
	PROPERTIES
		PUBLIC_HEADER "${NES_SND_EMU_HEADERS}"
		VERSION ${PACKAGE_VERSION}
		SOVERSION ${PACKAGE_VERSION_MAJOR}
)

include(GNUInstallDirs)

INSTALL(
	TARGETS ${TARGET_NAME}
	EXPORT ${TARGET_NAME}-export
	LIBRARY DESTINATION ${LIB_INSTALL_DIR}
	ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/nes_apu
)

install(EXPORT ${TARGET_NAME}-export
	DESTINATION lib/cmake/${TARGET_NAME}
	NAMESPACE ${TARGET_NAME}::
	FILE ${TARGET_NAME}-config.cmake
)
