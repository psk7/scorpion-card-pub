#include "KeysList.h"

KeysList::KeysList() {
    memset(&keys[0], 0, sizeof keys);
}

KeysList &KeysList::operator<<(uint8_t Key) {
    for (auto &item: keys)
        if (item == 0 || item == Key) {
            item = Key;
            break;
        }

    return *this;
}

KeysList &KeysList::operator+=(uint8_t Key) {
    *this << Key;
    return *this;
}

void KeysList::operator-=(const KeysList &B) {
    for (auto &i: keys)
        if (i != 0)
            for (const auto &j: B.keys)
                if (i == j)
                    i = 0;
}

void KeysList::operator-=(const uint8_t Key) {
    for (auto &item: keys)
        if (item == Key)
            item = 0;
}
