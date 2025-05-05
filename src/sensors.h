#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

class Sensors {
private:
    int sampling;
    int doutPinA, sckPinA;
    int doutPinB, sckPinB;
    long offsetA;
    long offsetB;

public:
    Sensors(int s, int dA, int sA, int dB, int sB);

    long readPressure(int doutPin, int sckPin);
    long avg(int doutPin, int sckPin);
    void setOffsets();

    long getOffsetA() const;
    long getOffsetB() const;
    long getCalibratedA();
    long getCalibratedB();
};

#endif