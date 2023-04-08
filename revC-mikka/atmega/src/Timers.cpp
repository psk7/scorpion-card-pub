#include "Timers.h"
#include <avr/io.h>
#include <avr/interrupt.h>

static uint8_t usb_counter;
static uint8_t usbtask_counter;
static uint8_t int_flags;

#define USB_INT_FLAG 1
#define USBTASK_INT_FLAG 2

ISR(TIMER0_COMPA_vect, ISR_BLOCK){
    if (usb_counter != 0 && --usb_counter == 0)
        int_flags |= USB_INT_FLAG;

    if (usbtask_counter != 0 && --usbtask_counter == 0)
        int_flags |= USBTASK_INT_FLAG;
}

void InitializeTimers() {
    TCCR0A = _BV(WGM01);                    // Waveform generation mode is CTC
    TCCR0B = _BV(CS01) | _BV(CS00);         // Timer0 clock prescaler is 64

    OCR0A = 249;                            // 250kHz / 250 = 1000 Hz = 1 mSec per overflow
    TIMSK0 = _BV(OCIE0A);

    usb_counter = 0;
    usbtask_counter = 0;
    int_flags = 0;
}

void Timers_SetupUSBDelay(uint8_t DelayMS) {
    TIMSK0 = 0;
    usb_counter = DelayMS;
    int_flags &= ~USB_INT_FLAG;
    TIMSK0 = _BV(OCIE0A);
}

void Timers_SetupUSBTaskDelay(uint8_t DelayMS) {
    TIMSK0 = 0;
    usbtask_counter = DelayMS;
    int_flags &= ~USBTASK_INT_FLAG;
    TIMSK0 = _BV(OCIE0A);
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

