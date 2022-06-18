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

uint8_t WriteSync(uint8_t val);

uint8_t WriteUSBControl(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6, uint8_t b7, uint8_t b8);

uint8_t WriteUSBControl(uint8_t (&b)[8]);

uint8_t WriteUSB(uint8_t reg, uint8_t value);

uint8_t WriteUSB(uint8_t reg, uint8_t size, uint8_t *buffer);

uint8_t ReadUSB(uint8_t reg, uint8_t size, uint8_t *buffer);

uint8_t ReadUSB(uint8_t reg);

void StartUSB();
void RunUSB(uint8_t *dbg, bool devices, bool kbdout);

void WriteZxKeyboard(uint16_t Value);

void WriteExt(uint16_t Value);

#endif //CH_LINK_H
