

/* Raspberry
 * 
 * echo "SZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ}+" | tdtool --raw -
 * echo "SZZZ####ZZZZ####}+" | tdtool --raw -
 *
 * check RCSwitch.cpp
 *
 */

#define RCSWITCH_MAX_CHANGES 67

const int MAX_CHANGES = 78;
int pin = 13;

volatile int state = LOW;
volatile int state2 = LOW;

volatile unsigned int changeCount;

volatile unsigned int duration;
volatile unsigned int highDuration;
volatile unsigned int flag;
volatile unsigned int preamble;

volatile unsigned int receivedData;

unsigned int timings[MAX_CHANGES];

void setup()
{
  Serial.begin(9600);
  Serial.println("START");

  pinMode(pin, OUTPUT);
  attachInterrupt(0, blink, CHANGE);

}

void loop()
{

  digitalWrite(pin, state2);

  if (receivedData)
  {
      Serial.println("flagg");
      for (int i=0;i<MAX_CHANGES;i++)
      {
        Serial.print(i);
        Serial.print(":");
        Serial.print(timings[i]);
        Serial.print(",");
      }
      Serial.println("");
      Serial.println(highDuration);
      Serial.println("");

      Serial.println("changeCount:");
      Serial.println(changeCount);
      Serial.println("");

      flag = false;
      preamble = false;
      receivedData = false;
  }
}

/*
Z gir rett 900 micros (0.9millis)



*/

bool protocol(int changeCount)
{
}


/*
bool receiveProtocol1(unsigned int changeCount){
      unsigned long code = 0;
      unsigned long delay = RCSwitch::timings[0] / 31;
      unsigned long delayTolerance = delay * RCSwitch::nReceiveTolerance * 0.01;    
      for (int i = 1; i<changeCount ; i=i+2) {
          if (RCSwitch::timings[i] > delay-delayTolerance && RCSwitch::timings[i] < delay+delayTolerance && RCSwitch::timings[i+1] > delay*3-delayTolerance && RCSwitch::timings[i+1] < delay*3+delayTolerance) {
            code = code << 1;
          } else if (RCSwitch::timings[i] > delay*3-delayTolerance && RCSwitch::timings[i] < delay*3+delayTolerance && RCSwitch::timings[i+1] > delay-delayTolerance && RCSwitch::timings[i+1] < delay+delayTolerance) {
            code+=1;
            code = code << 1;
          } else {
            // Failed
            i = changeCount;
            code = 0;
          }
      }      
      code = code >> 1;
    if (changeCount > 6) {    // ignore < 4bit values as there are no devices sending 4bit values => noise
      RCSwitch::nReceivedValue = code;
      RCSwitch::nReceivedBitlength = changeCount / 2;
      RCSwitch::nReceivedDelay = delay;
      RCSwitch::nReceivedProtocol = 1;
    }

    if (code == 0){
        return false;
    }else if (code != 0){
        return true;
    }
}
*/



bool checkPreamble(int changeCount)
{
  int l = 500;
  int h = 1600;
  int retVal = false;

  if (timings[1] > l && timings[1] < h )
  {
    if (timings[2] > l && timings[2] < h )
    {
      if (timings[3] > l && timings[3] < h )
      {
        /*for (int i=0;i<changeCount;i++)
        {
          if (timings[i] > 3000 )
          {
            //return false;
          }
        }
        */
        retVal = true;
      }
    }
  }

  //if (retVal && changeCount > 6)
  if (retVal)
  {
    // ignore < 4bit values as there are no devices sending 4bit values => noise
    //RCSwitch::nReceivedValue = code;
    //RCSwitch::nReceivedBitlength = changeCount / 2;
    //RCSwitch::nReceivedDelay = delay;
    //RCSwitch::nReceivedProtocol = 1;
    receivedData = true;
  }

  return retVal;  
}

void blink()
{
  static unsigned int duration;
  static unsigned long lastTime;
  static unsigned int repeatCount;
  long time = micros();

  duration = time - lastTime;

 

  //if (duration > 5000 && duration > RCSwitch::timings[0] - 200 && duration < RCSwitch::timings[0] + 200) {
  //if (duration > 5000 && duration > timings[0] - 2000 && duration < timings[0] + 2000)
  if (duration > 4000 && duration > timings[0] - 2000 && duration < timings[0] + 2000)
  {
    repeatCount++;
    changeCount--;
    if (repeatCount == 1) {
      if (checkPreamble(changeCount) == false){
        //failed
      }
      repeatCount = 0;
    }
    changeCount = 0;
  }
  else if (duration > 4000)
  {
    changeCount = 0;
  }

 

  if (changeCount >= RCSWITCH_MAX_CHANGES)
  {
    changeCount = 0;
    repeatCount = 0;
  }

  //RCSwitch::timings[changeCount++] = duration;
  timings[changeCount++] = duration;
  lastTime = time;  

  return;

  //if (duration > 5000 && duration > RCSwitch::timings[0] - 200 && duration < RCSwitch::timings[0] + 200) {
  if (duration > 5000)
  {
    //repeatCount++;
    //changeCount--;

    // @todo 60 is bogus...
    if ( changeCount >= 3 )
    {
      if ( checkPreamble(changeCount) )
      {
        preamble = true;
        state2 = !state2;
      }
    }
  }
  

  state = !state;
  timings[changeCount++] = duration;
  lastTime = time;
}




