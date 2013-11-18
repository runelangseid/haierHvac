

/* Raspberry
 * 
 * echo 'SZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ}+' | tdtool --raw -
 * echo 'S$k$k$k$+' | tdtool --raw -
 *
 * check RCSwitch.cpp
 *
 * Send/Receive
 * - PHP - send fra dec 32 (SPACE). Øk med 5 for hvert tegn. Gir oss 19 tegn.
 * - 1234567890 ABCDEFGHI
 *
 *
 * Conversion table for keys
 * http://no.wikipedia.org/wiki/ASCII
 *
 Status
 
 skissa tar i mot light fra openhab
 - utvikle skissa til å decode mottatt data
 
 * Numbers    (ASCII * 10)  char
 * 1          320           P
 * 2          400 Z
 * 3          480          d
 * 4          560          n
 * 5          640          
 * 6          720
 * 7          800
 * 8          880
 * 9          960
 * 0          104
 *
 * A          112
 * B          120
 * C          127
 *
 */

//#define RCSWITCH_MAX_CHANGES 67

const int MAX_CHANGES = 78;
int pin = 13;

volatile int state = LOW;

volatile unsigned int changeCount;
volatile unsigned int changeCountP;

volatile unsigned int duration;
volatile unsigned int highDuration;
volatile unsigned int flag;
volatile unsigned int preamble;

volatile unsigned int receivedData;

unsigned int timings[MAX_CHANGES];
unsigned int timingsP[MAX_CHANGES];

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

  char r;
  int errorsC = 0;
  unsigned int errors[MAX_CHANGES];
  unsigned int errorsT[MAX_CHANGES];

  if (receivedData)
  {
      Serial.println("flagg");
      for (int i=4;i<changeCountP+1;i++)
      {
        r = decode( timingsP[i] );
        if ( r == 'Z' )
        {
          errors[errorsC] = i;
          errorsT[errorsC++] = timingsP[i];
        }

        Serial.print(i-4);
        Serial.print(":");
        Serial.print(timingsP[i]);
        //Serial.print( decode( timingsP[i] ));
        Serial.print(",");
      }

      Serial.println("");
      for (int i=4;i<changeCountP+1;i++)
      {
        Serial.print( decode( timingsP[i] ));
      }

      Serial.println("");
      Serial.println("Errors identified:");
      for (int i=0;i<errorsC;i++)
      {
        Serial.print( errors[i] );
        Serial.print( ":" );
        Serial.print( errorsT[i] );
        Serial.print( "," );
      }

      Serial.println("");
      Serial.println("chars received:");
      Serial.println(changeCountP-3);
      Serial.println("");

      flag = false;
      preamble = false;
      receivedData = false;
  }
}

/*
Z gir rett 900 micros (0.9millis)



*/

char decode(int v)
{
  if ( v > 240 && v < 359 )
    return '1';
  else if ( v >= 360 && v < 440 )
    return '2';
  else if ( v >= 440 && v < 520 )
    return '3';
  else if ( v >= 520 && v < 600 )
    return '4';
  else if ( v >= 600 && v < 680 )
    return '5';
  else if ( v >= 680 && v < 760 )
    return '6';
  else if ( v >= 760 && v < 840 )
    return '7';
  else if ( v >= 840 && v < 920 )
    return '8';
  else if ( v >= 920 && v < 1000 )
    return '9';
  else if ( v >= 1000 && v < 1080 )
    return '0';

  else if ( v >= 1080 && v < 1160 )
    return 'A';
  else if ( v >= 1160 && v < 1240 )
    return 'B';
  else if ( v >= 1240 && v < 1320 )
    return 'C';

  return 'Z';
}



bool checkPreamble(int changeCount)
{
  int l = 800;
  int h = 1100;
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
    receivedData = true;
    memcpy( timingsP, timings, sizeof(timingsP) );
    changeCountP = changeCount;

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


  //if (duration > 5000 && duration > timings[0] - 2000 && duration < timings[0] + 2000)
  //if (duration > 7000 && duration > timings[0] - 2000 && duration < timings[0] + 2000)
  if (duration > 7000)
  {
    state = !state;

    repeatCount++;
    changeCount--;
    if (repeatCount == 2) {
      if (checkPreamble(changeCount) == false){
        //failed
      }
      repeatCount = 0;
    }
    changeCount = 0;
  }
  else if (duration > 7000)
  {
    changeCount = 0;
  }


  if (changeCount >= MAX_CHANGES)
  {
    changeCount = 0;
    repeatCount = 0;
  }

  timings[changeCount++] = duration;
  lastTime = time;

}




