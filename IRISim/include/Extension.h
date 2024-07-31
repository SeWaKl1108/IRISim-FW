#include <Arduino.h>
#include <queue.h>

//struct for ADC-Values
typedef struct { 
  String Name;
  int dValue;
  float Voltage;
} ADCValue;

