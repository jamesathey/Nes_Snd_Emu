// Sets up common environment for Shay Green's libraries.
// To change configuration options, modify blargg_config.h, not this file.

// $package
#ifndef BLARGG_COMMON_H
#define BLARGG_COMMON_H

#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <limits.h>

typedef const char* blargg_err_t; // 0 on success, otherwise error string

// Success; no error
blargg_err_t const blargg_ok = 0;

// BLARGG_RESTRICT: equivalent to C99's restrict, where supported
#if __GNUC__ >= 3 || _MSC_VER >= 1100
	#define BLARGG_RESTRICT __restrict
#else
	#define BLARGG_RESTRICT
#endif

/* BLARGG_4CHAR('a','b','c','d') = 'abcd' (four character integer constant).
I don't just use 'abcd' because that's implementation-dependent. */
#define BLARGG_4CHAR( a, b, c, d ) \
	((a&0xFF)*0x1000000 + (b&0xFF)*0x10000 + (c&0xFF)*0x100 + (d&0xFF))

/* Pure virtual functions cause a vtable entry to a "called pure virtual"
error handler, requiring linkage to the C++ runtime library. This macro is
used in place of the "= 0", and simply expands to its argument. During
development, it expands to "= 0", allowing detection of missing overrides. */
#define BLARGG_PURE( def ) def

// User configuration can override the above macros if necessary
#include "blargg_config.h"

#ifdef BLARGG_NAMESPACE
	#define BLARGG_NAMESPACE_BEGIN namespace BLARGG_NAMESPACE {
	#define BLARGG_NAMESPACE_END }

	BLARGG_NAMESPACE_BEGIN
	BLARGG_NAMESPACE_END
	using namespace BLARGG_NAMESPACE;
#else
	#define BLARGG_NAMESPACE_BEGIN
	#define BLARGG_NAMESPACE_END
#endif

BLARGG_NAMESPACE_BEGIN

/* BLARGG_DEPRECATED [_TEXT] for any declarations/text to be removed in a
future version. In GCC, we can let the compiler warn. In other compilers,
we strip it out unless BLARGG_LEGACY is true. */
#if BLARGG_LEGACY
	// Allow old client code to work without warnings
	#define BLARGG_DEPRECATED_TEXT( text ) text
	#define BLARGG_DEPRECATED(      text ) text
#elif __GNUC__ >= 4
	// In GCC, we can mark declarations and let the compiler warn
	#define BLARGG_DEPRECATED_TEXT( text ) text
	#define BLARGG_DEPRECATED(      text ) __attribute__ ((deprecated)) text
#else
	// By default, deprecated items are removed, to avoid use in new code
	#define BLARGG_DEPRECATED_TEXT( text )
	#define BLARGG_DEPRECATED(      text )
#endif

/* My code is not written with exceptions in mind, so either uses new (nothrow)
OR overrides operator new in my classes. The former is best since clients
creating objects will get standard exceptions on failure, but that causes it
to require the standard C++ library. So, when the client is using the C
interface, I override operator new to use malloc. */

// BLARGG_DISABLE_NOTHROW is put inside classes
#ifndef BLARGG_DISABLE_NOTHROW
	// throw spec mandatory in ISO C++ if NULL can be returned
	#if __cplusplus >= 199711 || __GNUC__ >= 3 || _MSC_VER >= 1300
		#define BLARGG_THROWS_NOTHING throw ()
	#else
		#define BLARGG_THROWS_NOTHING
	#endif

	#define BLARGG_DISABLE_NOTHROW \
		void* operator new ( size_t s ) BLARGG_THROWS_NOTHING { return malloc( s ); }\
		void operator delete( void* p ) BLARGG_THROWS_NOTHING { free( p ); }

	#define BLARGG_NEW new
#else
	// BLARGG_NEW is used in place of new in library code
	#include <new>
	#define BLARGG_NEW new (std::nothrow)
#endif

// Callback function with user data.
// blargg_callback<T> set_callback; // for user, this acts like...
// void set_callback( T func, void* user_data = NULL ); // ...this
// To call function, do set_callback.f( .. set_callback.data ... );
template<class T>
struct DLLEXPORT blargg_callback
{
	T f;
	void* data;
	blargg_callback() { f = NULL; }
	void operator () ( T callback, void* user_data = NULL ) { f = callback; data = user_data; }
};

BLARGG_NAMESPACE_END

#endif
