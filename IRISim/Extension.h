/* 
/// Structs
*/

//struct for ADC-Values
typedef struct { 
  String Name;
  int dValue;
  int Voltage;
} ADValue;

/* 
/// Variables
*/

// digital values from ADC
float tconst = 0;
float amp = 0;
float freq = 0;
float targetvalue = 0;
float pdvalue=0;

unsigned long startTime;   //timestamp calculation var

// forward declarations

QueueHandle_t qSerialOut;         // queue for output over serial line
QueueHandle_t qADValues;          // queue for ADC measurement

/* 
/// Methods
*/


// Method to insert a string into the respective queue
void StringToQueue(const String& str) {

  char *heapStr = strdup(str.c_str());   //string to heap to prevent getting chaffed
    xQueueSend(qSerialOut, &heapStr, portMAX_DELAY);
}

// Method to check if "value" deviates by more than the specified percentage (deviation) 
bool IsChange(int origval, int newval, int threshold=1)
{
  if(newval!=0)
  {
    float deviation=(abs(newval-origval)/origval)*100;
  #if DEBUG  
  Serial.println("**Method IsChange**\nOriginal Value:"+ String(origval)+"\nNew value: "+String(newval)+"\nDeviation: "+String(deviation));
  Serial.println();
  #endif
    return (deviation>=threshold)? true:false;
  }  
  return false;
}

/// get the elapsed time in seconds relative to "startTime" 
long GetTimestamp()
{
  return (millis() - startTime)/1000;
}

/// (re)set the "startTime" to current time 
void ResetStartTime()
{
  startTime=millis();
} 


/// DA-conversion. 
/// Reverse the measured voltage from a given digital value.
/// @param dval: the digital valu 0-1024.
/// @return aval: calculated analog value.
float CalVolt(int dval)
{
  // Calculate back voltage values
  float aval= ((float)dval/1024)*5;
  return aval;
  
  #if DEBUG  
  Serial.println("**Method CalVolt**\nValue In:"+ String(dval)+"\n--> Result (voltage): "+String(aval));
  Serial.println();
  #endif
}