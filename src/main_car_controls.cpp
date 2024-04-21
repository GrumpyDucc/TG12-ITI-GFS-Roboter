#if 1

#include "mbed.h"
#include "LCD.h"

lcd mylcd;

BufferedSerial hc05(PB_10, PB_11, 9600);

DigitalIn sensorRight(PA_1);
DigitalIn sensorLeft(PA_2);
InterruptIn manualDriveSwitch(PA_10);

PwmOut lv(PC_7);
PwmOut lr(PC_6);
PwmOut rr(PC_9);
PwmOut rv(PC_8);

#define STANDARD_SPEED 0.50
#define TURNTIME 25

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
        lr = 0;
        lv = speed;
    }
    else // rückwärts
    {
        lr = speed * -1;
        lv = 0;
    }
}

void stop()
{
    lv = 0;
    lr = 0;
    rr = 0;
    rv = 0;
}

void switchAutodrive()
{
    stop();                   // Auto anhalten
    followLine = !followLine; // Autofahr-Modus umschalten
}

void init()
{
    mylcd.clear();
    sensorLeft.mode(PullDown);
    sensorRight.mode(PullDown);
    manualDriveSwitch.mode(PullDown);
    manualDriveSwitch.rise(&switchAutodrive);
    manualDriveSwitch.enable_irq();
    __enable_irq();
}

int main()
{
    init();

    char data[15];

    while (true)
    {
        printf("Speed: %d\n", int(speed * 100));
        if (hc05.readable()) // Check if mode is switched by bluetooth
        {
            thread_sleep_for(20);
            hc05.read(data, 1);
            if (data[0] == 'F') // Autofahr-Modus bluetooth umschalten
            {
                switchAutodrive();
            }
            if (not followLine)
            {
                switch (data[0])
                {
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
                    break;
                case 'A':                           // Geschwindigkeitsänderung feststellen
                    hc05.read(data, 3);             // 2-stellige Zahl und das end "A" lesen
                    sscanf(data, "%d", &new_speed); // in zahl umwandeln
                    speed = float(new_speed) / 100; // zu korrektem float wandeln
                    break;
                }
            }
        }
        if (followLine)
        {
            if (sensorRight.read() == 1) // rechter Sensor über schwarz -> nach links lenken
            {
                for (int i = 0; i < TURNTIME; i++)
                {
                    printf("auto left\n");
                    leftWheel(STANDARD_SPEED + 0.1);
                    rightWheel(STANDARD_SPEED);
                }
            }
            else if (sensorLeft.read() == 1) // linker Sensor über schwarz -> nach rechts lenken
            {
                for (int i = 0; i < TURNTIME; i++)
                {
                    printf("auto right\n");
                    leftWheel(STANDARD_SPEED);
                    rightWheel(STANDARD_SPEED + 0.1);
                }
            }
            else // beide Sensoren über weiß -> geradeaus fahren
            {
                printf("auto forward\n");
                leftWheel(STANDARD_SPEED);
                rightWheel(STANDARD_SPEED);
            }
        }
    }
}

#endif