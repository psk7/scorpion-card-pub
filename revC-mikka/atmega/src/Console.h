#ifndef SC_MULTICARD_CONSOLE_H
#define SC_MULTICARD_CONSOLE_H

#include "avr/pgmspace.h"

class Console;

class endl {
public:
    endl() {}
};

template<typename T>
class HEX {
    friend Console;
private:
    T value;
public:
    HEX(T Value) : value(Value) {}
};

class DEC {
    friend Console;
private:
    uint16_t value;
public:
    DEC(uint16_t Value) : value(Value) {}
};

class Buffer {
    friend Console;
private:
    void *Ptr;
    uint16_t Size;
public:
    Buffer(void *Ptr, uint16_t Size) : Ptr(Ptr), Size(Size) {}
};

class Console {
public:
    Console();

    inline Console &operator<<(char Char);

    Console &operator<<(const char *String);

    Console &operator<<(const Buffer &Value);

    Console &operator<<(const HEX<uint8_t> &Value);

    Console &operator<<(const HEX<uint16_t> &Value);

    Console &operator<<(const DEC &Value);

    Console &operator<<(const endl &Value);
};

extern Console CON;

#endif //SC_MULTICARD_CONSOLE_H
