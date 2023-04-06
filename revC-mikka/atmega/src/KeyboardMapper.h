#ifndef CH_KEYBOARDMAPPER_H
#define CH_KEYBOARDMAPPER_H

#include "KeysList.h"
#include "ZxKeyboard.h"
#include "EEPROM.h"
#include "ZxLayout.h"

class KeyboardMapper {
    KeysList &ZxKeys;
    const ZxLayoutReference &Layout;

public:
    KeyboardMapper() = delete;
    explicit KeyboardMapper(KeysList &ZxKeys, const ZxLayoutReference &Layout);

    void MapKeyboard(bool IsLeftShiftPressed, bool RusLayout, const KeysList &ScanCodes);
};

#endif //CH_KEYBOARDMAPPER_H
