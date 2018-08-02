#ifndef BLARGG_CONFIG_H
#define BLARGG_CONFIG_H

#ifdef BLARGG_DYNAMIC
#if defined(_WIN32) && defined(_MSC_VER)
#ifdef BLARGG_BUILD
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport)
#endif // BLARGG_BUILD
#else
#define DLLEXPORT
#endif // defined(_WIN32) && defined(_MSC_VER)
#else
#define DLLEXPORT
#endif // BLARGG_DYNAMIC

#define HAVE_STDINT_H

#endif // BLARGG_CONFIG_H
