#ifndef STMFW_LARGE_SPI_H
#define STMFW_LARGE_SPI_H

#include <stdint.h>

#define NOINLINE __attribute__ ((noinline))

namespace SPI {
    void Init();

    uint8_t NOINLINE ReadUSB(uint8_t reg);

    void NOINLINE ReadUSB(uint8_t reg, uint8_t size, uint8_t *buffer);

    void NOINLINE WriteUSB(uint8_t reg, uint8_t value);

    void NOINLINE WriteUSB(uint8_t reg, uint8_t size, uint8_t *buffer);

    void NOINLINE WriteUSBControl(uint8_t *b);

    void NOINLINE WriteZxKeyboard(uint16_t Data);
}

#endif //STMFW_LARGE_SPI_H
