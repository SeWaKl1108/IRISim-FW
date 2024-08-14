#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <queue.h>
#include <time.h>
#include "heartbeat.h"

#pragma region typedef - structs - Enums

// serial out control flags
enum Verbosity
{
  quiet,
  heartbeat,
  verbose,
  trace,
  debug
};

typedef struct
{
  int tconst, amplifier, freq, targetvalue;
} PIDParam; //, *pPidValues;

typedef struct
{
  long TimeStamp;
  int pdvalue;
  float Voltage;
} Measurement; //, *pMValue;

#pragma endregion

#pragma region Constants

/* pin hr-name - human readable mapping of ADC pins */
const int amplifierPin = A0;
const int frequencyPin = A1;
const int targetValuePin = A2;
const int timeConstValPin = A3;
const int pdValuePin = A4;

/* Constants - task priority, ON/Off -time for blinking LED */
const int DEF_TASK_PRIO = 1; // Default-task priority 0-3; highest prio is 0 (fastest execution, superpasses all other prios)!
const int COM_PRIO = 2;      // COM-task priority 0-3 highest prio is 0!

const TickType_t COM_WAIT_TICK = 1; // ticks to wait for a semaphore before retrying
const TickType_t ADC_WAIT_TICK = 10; // ticks to wait for a semaphore before retrying
#pragma endregion

#pragma region Variables

/* Atomics */
volatile Verbosity vlevel = trace;
unsigned long startTime; // timestamp calculation var
PIDParam ppidval;

/* Objects */
SemaphoreHandle_t xComSemaphore;
SemaphoreHandle_t xADCSemaphore;
QueueHandle_t qSerialOut;
Measurement MValue;

#pragma endregion

#pragma region Forward Declaration(s)

void vtHeartBeat(void *pvParameters);
void vtSerialOut(void *pvParameters);
void vtADAmplifier(void *pvParameters);
void StringToQueue(const String &str);
void Init_Sys(void);
void Init_Semaphore(void);
void Init_Task(void);
void HandlePIDParams(void);
bool IsChange(int, int, int);

#pragma endregion

void setup()
{
  Init_Sys();
  Init_Semaphore();
  Init_Task();
  SetupTimerRegisters();
  vTaskStartScheduler(); // after all tasks are defined and queued,the scheduler can be started to do it's work.
}

#pragma region Init
void Init_Sys()
{
  /* define pin roles */
  pinMode(LED_BUILTIN, OUTPUT);                  // OnBoard LED used as indicator
  qSerialOut = xQueueCreate(10, sizeof(char *)); // 10 pointers

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
}

void Init_Task()
{
  /* task handle stuff */
  xTaskCreate(vtSerialOut, "serialout", 256, NULL, 2, NULL); // worker task for pushing data over the serial line
  xTaskCreate(vtADAmplifier, "adcampl", 64, NULL, 1, NULL);
}

#pragma endregion

#pragma region Tasks

// check AD knob of timeconstant value for changes
void vtADTimeConstant(void *pvParameters)
{
  int val = analogRead(timeConstValPin);
  ppidval.tconst = val;
}

// check AD knob of amplifier value for changes
void vtADAmplifier(void *pvParameters)
{
  for (;;)
  {
    if (xSemaphoreTake(xADCSemaphore, ADC_WAIT_TICK) == pdTRUE)
    {
      int val = analogRead(amplifierPin);
      xSemaphoreGive(xADCSemaphore);

      if (IsChange(ppidval.amplifier, val, 10))
      {
        ppidval.amplifier = val;
        HandlePIDParams();
      }
      vTaskDelay(1);
      StringToQueue("Time Constant: " + String(ppidval.tconst) + "\n" +
                    "Amplifier : " + String(ppidval.amplifier) + "\n" +
                    "Frequency : " + String(ppidval.freq) + "\n" +
                    "Target value : " + String(ppidval.targetvalue) + "\n");
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

void HandlePIDParams()
{
  switch (vlevel)
  {
  case verbose:
    StringToQueue("PID Parameter changed\n");
    break;
  case trace:
    StringToQueue("PID Parameter changed\n");
    StringToQueue("Time Constant: " + String(ppidval->tconst) + "\n" +
                  "Amplifier : " + String(ppidval->amplifier) + "\n" +
                  "Frequency : " + String(ppidval->freq) + "\n" +
                  "Target value : " + String(ppidval->targetvalue) + "\n");
  case debug:
    break;
  default:
    break;
  }
}

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