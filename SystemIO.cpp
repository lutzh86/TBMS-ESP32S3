
#include "config.h"
#include "Logger.h"
#include "SystemIO.h"

int DigitalInputs[2] = {DIN1, DIN2};
int DigitalOutputs[1][2] = { {DOUT1_L, DOUT1_H } };

SystemIO::SystemIO()
{
    
}

void SystemIO::setup()
{
    for (int x = 0; x < 2; x++) pinMode(DigitalInputs[x], INPUT);
    
    for (int y = 0; y < 1; y++) {
        pinMode(DigitalOutputs[y][0], OUTPUT);
        digitalWrite(DigitalOutputs[y][0], LOW);
        pinMode(DigitalOutputs[y][1], OUTPUT);
        digitalWrite(DigitalOutputs[y][1], LOW);
    }
}

bool SystemIO::readInput(int pin)
{
    if (pin < 0 || pin > 1) return false;
    return !digitalRead(DigitalInputs[pin]);
}

void SystemIO::setOutput(int pin, OUTPUTSTATE state)
{
    if (pin < 0 || pin > 1) return;
    //first set it floating
    digitalWrite(DigitalOutputs[pin][0], LOW);
    digitalWrite(DigitalOutputs[pin][1], LOW);
    delayMicroseconds(10); //give mosfets some time to turn off
    if (state == HIGH_12V) digitalWrite(DigitalOutputs[pin][1], HIGH);
    if (state == GND) digitalWrite(DigitalOutputs[pin][0], HIGH);
}

SystemIO systemIO;
