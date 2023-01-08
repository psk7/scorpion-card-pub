#include "SPI.h"
#include "avr/io.h"
#include "util/delay.h"


//#define DD_SPI_SS DDB0
#define DD_SPI_SCK DDB5
#define DD_SPI_MOSI DDB3

#define SPI_DDR DDRB

#define SPI_KBD_SS_DDR DDRC
#define SPI_KBD_SS_PORT PORTC
#define SPI_KBD_SS PORTC3
#define SPI_DD_KBD_SS DDC3

#define SPI_USB_SS_DDR DDRC
#define SPI_USB_SS_PORT PORTC
#define SPI_USB_SS PORTC5
#define SPI_DD_USB_SS DDC5

#define SPI_USB_URES_DDR DDRC
#define SPI_USB_URES_PORT PORTC
#define SPI_USB_URES PORTC4
#define SPI_DD_USB_URES DDC4

void SPI::Init() {
//    SPI_DDR |= _BV(DD_SPI_SS);   // SS - output
    SPI_DDR |= _BV(DD_SPI_SCK);  // SCK - output
    SPI_DDR |= _BV(DD_SPI_MOSI); // MOSI - output

    SPI_KBD_SS_DDR |= _BV(SPI_DD_KBD_SS);    // KBD SS - output
    SPI_KBD_SS_PORT |= _BV(SPI_KBD_SS);      // KBD SS - High

    SPI_USB_SS_DDR |= _BV(SPI_DD_USB_SS);    // USB SS - output
    SPI_USB_SS_PORT |= _BV(SPI_USB_SS);      // USB SS - High

    SPI_USB_URES_DDR |= _BV(SPI_DD_USB_URES);    // USB Res - output
    SPI_USB_URES_PORT |= _BV(SPI_USB_URES);      // USB Res - High

    SPSR |= _BV(SPI2X);
}

void SPI::WriteUSBControl(uint8_t *b) {
    // SPI mode == 0
    // SPI clock == 4 MHz
    // SPI enabled
    SPCR = _BV(MSTR) | _BV(SPE);

    SPI_USB_SS_PORT &= ~_BV(SPI_USB_SS);      // USB SS - low
    SPDR = (4 << 3) | 0x2;
    while (!(SPSR & _BV(SPIF)));
    uint8_t rval = SPDR;
    uint8_t *ptr = &b[0];

    for (uint8_t i = 0; i < 8; i++) {
        SPDR = *(ptr++);
        while (!(SPSR & _BV(SPIF)));
    }

    SPI_USB_SS_PORT |= _BV(SPI_USB_SS);      // USB SS - High
}

void SPI::WriteUSB(uint8_t reg, uint8_t value) {
    // SPI mode == 0
    // SPI clock == 4 MHz
    // SPI enabled
    SPCR = _BV(MSTR) | _BV(SPE);

    SPI_USB_SS_PORT &= ~_BV(SPI_USB_SS);      // USB SS - low

    SPDR = ((reg << 3) | 0x2) & 0xff;
    while (!(SPSR & _BV(SPIF)));

    uint8_t rval = SPDR;

    SPDR = value;
    while (!(SPSR & _BV(SPIF)));

    SPI_USB_SS_PORT |= _BV(SPI_USB_SS);      // USB SS - High
}

void SPI::WriteUSB(uint8_t reg, uint8_t size, uint8_t *buffer) {
    // SPI mode == 0
    // SPI clock == 4 MHz
    // SPI enabled
    SPCR = _BV(MSTR) | _BV(SPE);

    SPI_USB_SS_PORT &= ~_BV(SPI_USB_SS);      // USB SS - low

    SPDR = ((reg << 3) | 0x2) & 0xff;
    while (!(SPSR & _BV(SPIF)));

    uint8_t rval = SPDR;

    for (uint8_t i = 0; i < size; ++i) {
        SPDR = *buffer;
        while (!(SPSR & _BV(SPIF)));
        *(buffer++) = SPDR;
    }

    SPI_USB_SS_PORT |= _BV(SPI_USB_SS);      // USB SS - High
}

void SPI::ReadUSB(uint8_t reg, uint8_t size, uint8_t *buffer) {
    // SPI mode == 0
    // SPI clock == 4 MHz
    // SPI enabled
    SPCR = _BV(MSTR) | _BV(SPE);

    SPI_USB_SS_PORT &= ~_BV(SPI_USB_SS);      // USB SS - low
    SPDR = (reg << 3) & 0xff;
    while (!(SPSR & _BV(SPIF)));
    uint8_t rval = SPDR;

    for (uint8_t i = 0; i < size; ++i) {
        SPDR = 0;
        while (!(SPSR & _BV(SPIF)));
        *(buffer++) = SPDR;
    }

    SPI_USB_SS_PORT |= _BV(SPI_USB_SS);      // USB SS - High
}

uint8_t SPI::ReadUSB(uint8_t reg) {
    // SPI mode == 0
    // SPI clock == 4 MHz
    // SPI enabled
    SPCR = _BV(MSTR) | _BV(SPE);

    SPI_USB_SS_PORT &= ~_BV(SPI_USB_SS);      // USB SS - low

    SPDR = (reg << 3) & 0xff;
    while (!(SPSR & _BV(SPIF)));

    SPDR = 0;
    while (!(SPSR & _BV(SPIF)));

    uint8_t rval = SPDR;

    SPI_USB_SS_PORT |= _BV(SPI_USB_SS);      // USB SS - High

    return rval;
}

void SPI::WriteZxKeyboard(uint16_t Value) {
    // SPI mode == 1
    // SPI clock == 4 MHz
    // SPI enabled
    SPCR = _BV(MSTR) | _BV(CPHA) | _BV(SPE);

    SPI_KBD_SS_PORT &= ~_BV(SPI_KBD_SS);      // Sync SS - low
    SPDR = (Value >> 8) & 0xff;
    while (!(SPSR & _BV(SPIF)));
    SPDR = Value & 0xff;
    while (!(SPSR & _BV(SPIF)));
    SPI_KBD_SS_PORT |= _BV(SPI_KBD_SS);      // Sync SS - High
}
