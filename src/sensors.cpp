#include<Arduino.h>
#include "Sensors.h"

class Sensors{

private: 

  int sampling;
  int doutPinA, sckPinA;
  int doutPinB, sckPinB;
  long offsetA;
  long offsetB;

public:

Sensors(int s, int dA, int sA, int dB, int sB)
        : sampling(s), doutPinA(dA), sckPinA(sA), doutPinB(dB), sckPinB(sB),
          offsetA(0), offsetB(0) {} // constructor

 long readPressure(int doutPin, int sckPin) {
    long value = 0;
    while (digitalRead(doutPin)); // Tunggu sampai data siap
  
    for (int i = 0; i < 24; i++) {
      digitalWrite(sckPin, HIGH);
      delayMicroseconds(1);
      value = (value << 1) | digitalRead(doutPin);
      digitalWrite(sckPin, LOW);
      delayMicroseconds(1);
    }
  
   
    for (int i = 0; i < 3; i++) {
      digitalWrite(sckPin, HIGH);
      delayMicroseconds(1);
      digitalWrite(sckPin, LOW);
      delayMicroseconds(1);
    }
  
    
    return (value < 0 || value > 16777216) ? 0 : value;
  }

  long avg(int doutPin, int sckPin) {
    long sum = 0;
    for (int i = 0; i < sampling; i++) {
        sum += readPressure(doutPin, sckPin);
        delay(5);
    }
    return sum / sampling;
}

  void setOffsets(int doutPinA, int sckPinA, int doutPinB, int sckPinB) {
    offsetA = avg(doutPinA, sckPinA);
    offsetB = avg(doutPinB, sckPinB);
      }

  long getOffsetA() const { return offsetA; }
  long getOffsetB() const { return offsetB; }

  long getCalibratedA() {
    long raw = avg(doutPinA, sckPinA);
    return raw - offsetA;
      }

  long getCalibratedB() {
    long raw = avg(doutPinB, sckPinB);
    return raw - offsetB;
      }
};