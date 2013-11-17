

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
 *
 *
 *
flagg
0:12220,1:944,2:864,3:904,4:528,5:748,6:348,7:624,8:484,9:456,10:652,11:572,12:520,13:1588,14:484,15:772,16:380,17:672,18:428,19:492,20:556,21:808,22:260,23:672,24:432,25:560,26:504,27:612,28:540,29:752,30:336,31:492,32:460,33:540,34:528,35:652,36:2676,37:112,38:32,39:0,40:0,41:0,42:0,43:0,44:0,45:0,46:0,47:0,48:0,49:0,50:0,51:0,52:0,53:0,54:0,55:0,56:0,57:0,58:0,59:0,60:0,61:0,62:0,63:0,64:0,65:0,66:0,67:0,68:0,69:0,70:0,71:0,72:0,73:0,74:0,75:0,76:0,77:0,

 */

#define RCSWITCH_MAX_CHANGES 67

const int MAX_CHANGES = 78;
int pin = 13;

volatile int state = LOW;
volatile int state2 = LOW;

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

  digitalWrite(pin, state2);

  if (receivedData)
  {
      Serial.println("flagg");
      for (int i=4;i<changeCountP+1;i++)
      {
        Serial.print(i-4);
        Serial.print(":");
        Serial.print(timingsP[i]);
        //Serial.print( decode( timingsP[i] ));
        Serial.print(",");
      }
      Serial.println("");
      Serial.println(highDuration);
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
  if ( v < 240 && v > 359 )
    return '1';
  if ( v < 360 && v > 439 )
    return '2';
  if ( v < 440 && v > 519 )
    return '3';
  if ( v < 520 && v > 599 )
    return '4';
  if ( v < 600 && v > 679 )
    return '5';
  if ( v < 680 && v > 759 )
    return '6';
  if ( v < 760 && v > 839 )
    return '7';
  if ( v < 840 && v > 919 )
    return '8';
  if ( v < 920 && v > 999 )
    return '9';
  if ( v < 1000 && v > 1079 )
    return '0';

  if ( v < 1080 && v > 1159 )
    return 'A';
  if ( v < 1160 && v > 1239 )
    return 'B';
  if ( v < 1240 && v > 1319 )
    return 'C';
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
    // ignore < 4bit values as there are no devices sending 4bit values => noise
    //RCSwitch::nReceivedValue = code;
    //RCSwitch::nReceivedBitlength = changeCount / 2;
    //RCSwitch::nReceivedDelay = delay;
    //RCSwitch::nReceivedProtocol = 1;
    receivedData = true;
    memcpy( timingsP, timings, MAX_CHANGES );
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




