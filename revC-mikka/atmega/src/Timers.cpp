#include "Timers.h"
#include <avr/io.h>

static uint8_t usb_counter;
static uint8_t usbtask_counter;
static uint8_t int_flags;

#define USB_INT_FLAG 1
#define USBTASK_INT_FLAG 2

void InitializeTimers() {
    TCCR0A = _BV(WGM01);                    // Waveform generation mode is CTC
    TCCR0B = _BV(CS01) | _BV(CS00);         // Timer0 clock prescaler is 64

    OCR0A = 249;                            // 250kHz / 250 = 100kHz = 1 mSec per overflow
    TIMSK0 = 0;                             // No interrupts

    usb_counter = 0;
    usbtask_counter = 0;
    int_flags = 0;
}

void Timers_Task() {
    if (!(TIFR0 & _BV(OCF0A)))
        return;

    TIFR0 |= _BV(OCF0A);                    // Reset interrupt request

    if (usb_counter != 0 && --usb_counter == 0)
        int_flags |= USB_INT_FLAG;

    if (usb_counter != 0 && --usbtask_counter == 0)
        int_flags |= USBTASK_INT_FLAG;
}

void Timers_SetupUSBDelay(uint8_t DelayMS) {
    usb_counter = DelayMS;
    int_flags &= ~USB_INT_FLAG;
}

void Timers_SetupUSBTaskDelay(uint8_t DelayMS) {
    usbtask_counter = DelayMS;
    int_flags &= ~USBTASK_INT_FLAG;
}

int8_t Timers_CheckUSBDelay() {
    if (!(int_flags & USB_INT_FLAG))
        return false;

    int_flags &= ~USB_INT_FLAG;
    return true;
}

int8_t Timers_CheckUSBTaskDelay() {
    if (!(int_flags & USBTASK_INT_FLAG))
        return false;

    int_flags &= ~USBTASK_INT_FLAG;
    return true;
}

