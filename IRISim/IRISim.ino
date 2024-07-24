#include <Arduino_FreeRTOS.h>
#include <Arduino.h>
#include <queue.h>
#include <time.h>
#include "Extension.h"
#define DEBUG 1


// pin hr-name - human readable mapping of ADC pins
const int amplifierPin=A0;
const int frequencyPin=A1;
const int targetValuePin=A2;
const int timeConstValPin=A3;
const int pdValuePin=A4;

/* 
/// Tasks
*/

// Die LED an Pin 13 blinken lassen
void TaskBlink(void *pvParameters) 
{
  int cnt=0;
  pinMode(LED_BUILTIN, OUTPUT);
  for (;;) 
  {
    digitalWrite(LED_BUILTIN, HIGH);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    digitalWrite(LED_BUILTIN, LOW);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    StringToQueue("TaskBlink output:\nIteration "+ String(cnt++));
  }
}
//Task to drop a line to serial terminal. Receives a message from messagequeue
void vtSerialOut(void *pvParameters)
{
  char *received;
  while (1) {
    if (xQueueReceive(qSerialOut, &received, portMAX_DELAY)) 
    {
      Serial.print(received);
    }
    else
    {
      Serial.println("funzt nicht");
    }    
  }
}

// check AD knob of timeconstant value for changes
void vtADTimeConstant(void *pvParameters)
{
  int val= analogRead(timeConstValPin);  
}

// check AD knob of amp value for changes
void vtADAmplifier(void *pvParameters)
{
  int val= analogRead(amplifierPin);
  if(IsChange(amp,val))
  {
    amp=val;
  }
}

// check AD knob of frequency value for changes
void vtADFrequency(void *pvParameters)
{
  int val= analogRead(frequencyPin);
}

// check AD knob of target value for changes
void vtADTarget(void *pvParameters)
{
  int val= analogRead(targetValuePin);
}

// check AD knob of photoDiode value for changes
void vtADPhotoDiode(void *pvParameters)
{  
  int val= analogRead(pdValuePin);
}


/* 
/// Tasks
*/

/* 
/// Init
*/
void setup() {

  Serial.begin(57600);
  // setup queues 
  qSerialOut = xQueueCreate(10, sizeof(char*));      //10 pointers
  qADValues = xQueueCreate(10, sizeof(ADValue)); 
  // Every Task needs a line in "setup".
  xTaskCreate(vtSerialOut, "serialout", 128, NULL, 1, NULL);     //worker task for pushing data over the serial line
  xTaskCreate(vtADAmplifier, "ADAmpWatch", 128, NULL, 1, NULL);         //worker task for watching ADC pin resposible for reading preset values for amplification  
  #if DEBUG
  xTaskCreate(TaskBlink, "blink", 128, NULL, 1, NULL);
  #endif
  // start task scheduler
  vTaskStartScheduler();

  // Initialize the  start time 
  startTime=millis();
}


void loop() {
  // put your main code here, to run repeatedly:
  // No need to code anything as FreeRTOS is in charge to manage tasks!!!
}
