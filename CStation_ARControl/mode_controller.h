#ifndef MODE_CONTROLLER_H
#define MODE_CONTROLLER_H

#include "data_exchange.h"
#include "device_controller.h"

#define MODES_COUNT 6
enum Modes {MODE_TRACKING, MODE_INDICATION, MODE_SILENCE, MODE_CONTROL, MODE_SECURITY, MODE_AUTOANIMATOR};

enum TrackingModeState {TRACKING_ON, TRACKING_SENSORONLY, TRACKING_OFF};
enum IndicationModeState {INDICATION_ON, INDICATION_LOW, INDICATION_SCREENOFF, INDICATION_LEDOFF, INDICATION_OFF};
enum SilenceModeState {SILENCE_NO, SILENCE_MEDIUM, SILENCE_MAX};
enum ControlModeState {CONTROL_ON, CONTROL_OFF};
enum SecurityModeState {SECURITY_LOCKED, SECURITY_UNLOCKED};
enum AutoAnimatorModeState {AA_OFF, AA_ON};

struct ModeStruct {
    uint8_t state;
    uint8_t button_pin;
    uint8_t max_value;
    uint32_t ir_code;
};

/* IR Button codes */
#define CM_CTRL_MODE_TRACKING 0x001000
#define CM_CTRL_MODE_INDICATION 0x002000
#define CM_CTRL_MODE_SILENCE 0x003000
#define CM_CTRL_MODE_CONTROL 0x004000
#define CM_CTRL_MODE_SECURITY 0x005000
#define CM_CTRL_MODE_AUTOANIMATOR 0x006000
/* /IR Button codes */

class ModeController
{
  private:
    volatile IndicationController *indication_controller;
    volatile DeviceController *device_controller;
    volatile OneWireHelper *wire_helper;
    volatile ModeStruct modes[CTRL_COUNT];

    void initMode(Modes mode, uint8_t max_value, uint8_t btn_pin, uint8_t first_state = 0, uint32_t ir_code = 0) {
      modes[mode].button_pin = btn_pin;
      modes[mode].state = first_state;
      modes[mode].max_value = max_value;
      modes[mode].ir_code = ir_code;
      if (btn_pin > 0) {
        pinMode(btn_pin, INPUT);
      }
    }

  public:

    ModeController(DeviceController *_device_controller, OneWireHelper *_wire_helper)
    {
      memset(modes, 0, sizeof(modes));
      indication_controller = _device_controller->getIndicationController();
      device_controller = _device_controller;
      wire_helper = _wire_helper;
      initMode(MODE_TRACKING, TRACKING_OFF, 0, TRACKING_ON, CM_CTRL_MODE_TRACKING);
      initMode(MODE_INDICATION, INDICATION_OFF, 0, INDICATION_ON, CM_CTRL_MODE_INDICATION);
      initMode(MODE_SILENCE, SILENCE_MAX, 0, SILENCE_NO, CM_CTRL_MODE_SILENCE);
      initMode(MODE_CONTROL, CONTROL_OFF, 0, CONTROL_ON, CM_CTRL_MODE_CONTROL);
      initMode(MODE_SECURITY, SECURITY_UNLOCKED, 0, SECURITY_LOCKED, CM_CTRL_MODE_SECURITY);
      initMode(MODE_AUTOANIMATOR, AA_ON, 0, AA_OFF, CM_CTRL_MODE_AUTOANIMATOR);
    }

    uint8_t getModeState(Modes mode) {
      return modes[mode].state;
    }

    void setModeState(byte mode_code, byte mode_state, bool send_pc_notify = false) {
      if (send_pc_notify) {
        sendToMainPC(CMD_CMD_SETMODESTATE, mode_code, mode_state, 0);
      }
      switch (mode_code) {
        case MODE_TRACKING:
          device_controller->setDeviceState(CTRL_CAMERA, modes[mode_code].state == TRACKING_ON, send_pc_notify);
        break;
        case MODE_INDICATION:
          indication_controller->setIndicationShow(modes[mode_code].state != INDICATION_LEDOFF && modes[mode_code].state != INDICATION_OFF);
          // INDICATION_LOW & INDICATION_SCREENOFF (INDICATION_OFF) - pc control
        break;
        case MODE_SILENCE:
          if (modes[mode_code].state == SILENCE_NO) wire_helper->writeByteCommand(ONEWIRE_CODE_FAN_ON);
          else if (modes[mode_code].state == SILENCE_MEDIUM) wire_helper->writeByteCommand(ONEWIRE_CODE_FAN_MID);
          else if (modes[mode_code].state == SILENCE_MAX) wire_helper->writeByteCommand(ONEWIRE_CODE_FAN_OFF);
          // @todo set control state
        break;
        case MODE_CONTROL:
          // @todo set control state
        break;
        case MODE_SECURITY:
          device_controller->setDeviceState(CTRL_LOCKOPEN, modes[mode_code].state == SECURITY_UNLOCKED, send_pc_notify);
        break;
        case MODE_AUTOANIMATOR:
          if (modes[MODE_AUTOANIMATOR].state == AA_ON) {
            if (modes[MODE_TRACKING].state != TRACKING_ON)         setModeState(MODE_TRACKING, TRACKING_ON, send_pc_notify);
            if (modes[MODE_INDICATION].state != INDICATION_LEDOFF) setModeState(MODE_INDICATION, INDICATION_LEDOFF, send_pc_notify);
            if (modes[MODE_SILENCE].state != SILENCE_NO)           setModeState(MODE_SILENCE, SILENCE_NO, send_pc_notify);
          }
        break;
        default:
          return;
      }
      modes[mode_code].state = mode_state;
    }

    void ModeNext(Modes mode)
    {
      uint8_t next_value = modes[mode].state;
      if (next_value == modes[mode].max_value) {
        next_value = 0;
      } else {
        next_value++;
      }
      setModeState(mode, next_value, true);
    }

    int ModeControlIRCode(unsigned long int code)
    {
      for(byte i=0; i<MODES_COUNT; i++) {
        if (modes[i].ir_code == code) {
          ModeNext(i);
          return i;
        }
      }
      return -1;
    }
};

#endif
