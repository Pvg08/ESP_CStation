#ifndef INDICATION_CONTROLLER_H
#define INDICATION_CONTROLLER_H

#define LED_COUNT 9
enum LedPins {LED_MAIN_PC_READY, LED_PDU, LED_CARDREADING, LED_LEDOFF, LED_LOCKOPEN, LED_CONTROLSBLOCKED, LED_CAMERA, LED_UVLAMP, LED_PRESENCE};
enum BlinkState {BLINKING_NO, BLINKING_ONCE, BLINKING_SLOW, BLINKING_FAST};

/* led pins */
#define LED_MAIN_PC_READY_PIN 27
#define LED_PDU_PIN 29
#define LED_CARDREADING_PIN 31

#define LED_LEDOFF_PIN 33
#define LED_LOCKOPEN_PIN 35
#define LED_CONTROLSBLOCKED_PIN 37
#define LED_CAMERA_PIN 39
#define LED_UVLAMP_PIN 41
#define LED_PRESENCE_PIN 43
/* /led pins */

class IndicationController 
{
  private:
    bool indication_show;
    byte led_pins[LED_COUNT];
    bool led_states[LED_COUNT];
    BlinkState led_blinks[LED_COUNT];
    
  public:

    IndicationController() 
    {
      indication_show = true;
      led_pins[LED_MAIN_PC_READY] = LED_MAIN_PC_READY_PIN;
      led_pins[LED_PDU] = LED_PDU_PIN;
      led_pins[LED_CARDREADING] = LED_CARDREADING_PIN;
      led_pins[LED_LEDOFF] = LED_LEDOFF_PIN;
      led_pins[LED_LOCKOPEN] = LED_LOCKOPEN_PIN;
      led_pins[LED_CONTROLSBLOCKED] = LED_CONTROLSBLOCKED_PIN;
      led_pins[LED_CAMERA] = LED_CAMERA_PIN;
      led_pins[LED_UVLAMP] = LED_UVLAMP_PIN;
      led_pins[LED_PRESENCE] = LED_PRESENCE_PIN;
      for(byte i=0; i < LED_COUNT; i++) {
        pinMode(led_pins[i], OUTPUT);
        digitalWrite(led_pins[i], LOW);
        led_states[i] = false;
        led_blinks[i] = BLINKING_NO;
      }
    }

    void LedSet(LedPins led, bool state, BlinkState blinking_state = BLINKING_NO)
    {
      led_states[led] = state;
      led_blinks[led] = blinking_state;
      if (indication_show) {
        digitalWrite(led_pins[led], state ? HIGH : LOW);
      }
    }

    void updateIndicatorsState()
    {
      for(byte i=0; i < LED_COUNT; i++) {
        digitalWrite(led_pins[i], (indication_show && led_states[i]) ? HIGH : LOW);
      }
    }

    void setIndicationShow(bool new_indication_show)
    {
      if (new_indication_show != indication_show) {
        indication_show = new_indication_show;
        updateIndicatorsState();
      }
    }

    void onTimer()
    {
      // @todo blinking
    }
};

#endif
