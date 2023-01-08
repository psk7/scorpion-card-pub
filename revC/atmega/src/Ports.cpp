#include <avr/io.h>
#include <util/delay.h>
#include "SPI.h"
#include "UsbHostTask.h"
#include "Timers.h"

extern "C" int main(void);

int main(void) {
    PORTD &= ~_BV(PORTD5);                      // Hard reset Z state
    DDRD &= ~_BV(DDD5);                         // Hard reset Z state

    PORTD &= ~_BV(PORTD7);
    DDRD &= ~_BV(DDD7);                         // NMI Z state

    PORTD &= ~_BV(PORTD6);
    DDRD &= ~_BV(DDD6);                         // Turbo switch Z state

    DDRB &= ~_BV(DDB0);
    PORTB |= _BV(PORTB0);                      // Turbo LED in (PULLUP)

    DDRD |= _BV(DDD0);
    DDRD |= _BV(DDD1);
    DDRC |= _BV(DDC1);
    DDRC |= _BV(DDC0);

    SPI::Init();
    InitializeTimers();

    // Kempston Joystick outputs
    DDRB |= _BV(DDB1);
    DDRB |= _BV(DDB2);
    DDRD |= _BV(DDD2);
    DDRD |= _BV(DDD3);
    DDRD |= _BV(DDD4);

    _delay_ms(20);

    USBT::Init();

    while(1) {
        Timers_Task();
        USBT::Run();
    }
}