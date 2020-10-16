#ifndef BLARGG_COMMON_H
#define BLARGG_COMMON_H

typedef const char* blargg_err_t; // 0 on success, otherwise error string

// Success; no error
blargg_err_t const blargg_ok = 0;

/* BLARGG_4CHAR('a','b','c','d') = 'abcd' (four character integer constant).
I don't just use 'abcd' because that's implementation-dependent. */
#define BLARGG_4CHAR( a, b, c, d ) \
	((a&0xFF)*0x1000000 + (b&0xFF)*0x10000 + (c&0xFF)*0x100 + (d&0xFF))

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

#endif
