#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <queue.h>
#include <time.h>
#include "Extension.h"

#pragma region Variables & Constants

/* pin hr-name - human readable mapping of ADC pins */
const int amplifierPin = A0;
const int frequencyPin = A1;
const int targetValuePin = A2;
const int timeConstValPin = A3;
const int pdValuePin = A4;

/* Constants - task priority, ON/Off -time for blinking LED */
const int LED_ON_TIME = 50;    // in ms
const int LED_OFF_TIME = 5000; // in ms
const int DEF_TASK_PRIO = 1;   // Default-task priority 0-3; highest prio is 0 (fastest execution, superpasses all other prios)!
const int COM_PRIO = 2;        // COM-task priority 0-3 highest prio is 0!

const TickType_t COM_WAIT_TICK = 1; // ticks to wait for a semaphore before retrying
const TickType_t ADC_WAIT_TICK = 1; // ticks to wait for a semaphore before retrying

/* Globally used Variables */
int tconst = 0;
int amplifier = 0;
int freq = 0;
int targetvalue = 0;
int pdvalue = 0;
unsigned long startTime; // timestamp calculation var

/* Globally used Objects */
SemaphoreHandle_t xComSemaphore;
SemaphoreHandle_t xADCSemaphore;
QueueHandle_t qSerialOut;
QueueHandle_t qADCValue;

#pragma endregion

#pragma region Forward Declaration(s)

void vtSerialOut(void *pvParameters);
void TaskBlink(void *pvParameters);
void vtADAmplifier(void *pvParameters);
void StringToQueue(const String &str);
void Init_Sys(void);
void Init_Semaphore(void);
void Init_Task(void);
bool IsChange(int, int, int);

#pragma endregion

void setup()
{
  Init_Sys();
  Init_Semaphore();
  Init_Task();

  // start task scheduler after all tasks are defined and queued,
  // the scheduler can be started to do it's work.
  vTaskStartScheduler();
}

#pragma region Init
void Init_Sys()
{
  /* define pin roles */
  pinMode(LED_BUILTIN, OUTPUT);                  // OnBoard LED used as indicator
  qSerialOut = xQueueCreate(10, sizeof(char *)); // 10 pointers
  qADCValue = xQueueCreate(10, sizeof(ADCValue));

  /* serial communication */
  Serial.begin(115200, SERIAL_8N1);
}

void Init_Semaphore()
{ /* resource locking */
  xComSemaphore = xSemaphoreCreateBinary();
  if ((xComSemaphore) != NULL)
  {
    xSemaphoreGive((xComSemaphore));
  }
  xADCSemaphore = xSemaphoreCreateBinary();
  if ((xADCSemaphore) != NULL)
  {
    xSemaphoreGive((xADCSemaphore));
  }
}

void Init_Task()
{
  /* task handle stuff */
  xTaskCreate(TaskBlink, "blinkLED", 128, NULL, 1, NULL);
  xTaskCreate(vtSerialOut, "serialout", 256, NULL, 1, NULL); // worker task for pushing data over the serial line
}
#pragma endregion

#pragma region Tasks

// blink the onboard LED (pin13)
void TaskBlink(void *pvParameters)
{
  int cnt = 0;
  StringToQueue("\nFinished Initialisation.\nHeartbeat visual feedback:\nLED OnTime: " +
                 String(LED_ON_TIME/portTICK_PERIOD_MS)+"ms\nLED OffTime: "+String(LED_OFF_TIME/portTICK_PERIOD_MS)+"ms\nPortTickPeriod: "+
                 String(portTICK_PERIOD_MS)+"\n");
  for (;;)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    vTaskDelay(LED_ON_TIME / portTICK_PERIOD_MS);
    digitalWrite(LED_BUILTIN, LOW);  
    StringToQueue("*** Heartbeat ***\nStill alive!\n");  
    vTaskDelay(LED_OFF_TIME/portTICK_PERIOD_MS);
  }
}

// check AD knob of timeconstant value for changes
void vtADTimeConstant(void *pvParameters)
{
  int val = analogRead(timeConstValPin);
}

// check AD knob of amp value for changes
void vtADAmplifier(void *pvParameters)
{
  if (xSemaphoreTake(xADCSemaphore, ADC_WAIT_TICK) == pdTRUE)
  {
    int val = analogRead(amplifierPin);
    xSemaphoreGive(xADCSemaphore);
    vTaskDelay(1);
    if (IsChange(amplifier, val, 1))
    {
      amplifier = val;
    }
  }
}

// check AD knob of frequency value for changes
void vtADFrequency(void *pvParameters)
{
  int val = analogRead(frequencyPin);
}

// check AD knob of target value for changes
void vtADTarget(void *pvParameters)
{
  int val = analogRead(targetValuePin);
}

// check AD knob of photoDiode value for changes
void vtADPhotoDiode(void *pvParameters)
{
  int val = analogRead(pdValuePin);
}

// Task to drop a line to serial terminal. Receives a message from messagequeue
void vtSerialOut(void *pvParameters)
{
  char *received;
  for (;;)
  {
    if (xQueueReceive(qSerialOut, &received, portMAX_DELAY))
    {      
      if (xSemaphoreTake(xComSemaphore, COM_WAIT_TICK) == pdTRUE)
      {
      Serial.print(received);
       xSemaphoreGive(xComSemaphore);
     }
      vTaskDelay(1); // one tick delay (15ms) in between reads for stability
    }
  }
}
#pragma endregion

#pragma region Methods

// Method to insert a string into the respective queue
void StringToQueue(const String &str)
{
  char *heapStr = strdup(str.c_str()); // string to heap to prevent getting chaffed
  xQueueSend(qSerialOut, &heapStr, portMAX_DELAY);
}

// Method to check if "value" deviates by more than the specified percentage (deviation)
bool IsChange(int origval, int newval, int threshold = 1)
{
  if (newval != 0)
  {
    float deviation = (abs(newval - origval) / origval) * 100;
    return (deviation >= threshold) ? true : false;
  }
  return false;
}

/// get the elapsed time in seconds relative to "startTime"
long GetTimestamp()
{
  return (millis() - startTime) / 1000;
}

/// (re)set the "startTime" to current time
void ResetStartTime()
{
  startTime = millis();
}

/// DA-conversion.
/// Reverse the measured voltage from a given digital value.
/// @param dval: the digital valu 0-1024.
/// @return aval: calculated analog value.
float CalVolt(int dval)
{
  // Calculate back voltage values
  float aval = ((float)dval / 1024) * 5;
  return aval;
}
#pragma endregion

#pragma region unused stuff

void loop()
{
  /* as FreeRTOS is running
   --> nothig to be done here. */
}

#pragma endregion