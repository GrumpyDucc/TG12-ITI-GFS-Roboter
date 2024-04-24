#if 1

#include "mbed.h"
#include "LCD.h"

lcd mylcd;

BufferedSerial hc05(PB_10, PB_11, 9600);

DigitalIn sensorRight(PA_14);
DigitalIn sensorLeft(PA_15);
DigitalIn manualDriveSwitch(PA_1);

PwmOut leftForward(PC_7);   // Ausgang Rad links vorwärts
PwmOut leftBackward(PC_6);  // Ausgang Rad links rückwärts
PwmOut rightForward(PC_8);  // Ausgang Rad rechts vorwärts
PwmOut rightBackward(PC_9); // Ausgang Rad rechts rückwärts

#define STANDARD_SPEED 0.65  // Mittlere Geschwindikeit für das normale Fahren
#define TURN_SPEED_FAST 1    // Maxgeschwindigkeit, für ein schnelles Drehen des äußeren Rads
#define TURN_SPEED_SLOW 0.45 // Geringe Geschwindigkeit für langsames Drehen des inneren Rads

float speed = STANDARD_SPEED; // Startgeschwindigkeit setzen
bool followLine = false;      // Linie folgen JA (1) / NEIN (0)

/** Geschwindigkeit des rechten Rads einstellen (+ -> vorwärts, - -> rückwärts) */
void rightWheel(float speed)
{
    if (speed > 0) // vorwärts
    {
        rightBackward = speed;
        rightForward = 0;
    }
    else // rückwärts
    {
        rightBackward = 0;
        rightForward = speed * -1;
    }
}

/** Geschwindigkeit des linken Rads einstellen (+ -> vorwärts, - -> rückwärts) */
void leftWheel(float speed)
{
    if (speed > 0) // vorwärts
    {
        leftForward = speed;
        leftBackward = 0;
    }
    else // rückwärts
    {
        leftForward = 0;
        leftBackward = speed * -1;
    }
}

/** Alle Motoren stoppen*/
void stop() // Roboter anhalten
{
    leftForward = 0;
    leftBackward = 0;
    rightForward = 0;
    rightBackward = 0;
}

/** Geschwindigkeit auf LCD-Display anzeigen*/
void printSpeed()
{
    if (not followLine)
    {
        mylcd.cursorpos(0x7);                 // Cursor auf pos 8 in 1. Zeile setzen (nach "Speed: " Schriftzug)
        mylcd.printf("%d", int(speed * 100)); // Geschwindigkeit anzeigen
    }
}

/** Fahrmodus umschalten*/
void switchDriveMode()
{
    followLine = !followLine; // Fahrmodus umschalten
    stop();                   // Auto anhalten

    if (followLine) // Wenn im Linienmodus, dann keine Geschwindigkeit ausgeben
    {
        mylcd.cursorpos(0x7);  // Cursor nach "Speed: " setzen
        mylcd.printf("- ");    // "- " ausgeben, da geschwindigkeit automatisch kontrolliert wird,
                               // Leerzeichen um die letzte Stelle der Geschwindigkeit zu überschreiben
        mylcd.cursorpos(0x4C); // Cursor in 2. Zeile nach "Auto Drive: " setzen
        mylcd.printf("On ");   // "On " ausgeben,
                               // Leerzeichen um das letzte 'f' von Off zu überschreiben
    }
    else
    {
        printSpeed();
        mylcd.cursorpos(0x4C);
        mylcd.printf("Off");
    }
}

void init() // Initialisierung
{
    mylcd.cls();                     // LCD-Display leeren und cursor auf 0,0 setzen
    mylcd.printf("Speed:");          // Schriftzug für Geschwindigkeit anzeigen
    mylcd.cursorpos(0x40);           // in 2- Zeile springen
    mylcd.printf("Auto Drive: Off"); // Schriftzug für Fahrmodus anzeigen
    printSpeed();                    // Geschwindigkeit anzeigen
    // Widerstände einstellen
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
            thread_sleep_for(250); // Entprellen
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
            case 'O': // reset schalter
                stop();
                followLine = false;
                break;
            case 'A':                           // Geschwindigkeitsänderung feststellen
                hc05.read(data, 3);             // 2-stellige Zahl und das end "A" lesen
                int new_speed = 0;              // variable für neue Geschwindigkeit erzeigen
                sscanf(data, "%d", &new_speed); // in zahl umwandeln
                speed = float(new_speed) / 100; // zu korrektem float wandeln
                printSpeed();                   // neue Geschwindigkeit anzeigen
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