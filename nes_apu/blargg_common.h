// Sets up common environment for Shay Green's libraries.
// To change configuration options, modify blargg_config.h, not this file.

#ifndef BLARGG_COMMON_H
#define BLARGG_COMMON_H

#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <limits.h>

typedef const char* blargg_err_t; // 0 on success, otherwise error string

// Success; no error
blargg_err_t const blargg_ok = 0;

/* BLARGG_4CHAR('a','b','c','d') = 'abcd' (four character integer constant).
I don't just use 'abcd' because that's implementation-dependent. */
#define BLARGG_4CHAR( a, b, c, d ) \
	((a&0xFF)*0x1000000 + (b&0xFF)*0x10000 + (c&0xFF)*0x100 + (d&0xFF))

// User configuration can override the above macros if necessary
#include "blargg_config.h"

#endif
