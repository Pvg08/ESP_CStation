#include <SPI.h>
#include <MFRC522.h>
#include <DirectIO.h>
#include <IRremote.h>
#include <Timer5.h>
#include <TM1637Display.h>
#include <TimeLib.h>
#include <DS1302RTC.h>
#include <HMC5883L.h>
#include <Crc16.h>

#include "params.h"
#include "data_exchange.h"
#include "onewire_helper.h"
#include "indication_controller.h"
#include "device_controller.h"
#include "mode_controller.h"


MFRC522 mfrc522(RFID_SS_PIN, RFID_RST_PIN);
IRrecv irrecv(IR_RECV_PIN);
decode_results ir_results;
OneWireHelper *wire_helper;
IndicationController *indication_controller;
DeviceController *device_controller;
ModeController *mode_controller;
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
      mode_controller->setModeState(state->param1, state->param2, false);
    break;
    case CMD_CMD_SETDEVICESTATE:
      device_controller->setDeviceState(state->param1, state->param2 > 0, false);
    break;
    case CMD_CMD_SETRTCTIME:
      setCurrentTime(state->param0);
    break;
    case CMD_CMD_MAGNETIC_REQUEST:
      sendCurrentMagnetic();
    break;
  }
}

bool checkLightCMD(unsigned long int command) {
  ElightControl ctrl = LIGHT_NOP;
  switch (command) {
    case CM_LIGHT_ON:           ctrl = LIGHT_ON;          break;
    case CM_LIGHT_OFF:          ctrl = LIGHT_OFF;         break;
    case CM_LIGHT_B_DOWN:       ctrl = LIGHT_B_DOWN;      break;
    case CM_LIGHT_B_DOWN2:      ctrl = LIGHT_B_DOWN2;     break;
    case CM_LIGHT_B_UP:         ctrl = LIGHT_B_UP ;       break;
    case CM_LIGHT_B_UP2:        ctrl = LIGHT_B_UP2;       break;
    case CM_LIGHT_R:            ctrl = CM_LIGHT_R;        break;
    case CM_LIGHT_G:            ctrl = CM_LIGHT_G;        break;
    case CM_LIGHT_B:            ctrl = CM_LIGHT_B;        break;
    case CM_LIGHT_W:            ctrl = CM_LIGHT_W;        break;
    case CM_LIGHT_COL11:        ctrl = LIGHT_COL11;       break;
    case CM_LIGHT_COL21:        ctrl = LIGHT_COL21;       break;
    case CM_LIGHT_COL31:        ctrl = LIGHT_COL31;       break;
    case CM_LIGHT_COL41:        ctrl = LIGHT_COL41;       break;
    case CM_LIGHT_COL22:        ctrl = LIGHT_COL22;       break;
    case CM_LIGHT_COL32:        ctrl = LIGHT_COL32;       break;
    case CM_LIGHT_COL42:        ctrl = LIGHT_COL42;       break;
    case CM_LIGHT_COL13:        ctrl = LIGHT_COL13;       break;
    case CM_LIGHT_COL23:        ctrl = LIGHT_COL23;       break;
    case CM_LIGHT_COL33:        ctrl = LIGHT_COL33;       break;
    case CM_LIGHT_COL43:        ctrl = LIGHT_COL43;       break;
    case CM_LIGHT_MODE_FLASH:   ctrl = LIGHT_MODE_FLASH;  break;
    case CM_LIGHT_MODE_STROBE:  ctrl = LIGHT_MODE_STROBE; break;
    case CM_LIGHT_MODE_FADE:    ctrl = LIGHT_MODE_FADE;   break;
    case CM_LIGHT_MODE_SMOOTH:  ctrl = LIGHT_MODE_SMOOTH; break;

  }
  if (ctrl != LIGHT_NOP) {
    sendToMainPC(CMD_CMD_LIGHTPDUCMD, ctrl, 0, 0);
    return true;
  }
  return false;
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
  wire_helper = new OneWireHelper(POWER_SIGNAL_MAIN_PIN, NULL);
  mode_controller = ModeController::Instance(device_controller, wire_helper);

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
      indication_controller->LedSet(LED_PDU, true, BLINKING_ONCE);
      command_current = ir_results.value;
      if (command_current) {
        if (device_controller->DeviceControlIRCode(command_current) < 0) {
          if (!checkLightCMD(command_current)) {
            if (mode_controller->ModeControlIRCode(command_current) < 0) {
              // @todo check other commands (if they will be)
            }
          }
        }
      }
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
  if (!main_pc_send_off) {
    mode_controller->checkClicks();
    if (hc_info_need_to_send) {
      hc_info_need_to_send = false;
      indication_controller->LedSet(LED_PRESENCE, true, BLINKING_ONCE);
      sendToMainPC(CMD_CMD_PRESENCE, 0, 0, 0);
    }
    if (wire_helper->tryReadOffSignal()) {
      main_pc_send_off = true;
      if (main_pc_ready) {
        sendToMainPC(CMD_CMD_TURNOFF, 0, 0, 0);
      } else {
        turnOff(true, true);
      }
    }
  }
}

void loop() {
  checkState(r_loop, runExternalCommand);
}

void HC_State_Changed() {
  if (mode_controller->getModeState(MODE_TRACKING) != TRACKING_OFF) {
    hc_info_need_to_send = true;
  }
}

ISR(timer5Event)
{
  resetTimer5();
  indication_controller->onTimer();
}

