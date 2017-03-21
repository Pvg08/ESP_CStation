#ifndef DATA_EXCHANGE_H
#define DATA_EXCHANGE_H

#include <Crc16.h>

#define STATE_BUFFER_SIZE 6
#define DATAEXCHANGE_BAUD_RATE 115200
#define MAX_RECIEVE_INTERVAL 12000

#define CMD_PLAY 0
#define CMD_NONE 1
#define CMD_RESET 'R'

#define __PACKED __attribute__((packed))

struct StateStruct {
    uint32_t state_index __PACKED;
    uint32_t timeout __PACKED;
    uint16_t command __PACKED;
    uint16_t hash __PACKED;
} __PACKED;

struct MControllerState : StateStruct {
  uint8_t cmd __PACKED;
  uint32_t param0 __PACKED;
  uint8_t param1 __PACKED;
  uint8_t param2 __PACKED;
  uint8_t param3 __PACKED;
} __PACKED;

#define STATE_SIZE sizeof(MControllerState)

struct MControllerState state_buffer[STATE_BUFFER_SIZE] = {};

MControllerState state_old;

unsigned buffer_playposition;
unsigned buffer_writeposition;
bool first_reading;
uint32_t last_played_pos;
uint32_t last_readed_pos;
uint32_t last_recieve_millis;
uint32_t last_state_millis;
uint32_t last_state_delay;

Crc16 crc;

void resetState() {
  state_buffer[0].state_index = 0;
  last_played_pos = 0;
  last_readed_pos = 0;
  buffer_playposition = 0;
  buffer_writeposition = 0;
  last_recieve_millis = 0;
  last_state_millis = 0;
  last_state_delay = 0;
  first_reading = false;
  state_old.cmd = 0;
  state_old.param1 = 0;
  state_old.param2 = 0;
  state_old.param3 = 0;
}

unsigned nextBufferPosition(unsigned cpos) {
  cpos++;
  if (cpos>=STATE_BUFFER_SIZE) cpos = 0;
  return cpos;
}

bool setState(MControllerState* state, void (*runExternalCommand)(MControllerState*)) {
  if (state->command != CMD_PLAY) return false;
  state->command = CMD_NONE;
  last_played_pos = state->state_index;
  last_state_delay = state->timeout;
  last_state_millis = millis();
  if (runExternalCommand) runExternalCommand(state);
  return true;
}

void sendPosConfirmation(uint32_t pos, bool is_play, bool do_flush = true) {
  byte* last_pos = (byte*) (void*) &pos;
  Serial.write('C');
  Serial.write('S');
  Serial.write(is_play ? 'P' : 'W');
  Serial.write(last_pos[0]);
  Serial.write(last_pos[1]);
  Serial.write(last_pos[2]);
  Serial.write(last_pos[3]);
  if (do_flush) Serial.flush();
}

void ClearSerial() {
  while (Serial.available()) {
    Serial.read();
    delay(2);
  }
}

void sendToMainPC(uint8_t command, uint8_t param1, uint8_t param2, uint8_t param3) {
  ClearSerial();
  Serial.write('C');
  Serial.write('M');
  Serial.write('D');
  Serial.write(command);
  Serial.write(param1);
  Serial.write(param2);
  Serial.write(param3);
  Serial.flush();
}
void sendToMainPC(uint8_t command, uint16_t param1) {
  ClearSerial();
  byte* last_pos = (byte*) (void*) &param1;
  Serial.write('C');
  Serial.write('M');
  Serial.write('D');
  Serial.write(command);
  Serial.write(last_pos[0]);
  Serial.write(last_pos[1]);
  Serial.write(0);
  Serial.flush();
}

void WaitORClear() {
  if (Serial.available() >= STATE_SIZE) return;
  int avail = 0;
  while (avail < Serial.available()) {
    avail = Serial.available();
    delay(5);
  }
}

void checkState(void (*callback)(), void (*runExternalCommand)(MControllerState*)) {

  bool new_state, new_play_state;

  while (true) {
    new_state = false;
    new_play_state = false;
    while (Serial.available() >= STATE_SIZE) {
      last_recieve_millis = millis();
      char* next_pos = (char*) (void*) &(state_buffer[buffer_writeposition]);
      for(byte i=0; i<STATE_SIZE; i++) {
        next_pos[i] = Serial.read();
      }
      if (state_buffer[buffer_writeposition].hash == 0 && state_buffer[buffer_writeposition].state_index == 0 && state_buffer[buffer_writeposition].timeout == 0 && state_buffer[buffer_writeposition].command == CMD_RESET) {
        resetState();
        delay(100);
        continue;
      }
      uint16_t ihash = state_buffer[buffer_writeposition].hash;
      state_buffer[buffer_writeposition].hash = 0;
      if ((!first_reading || state_buffer[buffer_writeposition].state_index==(last_readed_pos+1)) && ihash==crc.XModemCrc((uint8_t*) (void*) &(state_buffer[buffer_writeposition]), 0, STATE_SIZE)) {
        first_reading = true;
        last_readed_pos = state_buffer[buffer_writeposition].state_index;
        buffer_writeposition=nextBufferPosition(buffer_writeposition);
        WaitORClear();
      } else {
        ClearSerial();
      }
      new_state = true;
    }
    if (first_reading) {
      if (buffer_playposition != buffer_writeposition) {
        if (!last_state_millis || (millis()-last_state_millis)>=last_state_delay) {
          setState(&(state_buffer[buffer_playposition]), runExternalCommand);
          buffer_playposition=nextBufferPosition(buffer_playposition);
          new_play_state = true;
        }
      } else if ((millis()-last_recieve_millis) >= MAX_RECIEVE_INTERVAL) {
        new_play_state = true;
        last_recieve_millis = millis();
      }
    }
    if (new_state || new_play_state) {
      sendPosConfirmation(last_readed_pos, false, false);
      sendPosConfirmation(last_played_pos, true, true);
    }

    if (callback) callback();
    
    delay(1);
  }
}

#endif
