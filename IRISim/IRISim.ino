#include <Arduino_FreeRTOS.h>
#include <queue.h>
#include <Arduino.h>
#include <time.h>

#define DEBUG 1

/* 
/// Structs
*/

//define struct for the serial out
typedef struct { 
  String message;
} DefSerMsg;


/* 
/// Variables
*/

// digital values from ADC
float tconst = 0;
float amp = 0;
float freq = 0;
float targetvalue = 0;
float pdval=0;

unsigned long startTime;   //timestamp calculation var

// forward declaration

QueueHandle_t qSerialOut;


/* 
/// Tasks
*/


void vtDebugOut(void *pvParameters) {
 
  while (1) {   
    DefSerMsg msg;
    xQueueSend(qSerialOut, &msg, portMAX_DELAY);
    vTaskDelay(1000 / portTICK_PERIOD_MS); // 1 Sekunde warten
  }
}

// Die LED an Pin 13 blinken lassen
void TaskBlink(void *pvParameters) {
  int cnt=0;
  pinMode(LED_BUILTIN, OUTPUT);
  for (;;) {
    digitalWrite(LED_BUILTIN, HIGH);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    digitalWrite(LED_BUILTIN, LOW);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    DefSerMsg msg= MsgBuilder("msgbuilder created this text.  Iteration "+ String(cnt++));
    MessageToQueue(msg);
  }
}
//Task to drop a line to serial terminal. Receives a message from messagequeue
void vtSerialOut(void *pvParameters)
{ 
  DefSerMsg msgout;  
  while (1) {
    if (xQueueReceive(qSerialOut, &msgout, portMAX_DELAY)) 
    {
      Serial.print(msgout.message);
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

}

// check AD knob of amp value for changes
void vtADAmplifier(void *pvParameters)
{

}

// check AD knob of frequency value for changes
void vtADFrequency(void *pvParameters)
{

}

// check AD knob of target value for changes
void vtADTarget(void *pvParameters)
{

}

// check AD knob of photoDiode value for changes
void vtADPhotoDioade(void *pvParameters)
{

}


/* 
/// Tasks
*/

// Method to insert a string into the respective queue
void StringToQueue(const String& str) {

  char *heapStr = strdup(str.c_str());   //string to heap to prevent getting chaffed
    xQueueSend(qSerialOut, &heapStr, portMAX_DELAY);
}
void MessageToQueue(DefSerMsg msg) {

  //char *heapStr = strdup(str.c_str());   //string to heap to prevent getting chaffed
    xQueueSend(qSerialOut, &msg, portMAX_DELAY);
}



/* 
/// Init
*/
void setup() {

  Serial.begin(57600);
  qSerialOut = xQueueCreate(10, sizeof(DefSerMsg));
  
  // Every Task needs a line in "setup".
  xTaskCreate(vtSerialOut, "serialout", 128, NULL, 1, NULL);     //worker task for pushing data over the serial line
  xTaskCreate(vtDebugOut, "sender", 128, NULL, 1, NULL);         //worker task for pushing debug information over the serial line
  xTaskCreate(TaskBlink, "blink", 128, NULL, 1, NULL);
  
  // start task scheduler
  vTaskStartScheduler();

  // Initialize the  start time 
  startTime=millis();
}


void loop() {
  // put your main code here, to run repeatedly:

}
