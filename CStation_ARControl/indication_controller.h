#ifndef INDICATION_CONTROLLER_H
#define INDICATION_CONTROLLER_H

#define LED_COUNT 13
enum LedIndicator {
    LED_MAIN_PC_READY = 0, 
    LED_PDU = 1, 
    LED_CARDREADING = 2, 
    LED_LOCKOPEN = 3, 
    LED_CONTROLSBLOCKED = 4, 
    LED_CAMERA = 5, 
    LED_UVLAMP = 6, 
    LED_PRESENCE = 7, 
    LED_BLAMP = 8, 
    LED_USBDEVICE = 9, 
    LED_STATE1 = 10, 
    LED_STATE2 = 11, 
    LED_NO_LED = 12
};
enum BlinkState {
    BLINKING_NO, 
    BLINKING_ONCE, 
    BLINKING_SLOW, 
    BLINKING_FAST
};

struct IndicatorStruct {
    uint8_t pin;
    bool state;
    BlinkState blinks;
};

/* led pins */
#define LED_MAIN_PC_READY_PIN 27
#define LED_PDU_PIN 29
#define LED_CARDREADING_PIN 31

#define LED_PRESENCE_PIN 33
#define LED_LOCKOPEN_PIN 35
#define LED_CONTROLSBLOCKED_PIN 37
#define LED_CAMERA_PIN 39
#define LED_UVLAMP_PIN 41
#define LED_BLAMP_PIN 43
#define LED_USBDEVICE_PIN 45
#define LED_STATE1_PIN 47
#define LED_STATE2_PIN 48
#define LED_NO_LED_PIN 49
/* /led pins */

class IndicationController 
{
  private:
    bool indication_show;
    IndicatorStruct indicators[LED_COUNT];

    void initLed(LedIndicator led, byte pin) {
      indicators[led].pin = pin;
      indicators[led].state = false;
      indicators[led].blinks = BLINKING_NO;
      pinMode(pin, OUTPUT);
      digitalWrite(pin, LOW);
    }

    void updateIndicatorsState()
    {
      for(byte i=0; i < LED_COUNT; i++) {
        if (i != LED_NO_LED) {
          digitalWrite(indicators[i].pin, (indication_show && indicators[i].state) ? HIGH : LOW);
        }
      }
    }

  public:

    IndicationController() 
    {
      memset(indicators, 0, sizeof(indicators));
      indication_show = true;
      initLed(LED_MAIN_PC_READY, LED_MAIN_PC_READY_PIN);
      initLed(LED_PDU, LED_PDU_PIN);
      initLed(LED_LOCKOPEN, LED_LOCKOPEN_PIN);
      initLed(LED_CONTROLSBLOCKED, LED_CONTROLSBLOCKED_PIN);
      initLed(LED_CAMERA, LED_CAMERA_PIN);
      initLed(LED_UVLAMP, LED_UVLAMP_PIN);
      initLed(LED_PRESENCE, LED_PRESENCE_PIN);
      initLed(LED_BLAMP, LED_BLAMP_PIN);
      initLed(LED_USBDEVICE, LED_USBDEVICE_PIN);
      initLed(LED_STATE1, LED_STATE1_PIN);
      initLed(LED_STATE2, LED_STATE2_PIN);
      initLed(LED_NO_LED, LED_NO_LED_PIN);
    }

    void LedSet(LedIndicator led, bool state, BlinkState blinking_state = BLINKING_NO)
    {
      indicators[led].state = state;
      indicators[led].blinks = blinking_state;
      if (indication_show || led == LED_NO_LED) {
        digitalWrite(indicators[led].pin, state ? HIGH : LOW);
      }
    }

    void setIndicationShow(bool new_indication_show)
    {
      if (new_indication_show != indication_show) {
        indication_show = new_indication_show;
        updateIndicatorsState();
        LedSet(LED_NO_LED, indication_show ? LOW : HIGH);
      }
    }

    bool getIndicationState(LedIndicator led)
    {
      return indicators[led].state;
    }

    void onTimer()
    {
      // @todo blinking
    }
};

#endif
