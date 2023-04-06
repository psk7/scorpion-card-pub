#ifndef CH_KEYSLIST_H
#define CH_KEYSLIST_H

#include <stdint.h>
#include <string.h>

#define KEYLIST_MAX_SIZE 16

class KeysList {
private:
    uint8_t keys[KEYLIST_MAX_SIZE];

public:
    class iterator {
    private:
        uint8_t *ptr;

    public:
        iterator(const uint8_t *p) : ptr(const_cast<uint8_t *>(p)) {};

        bool operator!=(const iterator &B) { return ptr != B.ptr; }

        void operator++() { ptr++; };

        uint8_t operator*() { return *ptr; };
    };

    KeysList();

    iterator begin() const { return {&keys[0]}; }

    iterator end() const { return {&keys[KEYLIST_MAX_SIZE]}; }

    KeysList &operator<<(uint8_t Key);
    KeysList &operator+=(uint8_t Key);

    void operator-=(const KeysList &B);

    void operator-=(uint8_t Key);

    void clear();

    bool contains(uint8_t val) const;

    bool empty();
};


#endif //CH_KEYSLIST_H
