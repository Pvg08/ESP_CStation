#ifndef DEVICE_CONTROLLER_H
#define DEVICE_CONTROLLER_H

#include "indication_controller.h"

#define CTRL_COUNT 5
enum ControlDevice {
    CTRL_LOCKOPEN = 0,
    CTRL_CAMERA = 1,
    CTRL_UVLAMP = 2,
    CTRL_BLAMP = 3,
    CTRL_USBDEVICE = 4
};

struct DeviceStruct {
    uint8_t pin;
    bool state;
    LedIndicator led;
    uint32_t ir_code;
};

/* control pins */
#define CTRL_LOCKOPEN_PIN 36
#define CTRL_CAMERA_PIN 40
#define CTRL_UVLAMP_PIN 42
#define CTRL_BLAMP_PIN 44
#define CTRL_USBDEVICE_PIN 46
/* /control pins */

/* IR Button codes */
#define CM_CTRL_UVLAMP 0x000000
#define CM_CTRL_BLAMP 0x000000
#define CM_CTRL_USBDEVICE 0x000000
/* /IR Button codes */

class DeviceController 
{
  private:
    bool blocking;
    IndicationController *indication_controller;
    DeviceStruct controls[CTRL_COUNT];

    void initControl(ControlDevice device, byte pin, LedIndicator led, uint32_t ir_code = 0, bool first_state = false) {
      controls[device].pin = pin;
      controls[device].led = led;
      controls[device].ir_code = ir_code;
      controls[device].state = first_state;
      pinMode(pin, OUTPUT);
      digitalWrite(pin, first_state ? HIGH : LOW);
      if (first_state) {
        indication_controller->LedSet(led, true);
      }
    }

  public:

    DeviceController(IndicationController *_indication_controller) 
    {
      memset(controls, 0, sizeof(controls));
      blocking = false;
      indication_controller = _indication_controller;
      initControl(CTRL_LOCKOPEN, CTRL_LOCKOPEN_PIN, LED_LOCKOPEN, 0, false);
      initControl(CTRL_CAMERA, CTRL_CAMERA_PIN, LED_CAMERA, 0, true);
      initControl(CTRL_UVLAMP, CTRL_UVLAMP_PIN, LED_UVLAMP, CM_CTRL_UVLAMP, false);
      initControl(CTRL_BLAMP, CTRL_BLAMP_PIN, LED_BLAMP, CM_CTRL_BLAMP, false);
      initControl(CTRL_USBDEVICE, CTRL_USBDEVICE_PIN, LED_USBDEVICE, CM_CTRL_USBDEVICE, false);
    }

    void ControlSet(ControlDevice device, bool state)
    {
      if (!blocking && controls[device].state != state) {
        indication_controller->LedSet(controls[device].led, state);
        controls[device].state = state;
        digitalWrite(controls[device].pin, state ? HIGH : LOW);
      }
    }

    int DeviceControlIRCode(unsigned long int code)
    {
      if (!blocking) {
        for(byte i=0; i<CTRL_COUNT; i++) {
          if (controls[i].ir_code == code) {
            ControlSet(i, !getDeviceState(i));
            return i;
          }
        }
      }
      return -1;
    }

    bool getDeviceState(ControlDevice device)
    {
      return controls[device].state;
    }

    void setBlocking(bool new_blocking)
    {
      blocking = new_blocking;
    }

    bool isBlocking()
    {
      return blocking;
    }
};

#endif
