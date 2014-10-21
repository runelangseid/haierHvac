
/** Arduino HVAC wireless IR transmitter
 * Connect wireless 433.92mhz to D2 - Interrupt 1 on UNO
 *
 * Notes
 * - tdtool sends a raw command 10 times
 *
 *
 * First three Z's are ignored (used as preamble)
 * Next, use Z as logic 1, \20x as logic 0
 * Z is 900, \20x is > 100
 *
 * Received value is 128 - echo -e "SZZZZ\x20\x20\x20\x20\x20\x20\x20+" | tdtool --raw - 2>&1
 *
 * Codes
 * 20 - 17C - 0001 0100
 * echo -e "SZZZ\x20\x20\x20Z\x20Z\x20\x20+" | tdtool --raw - 2>&1
 *
 * 25 - 22C - 0001 1001
 * echo -e "SZZZ\x20\x20\x20ZZ\x20\x20Z+" | tdtool --raw - 2>&1
 *
 * 30 - Soft On - 0001 1110
 * echo -e "SZZZ\x20\x20\x20ZZZZ\x20+" | tdtool --raw - 2>&1
 *
 * 35 - Soft Off - 0010 0011
 * echo -e "SZZZ\x20\x20Z\x20\x20\x20ZZ+" | tdtool --raw - 2>&1
 *
 * Bugs
 * - first transmission after successfull retrieval submit value again
 * - preamble does not work correctly / other things gets through
 * - better handling of repeated codes within 1(?) seconds
 *
 */

const int MAX_CHANGES = 78;

volatile unsigned int duration;
volatile unsigned int highDuration;
volatile unsigned int start;

unsigned int timings[MAX_CHANGES];

volatile int nReceivedValue;
volatile int nReceivedBitlength;
volatile int nReceivedDelay;
volatile int nReceivedProtocol = 0;

// IR
int IRledPin =  13;    // LED connected to digital pin 13


void setup()
{
  Serial.begin(9600);
  Serial.println("START");

  // Setup interrupt routine
  attachInterrupt(0, handleInterrupt, CHANGE);

  // IR - Initialize the digital pin as an output:
  pinMode(IRledPin, OUTPUT);

}

void loop()
{
  if (nReceivedProtocol == 1)
  {
    Serial.print("nReceivedValue:");
    Serial.println(nReceivedValue);

    if ( nReceivedValue == 20 )
    {
      Serial.println("Sending 17");
      Send17();
    }
    else if ( nReceivedValue == 25 )
    {
      Serial.println("Sending 22");
      Send22();
    }
    else if ( nReceivedValue == 30 )
    {
      Serial.println("Sending Soft on");
      SendSoftOn();
    }
    else if ( nReceivedValue == 35 )
    {
      Serial.println("Sending Soft off");
      SendSoftOff();
    }

    nReceivedProtocol = 0;
    nReceivedBitlength = 0;
    nReceivedValue = 0;

  }
}

/** Expected preamble is 3 pules of =900 (Z)
 * Send command: echo -e "SZZZABCD+" | tdtool --raw - 2>&1
 *
 */
bool checkPreamble()
{
  int l = 800;
  int h = 1000;

  if (timings[1] > l && timings[1] < h )
  {
    if (timings[2] > l && timings[2] < h )
    {
      if (timings[3] > l && timings[3] < h )
      {
        return true;
      }
    }
  }

  return false;
}

/** Check if code has been sent
 *
 */
bool receivedCode(unsigned int changedCount)
{
  if ( checkPreamble() )
  {

    for (int i = 4; i<changedCount; i++)
    {
      if (timings[i] > 100)
      {
        // Got a 1 bit
        nReceivedValue = nReceivedValue << 1;
        if (timings[i] > 800)
        {
          nReceivedValue++;
        }
      }

    }

    // 12 is 8 bit
    if (changedCount >= 12)
    {
      //detachInterrupt(0);
      nReceivedBitlength = changedCount;
      nReceivedProtocol = 1;
    }
    return true;

  }

}

void handleInterrupt()
{
  static unsigned int duration;
  static unsigned int changeCount;
  static unsigned long lastTime;
  static unsigned int repeatCount;
  long time = micros();
  duration = time - lastTime;

  //if (duration > 5000 && duration > RCSwitch::timings[0] - 200 && duration < RCSwitch::timings[0] + 200) {
  // - tellstick sends 0 for > 5000 before each packet
  if (duration > 5000)
  {
    //repeatCount++;
    //changeCount--;

    if ( changeCount >= 3 )
    {
      if ( receivedCode(changeCount) == false )
      {
        // failed
      }
    }

    changeCount = 0;
  }
  
  // highDuration is debug
  if ( duration > highDuration )
  {
    highDuration = duration;
  }

 
  // 12 is 8 bit
  if (changeCount >= 12)
  {
    changeCount = 0;
    repeatCount = 0;
  }

  timings[changeCount++] = duration;
  lastTime = time;
}


// IR stuff
// This procedure sends a 38KHz pulse to the IRledPin 
// for a certain # of microseconds. We'll use this whenever we need to send codes
void pulseIR(long microsecs) {
  // we'll count down from the number of microseconds we are told to wait

  cli();  // this turns off any background interrupts

  while (microsecs > 0) {
    // 38 kHz is about 13 microseconds high and 13 microseconds low
   digitalWrite(IRledPin, HIGH);  // this takes about 3 microseconds to happen
   delayMicroseconds(10);         // hang out for 10 microseconds
   digitalWrite(IRledPin, LOW);   // this also takes about 3 microseconds
   delayMicroseconds(10);         // hang out for 10 microseconds

   // so 26 microseconds altogether
   microsecs -= 26;
  }

  sei();  // this turns them back on
}

void SendLight() {
  pulseIR(3000);delayMicroseconds(3000);pulseIR(3040);delayMicroseconds(4320); pulseIR(580);delayMicroseconds(1600);pulseIR(580);delayMicroseconds(540);pulseIR(560);delayMicroseconds(1620);pulseIR(540);delayMicroseconds(540);pulseIR(580);delayMicroseconds(520); pulseIR(580); delayMicroseconds(1600); pulseIR(580); delayMicroseconds(1580); pulseIR(560); delayMicroseconds(540); pulseIR(580); delayMicroseconds(500); pulseIR(580); delayMicroseconds(1600); pulseIR(580); delayMicroseconds(540); pulseIR(540); delayMicroseconds(540); pulseIR(560); delayMicroseconds(1620); pulseIR(560); delayMicroseconds(520); pulseIR(600); delayMicroseconds(1580); pulseIR(580); delayMicroseconds(520); pulseIR(580); delayMicroseconds(500); pulseIR(580); delayMicroseconds(540); pulseIR(540); delayMicroseconds(1620); pulseIR(580); delayMicroseconds(1600); pulseIR(580); delayMicroseconds(520); pulseIR(560); delayMicroseconds(1620); pulseIR(560); delayMicroseconds(1600); pulseIR(580); delayMicroseconds(1600); pulseIR(540); delayMicroseconds(540); pulseIR(580); delayMicroseconds(520); pulseIR(580); delayMicroseconds(520); pulseIR(540); delayMicroseconds(540); pulseIR(580); delayMicroseconds(520); pulseIR(580); delayMicroseconds(520); pulseIR(560); delayMicroseconds(520); pulseIR(580); delayMicroseconds(520); pulseIR(560); delayMicroseconds(540); pulseIR(580); delayMicroseconds(1600); pulseIR(580); delayMicroseconds(500); pulseIR(580); delayMicroseconds(1600); pulseIR(580); delayMicroseconds(1600); pulseIR(580); delayMicroseconds(500); pulseIR(580); delayMicroseconds(520); pulseIR(580); delayMicroseconds(520); pulseIR(560); delayMicroseconds(540); pulseIR(580); delayMicroseconds(1600); pulseIR(560); delayMicroseconds(1620); pulseIR(560); delayMicroseconds(520); pulseIR(580); delayMicroseconds(500); pulseIR(580); delayMicroseconds(540); pulseIR(480); delayMicroseconds(600); pulseIR(540); delayMicroseconds(560); pulseIR(580); delayMicroseconds(500); pulseIR(580); delayMicroseconds(540); pulseIR(540); delayMicroseconds(540); pulseIR(560); delayMicroseconds(540); pulseIR(560); delayMicroseconds(520); pulseIR(580); delayMicroseconds(540); pulseIR(520); delayMicroseconds(560); pulseIR(540); delayMicroseconds(540); pulseIR(580); delayMicroseconds(1600); pulseIR(580); delayMicroseconds(520); pulseIR(580); delayMicroseconds(500); pulseIR(580); delayMicroseconds(540); pulseIR(540); delayMicroseconds(540); pulseIR(580); delayMicroseconds(500); pulseIR(580); delayMicroseconds(520); pulseIR(580); delayMicroseconds(520); pulseIR(540); delayMicroseconds(540); pulseIR(580); delayMicroseconds(520); pulseIR(580); delayMicroseconds(520); pulseIR(560); delayMicroseconds(540); pulseIR(480); delayMicroseconds(600); pulseIR(580); delayMicroseconds(520); pulseIR(560); delayMicroseconds(520); pulseIR(580); delayMicroseconds(540); pulseIR(540); delayMicroseconds(540); pulseIR(580); delayMicroseconds(500); pulseIR(580); delayMicroseconds(520); pulseIR(580); delayMicroseconds(520); pulseIR(540); delayMicroseconds(560); pulseIR(580); delayMicroseconds(500); pulseIR(580); delayMicroseconds(500); pulseIR(600); delayMicroseconds(520); pulseIR(560); delayMicroseconds(520); pulseIR(560); delayMicroseconds(540); pulseIR(580); delayMicroseconds(500); pulseIR(580); delayMicroseconds(540); pulseIR(520); delayMicroseconds(560); pulseIR(580); delayMicroseconds(500); pulseIR(580); delayMicroseconds(520); pulseIR(580); delayMicroseconds(520); pulseIR(560); delayMicroseconds(540); pulseIR(540); delayMicroseconds(540); pulseIR(580); delayMicroseconds(520); pulseIR(560); delayMicroseconds(540); pulseIR(560); delayMicroseconds(540); pulseIR(540); delayMicroseconds(540); pulseIR(580); delayMicroseconds(500); pulseIR(580); delayMicroseconds(520); pulseIR(580); delayMicroseconds(520); pulseIR(560); delayMicroseconds(520); pulseIR(600); delayMicroseconds(500); pulseIR(580); delayMicroseconds(520); pulseIR(560); delayMicroseconds(540); pulseIR(560); delayMicroseconds(520); pulseIR(580); delayMicroseconds(520); pulseIR(580); delayMicroseconds(500); pulseIR(580); delayMicroseconds(540); pulseIR(540); delayMicroseconds(1640); pulseIR(540); delayMicroseconds(540); pulseIR(580); delayMicroseconds(1600); pulseIR(560); delayMicroseconds(1620); pulseIR(560); delayMicroseconds(1620); pulseIR(540); delayMicroseconds(1620); pulseIR(580); delayMicroseconds(1600); pulseIR(540);
}

void SendOff() {
  pulseIR(3040); delayMicroseconds(2980); pulseIR(2940); delayMicroseconds(4420); pulseIR(520); delayMicroseconds(1660); pulseIR(580); delayMicroseconds(520); pulseIR(600); delayMicroseconds(1560); pulseIR(540); delayMicroseconds(560); pulseIR(600); delayMicroseconds(500); pulseIR(520); delayMicroseconds(1660); pulseIR(580); delayMicroseconds(1580); pulseIR(540); delayMicroseconds(580); pulseIR(500); delayMicroseconds(580); pulseIR(560); delayMicroseconds(1620); pulseIR(540); delayMicroseconds(1640); pulseIR(480); delayMicroseconds(600); pulseIR(580); delayMicroseconds(1600); pulseIR(520); delayMicroseconds(580); pulseIR(600); delayMicroseconds(1560); pulseIR(500); delayMicroseconds(600); pulseIR(600); delayMicroseconds(520); pulseIR(580); delayMicroseconds(500); pulseIR(480); delayMicroseconds(1700); pulseIR(480); delayMicroseconds(1700); pulseIR(500); delayMicroseconds(580); pulseIR(500); delayMicroseconds(1680); pulseIR(500); delayMicroseconds(1660); pulseIR(600); delayMicroseconds(1580); pulseIR(600); delayMicroseconds(500); pulseIR(600); delayMicroseconds(500); pulseIR(580); delayMicroseconds(500); pulseIR(480); delayMicroseconds(620); pulseIR(600); delayMicroseconds(500); pulseIR(580); delayMicroseconds(520); pulseIR(580); delayMicroseconds(500); pulseIR(580); delayMicroseconds(520); pulseIR(560); delayMicroseconds(520); pulseIR(600); delayMicroseconds(500); pulseIR(580); delayMicroseconds(1580); pulseIR(540); delayMicroseconds(1640); pulseIR(600); delayMicroseconds(520); pulseIR(500); delayMicroseconds(1680); pulseIR(500); delayMicroseconds(1660); pulseIR(600); delayMicroseconds(500); pulseIR(520); delayMicroseconds(560); pulseIR(500); delayMicroseconds(1680); pulseIR(500); delayMicroseconds(1680); pulseIR(500); delayMicroseconds(600); pulseIR(560); delayMicroseconds(520); pulseIR(600); delayMicroseconds(500); pulseIR(600); delayMicroseconds(520); pulseIR(560); delayMicroseconds(520); pulseIR(500); delayMicroseconds(580); pulseIR(600); delayMicroseconds(500); pulseIR(580); delayMicroseconds(520); pulseIR(480); delayMicroseconds(600); pulseIR(580); delayMicroseconds(520); pulseIR(580); delayMicroseconds(500); pulseIR(600); delayMicroseconds(520); pulseIR(500); delayMicroseconds(580); pulseIR(480); delayMicroseconds(1700); pulseIR(480); delayMicroseconds(600); pulseIR(600); delayMicroseconds(500); pulseIR(600); delayMicroseconds(500); pulseIR(580); delayMicroseconds(520); pulseIR(480); delayMicroseconds(600); pulseIR(600); delayMicroseconds(480); pulseIR(600); delayMicroseconds(500); pulseIR(520); delayMicroseconds(580); pulseIR(500); delayMicroseconds(600); pulseIR(560); delayMicroseconds(520); pulseIR(600); delayMicroseconds(520); pulseIR(500); delayMicroseconds(580); pulseIR(480); delayMicroseconds(600); pulseIR(620); delayMicroseconds(480); pulseIR(580); delayMicroseconds(500); pulseIR(600); delayMicroseconds(520); pulseIR(480); delayMicroseconds(600); pulseIR(520); delayMicroseconds(580); pulseIR(520); delayMicroseconds(600); pulseIR(500); delayMicroseconds(580); pulseIR(480); delayMicroseconds(600); pulseIR(500); delayMicroseconds(580); pulseIR(580); delayMicroseconds(520); pulseIR(600); delayMicroseconds(500); pulseIR(520); delayMicroseconds(580); pulseIR(500); delayMicroseconds(580); pulseIR(600); delayMicroseconds(500); pulseIR(520); delayMicroseconds(580); pulseIR(480); delayMicroseconds(600); pulseIR(520); delayMicroseconds(580); pulseIR(500); delayMicroseconds(1680); pulseIR(560); delayMicroseconds(520); pulseIR(480); delayMicroseconds(600); pulseIR(540); delayMicroseconds(560); pulseIR(600); delayMicroseconds(500); pulseIR(600); delayMicroseconds(500); pulseIR(480); delayMicroseconds(600); pulseIR(540); delayMicroseconds(560); pulseIR(520); delayMicroseconds(580); pulseIR(520); delayMicroseconds(580); pulseIR(480); delayMicroseconds(600); pulseIR(600); delayMicroseconds(500); pulseIR(520); delayMicroseconds(580); pulseIR(500); delayMicroseconds(580); pulseIR(480); delayMicroseconds(1700); pulseIR(480); delayMicroseconds(600); pulseIR(500); delayMicroseconds(1660); pulseIR(540); delayMicroseconds(580); pulseIR(580); delayMicroseconds(1600); pulseIR(560); delayMicroseconds(1600); pulseIR(600); delayMicroseconds(500); pulseIR(480); delayMicroseconds(600); pulseIR(580); delayMicroseconds(520); pulseIR(540); delayMicroseconds(1640); pulseIR(480); delayMicroseconds(1680); pulseIR(520);
}

void SendOn() {
  pulseIR(2940); delayMicroseconds(3080); pulseIR(2980); delayMicroseconds(4380); pulseIR(540); delayMicroseconds(1640); pulseIR(540); delayMicroseconds(600); pulseIR(460); delayMicroseconds(1680); pulseIR(520); delayMicroseconds(600); pulseIR(480); delayMicroseconds(580); pulseIR(540); delayMicroseconds(1640); pulseIR(540); delayMicroseconds(1640); pulseIR(480); delayMicroseconds(600); pulseIR(540); delayMicroseconds(580); pulseIR(520); delayMicroseconds(1640); pulseIR(540); delayMicroseconds(1620); pulseIR(560); delayMicroseconds(560); pulseIR(520); delayMicroseconds(1640); pulseIR(540); delayMicroseconds(580); pulseIR(480); delayMicroseconds(1660); pulseIR(540); delayMicroseconds(620); pulseIR(440); delayMicroseconds(600); pulseIR(520); delayMicroseconds(580); pulseIR(540); delayMicroseconds(1620); pulseIR(560); delayMicroseconds(1620); pulseIR(560); delayMicroseconds(540); pulseIR(540); delayMicroseconds(1640); pulseIR(540); delayMicroseconds(1640); pulseIR(500); delayMicroseconds(1680); pulseIR(520); delayMicroseconds(600); pulseIR(460); delayMicroseconds(600); pulseIR(540); delayMicroseconds(540); pulseIR(540); delayMicroseconds(580); pulseIR(500); delayMicroseconds(580); pulseIR(480); delayMicroseconds(620); pulseIR(540); delayMicroseconds(560); pulseIR(520); delayMicroseconds(560); pulseIR(520); delayMicroseconds(580); pulseIR(480); delayMicroseconds(1700); pulseIR(480); delayMicroseconds(1680); pulseIR(520); delayMicroseconds(1660); pulseIR(520); delayMicroseconds(580); pulseIR(480); delayMicroseconds(1700); pulseIR(480); delayMicroseconds(1680); pulseIR(540); delayMicroseconds(1640); pulseIR(500); delayMicroseconds(580); pulseIR(540); delayMicroseconds(1640); pulseIR(540); delayMicroseconds(1640); pulseIR(540); delayMicroseconds(580); pulseIR(520); delayMicroseconds(560); pulseIR(520); delayMicroseconds(560); pulseIR(500); delayMicroseconds(600); pulseIR(480); delayMicroseconds(620); pulseIR(520); delayMicroseconds(560); pulseIR(520); delayMicroseconds(580); pulseIR(480); delayMicroseconds(640); pulseIR(500); delayMicroseconds(560); pulseIR(540); delayMicroseconds(600); pulseIR(480); delayMicroseconds(580); pulseIR(480); delayMicroseconds(600); pulseIR(500); delayMicroseconds(600); pulseIR(520); delayMicroseconds(1640); pulseIR(540); delayMicroseconds(600); pulseIR(500); delayMicroseconds(580); pulseIR(500); delayMicroseconds(580); pulseIR(500); delayMicroseconds(580); pulseIR(540); delayMicroseconds(580); pulseIR(520); delayMicroseconds(580); pulseIR(500); delayMicroseconds(600); pulseIR(460); delayMicroseconds(600); pulseIR(540); delayMicroseconds(580); pulseIR(500); delayMicroseconds(580); pulseIR(520); delayMicroseconds(560); pulseIR(520); delayMicroseconds(580); pulseIR(540); delayMicroseconds(560); pulseIR(520); delayMicroseconds(580); pulseIR(500); delayMicroseconds(580); pulseIR(480); delayMicroseconds(600); pulseIR(540); delayMicroseconds(560); pulseIR(540); delayMicroseconds(560); pulseIR(520); delayMicroseconds(580); pulseIR(480); delayMicroseconds(640); pulseIR(500); delayMicroseconds(560); pulseIR(520); delayMicroseconds(600); pulseIR(500); delayMicroseconds(600); pulseIR(460); delayMicroseconds(600); pulseIR(540); delayMicroseconds(560); pulseIR(520); delayMicroseconds(580); pulseIR(520); delayMicroseconds(600); pulseIR(460); delayMicroseconds(600); pulseIR(540); delayMicroseconds(540); pulseIR(540); delayMicroseconds(560); pulseIR(520); delayMicroseconds(1640); pulseIR(520); delayMicroseconds(600); pulseIR(520); delayMicroseconds(580); pulseIR(500); delayMicroseconds(580); pulseIR(480); delayMicroseconds(600); pulseIR(520); delayMicroseconds(560); pulseIR(540); delayMicroseconds(580); pulseIR(520); delayMicroseconds(580); pulseIR(500); delayMicroseconds(580); pulseIR(500); delayMicroseconds(580); pulseIR(540); delayMicroseconds(560); pulseIR(520); delayMicroseconds(620); pulseIR(440); delayMicroseconds(620); pulseIR(500); delayMicroseconds(580); pulseIR(540); delayMicroseconds(1620); pulseIR(560); delayMicroseconds(580); pulseIR(500); delayMicroseconds(1640); pulseIR(500); delayMicroseconds(1680); pulseIR(520); delayMicroseconds(560); pulseIR(520); delayMicroseconds(1660); pulseIR(500); delayMicroseconds(600); pulseIR(540); delayMicroseconds(560); pulseIR(520); delayMicroseconds(1640); pulseIR(540); delayMicroseconds(580); pulseIR(520); delayMicroseconds(540); pulseIR(540);

}

void SendSoftOn() {
 pulseIR(2980); delayMicroseconds(3060); pulseIR(2920); delayMicroseconds(4420); pulseIR(520); delayMicroseconds(1660); pulseIR(520); delayMicroseconds(580); pulseIR(540); delayMicroseconds(1640); pulseIR(540); delayMicroseconds(580); pulseIR(500); delayMicroseconds(580); pulseIR(500); delayMicroseconds(1680); pulseIR(500); delayMicroseconds(1640); pulseIR(560); delayMicroseconds(600); pulseIR(480); delayMicroseconds(580); pulseIR(500); delayMicroseconds(1660); pulseIR(500); delayMicroseconds(1680); pulseIR(500); delayMicroseconds(600); pulseIR(500); delayMicroseconds(1680); pulseIR(500); delayMicroseconds(600); pulseIR(520); delayMicroseconds(1640); pulseIR(500); delayMicroseconds(600); pulseIR(540); delayMicroseconds(560); pulseIR(520); delayMicroseconds(580); pulseIR(480); delayMicroseconds(1680); pulseIR(520); delayMicroseconds(580); pulseIR(500); delayMicroseconds(580); pulseIR(540); delayMicroseconds(580); pulseIR(520); delayMicroseconds(560); pulseIR(520); delayMicroseconds(580); pulseIR(480); delayMicroseconds(600); pulseIR(540); delayMicroseconds(560); pulseIR(520); delayMicroseconds(580); pulseIR(520); delayMicroseconds(560); pulseIR(520); delayMicroseconds(580); pulseIR(540); delayMicroseconds(580); pulseIR(500); delayMicroseconds(560); pulseIR(520); delayMicroseconds(580); pulseIR(480); delayMicroseconds(620); pulseIR(520); delayMicroseconds(1640); pulseIR(520); delayMicroseconds(580); pulseIR(540); delayMicroseconds(560); pulseIR(520); delayMicroseconds(1640); pulseIR(540); delayMicroseconds(580); pulseIR(480); delayMicroseconds(600); pulseIR(520); delayMicroseconds(1640); pulseIR(540); delayMicroseconds(580); pulseIR(500); delayMicroseconds(1660); pulseIR(520); delayMicroseconds(1640); pulseIR(540); delayMicroseconds(580); pulseIR(500); delayMicroseconds(580); pulseIR(520); delayMicroseconds(580); pulseIR(540); delayMicroseconds(540); pulseIR(540); delayMicroseconds(580); pulseIR(500); delayMicroseconds(1660); pulseIR(520); delayMicroseconds(620); pulseIR(440); delayMicroseconds(600); pulseIR(540); delayMicroseconds(560); pulseIR(540); delayMicroseconds(560); pulseIR(520); delayMicroseconds(560); pulseIR(500); delayMicroseconds(600); pulseIR(480); delayMicroseconds(600); pulseIR(540); delayMicroseconds(1640); pulseIR(540); delayMicroseconds(580); pulseIR(500); delayMicroseconds(620); pulseIR(480); delayMicroseconds(580); pulseIR(500); delayMicroseconds(580); pulseIR(540); delayMicroseconds(580); pulseIR(500); delayMicroseconds(560); pulseIR(540); delayMicroseconds(560); pulseIR(500); delayMicroseconds(600); pulseIR(540); delayMicroseconds(540); pulseIR(540); delayMicroseconds(580); pulseIR(500); delayMicroseconds(580); pulseIR(500); delayMicroseconds(600); pulseIR(520); delayMicroseconds(580); pulseIR(520); delayMicroseconds(580); pulseIR(500); delayMicroseconds(580); pulseIR(480); delayMicroseconds(600); pulseIR(540); delayMicroseconds(560); pulseIR(540); delayMicroseconds(600); pulseIR(480); delayMicroseconds(580); pulseIR(480); delayMicroseconds(600); pulseIR(540); delayMicroseconds(560); pulseIR(520); delayMicroseconds(560); pulseIR(520); delayMicroseconds(580); pulseIR(480); delayMicroseconds(620); pulseIR(500); delayMicroseconds(580); pulseIR(540); delayMicroseconds(560); pulseIR(520); delayMicroseconds(580); pulseIR(480); delayMicroseconds(600); pulseIR(540); delayMicroseconds(580); pulseIR(520); delayMicroseconds(580); pulseIR(500); delayMicroseconds(1640); pulseIR(500); delayMicroseconds(600); pulseIR(520); delayMicroseconds(580); pulseIR(520); delayMicroseconds(560); pulseIR(500); delayMicroseconds(600); pulseIR(500); delayMicroseconds(580); pulseIR(540); delayMicroseconds(580); pulseIR(500); delayMicroseconds(580); pulseIR(520); delayMicroseconds(600); pulseIR(480); delayMicroseconds(580); pulseIR(540); delayMicroseconds(580); pulseIR(500); delayMicroseconds(600); pulseIR(480); delayMicroseconds(580); pulseIR(520); delayMicroseconds(1660); pulseIR(480); delayMicroseconds(640); pulseIR(500); delayMicroseconds(560); pulseIR(540); delayMicroseconds(580); pulseIR(500); delayMicroseconds(1640); pulseIR(540); delayMicroseconds(1640); pulseIR(540); delayMicroseconds(1640); pulseIR(540); delayMicroseconds(620); pulseIR(460); delayMicroseconds(620); pulseIR(440); delayMicroseconds(620); pulseIR(520); delayMicroseconds(1640); pulseIR(500); delayMicroseconds(600); pulseIR(500);

}

void SendSoftOff() {
  pulseIR(3040); delayMicroseconds(2980); pulseIR(2920); delayMicroseconds(4440); pulseIR(500); delayMicroseconds(1680); pulseIR(500); delayMicroseconds(580); pulseIR(540); delayMicroseconds(1640); pulseIR(540); delayMicroseconds(580); pulseIR(500); delayMicroseconds(580); pulseIR(520); delayMicroseconds(1660); pulseIR(520); delayMicroseconds(1640); pulseIR(540); delayMicroseconds(580); pulseIR(500); delayMicroseconds(580); pulseIR(540); delayMicroseconds(540); pulseIR(540); delayMicroseconds(560); pulseIR(580); delayMicroseconds(1600); pulseIR(520); delayMicroseconds(1660); pulseIR(520); delayMicroseconds(580); pulseIR(520); delayMicroseconds(1640); pulseIR(540); delayMicroseconds(560); pulseIR(520); delayMicroseconds(580); pulseIR(480); delayMicroseconds(600); pulseIR(600); delayMicroseconds(1580); pulseIR(520); delayMicroseconds(580); pulseIR(520); delayMicroseconds(580); pulseIR(520); delayMicroseconds(580); pulseIR(480); delayMicroseconds(600); pulseIR(480); delayMicroseconds(600); pulseIR(540); delayMicroseconds(560); pulseIR(600); delayMicroseconds(500); pulseIR(480); delayMicroseconds(620); pulseIR(560); delayMicroseconds(520); pulseIR(540); delayMicroseconds(560); pulseIR(520); delayMicroseconds(580); pulseIR(480); delayMicroseconds(600); pulseIR(500); delayMicroseconds(600); pulseIR(540); delayMicroseconds(540); pulseIR(540); delayMicroseconds(1640); pulseIR(540); delayMicroseconds(580); pulseIR(500); delayMicroseconds(1680); pulseIR(500); delayMicroseconds(1660); pulseIR(520); delayMicroseconds(580); pulseIR(480); delayMicroseconds(1680); pulseIR(540); delayMicroseconds(560); pulseIR(500); delayMicroseconds(600); pulseIR(580); delayMicroseconds(1600); pulseIR(500); delayMicroseconds(1680); pulseIR(480); delayMicroseconds(600); pulseIR(540); delayMicroseconds(560); pulseIR(520); delayMicroseconds(580); pulseIR(500); delayMicroseconds(580); pulseIR(520); delayMicroseconds(580); pulseIR(520); delayMicroseconds(560); pulseIR(540); delayMicroseconds(580); pulseIR(500); delayMicroseconds(580); pulseIR(480); delayMicroseconds(620); pulseIR(580); delayMicroseconds(500); pulseIR(540); delayMicroseconds(560); pulseIR(520); delayMicroseconds(580); pulseIR(560); delayMicroseconds(520); pulseIR(580); delayMicroseconds(1600); pulseIR(500); delayMicroseconds(600); pulseIR(520); delayMicroseconds(560); pulseIR(540); delayMicroseconds(560); pulseIR(500); delayMicroseconds(600); pulseIR(500); delayMicroseconds(580); pulseIR(600); delayMicroseconds(500); pulseIR(580); delayMicroseconds(520); pulseIR(520); delayMicroseconds(580); pulseIR(480); delayMicroseconds(600); pulseIR(540); delayMicroseconds(560); pulseIR(580); delayMicroseconds(520); pulseIR(480); delayMicroseconds(600); pulseIR(580); delayMicroseconds(520); pulseIR(600); delayMicroseconds(480); pulseIR(600); delayMicroseconds(520); pulseIR(560); delayMicroseconds(520); pulseIR(480); delayMicroseconds(600); pulseIR(600); delayMicroseconds(500); pulseIR(600); delayMicroseconds(500); pulseIR(520); delayMicroseconds(580); pulseIR(500); delayMicroseconds(580); pulseIR(540); delayMicroseconds(540); pulseIR(540); delayMicroseconds(580); pulseIR(520); delayMicroseconds(560); pulseIR(500); delayMicroseconds(600); pulseIR(600); delayMicroseconds(480); pulseIR(600); delayMicroseconds(520); pulseIR(500); delayMicroseconds(580); pulseIR(480); delayMicroseconds(620); pulseIR(580); delayMicroseconds(500); pulseIR(600); delayMicroseconds(520); pulseIR(560); delayMicroseconds(520); pulseIR(480); delayMicroseconds(600); pulseIR(600); delayMicroseconds(500); pulseIR(520); delayMicroseconds(580); pulseIR(580); delayMicroseconds(520); pulseIR(480); delayMicroseconds(600); pulseIR(540); delayMicroseconds(540); pulseIR(600); delayMicroseconds(500); pulseIR(520); delayMicroseconds(580); pulseIR(480); delayMicroseconds(620); pulseIR(560); delayMicroseconds(520); pulseIR(600); delayMicroseconds(520); pulseIR(560); delayMicroseconds(1580); pulseIR(600); delayMicroseconds(520); pulseIR(480); delayMicroseconds(600); pulseIR(520); delayMicroseconds(580); pulseIR(600); delayMicroseconds(480); pulseIR(540); delayMicroseconds(580); pulseIR(500); delayMicroseconds(1680); pulseIR(500); delayMicroseconds(580); pulseIR(480); delayMicroseconds(600); pulseIR(540); delayMicroseconds(560); pulseIR(540); delayMicroseconds(1640); pulseIR(500); delayMicroseconds(580); pulseIR(540);

}

void Send17() {
  pulseIR(3000); delayMicroseconds(3000); pulseIR(3060); delayMicroseconds(4300); pulseIR(540); delayMicroseconds(1640); pulseIR(540); delayMicroseconds(580); pulseIR(480); delayMicroseconds(1680); pulseIR(520); delayMicroseconds(580); pulseIR(500); delayMicroseconds(580); pulseIR(620); delayMicroseconds(1560); pulseIR(540); delayMicroseconds(1640); pulseIR(560); delayMicroseconds(540); pulseIR(520); delayMicroseconds(560); pulseIR(600); delayMicroseconds(520); pulseIR(500); delayMicroseconds(580); pulseIR(580); delayMicroseconds(1600); pulseIR(480); delayMicroseconds(1700); pulseIR(480); delayMicroseconds(600); pulseIR(600); delayMicroseconds(1580); pulseIR(520); delayMicroseconds(580); pulseIR(600); delayMicroseconds(480); pulseIR(600); delayMicroseconds(520); pulseIR(580); delayMicroseconds(1540); pulseIR(560); delayMicroseconds(580); pulseIR(480); delayMicroseconds(600); pulseIR(600); delayMicroseconds(500); pulseIR(600); delayMicroseconds(480); pulseIR(600); delayMicroseconds(520); pulseIR(480); delayMicroseconds(600); pulseIR(580); delayMicroseconds(520); pulseIR(600); delayMicroseconds(520); pulseIR(560); delayMicroseconds(520); pulseIR(480); delayMicroseconds(600); pulseIR(580); delayMicroseconds(500); pulseIR(600); delayMicroseconds(500); pulseIR(540); delayMicroseconds(560); pulseIR(520); delayMicroseconds(580); pulseIR(560); delayMicroseconds(1600); pulseIR(500); delayMicroseconds(600); pulseIR(600); delayMicroseconds(500); pulseIR(520); delayMicroseconds(1660); pulseIR(520); delayMicroseconds(580); pulseIR(500); delayMicroseconds(1660); pulseIR(540); delayMicroseconds(560); pulseIR(520); delayMicroseconds(580); pulseIR(480); delayMicroseconds(1700); pulseIR(480); delayMicroseconds(1700); pulseIR(480); delayMicroseconds(600); pulseIR(560); delayMicroseconds(540); pulseIR(580); delayMicroseconds(500); pulseIR(600); delayMicroseconds(500); pulseIR(580); delayMicroseconds(520); pulseIR(480); delayMicroseconds(1700); pulseIR(480); delayMicroseconds(600); pulseIR(600); delayMicroseconds(500); pulseIR(580); delayMicroseconds(520); pulseIR(500); delayMicroseconds(580); pulseIR(560); delayMicroseconds(540); pulseIR(600); delayMicroseconds(480); pulseIR(600); delayMicroseconds(520); pulseIR(580); delayMicroseconds(1580); pulseIR(520); delayMicroseconds(580); pulseIR(480); delayMicroseconds(600); pulseIR(520); delayMicroseconds(580); pulseIR(540); delayMicroseconds(560); pulseIR(580); delayMicroseconds(520); pulseIR(480); delayMicroseconds(600); pulseIR(480); delayMicroseconds(620); pulseIR(580); delayMicroseconds(500); pulseIR(600); delayMicroseconds(520); pulseIR(500); delayMicroseconds(580); pulseIR(560); delayMicroseconds(520); pulseIR(600); delayMicroseconds(500); pulseIR(600); delayMicroseconds(500); pulseIR(500); delayMicroseconds(600); pulseIR(500); delayMicroseconds(580); pulseIR(600); delayMicroseconds(500); pulseIR(580); delayMicroseconds(520); pulseIR(580); delayMicroseconds(500); pulseIR(560); delayMicroseconds(540); pulseIR(600); delayMicroseconds(480); pulseIR(540); delayMicroseconds(580); pulseIR(500); delayMicroseconds(580); pulseIR(520); delayMicroseconds(560); pulseIR(600); delayMicroseconds(500); pulseIR(540); delayMicroseconds(560); pulseIR(580); delayMicroseconds(520); pulseIR(480); delayMicroseconds(600); pulseIR(540); delayMicroseconds(560); pulseIR(580); delayMicroseconds(520); pulseIR(520); delayMicroseconds(580); pulseIR(560); delayMicroseconds(520); pulseIR(580); delayMicroseconds(500); pulseIR(540); delayMicroseconds(580); pulseIR(500); delayMicroseconds(580); pulseIR(480); delayMicroseconds(620); pulseIR(600); delayMicroseconds(480); pulseIR(600); delayMicroseconds(520); pulseIR(560); delayMicroseconds(520); pulseIR(580); delayMicroseconds(500); pulseIR(580); delayMicroseconds(520); pulseIR(600); delayMicroseconds(520); pulseIR(560); delayMicroseconds(500); pulseIR(500); delayMicroseconds(600); pulseIR(600); delayMicroseconds(500); pulseIR(580); delayMicroseconds(520); pulseIR(580); delayMicroseconds(480); pulseIR(540); delayMicroseconds(580); pulseIR(500); delayMicroseconds(1680); pulseIR(480); delayMicroseconds(600); pulseIR(600); delayMicroseconds(500); pulseIR(580); delayMicroseconds(520); pulseIR(520); delayMicroseconds(1660); pulseIR(520); delayMicroseconds(560); pulseIR(500); delayMicroseconds(1660); pulseIR(540); delayMicroseconds(580); pulseIR(480);
}

void Send22() {
  pulseIR(2940); delayMicroseconds(3080); pulseIR(2980); delayMicroseconds(4380); pulseIR(540); delayMicroseconds(1640); pulseIR(540); delayMicroseconds(560); pulseIR(500); delayMicroseconds(1680); pulseIR(520); delayMicroseconds(560); pulseIR(520); delayMicroseconds(620); pulseIR(500); delayMicroseconds(1640); pulseIR(540); delayMicroseconds(1640); pulseIR(480); delayMicroseconds(600); pulseIR(540); delayMicroseconds(580); pulseIR(520); delayMicroseconds(1640); pulseIR(540); delayMicroseconds(1640); pulseIR(520); delayMicroseconds(580); pulseIR(520); delayMicroseconds(1640); pulseIR(540); delayMicroseconds(580); pulseIR(480); delayMicroseconds(1680); pulseIR(520); delayMicroseconds(620); pulseIR(440); delayMicroseconds(600); pulseIR(520); delayMicroseconds(600); pulseIR(520); delayMicroseconds(1620); pulseIR(560); delayMicroseconds(1620); pulseIR(560); delayMicroseconds(580); pulseIR(500); delayMicroseconds(1640); pulseIR(540); delayMicroseconds(1620); pulseIR(520); delayMicroseconds(1680); pulseIR(520); delayMicroseconds(600); pulseIR(460); delayMicroseconds(580); pulseIR(560); delayMicroseconds(580); pulseIR(500); delayMicroseconds(600); pulseIR(480); delayMicroseconds(620); pulseIR(440); delayMicroseconds(600); pulseIR(540); delayMicroseconds(580); pulseIR(520); delayMicroseconds(580); pulseIR(500); delayMicroseconds(600); pulseIR(460); delayMicroseconds(1680); pulseIR(500); delayMicroseconds(1680); pulseIR(520); delayMicroseconds(620); pulseIR(440); delayMicroseconds(1700); pulseIR(480); delayMicroseconds(640); pulseIR(500); delayMicroseconds(580); pulseIR(500); delayMicroseconds(580); pulseIR(520); delayMicroseconds(600); pulseIR(460); delayMicroseconds(1680); pulseIR(520); delayMicroseconds(1660); pulseIR(520); delayMicroseconds(620); pulseIR(440); delayMicroseconds(600); pulseIR(540); delayMicroseconds(580); pulseIR(520); delayMicroseconds(580); pulseIR(500); delayMicroseconds(620); pulseIR(440); delayMicroseconds(640); pulseIR(480); delayMicroseconds(560); pulseIR(540); delayMicroseconds(580); pulseIR(520); delayMicroseconds(600); pulseIR(460); delayMicroseconds(600); pulseIR(540); delayMicroseconds(540); pulseIR(540); delayMicroseconds(560); pulseIR(520); delayMicroseconds(620); pulseIR(440); delayMicroseconds(1700); pulseIR(520); delayMicroseconds(600); pulseIR(460); delayMicroseconds(640); pulseIR(500); delayMicroseconds(580); pulseIR(500); delayMicroseconds(600); pulseIR(480); delayMicroseconds(580); pulseIR(520); delayMicroseconds(560); pulseIR(520); delayMicroseconds(580); pulseIR(540); delayMicroseconds(600); pulseIR(480); delayMicroseconds(620); pulseIR(440); delayMicroseconds(640); pulseIR(500); delayMicroseconds(580); pulseIR(520); delayMicroseconds(600); pulseIR(480); delayMicroseconds(600); pulseIR(480); delayMicroseconds(580); pulseIR(480); delayMicroseconds(600); pulseIR(540); delayMicroseconds(580); pulseIR(520); delayMicroseconds(600); pulseIR(460); delayMicroseconds(600); pulseIR(540); delayMicroseconds(580); pulseIR(500); delayMicroseconds(600); pulseIR(480); delayMicroseconds(620); pulseIR(440); delayMicroseconds(640); pulseIR(460); delayMicroseconds(620); pulseIR(520); delayMicroseconds(580); pulseIR(500); delayMicroseconds(600); pulseIR(460); delayMicroseconds(600); pulseIR(540); delayMicroseconds(540); pulseIR(560); delayMicroseconds(560); pulseIR(520); delayMicroseconds(600); pulseIR(460); delayMicroseconds(600); pulseIR(480); delayMicroseconds(600); pulseIR(540); delayMicroseconds(580); pulseIR(500); delayMicroseconds(620); pulseIR(440); delayMicroseconds(640); pulseIR(480); delayMicroseconds(580); pulseIR(540); delayMicroseconds(580); pulseIR(500); delayMicroseconds(600); pulseIR(460); delayMicroseconds(600); pulseIR(500); delayMicroseconds(580); pulseIR(560); delayMicroseconds(580); pulseIR(500); delayMicroseconds(600); pulseIR(460); delayMicroseconds(600); pulseIR(500); delayMicroseconds(580); pulseIR(540); delayMicroseconds(580); pulseIR(520); delayMicroseconds(600); pulseIR(460); delayMicroseconds(580); pulseIR(500); delayMicroseconds(600); pulseIR(540); delayMicroseconds(1640); pulseIR(540); delayMicroseconds(540); pulseIR(540); delayMicroseconds(620); pulseIR(480); delayMicroseconds(600); pulseIR(460); delayMicroseconds(1680); pulseIR(500); delayMicroseconds(1680); pulseIR(500); delayMicroseconds(1660); pulseIR(540); delayMicroseconds(1640); pulseIR(480);
}
