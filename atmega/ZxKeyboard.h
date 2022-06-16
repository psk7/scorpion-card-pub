#ifndef CH_ZXKEYBOARD_H
#define CH_ZXKEYBOARD_H


#ifdef WIN32
#include <cstdint>
#define STORAGE
#else

#include "stdint.h"
#include <avr/pgmspace.h>

#define STORAGE PROGMEM
#endif

enum ZXKey {
    NoKey = 0,
    CS, Z, X, C, V,
    A, S, D, F, G,

    Q, W, E, R, T,
    _1, _2, _3, _4, _5,

    _0, _9, _8, _7, _6,
    P, O, I, U, Y,

    Ent, L, K, J, H,
    Sp, SS, M, N, B
};

#define SYM_SHIFT 0x40
#define CAPS_SHIFT 0x80

struct ZxLayoutKeyRef {
    //ZxLayoutKeyRef() : Key1(0), Key2(0) {}
    //ZxLayoutKeyRef(uint16_t Val) : Key1(Val & 0xff), Key2((Val >> 8) & 0xff) {}

    uint8_t Key1 = 0;
    uint8_t Key2 = 0;
};

extern const ZxLayoutKeyRef NormalLayout[256];
extern const ZxLayoutKeyRef LeftShiftLayout[256];

#ifdef __cplusplus
extern "C" {
#endif

void ZxKeyboard_Init();

void ZxKeyboard_ProcessKeyPress(uint8_t ScanCode);

void ZxKeyboard_ProcessKeyRelease(uint8_t ScanCode);

void ZxKeyboard_ParseBootProtocolKeyboardReport(uint8_t *Data);

#ifdef __cplusplus
};
#endif

#endif //CH_ZXKEYBOARD_H
