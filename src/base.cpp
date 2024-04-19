#if 1

#include "mbed.h"
#include "LCD.h"

BufferedSerial hc05(PB_10, PB_11, 9600);
PortOut motor(PortC, 0b11000011);

int main()
{
    char daten[15];

    while (true)
    {
        if (hc05.readable())
        {
            thread_sleep_for(20);
            hc05.read(daten, 1);
            switch (daten[0])
            {
            case 'U':
                motor = 0b10000010;
                break; // vorwärts
            case 'R':
                motor = 0b10000001;
                break; // rechts
            case 'L':
                motor = 0b01000010;
                break; // links
            case 'D':
                motor = 0b01000001;
                break; // rückwärts
            case 'O':
                motor = 0;
                break; // aus
            }
        }
    }
}

#endif