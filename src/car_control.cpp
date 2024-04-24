#if 1

#include "mbed.h"
#include "LCD.h"

lcd mylcd;

BufferedSerial hc05(PB_10, PB_11, 9600);

DigitalIn sensorRight(PA_14);
DigitalIn sensorLeft(PA_15);
DigitalIn manualDriveSwitch(PA_1);

PwmOut lv(PC_7);
PwmOut lr(PC_6);
PwmOut rr(PC_8);
PwmOut rv(PC_9);

#define STANDARD_SPEED 0.40
#define TURN_SPEED_FAST 1
#define TURN_SPEED_SLOW 0.35
#define TURNTIME 50

float speed = STANDARD_SPEED; // minimum value for the car to move
int new_speed = 0;
bool followLine = false;

void rightWheel(float speed)
{
    if (speed > 0) // vorwärts
    {
        rv = speed;
        rr = 0;
    }
    else // rückwärts
    {
        rv = 0;
        rr = speed * -1;
    }
}

void leftWheel(float speed)
{
    if (speed > 0) // vorwärts
    {
        lv = speed;
        lr = 0;
    }
    else // rückwärts
    {
        lv = 0;
        lr = speed * -1;
    }
}

void stop()
{
    lv = 0;
    lr = 0;
    rr = 0;
    rv = 0;
}

void switchDriveMode()
{
    followLine = !followLine; // Fahrmodus umschalten
    stop();                   // Auto anhalten
}

void init()
{
    mylcd.clear();
    sensorLeft.mode(PullDown);
    sensorRight.mode(PullDown);
    manualDriveSwitch.mode(PullDown);
}

int main()
{
    init();
    char data[15];

    while (true)
    {
        if (manualDriveSwitch) // manueller Fahrmoduswechsel
        {
            switchDriveMode();
            thread_sleep_for(250);
        }
        if (hc05.readable())
        {
            thread_sleep_for(20);
            hc05.read(data, 1);
            switch (data[0])
            {
            case 'F':
                switchDriveMode();
                break;
            case 'U': // vorwärts
                rightWheel(speed);
                leftWheel(speed);
                break;
            case 'R': // rechts
                rightWheel(speed * -1);
                leftWheel(speed);
                break;
            case 'L': // links
                rightWheel(speed);
                leftWheel(speed * -1);
                break;
            case 'D': // rückwärts
                rightWheel(speed * -1);
                leftWheel(speed * -1);
                break;
            case 'O': // aus
                stop();
                followLine = false;
                break;
            case 'A':                           // Geschwindigkeitsänderung feststellen
                hc05.read(data, 3);             // 2-stellige Zahl und das end "A" lesen
                sscanf(data, "%d", &new_speed); // in zahl umwandeln
                speed = float(new_speed) / 100; // zu korrektem float wandeln
                break;
            }
        }
        if (followLine)
        {
            if (sensorRight.read()) // rechter Sensor über schwarz -> nach links lenken
            {
                rightWheel(-TURN_SPEED_SLOW);
                leftWheel(-TURN_SPEED_FAST);
            }
            else if (sensorLeft.read()) // linker Sensor über schwarz -> nach rechts lenken
            {
                rightWheel(TURN_SPEED_FAST);
                leftWheel(TURN_SPEED_SLOW);
            }
            else // beide Sensoren über weiß oder beide über schwarz-> geradeaus fahren
            {
                rightWheel(STANDARD_SPEED);
                leftWheel(-STANDARD_SPEED);
            }
        }
    }
}

#endif