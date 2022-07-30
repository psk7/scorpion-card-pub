#include "Link.h"
#include "avr/io.h"
#include "util/delay.h"

#define SPI_SS PORTB0
#define SPI_SCK PORTB1
#define SPI_MOSI PORTB2
#define SPI_MISO PORTB3

#define DD_SPI_SS DDB0
#define DD_SPI_SCK DDB1
#define DD_SPI_MOSI DDB2
#define DD_SPI_MISO DDB3

#define SPI PORTB
#define SPI_DDR DDRB

#define SPI_SYNC_SS_DDR DDRB
#define SPI_SYNC_SS_PORT PORTB
#define SPI_SYNC_SS PORTB5
#define SPI_DD_SYNC_SS DDB5

#define SPI_KBD_SS_DDR DDRB
#define SPI_KBD_SS_PORT PORTB
#define SPI_KBD_SS PORTB7
#define SPI_DD_KBD_SS DDB7

#define SPI_USB_SS_DDR DDRB
#define SPI_USB_SS_PORT PORTB
#define SPI_USB_SS PORTB4
#define SPI_DD_USB_SS DDB4

#define SPI_USB_URES_DDR DDRB
#define SPI_USB_URES_PORT PORTB
#define SPI_USB_URES PORTB6
#define SPI_DD_USB_URES DDB6

#define SPI_EXT_SS_DDR DDRC
#define SPI_EXT_SS_PORT PORTC
#define SPI_EXT_SS PORTC6
#define SPI_DD_EXT_SS DDC6

void InitializeLinks() {
    SPI_DDR |= _BV(DD_SPI_SS);   // SS - output
    SPI_DDR |= _BV(DD_SPI_SCK);  // SCK - output
    SPI_DDR |= _BV(DD_SPI_MOSI); // MOSI - output

    SPI_SYNC_SS_DDR |= _BV(SPI_DD_SYNC_SS);    // Sync SS - output
    SPI_SYNC_SS_PORT |= _BV(SPI_SYNC_SS);      // Sync SS - High

    SPI_KBD_SS_DDR |= _BV(SPI_DD_KBD_SS);    // KBD SS - output
    SPI_KBD_SS_PORT |= _BV(SPI_KBD_SS);      // KBD SS - High

    SPI_USB_SS_DDR |= _BV(SPI_DD_USB_SS);    // USB SS - output
    SPI_USB_SS_PORT |= _BV(SPI_USB_SS);      // USB SS - High

    SPI_USB_URES_DDR |= _BV(SPI_DD_USB_URES);    // USB Res - output
    SPI_USB_URES_PORT |= _BV(SPI_USB_URES);      // USB Res - High

    SPI_EXT_SS_DDR |= _BV(SPI_DD_EXT_SS);    // EXT SS - output
    SPI_EXT_SS_PORT |= _BV(SPI_EXT_SS);      // EXT SS - High

    PORTD &= ~_BV(PORTD3);                      // Hard reset Z state
    DDRD &= ~_BV(DDD3);                         // Hard reset Z state

    PORTD &= ~_BV(PORTD7);
    DDRD &= ~_BV(DDD7);                         // NMI Z state

    PORTD &= ~_BV(PORTD6);
    DDRD &= ~_BV(DDD6);                         // Turbo switch Z state

    DDRC &= ~_BV(DDC7);                         // Ext interrupt input
    PORTC |= _BV(PORTC7);                       // Ext interrupt pulled up
}

uint8_t WriteSync(uint8_t val) {
    // SPI mode == 1
    // SPI clock == 4 MHz
    // SPI enabled
    SPCR = _BV(MSTR) | _BV(CPHA) | _BV(SPE);

    SPI_SYNC_SS_PORT &= ~_BV(SPI_SYNC_SS);      // Sync SS - low
    SPDR = val;
    while (!(SPSR & _BV(SPIF)));
    uint8_t rval = SPDR;
    SPI_SYNC_SS_PORT |= _BV(SPI_SYNC_SS);      // Sync SS - High

    return rval;
}

uint8_t WriteUSBControl(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6, uint8_t b7,
                        uint8_t b8) {
    uint8_t buf[8] = {b1, b2, b3, b4, b5, b6, b7, b8};

    // SPI mode == 0
    // SPI clock == 4 MHz
    // SPI enabled
    SPCR = _BV(MSTR) | _BV(SPE);

    SPI_USB_SS_PORT &= ~_BV(SPI_USB_SS);      // USB SS - low
    SPDR = (4 << 3) | 0x2;
    while (!(SPSR & _BV(SPIF)));
    uint8_t rval = SPDR;
    uint8_t *ptr = &buf[0];

    for (uint8_t i = 0; i < 8; i++) {
        SPDR = *(ptr++);
        while (!(SPSR & _BV(SPIF)));
    }

    SPI_USB_SS_PORT |= _BV(SPI_USB_SS);      // USB SS - High

    return rval;
}

uint8_t WriteUSBControl(uint8_t (&b)[8]) {
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

    return rval;
}

uint8_t WriteUSB(uint8_t reg, uint8_t value) {
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

    return rval;
}

uint8_t WriteUSB(uint8_t reg, uint8_t size, uint8_t *wr_buf, uint8_t *rd_buf) {
    // SPI mode == 0
    // SPI clock == 4 MHz
    // SPI enabled
    SPCR = _BV(MSTR) | _BV(SPE);

    SPI_USB_SS_PORT &= ~_BV(SPI_USB_SS);      // USB SS - low
    SPDR = ((reg << 3) | 0x2) & 0xff;
    while (!(SPSR & _BV(SPIF)));
    uint8_t rval = SPDR;

    for (uint8_t i = 0; i < size; ++i) {
        SPDR = *(wr_buf++);
        while (!(SPSR & _BV(SPIF)));
        *(rd_buf++) = SPDR;
    }

    SPI_USB_SS_PORT |= _BV(SPI_USB_SS);      // USB SS - High

    return rval;
}

uint8_t WriteUSB(uint8_t reg, uint8_t size, uint8_t *buffer) {
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

    return rval;
}

uint8_t ReadUSB(uint8_t reg, uint8_t size, uint8_t *buffer) {
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

    return rval;
}

uint8_t ReadUSB(uint8_t reg) {
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

void WriteZxKeyboard(uint16_t Value) {
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

uint16_t WriteExt(uint16_t Value) {
    // SPI mode == 1
    // SPI clock == 1 MHz
    // SPI enabled
    SPCR = _BV(MSTR) | _BV(CPHA) | _BV(SPE) | _BV(SPR0);

    uint16_t r;

    SPI_EXT_SS_PORT &= ~_BV(SPI_EXT_SS);      // EXT SS - low
    _delay_us(10);

    SPDR = (Value >> 8) & 0xff;
    while (!(SPSR & _BV(SPIF)));
    r = SPDR << 8;

    SPDR = Value & 0xff;
    while (!(SPSR & _BV(SPIF)));
    r |= SPDR;

    _delay_us(10);
    SPI_EXT_SS_PORT |= _BV(SPI_EXT_SS);      // EXT SS - High

    _delay_us(50);

    return r;
}

uint16_t WriteExtAndReadBack(uint16_t Value) {
    // SPI mode == 1
    // SPI clock == 1 MHz
    // SPI enabled
    SPCR = _BV(MSTR) | _BV(CPHA) | _BV(SPE) | _BV(SPR0);

    uint16_t r;

    SPI_EXT_SS_PORT &= ~_BV(SPI_EXT_SS);      // EXT SS - low
    _delay_us(10);

    SPDR = (Value >> 8) & 0xff;
    while (!(SPSR & _BV(SPIF)));

    SPDR = Value & 0xff;
    while (!(SPSR & _BV(SPIF)));

    SPDR = (Value >> 8) & 0xff;
    while (!(SPSR & _BV(SPIF)));
    r = SPDR << 8;

    SPDR = Value & 0xff;
    while (!(SPSR & _BV(SPIF)));
    r |= SPDR;

    _delay_us(10);
    SPI_EXT_SS_PORT |= _BV(SPI_EXT_SS);      // EXT SS - High

    _delay_us(50);

    return r;
}
