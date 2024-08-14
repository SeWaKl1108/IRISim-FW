#include "Arduino_FreeRTOS.h"
#include "PIDController.h"

PIDController pidCtrl;

int input;
int output;

// Initialize the PID Controller
// @ targetvalue: setpoint (very right) Sollwert
// @ pGain: amplification  (middle) 
// @ iGain: frequency      (right) 
// @ dGain: time constant  (left) 
void Init_PID(int targetvalue,int pGain ,int iGain ,int dGain )
{
  pidCtrl.begin();

  pidCtrl.limit(0, 255);
  pidCtrl.setpoint(targetvalue);
  pidCtrl.tune(pGain,iGain,dGain);
}

// Method to be executed continously to adjust the target value
// Interval: min 30ms
void PIDCtrl_TaskMethod(int sensorvalue)
{
  pidCtrl.compute(sensorvalue);
}

// Minimize the output value (linear action) 
void PIDCtrl_Minimize(double divisor)
{
  pidCtrl.minimize(divisor);
}
void PIDCtrl_Start()
{
  pidCtrl.start();
}
void PIDCtrl_Reset()
{
  pidCtrl.reset();
}

void puttotask()
{
  input = analogRead(A0); // Replace with photoTransistor value feedback
  pidCtrl.compute();
  analogWrite(3, output); // Replace with plant control signal 3,
}
void loop()
{

  
  /*  pidCtrl.debug(&Serial, "myController", PRINT_INPUT    | // Can include or comment out any of these terms to print
                                               PRINT_OUTPUT   | // in the Serial plotter
                                               PRINT_SETPOINT |
                                               PRINT_BIAS     |
                                               PRINT_P        |
                                               PRINT_I        |
                                               PRINT_D);
    */
  
}