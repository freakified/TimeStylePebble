
#include <pebble.h>
#include "pebble-utf8.h"

uint16_t utf8_decode_byte(uint8_t byte, uint16_t* state, uint16_t* cp) {

    /* unicode code points are encoded as follows.
     * U+00000000 – U+0000007F: 0xxxxxxx
     * U+00000080 – U+000007FF: 110xxxxx 10xxxxxx
     * U+00000800 – U+0000FFFF: 1110xxxx 10xxxxxx 10xxxxxx
     * U+00010000 – U+001FFFFF: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
     * U+00200000 – U+03FFFFFF: 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
     * U+04000000 – U+7FFFFFFF: 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
     */

   if (*state == 0 || *state == 6) {
       if (byte < 0b10000000) {        // (<128) ascii character
           *cp = byte;
       } else if (byte < 0b11000000) { // (<192) unexpected continuation byte
           *cp = 0;
           *state = 6;
       } else if (byte < 0b11100000) { // (<224) 2 byte sequence
           *cp = byte & 0b00011111;
           *state = 1;
       } else if (byte < 0b11110000) { // (<240) 3 byte sequence
           *cp = byte & 0b00001111;
           *state = 2;
       } else if (byte < 0b11111000) { // (<248) 4 byte sequence
           *cp = byte & 0b00000111;
           *state = 3;
       } else if (byte < 0b11111100) { // (<252) 5 byte sequence
           *cp = byte & 0b00000011;
           *state = 4;
       } else if (byte < 0b11111110) { // (<254) 6 byte sequence
           *cp = byte & 0b00000001;
           *state = 5;
       }
   } else if (*state < 6) {
       if (byte < 0b11000000) {
           *cp = (*cp << 6) | (byte & 0b00111111);
           *state = *state - 1;
       } else {
           *cp = 0;
           *state = 6;
       }
   }
   return *state;
}
