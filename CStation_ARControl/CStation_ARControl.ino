#include <SPI.h>
#include <MFRC522.h>
#include <IRremote.h>
#include <TM1637Display.h>
#include <TimeLib.h>
#include <DS1302RTC.h>
#include <HMC5883L.h>
#include <Crc16.h>
#include "data_exchange.h"
#include "onewire_helper.h"
#include "indication_controller.h"
#include "device_controller.h"
#include "params.h"

MFRC522 mfrc522(RFID_SS_PIN, RFID_RST_PIN);
IRrecv irrecv(IR_RECV_PIN);
decode_results ir_results;
OneWireHelper *wire_helper;
IndicationController *indication_controller;
DeviceController *device_controller;
HMC5883L magnetic_meter;
bool MXYZ_init = false;
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

volatile bool hc_info_need_to_send = false;

volatile TrackingModeState mode_state_tracking = TRACKING_ON;
volatile IndicationModeState mode_state_indication = INDICATION_ON;
volatile SilenceModeState mode_state_silence = SILENCE_NO;
volatile ControlModeState mode_state_control = CONTROL_ON;
volatile SecurityModeState mode_state_security = SECURITY_LOCKED;
volatile AutoAnimatorModeState mode_state_autoanimator = AA_OFF;

void turnOff(bool send_pc_notify, bool send_fd_notify) {
  indication_controller->LedSet(LED_MAIN_PC_READY, false);
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

void sendCurrentMagnetic() {
  if (MXYZ_init) {
    Vector norm = magnetic_meter.readNormalize();
    sendToMainPC(CMD_CMD_MAGNETIC_SETX, round(norm.XAxis*100));
    sendToMainPC(CMD_CMD_MAGNETIC_SETY, round(norm.YAxis*100));
    sendToMainPC(CMD_CMD_MAGNETIC_SETZ, round(norm.ZAxis*100));
  }
}


void setDeviceState(ControlDevice device, bool device_state, bool send_pc_notify = false) {
  device_controller->ControlSet(device, device_state);
  if (main_pc_ready && send_pc_notify) {
    sendToMainPC(CMD_CMD_SETDEVICESTATE, device, device_state ? 1 : 0, 0);
  }
}

void setModeState(byte mode_code, byte mode_state, bool send_pc_notify = false) {
  if (main_pc_ready && send_pc_notify) {
    sendToMainPC(CMD_CMD_SETMODESTATE, mode_code, mode_state, 0);
  }
  switch (mode_code) {
    case CMD_MODE_TRACKING:
      mode_state_tracking = mode_state;
      setDeviceState(CTRL_CAMERA, mode_state_tracking == TRACKING_ON, send_pc_notify);
    break;
    case CMD_MODE_INDICATION:
      mode_state_indication = mode_state;
      indication_controller->setIndicationShow(mode_state_indication != INDICATION_LEDOFF && mode_state_indication != INDICATION_OFF);
      // INDICATION_LOW & INDICATION_SCREENOFF (INDICATION_OFF) - pc control
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
      setDeviceState(CTRL_LOCKOPEN, mode_state_security == SECURITY_UNLOCKED, send_pc_notify);
    break;
    case CMD_MODE_AUTOANIMATOR:
      mode_state_autoanimator = mode_state;
      if (mode_state_autoanimator == AA_ON) {
        if (mode_state_tracking != TRACKING_ON)         setModeState(CMD_MODE_TRACKING, TRACKING_ON, send_pc_notify);
        if (mode_state_indication != INDICATION_LEDOFF) setModeState(CMD_MODE_INDICATION, INDICATION_LEDOFF, send_pc_notify);
        if (mode_state_silence != SILENCE_NO)           setModeState(CMD_MODE_SILENCE, SILENCE_NO, send_pc_notify);
      }
    break;
  }
}

void runExternalCommand(MControllerState* state) {
  switch (state->cmd) {
    case CMD_CMD_TURNINGON:
      main_pc_ready = true;
      main_pc_turnon_time = millis();
      indication_controller->LedSet(LED_MAIN_PC_READY, true);
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
    case CMD_CMD_SETDEVICESTATE:
      setDeviceState(state->param1, state->param2 > 0, false);
    break;
    case CMD_CMD_SETRTCTIME:
      setCurrentTime(state->param0);
    break;
    case CMD_CMD_MAGNETIC_REQUEST:
      sendCurrentMagnetic();
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
  if (clock_last_show_millis > 0 && (millis() - clock_last_show_millis) < CLOCK_DELAY_MS) {
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
  hc_info_need_to_send = false;
  MXYZ_init = false;
  mode_state_tracking = TRACKING_ON;
  mode_state_indication = INDICATION_ON;
  mode_state_silence = SILENCE_NO;
  mode_state_control = CONTROL_ON;
  mode_state_security = SECURITY_LOCKED;
  mode_state_autoanimator = AA_OFF;

  TCCR1B = (TCCR1B & 0b11111000) | 0x01;

  // Activate RTC module
  pinMode(DS1302_GND_PIN, OUTPUT);
  pinMode(DS1302_VCC_PIN, OUTPUT);
  digitalWrite(DS1302_GND_PIN, LOW);
  digitalWrite(DS1302_VCC_PIN, HIGH);

  // Init Magnetic meter
  MXYZ_init = !!magnetic_meter.begin();
  if (MXYZ_init) 
  {
    magnetic_meter.setRange(HMC5883L_RANGE_1_3GA);
    magnetic_meter.setMeasurementMode(HMC5883L_CONTINOUS);
    magnetic_meter.setDataRate(HMC5883L_DATARATE_30HZ);
    magnetic_meter.setSamples(HMC5883L_SAMPLES_2);
  }

  indication_controller = new IndicationController();
  device_controller = new DeviceController(indication_controller);
  wire_helper = new OneWireHelper(POWER_SIGNAL_MAIN_PIN, onTimerCheck);

  SPI.begin();
  irrecv.enableIRIn();
  display.setBrightness(0x0f);

  RTC.haltRTC();
  RTC.writeEN();
  setSyncProvider(RTC.get);
  mfrc522.PCD_Init();

  resetState();

  delay(500);

  wire_helper->writeSignal(HIGH, 200);

  attachInterrupt(digitalPinToInterrupt(HC_PIN), HC_State_Changed, HC_INTERRUPT_MODE);
}

void r_loop() {
  if (irrecv.decode(&ir_results)) {
    if (ir_results.decode_type == NEC) {
      command_current = ir_results.value;
      // @todo
      indication_controller->LedSet(LED_PDU, true, BLINKING_ONCE);
    }
    irrecv.resume();
  }
  if (mfrc522.PICC_IsNewCardPresent()) {
    if (mfrc522.PICC_ReadCardSerial()) {
      Crc16 crc;
      sendToMainPC(CMD_CMD_CARDFOUND, crc.XModemCrc(mfrc522.uid.uidByte, 0, mfrc522.uid.size));
    }
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
  if (hc_info_need_to_send) {
    hc_info_need_to_send = false;
    indication_controller->LedSet(LED_PRESENCE, true, BLINKING_ONCE);
    sendToMainPC(CMD_CMD_PRESENCE, 0, 0, 0);
  }
}

void loop() {
  checkState(r_loop, runExternalCommand);
}

void HC_State_Changed() {
  if (mode_state_tracking != TRACKING_OFF) {
    hc_info_need_to_send = true;
  }
}

void onTimerCheck() {
  // @todo
}

