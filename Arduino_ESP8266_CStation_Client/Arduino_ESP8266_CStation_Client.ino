
// 3 chars station code
#define IDENTIFICATION_CODE "DS1"

#define BAUD_RATE 38400

#define CSTATION_DEBUG

#ifdef CSTATION_DEBUG
  #define DEBUG_WRITE(x) { Serial.println(x); }
#else
  #define DEBUG_WRITE(x) {}
#endif

enum lcdmode {
  lm_info,
  lm_message
};

unsigned long int millis_sum_delay = 0;
byte errors_count = 0;
lcdmode lmode = lm_info;

void setup()
{
  Serial.begin(BAUD_RATE);
  initESP();
  initLCD();
  delay(100);
  initSensors();
  delay(100);
  DEBUG_WRITE("Start\r\n\r\n");
  delay(100);
  StartConnection(true);
}

void loop()
{
  executeCommands();
  millis_sum_delay += 100;
  delay (100);
  sensorsSending();
}

void executeCommands() 
{
  char *message = readReply( 1000 );
  if (message) {
    bool foundConnect = false;
    int i;
    int reply_len = strlen(message);
    
    for (i=0; i<reply_len-5; i++)
    {
      if ( (message[i]=='+') && (message[i+1]=='I') && (message[i+2]=='P') && (message[i+3]=='D') && (message[i+4]==',') ) { 
        foundConnect = true;
        break;
      }
    }

    if (foundConnect)
    {
      for (; i<reply_len && message[i]!=':'; i++);
      i++;

      if (i<reply_len) 
      {
        message = message+i;
        DEBUG_WRITE(message);

        if (strncmp(message, "RST\r\n\r\n", 7)==0) 
        {
          StartConnection(true);
          delay(1000);
          return;
        }
      }
    }
  }
}


