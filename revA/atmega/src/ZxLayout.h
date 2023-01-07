#ifndef SC_MULTICARD_ZXLAYOUT_H
#define SC_MULTICARD_ZXLAYOUT_H

#ifdef WIN32
#define STORAGE
#else
#include <avr/pgmspace.h>
#define STORAGE PROGMEM
#endif

#endif //SC_MULTICARD_ZXLAYOUT_H
