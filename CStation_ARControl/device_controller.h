#ifndef DEVICE_CONTROLLER_H
#define DEVICE_CONTROLLER_H

#include "indication_controller.h"

#define CTRL_COUNT 5
enum ControlDevice {
    CTRL_LOCKOPEN,
    CTRL_CAMERA,
    CTRL_UVLAMP,
    CTRL_BLAMP,
    CTRL_USBDEVICE
};

/* control pins */
#define CTRL_LOCKOPEN_PIN 36
#define CTRL_CONTROLSBLOCKED_PIN 38
#define CTRL_CAMERA_PIN 40
#define CTRL_UVLAMP_PIN 42
#define CTRL_BLAMP_PIN 44
#define CTRL_USBDEVICE_PIN 46
/* /control pins */

class DeviceController 
{
  private:
    IndicationController *indication_controller;
    byte ctrl_pins[CTRL_COUNT];
    bool ctrl_states[CTRL_COUNT];
    LedIndicator ctrl_indicators[CTRL_COUNT];

    void initControl(ControlDevice device, byte pin, LedIndicator led, bool first_state = false) {
      ctrl_pins[device] = pin;
      ctrl_indicators[device] = led;
      ctrl_states[device] = false;
      pinMode(pin, OUTPUT);
      digitalWrite(pin, first_state ? HIGH : LOW);
      if (first_state) {
        indication_controller->LedSet(led, true);
      }
    }

  public:

    DeviceController(IndicationController *_indication_controller) 
    {
      indication_controller = _indication_controller;
      initControl(CTRL_LOCKOPEN, CTRL_LOCKOPEN_PIN, LED_LOCKOPEN, false);
      initControl(CTRL_CAMERA, CTRL_CAMERA_PIN, LED_CAMERA, true);
      initControl(CTRL_UVLAMP, CTRL_UVLAMP_PIN, LED_UVLAMP, false);
      initControl(CTRL_BLAMP, CTRL_BLAMP_PIN, LED_BLAMP, false);
      initControl(CTRL_USBDEVICE, CTRL_USBDEVICE_PIN, LED_USBDEVICE, false);
    }

    void ControlSet(ControlDevice device, bool state)
    {
      if (ctrl_states[device] != state) {
        indication_controller->LedSet(ctrl_indicators[device], state);
        ctrl_states[device] = state;
        digitalWrite(ctrl_pins[device], state ? HIGH : LOW);
      }
    }
};

#endif
