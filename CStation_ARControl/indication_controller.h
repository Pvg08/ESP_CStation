#ifndef INDICATION_CONTROLLER_H
#define INDICATION_CONTROLLER_H

#define LED_COUNT 13
enum LedIndicator {
    LED_MAIN_PC_READY, 
    LED_PDU, 
    LED_CARDREADING, 
    LED_LOCKOPEN, 
    LED_CONTROLSBLOCKED, 
    LED_CAMERA, 
    LED_UVLAMP, 
    LED_PRESENCE, 
    LED_BLAMP, 
    LED_USBDEVICE, 
    LED_STATE1, 
    LED_STATE2
};
enum BlinkState {
    BLINKING_NO, 
    BLINKING_ONCE, 
    BLINKING_SLOW, 
    BLINKING_FAST
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
    byte led_pins[LED_COUNT];
    bool led_states[LED_COUNT];
    BlinkState led_blinks[LED_COUNT];

    void initLed(LedIndicator led, byte pin) {
      led_pins[led] = LED_MAIN_PC_READY_PIN;
      led_states[led] = false;
      led_blinks[led] = BLINKING_NO;
      pinMode(pin, OUTPUT);
      digitalWrite(pin, LOW);
    }

  public:

    IndicationController() 
    {
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
      pinMode(LED_NO_LED_PIN, OUTPUT);
      digitalWrite(LED_NO_LED_PIN, LOW);
    }

    void LedSet(LedIndicator led, bool state, BlinkState blinking_state = BLINKING_NO)
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
        digitalWrite(LED_NO_LED_PIN, indication_show ? LOW : HIGH);
      }
    }

    void onTimer()
    {
      // @todo blinking
    }
};

#endif
