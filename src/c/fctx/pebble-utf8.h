#pragma once
#include <pebble.h>

/* Vendored from pebble-utf8 1.0.4 (c) JR Mobley, MIT license (see LICENSE).
 * Trimmed to the single decode function fctx needs. */

uint16_t utf8_decode_byte(uint8_t byte, uint16_t* state, uint16_t* cp);
