#ifndef CH_KEYBOARDMAPPER_H
#define CH_KEYBOARDMAPPER_H

#include "KeysList.h"
#include "ZxKeyboard.h"

void MapKeyboard(bool IsLeftShiftPressed, const KeysList &ScanCodes, uint16_t JoystickBits, KeysList &Target);

#endif //CH_KEYBOARDMAPPER_H
