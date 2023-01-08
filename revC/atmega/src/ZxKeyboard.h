#ifndef CH_ZXKEYBOARD_H
#define CH_ZXKEYBOARD_H

#include "stdint.h"
#include <avr/pgmspace.h>

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
    //uint8_t Key2 = 0;
};

struct ZxLayoutReference {
    const ZxLayoutKeyRef *BaseLayout;
    const ZxLayoutKeyRef *LeftShiftLayout;
    const ZxLayoutKeyRef *RusLayout;
    const ZxLayoutKeyRef *LeftShiftRusLayout;
    const ZxLayoutKeyRef *FallbackLayout;
};

extern const ZxLayoutReference LayoutReferences[4];

#ifdef __cplusplus
extern "C" {
#endif

class ZxKeyboard {
private:
    KeysList Keys, Pressed, Released;

    struct ZxLayoutReference zxLayoutReference;

    void UpdateZxKeys(const KeysList &ZxKeys);

    void RereadLayout(uint8_t LayoutNum);

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
        uint8_t JoyMapNum: 2;
        bool CapsLock: 1;
        bool NumLock: 1;
        bool RusLayout: 1;
    };

public:
    void Init();

    void ParseBootProtocolKeyboardReport(uint8_t *Data);

    bool MapJoystickAndSend(uint16_t JoystickBits, uint8_t ZxJoystickBits);

    void KeyPress(uint8_t ScanCode);

    void KeyRelease(uint8_t ScanCode);

    bool IsCtrlPressed() const;

    bool IsAltPressed() const;

    bool IsKeyPressed(uint8_t ScanCode);

    bool CapsLockStatus();

    bool NumLockStatus();

    bool RusLayoutStatus();
};

#ifdef __cplusplus
};
#endif

#endif //CH_ZXKEYBOARD_H
