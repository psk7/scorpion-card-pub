#ifndef SC_MULTICARD_TIMERS_H
#define SC_MULTICARD_TIMERS_H

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

void InitializeTimers();
void Timers_Task();

void Timers_SetupUSBDelay(uint8_t DelayMS);
int8_t Timers_CheckUSBDelay();

void Timers_SetupUSBTaskDelay(uint8_t DelayMS);
int8_t Timers_CheckUSBTaskDelay();

#if defined(__cplusplus)
}
#endif

#endif //SC_MULTICARD_TIMERS_H
