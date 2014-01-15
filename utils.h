#ifndef _UTILS_H
#define _UTILS_H 1

#include <avr/pgmspace.h>

#define set_bits(reg,mask,value) ((reg) = ((reg) & ~(mask)) | ((value) & (mask)))

//To make getting stuff out of PROGMEM structs a little easier
#define WORD_FIELD(s,f) pgm_read_word(&((s)->f))
#define BYTE_FIELD(s,f) pgm_read_byte(&((s)->f))

#endif