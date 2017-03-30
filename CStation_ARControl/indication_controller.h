#ifndef INDICATION_CONTROLLER_H
#define INDICATION_CONTROLLER_H

#include <Timer5.h>

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
    LED_AUTO = 9, 
    LED_STATE1 = 10, 
    LED_STATE2 = 11, 
    LED_NO_LED = 12
};
enum BlinkState {
    BLINKING_NO, 
    BLINKING_ONCE, 
    BLINKING_SLOW, 
    BLINKING_NORMAL, 
    BLINKING_FAST
};

struct IndicatorStruct {
    OutputPin *pin;
    bool state;
    BlinkState blinks;
    byte tmp_blink_counter, max_blink_counter;
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
#define LED_AUTO_PIN 45
#define LED_STATE1_PIN 47
#define LED_STATE2_PIN 49
#define LED_NO_LED_PIN 50
/* /led pins */

#define FAST_BLINK_INTERVAL_MS 150
#define NORMAL_BLINK_COUNTMAX 4
#define SLOW_BLINK_COUNTMAX 10
#define ONCE_BLINK_COUNTMAX 5

class IndicationController 
{
  private:
    volatile byte blinking_count;
    volatile bool indication_show;
    volatile IndicatorStruct indicators[LED_COUNT];

    void initLed(LedIndicator led, byte pin) {
      indicators[led].pin = new OutputPin(pin, false);
      indicators[led].state = false;
      indicators[led].blinks = BLINKING_NO;
      indicators[led].tmp_blink_counter = 0;
      indicators[led].max_blink_counter = 0;
      indicators[led].pin->write(LOW);
    }

    void updateIndicatorsState()
    {
      for(byte i=0; i < LED_COUNT; i++) {
        if (i != LED_NO_LED) {
          indicators[i].pin->write((indication_show && indicators[i].state) ? HIGH : LOW);
        }
      }
    }

    void blinkingUp() {
      blinking_count++;
      if (blinking_count == 1) {
        startTimer5(FAST_BLINK_INTERVAL_MS*1000);
        resetTimer5();
      }
    }

    void blinkingDown() {
      blinking_count--;
      if (blinking_count == 0) {
        pauseTimer5();
      }
    }

  public:

    IndicationController() 
    {
      blinking_count = 0;
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
      initLed(LED_AUTO, LED_AUTO_PIN);
      initLed(LED_STATE1, LED_STATE1_PIN);
      initLed(LED_STATE2, LED_STATE2_PIN);
      initLed(LED_NO_LED, LED_NO_LED_PIN);
    }

    void LedSet(LedIndicator led, bool state, BlinkState blinking_state = BLINKING_NO)
    {
      indicators[led].state = state;
      if (blinking_state == BLINKING_NO) {
        if (indicators[led].blinks != BLINKING_NO) blinkingDown();
      } else {
        indicators[led].tmp_blink_counter = 0;
        switch(blinking_state) {
          case BLINKING_ONCE:
            indicators[led].max_blink_counter = ONCE_BLINK_COUNTMAX;
          break;
          case BLINKING_SLOW:
            indicators[led].max_blink_counter = SLOW_BLINK_COUNTMAX;
          break;
          case BLINKING_NORMAL:
            indicators[led].max_blink_counter = NORMAL_BLINK_COUNTMAX;
          break;
          case BLINKING_FAST:
            indicators[led].max_blink_counter = 1;
          break;
        }
        if (indicators[led].blinks == BLINKING_NO) blinkingUp();
      }
      indicators[led].blinks = blinking_state;
      if (indication_show || led == LED_NO_LED) {
        indicators[led].pin->write(state ? HIGH : LOW);
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
      for(byte i=0; i < LED_COUNT; i++) {
        if (indicators[i].blinks != BLINKING_NO) {
          indicators[i].tmp_blink_counter++;
          if (indicators[i].tmp_blink_counter >= indicators[i].max_blink_counter) {
            indicators[i].pin->toggle();
            indicators[i].tmp_blink_counter = 0;
            if (indicators[i].blinks == BLINKING_ONCE) {
              indicators[i].blinks = BLINKING_NO;
              blinkingDown();
            }
          }
        }
      }
    }
};

#endif
