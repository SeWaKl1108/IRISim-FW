
/* 
/// Methods
*/
//Method to check if "value" deviates by more than the specified percentage (deviation) 
bool IsChange(float origval,float newval, int deviation,int threshold=1)
{
  if(origval!=0 && newval!=0)
  {
    float deviation=(abs(newval-origval)/origval)*100;
    return (deviation>=1)? true:false;
  }  
  return false;
}

long GetTimestamp()
{
  return (millis() - startTime)/1000;
}

// create a message object from given string
DefSerMsg MsgBuilder(String str)
{  
  DefSerMsg msgToSend;
  msgToSend.message = str+"\n";  
  return msgToSend;
}


void CalVolt(int v1, int v2, int v3)
{
  // Calculate back voltage values
  float v_v1= ((float)v1/1024)*5;
  float v_v2=((float)v2/1024)*5;
  float v_v3=((float)v3/1024)*5;

  Serial.println("Value1: "+ String(v1)+" --> Voltage: "+String(v_v1));
  Serial.println("Value2: "+ String(v2)+" --> Voltage: "+String(v_v2));
  Serial.println("Value3: "+ String(v3)+" --> Voltage: "+String(v_v3));
  Serial.println();
}