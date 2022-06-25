#include "UsbHostTask.h"
#include "Timers.h"
#include "ZxKeyboard.h"
#include "Link.h"
#include <util/delay.h>

int main(void) {
    InitializeLinks();

    _delay_ms(10);

    bool mode = (PINC & _BV(PINC7)) == 0;

    if (mode) {
        InitializeTimers();
        UsbHostTask_Init();

        for (;;) {
            UsbHostTask_Run(nullptr, false, true);
            Timers_Task();
        }
    } else {
        while (1);
    }
}
