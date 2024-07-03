#pragma once
#include <Arduino.h>

#define LEDOFF 0
#define RED 0b100
#define YELLOW 0b010
#define GREEN 0b001

class ShiftLed
{
private:
    byte groups[3];
    byte latchPin;
    byte clockPin;
    byte dataPin;

public:
    ShiftLed(byte latch, byte clock, byte data);
    ShiftLed &setGroup(byte number, byte color);
    ShiftLed &setAll(byte color);
    void send();
    byte getGroupColor(byte number);
};

ShiftLed::ShiftLed(byte latch, byte clock, byte data) : latchPin(latch), clockPin(clock), dataPin(data)
{
    setAll(LEDOFF);
    pinMode(latchPin, OUTPUT);
    pinMode(clockPin, OUTPUT);
    pinMode(dataPin, OUTPUT);
    digitalWrite(latchPin, HIGH);
}

inline ShiftLed &ShiftLed::setGroup(byte number, byte color)
{
    groups[number - 1] = color;
    return *this;
}

inline ShiftLed &ShiftLed::setAll(byte color)
{
    groups[0] = groups[1] = groups[2] = color;
    return *this;
}

inline byte ShiftLed::getGroupColor(byte number)
{
    return groups[number - 1];
}

inline void ShiftLed::send()
{
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, MSBFIRST, (groups[2] << 7));
    shiftOut(dataPin, clockPin, MSBFIRST, ((groups[0] << 5) | (groups[1] << 2) | (groups[2] >> 1)));
    digitalWrite(latchPin, HIGH);
}
