#include "onewire_helper.h"

#define POWER_SIGNAL_MAIN_PIN 12

#define ONEWIRE_CODE_NOOP 0x00
#define ONEWIRE_CODE_OFF 0b11011001
#define ONEWIRE_CODE_SILENT_MODE_OFF 0b11100000
#define ONEWIRE_CODE_SILENT_MODE_MID 0b10100001
#define ONEWIRE_CODE_SILENT_MODE_ON 0b01100111

OneWireHelper *wire_helper;

void setup() {
  wire_helper = new OneWireHelper(POWER_SIGNAL_MAIN_PIN, NULL);
}

void loop() {
  delay(500);
  wire_helper->writeSignal(HIGH, 1000);
  delay(1500);
  wire_helper->writeSignal(LOW, 1);

  delay(40000);
  wire_helper->writeByteCommand(ONEWIRE_CODE_SILENT_MODE_MID);
  delay(10000);
  wire_helper->writeByteCommand(ONEWIRE_CODE_SILENT_MODE_ON);
  delay(10000);
  wire_helper->writeByteCommand(ONEWIRE_CODE_SILENT_MODE_OFF);
  delay(15000);
  wire_helper->writeByteCommand(ONEWIRE_CODE_OFF);
  delay(15000);

  do {
    delay(500);
  } while(true);
}
