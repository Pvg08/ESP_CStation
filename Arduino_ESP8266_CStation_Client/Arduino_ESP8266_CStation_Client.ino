
#include <TimerOne.h>

#define BAUD_RATE 38400

#define CSTATION_DEBUG

#ifdef CSTATION_DEBUG
  #define DEBUG_WRITE(...) { Serial.print(__VA_ARGS__); }
  #define DEBUG_WRITELN(...) { Serial.println(__VA_ARGS__); }
#else
  #define DEBUG_WRITE(...) {}
  #define DEBUG_WRITELN(...) {}
#endif

#define TONE_PIN 7
#define RESET_BTN_PIN 2
#define RESET_BTN_INTERRUPT 0
#define RESET_BTN_INTERRUPT_MODE RISING
#define CONFIG_BTN_PIN 3
#define CONFIG_BTN_INTERRUPT 1
#define CONFIG_BTN_INTERRUPT_MODE RISING

byte errors_count = 0;

volatile unsigned tone_frequency;
volatile bool tone_state;
volatile bool reset_btn_pressed = false;
volatile bool config_btn_pressed = false;

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
  attachInterrupt(RESET_BTN_INTERRUPT, ResetBTN_Pressed, RESET_BTN_INTERRUPT_MODE);
  attachInterrupt(CONFIG_BTN_INTERRUPT, ConfigurationBTN_Pressed, CONFIG_BTN_INTERRUPT_MODE);
  reset_btn_pressed = false;
  config_btn_pressed = false;
}

void loop()
{
  if (config_btn_pressed) {
    DEBUG_WRITELN("Config BTN pressed. Entering configuration mode\r\n");
    StartConfiguringMode();
    config_btn_pressed = false;
    reset_btn_pressed = true;
    return;
  }
  if (reset_btn_pressed) {
    DEBUG_WRITELN("Reset BTN pressed. Resetting\r\n");
    StartConnection(true);
    reset_btn_pressed = false;
    config_btn_pressed = false;
    return;
  }
  
  executeCommands();
  sensorsSending();
}

void ResetBTN_Pressed() 
{
  reset_btn_pressed = true;
}

void ConfigurationBTN_Pressed() 
{
  config_btn_pressed = true;
}

void tone_period() 
{
  if (tone_state) {
    tone(TONE_PIN, tone_frequency);
  } else {
    noTone(TONE_PIN);
  }
  tone_state = !tone_state;
}

void executeCommands() 
{
  char *message;
  message = readTCPMessage( 1000, NULL, true );
  
  if (message && !config_btn_pressed && !reset_btn_pressed) {
    char* param;
    if ((param = getMessageParam(message, "SERV_RST=1", true))) 
    {
      StartConnection(true);
      reset_btn_pressed = false;
      config_btn_pressed = false;
      delay(1000);
    } else if ((param = getMessageParam(message, "SERV_CONF=1", true))) {
      StartConfiguringMode();
      reset_btn_pressed = false;
      config_btn_pressed = false;
      delay(1000);
    } else if ((param = getMessageParam(message, "TONE=", true))) {
      tone_frequency = readIntFromString(param, 0);
      Timer1.detachInterrupt();
      Timer1.stop();
      if (tone_frequency) {
        unsigned long int period = readIntFromString(param, String(tone_frequency).length()+1);
        if (period>0) {
          DEBUG_WRITE("Starting tone. F="); DEBUG_WRITE(tone_frequency); DEBUG_WRITE(" P="); DEBUG_WRITELN(period);
          tone_state = false;
          Timer1.initialize(period*1000);
          Timer1.attachInterrupt(tone_period);
        } else {
          DEBUG_WRITE("Starting tone. F="); DEBUG_WRITELN(tone_frequency);
          tone(TONE_PIN, tone_frequency);
        }
      } else {
        DEBUG_WRITELN("Stopping tone");
        noTone(TONE_PIN);
      }
    } else if ((param = getMessageParam(message, "SERV_LT=", true))) {
      setLCDFixed(param);
    } else if ((param = getMessageParam(message, "SERV_LR=1", true))) {
      resetLCDFixed();
    }

    delay (100);
  }
}


