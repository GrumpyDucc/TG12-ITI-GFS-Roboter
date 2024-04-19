#if 0
/* TI4 03.1 PWM
 * (C) Jan Waldinger, 2024
 */

#include "mbed.h"

PwmOut led(PC_8);
AnalogIn poti(PA_0);
int main()
{
    led.period_us(100);
    while (true)
    {
        printf("Poti: %d\n", int(poti.read() * 100));
        led.pulsewidth_us(poti.read()*100);
    }
}

#endif