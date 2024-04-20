#if 1

#include "mbed.h"
#include "LCD.h"

lcd mylcd;

BufferedSerial hc05(PB_10, PB_11, 9600);

PwmOut lv(PC_7);
PwmOut lr(PC_6);
PwmOut rr(PC_9);
PwmOut rv(PC_8);

float speed = 0.45;
int new_speed = 0;
bool readingFlag = false;

int main()
{
    char data[15];

    while (true)
    {
        if (hc05.readable())
        {
            thread_sleep_for(20);
            hc05.read(data, 1);
            switch (data[0])
            {
            case 'U':
                rv = speed;
                lv = speed;
                break; // vorwärts
            case 'R':
                rr = speed;
                lv = speed;
                break; // rechts
            case 'L':
                rv = speed;
                lr = speed;
                break; // links
            case 'D':
                rr = speed;
                lr = speed;
                break; // rückwärts
            case 'O':
                lv = 0;
                lr = 0;
                rv = 0;
                rr = 0;
                break; // aus
            case 'A':
                hc05.read(data, 2);             // 2-stellige Zahl nach A lesen
                sscanf(data, "%d", &new_speed); // in zahl umwandeln
                speed = float(new_speed) / 100; // zu korrektem float wandeln
                hc05.read(data, 1);             // letztes A lesen
                break;
            }
        }
    }
}

#endif