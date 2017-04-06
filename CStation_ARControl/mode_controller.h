#ifndef MODE_CONTROLLER_H
#define MODE_CONTROLLER_H

#include "data_exchange.h"
#include "device_controller.h"

#define BUTTONS_COMMON_INTERRUPT_PIN 19

#define MODES_COUNT 6
enum Modes {MODE_TRACKING, MODE_INDICATION, MODE_SILENCE, MODE_CONTROL, MODE_SECURITY, MODE_AUTOANIMATOR, MODE_NONE};

enum TrackingModeState {TRACKING_ON, TRACKING_SENSORONLY, TRACKING_OFF};
enum IndicationModeState {INDICATION_ON, INDICATION_LOW, INDICATION_SCREENOFF, INDICATION_LEDOFF, INDICATION_OFF};
enum SilenceModeState {SILENCE_NO, SILENCE_MEDIUM, SILENCE_MAX};
enum ControlModeState {CONTROL_ON, CONTROL_OFF};
enum SecurityModeState {SECURITY_LOCKED, SECURITY_UNLOCKED};
enum AutoAnimatorModeState {AA_OFF, AA_ON};

struct ModeStruct {
    uint8_t state;
    uint8_t button_pin;
    InputPin *pin;
    uint8_t max_value;
    uint32_t ir_code;
};

/* Mode button pins */
#define MODE_TRACKING_BTN_PIN     12
#define MODE_INDICATION_BTN_PIN   13
#define MODE_SILENCE_BTN_PIN      14
#define MODE_CONTROL_BTN_PIN      15
#define MODE_SECURITY_BTN_PIN     16
#define MODE_AUTOANIMATOR_BTN_PIN 17
/* /Mode button pins */

/* IR Button codes */
#define CM_CTRL_MODE_TRACKING     0x001000
#define CM_CTRL_MODE_INDICATION   0x002000
#define CM_CTRL_MODE_SILENCE      0x003000
#define CM_CTRL_MODE_CONTROL      0x004000
#define CM_CTRL_MODE_SECURITY     0x005000
#define CM_CTRL_MODE_AUTOANIMATOR 0x006000
/* /IR Button codes */

void onModeButtonInterrupt();

class ModeController
{
  private:
    IndicationController *indication_controller;
    DeviceController *device_controller;
    OneWireHelper *wire_helper;
    volatile ModeStruct modes[CTRL_COUNT];
    volatile Modes mode_btn_action;

    void initMode(Modes mode, uint8_t max_value, uint8_t btn_pin, uint8_t first_state = 0, uint32_t ir_code = 0) {
      modes[mode].button_pin = btn_pin;
      if (btn_pin > 0) {
        modes[mode].pin = new InputPin(btn_pin, true);
      } else {
        modes[mode].pin = NULL;
      }
      modes[mode].state = first_state;
      modes[mode].max_value = max_value;
      modes[mode].ir_code = ir_code;
      if (btn_pin > 0) {
        pinMode(btn_pin, INPUT);
      }
    }

    ModeController(DeviceController *_device_controller, OneWireHelper *_wire_helper)
    {
      memset(modes, 0, sizeof(modes));
      mode_btn_action = MODE_NONE;
      indication_controller = _device_controller->getIndicationController();
      device_controller = _device_controller;
      wire_helper = _wire_helper;
      initMode(MODE_TRACKING, TRACKING_OFF, MODE_TRACKING_BTN_PIN, TRACKING_ON, CM_CTRL_MODE_TRACKING);
      initMode(MODE_INDICATION, INDICATION_OFF, MODE_INDICATION_BTN_PIN, INDICATION_ON, CM_CTRL_MODE_INDICATION);
      initMode(MODE_SILENCE, SILENCE_MAX, MODE_SILENCE_BTN_PIN, SILENCE_NO, CM_CTRL_MODE_SILENCE);
      initMode(MODE_CONTROL, CONTROL_OFF, MODE_CONTROL_BTN_PIN, CONTROL_ON, CM_CTRL_MODE_CONTROL);
      initMode(MODE_SECURITY, SECURITY_UNLOCKED, MODE_SECURITY_BTN_PIN, SECURITY_LOCKED, CM_CTRL_MODE_SECURITY);
      initMode(MODE_AUTOANIMATOR, AA_ON, MODE_AUTOANIMATOR_BTN_PIN, AA_OFF, CM_CTRL_MODE_AUTOANIMATOR);
      pinMode(BUTTONS_COMMON_INTERRUPT_PIN, INPUT_PULLUP);
      attachInterrupt(digitalPinToInterrupt(BUTTONS_COMMON_INTERRUPT_PIN), onModeButtonInterrupt, RISING);
    }

  public:

    volatile static ModeController *_self_controller;

    static ModeController* Instance(DeviceController *_device_controller, OneWireHelper *_wire_helper) {
      if(!_self_controller)
      {
          _self_controller = new ModeController(_device_controller, _wire_helper);
      }
      return _self_controller;
    }
    static ModeController* getInstance() {
      return _self_controller;
    }
    static bool DeleteInstance() {
      if(_self_controller)
      {
          delete _self_controller;
          _self_controller = NULL;
          return true;
      }
      return false;
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

    void onButtonClick()
    {
      if (mode_btn_action != MODE_NONE) {
        for(byte i=0; i<MODES_COUNT; i++) {
          if (modes[i].button_pin > 0) {
            if (modes[i].pin->read()) {
              mode_btn_action = i;
              return;
            }
          }
        }
      }
    }

    void checkClicks() {
      if (mode_btn_action != MODE_NONE) {
        ModeNext(mode_btn_action);
        mode_btn_action = MODE_NONE;
      }
    }
};

volatile ModeController *ModeController::_self_controller = NULL;

void onModeButtonInterrupt() {
  if (ModeController::getInstance()) {
    ModeController::getInstance()->onButtonClick();
  }
}

#endif
