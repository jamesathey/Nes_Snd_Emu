/* Included at the beginning of library source files, AFTER all other #include
lines. Sets up helpful macros and services used in my source code. Since this
is only "active" in my source code, I don't have to worry about polluting the
global namespace with unprefixed names. */

// $package
#ifndef BLARGG_SOURCE_H
#define BLARGG_SOURCE_H

#ifndef BLARGG_COMMON_H // optimization only
	#include "blargg_common.h"
#endif
#include "blargg_errors.h"
#include "gme_custom_dprintf.h"

#include <string.h> /* memcpy(), memset(), memmove() */
#include <stddef.h> /* offsetof() */

/* The following four macros are for debugging only. Some or all might be
defined to do nothing, depending on the circumstances. Described is what
happens when a particular macro is defined to do something. When defined to
do nothing, the macros do NOT evaluate their argument(s). */

/* If expr is false, prints file and line number, then aborts program. Meant
for checking internal state and consistency. A failed assertion indicates a bug
in MY code.

void assert( bool expr ); */
#include <assert.h>

/* If expr is false, prints file and line number, then aborts program. Meant
for checking caller-supplied parameters and operations that are outside the
control of the module. A failed requirement probably indicates a bug in YOUR
code.

void require( bool expr ); */
#undef  require
#define require( expr ) assert( expr )

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

/* If expr yields non-NULL error string, returns it from current function,
otherwise continues normally. */
#undef  RETURN_ERR
#define RETURN_ERR( expr ) \
	do {\
		blargg_err_t blargg_return_err_ = (expr);\
		if ( blargg_return_err_ )\
			return blargg_return_err_;\
	} while ( 0 )

/* If ptr is NULL, returns out-of-memory error, otherwise continues normally. */
#undef  CHECK_ALLOC
#define CHECK_ALLOC( ptr ) \
	do {\
		if ( !(ptr) )\
			return blargg_err_memory;\
	} while ( 0 )

#if BLARGG_LEGACY
	#define BLARGG_CHECK_ALLOC CHECK_ALLOC
	#define BLARGG_RETURN_ERR  RETURN_ERR
#endif

/* BLARGG_SOURCE_BEGIN: If defined, #included, allowing redefition of dprintf etc.
and check */
#ifdef BLARGG_SOURCE_BEGIN
	#include BLARGG_SOURCE_BEGIN
#endif

#endif
