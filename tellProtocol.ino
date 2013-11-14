

/* Raspberry
 * 
 * echo "SZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ}+" | tdtool --raw -
 * echo "SZZZ####ZZZZ####}+" | tdtool --raw -
 *
 * check RCSwitch.cpp
 *
 */

const int MAX_CHANGES = 78;
int pin = 13;

volatile int state = LOW;
volatile unsigned int duration;
volatile unsigned int highDuration;
volatile unsigned int flag;
volatile unsigned int preamble;

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

  digitalWrite(pin, state);

  if (preamble)
  {
      Serial.println("flagg");
      for (int i=0;i<MAX_CHANGES;i++)
      {
        Serial.print(i);
        Serial.print(":");
        Serial.print(timings[i]);
        Serial.print(",");
        timings[i] = 1;
      }
      Serial.println("");
      Serial.println(highDuration);
      Serial.println("");
      flag = false;
      preamble = false;
  }
}

/*
Z gir rett 900 micros (0.9millis)



*/

bool protocol(int changeCount)
{
}

bool checkPreamble()
{
  /*
  if (timings[0] > 500 && timings[0] < 1300 )
  {
    if (timings[1] > 500 && timings[1] < 1300 )
    {
      if (timings[2] > 500 && timings[2] < 1300 )
      {
        return true;
      }
    }
  }
  */
  int l = 100;
  int h = 1700;

  if (timings[0] > l && timings[0] < h )
  {
    if (timings[1] > l && timings[1] < h )
    {
      if (timings[2] > l && timings[2] < h )
      {
        return true;
      }
    }
  }

  return false;  
}

void blink()
{
  static unsigned int duration;
  static unsigned int changeCount;
  static unsigned long lastTime;
  static unsigned int repeatCount;
  long time = micros();
  duration = time - lastTime;

  //if (duration > 5000 && duration > RCSwitch::timings[0] - 200 && duration < RCSwitch::timings[0] + 200) {
  if (duration > 5000)
  {
    //repeatCount++;
    //changeCount--;

    // @todo 60 is bogus...
    if ( changeCount >= 3 )
    {
      if ( checkPreamble() )
      {
        preamble = true;
      }
    }
  }
  
  if ( duration > highDuration )
  {
    highDuration = duration;
  }

 
  // @todo 70 is bogus, should be MAX_CHANGES
  if (changeCount >= 70)
  {
    changeCount = 0;
    repeatCount = 0;

    flag = true;
  }

  state = !state;
  timings[changeCount++] = duration;
  lastTime = time;
}




