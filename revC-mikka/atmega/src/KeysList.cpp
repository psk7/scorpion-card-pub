#include "KeysList.h"

KeysList::KeysList() {
    clear();
}

void KeysList::clear() {
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

bool KeysList::contains(uint8_t val) const {
    for (auto &item: keys)
        if (item == val)
            return true;

    return false;
}

bool KeysList::empty() {
    for (auto &item: keys)
        if (item != 0)
            return false;

    return true;
}