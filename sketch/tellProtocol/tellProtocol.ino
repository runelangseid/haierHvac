
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


void setup()
{
  Serial.begin(9600);
  Serial.println("START");

  // Setup interrupt routine
  attachInterrupt(0, handleInterrupt, CHANGE);

}

void loop()
{
  if (nReceivedProtocol == 1)
  {
      Serial.print("nReceivedValue:");
      Serial.println(nReceivedValue);

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




