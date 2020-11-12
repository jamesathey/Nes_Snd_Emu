/* Included at the beginning of library source files, AFTER all other #include
lines. Sets up helpful macros and services used in my source code. Since this
is only "active" in my source code, I don't have to worry about polluting the
global namespace with unprefixed names. */

#ifndef BLARGG_SOURCE_H
#define BLARGG_SOURCE_H

#ifndef BLARGG_COMMON_H // optimization only
	#include "blargg_common.h"
#endif
#include "gme_custom_dprintf.h"

/* Like printf() except output goes to debugging console/file.

void dprintf( const char format [], ... ); */

#ifdef CUSTOM_DPRINTF_FUNCTION

static inline void dprintf( const char * fmt, ... )
{
	if (gme_custom_dprintf)
	{
		va_list vl;
		va_start(vl, fmt);
		gme_custom_dprintf(fmt, vl);
		va_end(vl);
	}
}

#else

#ifdef NDEBUG
static inline void blargg_dprintf_( const char [], ... ) { }
#undef  dprintf
#define dprintf (1) ? (void) 0 : blargg_dprintf_
#else
#include <stdarg.h>
#include <stdio.h>
#undef  dprintf
#define dprintf (1) ? (void) 0 : blargg_dprintf_
#ifndef _WIN32
#include <stdio.h>
static inline void blargg_dprintf_( const char * fmt, ... )
{
	char error[512];
	va_list vl;
	va_start(vl, fmt);
	vsnprintf( error, 511, fmt, vl );
	va_end(vl);
	fputs( error, stderr );
}
#else
#include <windows.h>
static inline void blargg_dprintf_( const char * fmt, ... )
{
	char error[512];
	va_list vl;
	va_start(vl, fmt);
	vsnprintf_s( error, 511, 511, fmt, vl );
	va_end(vl);
	OutputDebugStringA( error );
}
#endif
#endif

#endif

/* If expr is false, prints file and line number to debug console/log, then
continues execution normally. Meant for flagging potential problems or things
that should be looked into, but that aren't serious problems.

void check( bool expr ); */
#undef  check
#define check( expr ) ((void) 0)

/* BLARGG_SOURCE_BEGIN: If defined, #included, allowing redefition of dprintf etc.
and check */
#ifdef BLARGG_SOURCE_BEGIN
	#include BLARGG_SOURCE_BEGIN
#endif

#endif
