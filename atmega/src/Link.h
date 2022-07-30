#ifndef CH_LINK_H
#define CH_LINK_H

#ifdef WIN32
#include <Windows.h>
#include <cstdint>
#include <vector>
typedef std::vector<UINT8> Bytes;
extern HANDLE hComm;
#else

#include <stdint.h>

#endif

#if defined(__cplusplus)
uint8_t WriteUSBControl(uint8_t (&b)[8]);
uint8_t ReadUSB(uint8_t reg);
uint8_t WriteUSB(uint8_t reg, uint8_t value);
uint8_t WriteUSBControl(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6, uint8_t b7, uint8_t b8);
uint8_t WriteUSB(uint8_t reg, uint8_t size, uint8_t *buffer);

extern "C" {
#endif

void InitializeLinks();

uint8_t WriteSync(uint8_t val);
uint8_t ReadUSB(uint8_t reg, uint8_t size, uint8_t *buffer);
uint8_t WriteUSB(uint8_t reg, uint8_t size, uint8_t *rd_buf, uint8_t *rdbuffer);
void WriteZxKeyboard(uint16_t Value);
uint16_t WriteExt(uint16_t Value);
uint16_t WriteExtAndReadBack(uint16_t Value);

#if defined(__cplusplus)
}
#endif

#endif //CH_LINK_H
