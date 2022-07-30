#ifndef CH_ZXKEYBOARD_H
#define CH_ZXKEYBOARD_H


#ifdef WIN32
#include <cstdint>
#else

#include "stdint.h"
#include <avr/pgmspace.h>

#endif

#include "KeysList.h"

enum ZXKey {
    NoKey = 0,
    CS, Z, X, C, V,
    A, S, D, F, G,

    Q, W, E, R, T,
    _1, _2, _3, _4, _5,

    _0, _9, _8, _7, _6,
    P, O, I, U, Y,

    Ent, L, K, J, H,
    Sp, SS, M, N, B,

    Joy_Right, Joy_Left, Joy_Down, Joy_Up, Joy_Fire,

    _End
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

class ZxKeyboard{
private:
    KeysList Keys;

    void UpdateZxKeys(const KeysList &ZxKeys);

public:
    struct {
        bool WasCS: 1;
        bool WasSS: 1;
        bool IsLeftShiftPressed: 1;
        bool IsLeftCtrlPressed: 1;
        bool IsLeftAltPressed: 1;
        bool IsRightShiftPressed: 1;
        bool IsRightCtrlPressed: 1;
        bool IsRightAltPressed: 1;
        bool IsWinPressed: 1;
        uint8_t JoyMapNum : 2;
    };

    void Init();

    void ParseBootProtocolKeyboardReport(uint8_t *Data);

    bool MapJoystickAndSend(uint16_t JoystickBits, uint8_t ZxJoystickBits);

    void ProcessKeysList(const KeysList &List);

    void KeyPress(uint8_t ScanCode);

    void KeyRelease(uint8_t ScanCode);

    bool IsCtrlPressed() const;

    bool IsAltPressed() const;

    bool IsKeyPressed(uint8_t ScanCode);
};

#ifdef __cplusplus
};
#endif

#endif //CH_ZXKEYBOARD_H
