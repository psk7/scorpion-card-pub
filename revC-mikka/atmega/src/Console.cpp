#include "Console.h"
#include <avr/io.h>

Console CON;

Console::Console() {
    UBRR0L = 16; // 115200 bps
    UBRR0H = 0;
    UCSR0A = _BV(U2X0);
    UCSR0B = _BV(TXEN0);
    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
}

const char digits[] PROGMEM = "0123456789ABCDEF";

inline Console &Console::operator<<(const char Char) {
    while ((UCSR0A & _BV(UDRE0)) == 0);

    UDR0 = Char;

    return *this;
}

Console &Console::operator<<(const HEX<uint8_t> &Value) {
    auto v = Value.value;
    return *this << (char) pgm_read_byte(&digits[(v >> 4) & 0xf]) << (char) pgm_read_byte(&digits[v & 0xf]);
}

Console &Console::operator<<(const HEX<uint16_t> &Value) {
    auto v = Value.value;
    *this << (char) pgm_read_byte(&digits[(v >> 12) & 0xf]) << (char) pgm_read_byte(&digits[(v >> 8) & 0xf]);
    return *this << (char) pgm_read_byte(&digits[(v >> 4) & 0xf]) << (char) pgm_read_byte(&digits[v & 0xf]);
}

Console &Console::operator<<(const DEC &Value) {
    uint16_t v = Value.value;

    char digits[5];

    for (char &digit: digits) {
        digit = '0' + (v % 10);
        v /= 10;
    }

    auto f = false;

    for (int8_t i = 4; i >= 0; --i) {
        auto d = digits[i];
        if (d == '0' && !f)
            continue;

        *this << d;
        f = true;
    }

    return *this;
}

Console &Console::operator<<(const char *String) {
    auto ptr = &String[0];

    char ch;

    while ((ch = pgm_read_byte(ptr++)) != 0)
        *this << ch;

    return *this;
}

Console &Console::operator<<(const Buffer &Value) {
    auto ptr = (char *) Value.Ptr;

    for (uint16_t i = 0; i < Value.Size; ++i)
        *this << HEX((uint8_t)(*(ptr++))) << ' ';

    return *this;
}

Console &Console::operator<<(const endl &Value) {
    *this << (char) 13;
    *this << (char) 10;

    return *this;
}



