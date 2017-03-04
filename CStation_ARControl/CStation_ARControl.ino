#include <IRremote.h>
#include <TM1637Display.h>
#include <TimeLib.h>
#include <DS1302RTC.h>
#include "data_exchange.h"
#include "onewire_helper.h"

/* RTC and clock display pins & params */
#define TM_CLK 22
#define TM_DIO 23

#define DS1302_GND_PIN 24
#define DS1302_VCC_PIN 25
#define DS1302_RST_PIN 26
#define DS1302_DAT_PIN 28
#define DS1302_CLK_PIN 30

#define CLOCK_DELAY_MS 500
/* /RTC and clock display pins & params */

/* led pins */
#define LED_MAIN_PC_READY_PIN 27
#define LED_PDU_PIN 29
#define LED_CARDREADING_PIN 31

#define LED_LEDOFF_PIN 33
#define LED_LOCKOPEN_PIN 35
#define LED_CONTROLSBLOCKED_PIN 37
#define LED_CAMERA_PIN 39
#define LED_UVLAMP_PIN 41
/* /led pins */

/* control pins */
#define CTRL_LEDOFF_PIN 34
#define CTRL_LOCKOPEN_PIN 36
#define CTRL_CONTROLSBLOCKED_PIN 38
#define CTRL_CAMERA_PIN 40
#define CTRL_UVLAMP_PIN 42
/* /control pins */

/* other pins */
#define IR_RECV_PIN 13
/* /other pins */

/* IR Button codes */
#define CM_ON 0xFFB04F
#define CM_OFF 0xFFF807
#define CM_REPEAT 0xFFFFFFFF
/* /IR Button codes */

/* OneWire codes */
#define POWER_SIGNAL_MAIN_PIN 24
#define ONEWIRE_CODE_NOOP 0x00
#define ONEWIRE_CODE_OFF 0b11011001
#define ONEWIRE_CODE_SILENT_MODE_OFF 0b11100000
#define ONEWIRE_CODE_SILENT_MODE_MID 0b10100001
#define ONEWIRE_CODE_SILENT_MODE_ON 0b01100111
/* /OneWire codes */

/* Data Exchange Params */
#define CMD_CMD_TURNINGON 0x01
#define CMD_CMD_TURNOFFBEGIN 0x03
#define CMD_CMD_TURNOFFREADY 0x04
#define CMD_CMD_TURNOFF 0x05
#define CMD_CMD_SETMODESTATE 0x10
#define CMD_CMD_SETRTCTIME 0x20

#define CMD_MODE_TRACKING 0x11
#define CMD_MODE_INDICATION 0x12
#define CMD_MODE_SILENCE 0x13
#define CMD_MODE_CONTROL 0x14
#define CMD_MODE_SECURITY 0x15
/* /Data Exchange Params */

IRrecv irrecv(IR_RECV_PIN);
decode_results ir_results;
OneWireHelper *wire_helper;

TM1637Display display(TM_CLK, TM_DIO);
DS1302RTC RTC(DS1302_RST_PIN, DS1302_DAT_PIN, DS1302_CLK_PIN);

unsigned long int command_current = 0;
unsigned long int command_last = 0;
unsigned long int command_active = 0;

bool main_pc_send_off = false;
bool main_pc_ready = false;
unsigned long int main_pc_turnon_time = 0;

bool clock_show_dots = false;
unsigned long int clock_last_show_millis = 0;

byte mode_state_tracking = 0;
byte mode_state_indication = 0;
byte mode_state_silence = 0;
byte mode_state_control = 0;
byte mode_state_security = 0;

void turnOff(bool send_pc_notify, bool send_fd_notify) {
  main_pc_ready = false;
  main_pc_turnon_time = 0;
  digitalWrite(DS1302_VCC_PIN, LOW);
  clearClockDisplay();
  display.setBrightness(0x00);
  while (true) {
    if (send_pc_notify) {
      sendToMainPC(CMD_CMD_TURNOFF, 0, 0, 0);
      delay(500);
    }
    if (send_fd_notify) {
      wire_helper->writeByteCommand(ONEWIRE_CODE_OFF);
      delay(1000);
    } else if (!send_pc_notify) {
      wire_helper->writeSignal(HIGH, 1000);
    }
  }
}

void setCurrentTime(uint32_t t) {
  if(RTC.set((time_t) t) == 0) {
    setTime((time_t) t);
  }
}

void setModeState(byte mode_code, byte mode_state, bool send_pc_notify = false) {
  switch (mode_code) {
    case CMD_MODE_TRACKING:
      mode_state_tracking = mode_state;
      // @todo set tracking state
    break;
    case CMD_MODE_INDICATION:
      mode_state_indication = mode_state;
      // @todo set indication state
    break;
    case CMD_MODE_SILENCE:
      mode_state_silence = mode_state;
      // @todo set silence state
    break;
    case CMD_MODE_CONTROL:
      mode_state_control = mode_state;
      // @todo set control state
    break;
    case CMD_MODE_SECURITY:
      mode_state_security = mode_state;
      // @todo set security state
    break;
  }
  if (main_pc_ready && send_pc_notify) {
    sendToMainPC(CMD_CMD_SETMODESTATE, mode_code, mode_state, 0);
  }
}

void runExternalCommand(MControllerState* state) {
  switch (state->cmd) {
    case CMD_CMD_TURNINGON:
      main_pc_ready = true;
      main_pc_turnon_time = millis();
    break;
    case CMD_CMD_TURNOFF:
      turnOff(false, true);
    break;
    case CMD_CMD_TURNOFFREADY:
      turnOff(false, false);
    break;
    case CMD_CMD_SETMODESTATE:
      setModeState(state->param1, state->param2, false);
    break;
    case CMD_CMD_SETRTCTIME:
      setCurrentTime(state->param0);
    break;
  }
}

void clearClockDisplay() {
  uint8_t segto = SEG_G;
  for (byte i = 0; i < 4; i++) {
    display.setSegments(&segto, 1, i);
  }
}

void showTime() {
  if (clock_last_show_millis < 0 && (millis() - clock_last_show_millis) < CLOCK_DELAY_MS) {
    return;
  }
  clock_last_show_millis = millis();
  if (timeStatus() == timeSet) {
    unsigned int cont = hour()*100 + minute();
    display.showNumberDec(cont, true);
    if (clock_show_dots) {
      uint8_t segto;
      segto = 0x80 | display.encodeDigit((cont / 100)%10);
      display.setSegments(&segto, 1, 1);
    }
    clock_show_dots = !clock_show_dots;
  } else {
    clearClockDisplay();
  }
}

void setup() {
  Serial.begin(DATAEXCHANGE_BAUD_RATE);
  clock_show_dots = false;
  main_pc_send_off = false;
  main_pc_ready = false;
  main_pc_turnon_time = 0;

  TCCR1B = (TCCR1B & 0b11111000) | 0x01;

  // Activate RTC module
  pinMode(DS1302_GND_PIN, OUTPUT);
  pinMode(DS1302_VCC_PIN, OUTPUT);
  digitalWrite(DS1302_GND_PIN, LOW);
  digitalWrite(DS1302_VCC_PIN, HIGH);

  wire_helper = new OneWireHelper(POWER_SIGNAL_MAIN_PIN, onTimerCheck);

  irrecv.enableIRIn();
  display.setBrightness(0x0f);

  RTC.haltRTC();
  RTC.writeEN();
  setSyncProvider(RTC.get);

  resetState();

  delay(500);

  wire_helper->writeSignal(HIGH, 200);
}

void r_loop() {
  if (irrecv.decode(&ir_results)) {
    if (ir_results.decode_type == NEC) {
      command_current = ir_results.value;
      // @todo
    }
    irrecv.resume();
  }
  showTime();
  if (!main_pc_send_off && wire_helper->tryReadOffSignal()) {
    main_pc_send_off = true;
    if (main_pc_ready) {
      sendToMainPC(CMD_CMD_TURNOFF, 0, 0, 0);
    } else {
      turnOff(true, true);
    }
  }
}

void loop() {
  checkState(r_loop, runExternalCommand);
}

void onTimerCheck() {
  // @todo
}

