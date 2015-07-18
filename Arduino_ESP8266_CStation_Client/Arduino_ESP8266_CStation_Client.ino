
#define BAUD_RATE 38400

#define CSTATION_DEBUG

#ifdef CSTATION_DEBUG
  #define DEBUG_WRITE(...) { Serial.print(__VA_ARGS__); }
  #define DEBUG_WRITELN(...) { Serial.println(__VA_ARGS__); }
#else
  #define DEBUG_WRITE(...) {}
  #define DEBUG_WRITELN(...) {}
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
  DEBUG_WRITELN("Start\r\n");
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
  char *message;
  message = readTCPMessage( 1000, NULL, true );
  
  if (message) {
    char* param;
    if ((param = getMessageParam(message, "SERV_RST=1", true))) 
    {
      closeConnection(5);
      StartConnection(true);
      delay(1000);
    } else if ((param = getMessageParam(message, "SERV_CONF=1", true))) {
      closeConnection(5);
      StartConfiguringMode();
      delay(1000);
    } else if ((param = getMessageParam(message, "TONE=", true))) {
      unsigned frequency = readIntFromString(param, 0);
      if (frequency) {
        DEBUG_WRITE("Starting tone. F="); DEBUG_WRITELN(frequency);
        tone(7, frequency);
      } else {
        DEBUG_WRITELN("Stopping tone");
        noTone(7);
      }
    }
  }
}


