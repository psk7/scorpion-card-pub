#ifndef SC_MULTICARD_TIMERS_H
#define SC_MULTICARD_TIMERS_H

#ifdef WIN32

#include <Windows.h>
#include <cstdint>

#else
#include <stdint.h>
#endif

#if defined(__cplusplus)
extern "C" {
#endif

void InitializeTimers();
void Timers_Task();

void Timers_SetupUSBDelay(uint8_t DelayMS);
bool Timers_CheckUSBDelay();

#if defined(__cplusplus)
}
#endif

#endif //SC_MULTICARD_TIMERS_H
