#pragma once

#ifdef NES_SND_DYNAMIC
#if defined _WIN32 || defined __CYGWIN__
  #ifdef NES_SND_BUILD
    #define DLLEXPORT __declspec(dllexport) // mingw-gcc and msvc both support __declspec
  #else
    #define DLLEXPORT __declspec(dllimport) // mingw-gcc and msvc both support __declspec
  #endif
#elif __GNUC__ >= 4
  #define DLLEXPORT __attribute__ ((visibility ("default")))
#else
  #define DLLEXPORT
#endif
#else
  #define DLLEXPORT
#endif
