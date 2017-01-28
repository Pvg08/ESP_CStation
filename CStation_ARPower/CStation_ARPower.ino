#include "DHT.h"
#include <IRremote.h>
#include "onewire_helper.h"

/* fan-control */
#define TEMP_LOWLEVEL 18
#define TEMP_HIGHLEVEL 40
#define TEMP_CRITICALLEVEL 60
#define HUMIDITY_BASELEVEL 50
#define FAN_ZERO_SPEED 1
#define FAN_MIN_SPEED 140
#define FAN_MAX_SPEED 255
#define FAN_UPDATE_TIME 1120
/* /fan-control */

/* pins */
#define AMPERAGE_ANALOG_PIN A0
#define AMPERAGE_MVPERAMP 66
#define AMPERAGE_ACSOFFSET 2500
#define AMPERAGE_VK (5000 / 1024.0)

#define VOLTAGE_ANALOG_PIN A3
#define VOLTAGE_R1 59850.0
#define VOLTAGE_R2 10000.0
#define BASE_VOLTAGE 5.0
#define VOLTAGE_K (BASE_VOLTAGE * (VOLTAGE_R1+VOLTAGE_R2) / (1024.0 * VOLTAGE_R2))

#define FAN_IN_PIN 10
#define FAN_OUT_PIN 9
#define TONE_PIN 5
#define IRECV_PIN 12

#define POWER_PIN 4
#define CHARGE_PIN 8

/* OneWire */
#define POWER_SIGNAL_MAIN_PIN 7
#define ONEWIRE_CODE_OFF 0x11
#define ONEWIRE_CODE_SILENT_MODE_OFF 0xA2
#define ONEWIRE_CODE_SILENT_MODE_ON 0x78

#define POWER_LED_OFF 0
#define POWER_LED_ON 1
#define POWER_LED_FLASH_FAST 2
#define POWER_LED_FLASH_NORM 3
#define POWER_LED_FLASH_SLOW 4

#define BASE_CHARGING_TIME 14400000
#define BASE_VOLTAGE_ERROR_TIME 45000

#define BTN_SHORTEST_PRESS 20
#define BTN_LONG_PRESS 2000
#define BTN_POWER_PIN 2
#define BTN_POWER_INT 0
#define BTN_CHARGE_PIN 3
#define BTN_CHARGE_INT 1

#define LED_POWER_ALERT_PIN 11

#define DHTPIN 6
/* /pins */

#define TONE_LEN_SHORT 250
#define TONE_LEN_NORMAL 750
#define TONE_LEN_LONG 1500

#define TONE_FREQ_INFO 1000
#define TONE_FREQ_MESSAGE 700
#define TONE_FREQ_ERROR 400

#define VOLTAGE_HAS_SIGNAL 3.0
#define VOLTAGE_LOWLEVEL 10.5
#define VOLTAGE_HIGHLEVEL 12.4
#define VOLTAGE_CRITICALLEVEL 13

#define AMPERAGE_HIGHLEVEL 1
#define AMPERAGE_CRITICALLEVEL 2

/* Command codes */
#define CM_TOGGLEONOFF 0xFFB04F
#define CM_TOGGLECHARGE 0xFFF807

/* dht-sensor */
#define DHT_READ_MS 20000
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
/* /dht-sensor */

/* IR Reciever */
IRrecv irrecv(IRECV_PIN);
decode_results ir_results;

OneWireHelper *wire_helper;

unsigned long int last_dht_read;
byte current_fan_speed;
float voltage;
float amperage;
float temperature;
float humidity;

bool turned_on;
bool turning_off;
unsigned long int turn_on_time, turning_off_start_time;
volatile bool power_btn;
volatile unsigned long int power_btn_press_millis, power_press_duration;

bool charging;
bool constant_charging;
unsigned long int charging_start_time;
unsigned long int last_voltage_error_time;
unsigned long int last_fan_update_time;
volatile bool charge_btn;
volatile unsigned long int charge_btn_press_millis, charge_press_duration;
volatile byte power_led_repeats_cnt;
volatile bool power_led_state;
volatile bool power_led_curr_mode;

unsigned long int last_timer_time;
unsigned timer_interval;

bool silence_mode;

void PowerLedSet(byte mode, byte repeats = 0) {
  if (power_led_curr_mode == mode && power_led_curr_mode == repeats) {
    return;
  }
  power_led_curr_mode = mode;
  power_led_repeats_cnt = 0;
  last_timer_time = millis();
  timer_interval = 0;
  if (mode == POWER_LED_OFF) {
    power_led_state = false;
    digitalWrite(LED_POWER_ALERT_PIN, LOW);
  } else if (mode == POWER_LED_ON) {
    power_led_state = true;
    digitalWrite(LED_POWER_ALERT_PIN, HIGH);
  } else if (mode == POWER_LED_FLASH_FAST) {
    power_led_state = true;
    digitalWrite(LED_POWER_ALERT_PIN, HIGH);
    timer_interval = 100;
    power_led_repeats_cnt = repeats;
  } else if (mode == POWER_LED_FLASH_NORM) {
    power_led_state = true;
    digitalWrite(LED_POWER_ALERT_PIN, HIGH);
    timer_interval = 400;
    power_led_repeats_cnt = repeats;
  } else if (mode == POWER_LED_FLASH_SLOW) {
    power_led_state = true;
    digitalWrite(LED_POWER_ALERT_PIN, HIGH);
    timer_interval = 1000;
    power_led_repeats_cnt = repeats;
  }
}

void fastBeep(unsigned msec, unsigned frequency = TONE_FREQ_MESSAGE) {
  digitalWrite(TONE_PIN, LOW);
  tone(TONE_PIN, frequency);
  unsigned delta = msec >> 3;
  if (delta == 0) delta = 1;
  for(int i=0; i<8; i++) {
    delay(delta);
    onTimerCheck();
  }
  noTone(TONE_PIN);
  digitalWrite(TONE_PIN, HIGH);
}

void updateVoltage() {
  int value = analogRead(VOLTAGE_ANALOG_PIN);
  voltage = value * VOLTAGE_K;
  if (voltage<0.09) {
    voltage=0.0;
  }
}

void updateAmperage() {
  int value = analogRead(AMPERAGE_ANALOG_PIN);
  amperage = value * AMPERAGE_VK;
  amperage = (amperage - AMPERAGE_ACSOFFSET) / AMPERAGE_MVPERAMP;
  if (amperage<0.001) {
    amperage=0.0;
  }
}

void updateFanSpeeds() {
  int calc_speed = FAN_MIN_SPEED;
  bool is_voltage_error = false;
  unsigned long int cmillis = millis();

  if (turn_on_time && (cmillis > turn_on_time) && (cmillis-turn_on_time)<60000) calc_speed+=(FAN_MAX_SPEED - FAN_MIN_SPEED)*(60000+turn_on_time-cmillis)/60000.0;
  
  if (voltage>VOLTAGE_HIGHLEVEL) calc_speed+=(int) (voltage-VOLTAGE_HIGHLEVEL)*80;
  if (voltage>VOLTAGE_CRITICALLEVEL) calc_speed+=400;

  if (amperage > AMPERAGE_CRITICALLEVEL) {
    PowerLedSet(POWER_LED_FLASH_FAST);
    turning_off = true;
    turning_off_start_time = cmillis;
    current_fan_speed = FAN_ZERO_SPEED;
    analogWrite(FAN_IN_PIN, current_fan_speed);
    analogWrite(FAN_OUT_PIN, current_fan_speed);
    fastBeep(TONE_LEN_LONG, TONE_FREQ_ERROR);
    is_voltage_error = true;
    return;
  }

  if (voltage>VOLTAGE_CRITICALLEVEL) {
    PowerLedSet(POWER_LED_FLASH_FAST);
    turning_off = true;
    turning_off_start_time = cmillis;
    current_fan_speed = FAN_MAX_SPEED;
    analogWrite(FAN_IN_PIN, current_fan_speed);
    analogWrite(FAN_OUT_PIN, current_fan_speed);
    fastBeep(TONE_LEN_LONG, TONE_FREQ_ERROR);
    is_voltage_error = true;
    return;
  } else if (amperage > AMPERAGE_HIGHLEVEL || voltage > VOLTAGE_HIGHLEVEL) {
    PowerLedSet(POWER_LED_FLASH_NORM);
    is_voltage_error = true;
  } else if (power_led_curr_mode != POWER_LED_OFF && power_led_curr_mode != POWER_LED_ON) {
    PowerLedSet(POWER_LED_ON);
  }

  if (!isnan(humidity)) {
    calc_speed+=(int) (humidity-HUMIDITY_BASELEVEL);
  }
  if (!isnan(temperature)) {
    if (temperature < TEMP_LOWLEVEL) calc_speed+=10;
    else if (temperature > TEMP_HIGHLEVEL) {
      calc_speed+=(int) (temperature-TEMP_HIGHLEVEL)*5;
      is_voltage_error = true;
    }
  }

  if (silence_mode && !is_voltage_error) {
    calc_speed = calc_speed/2;
  }

  if (calc_speed>FAN_ZERO_SPEED) {
    calc_speed+=FAN_MIN_SPEED;
    if (calc_speed < FAN_MIN_SPEED) calc_speed = FAN_MIN_SPEED;
  } else if (calc_speed<FAN_ZERO_SPEED) {
    calc_speed = FAN_ZERO_SPEED;
  }

  if (calc_speed > FAN_MAX_SPEED) calc_speed = FAN_MAX_SPEED;

  if (silence_mode && !is_voltage_error && calc_speed <= FAN_MIN_SPEED) {
    calc_speed = FAN_ZERO_SPEED;
  }

  if (is_voltage_error && (cmillis > last_voltage_error_time) && (cmillis - last_voltage_error_time) > BASE_VOLTAGE_ERROR_TIME) {
    last_voltage_error_time = cmillis;
    fastBeep(TONE_LEN_NORMAL, TONE_FREQ_ERROR);
  }

  if (current_fan_speed != calc_speed) {
    current_fan_speed = calc_speed;
    analogWrite(FAN_IN_PIN, current_fan_speed);
    analogWrite(FAN_OUT_PIN, current_fan_speed);
  }
}

void PowerBTN_Change()
{
  bool is_pressed = digitalRead(BTN_POWER_PIN) == HIGH;
  unsigned long int cmillis = millis();
  if (is_pressed || !power_btn_press_millis) {
    power_btn_press_millis = cmillis;
  } else {
    power_btn_press_millis = cmillis-power_btn_press_millis;
    if (power_btn_press_millis>BTN_SHORTEST_PRESS) {
      if (power_press_duration<power_btn_press_millis) power_press_duration = power_btn_press_millis;
      power_btn = true;
    }
    power_btn_press_millis = cmillis;
  }
}

void ChargeBTN_Change() 
{
  bool is_pressed = digitalRead(BTN_CHARGE_PIN) == HIGH;
  unsigned long int cmillis = millis();
  if (is_pressed || !charge_btn_press_millis) {
    charge_btn_press_millis = cmillis;
  } else {
    charge_btn_press_millis = cmillis-charge_btn_press_millis;
    if (charge_btn_press_millis>BTN_SHORTEST_PRESS) {
      if (charge_press_duration<charge_btn_press_millis) charge_press_duration = charge_btn_press_millis;
      charge_btn = true;
    }
    charge_btn_press_millis = cmillis;
  }
}

void fastOff()
{
  turned_on = turning_off = false;
  turning_off_start_time = 0;
  PowerLedSet(POWER_LED_OFF);
  digitalWrite(POWER_PIN, LOW);
  power_btn = false;
  power_btn_press_millis = power_press_duration = turn_on_time = 0;
  irrecv.enableIRIn();
}

void turnOn(bool isfast)
{
  last_dht_read = 0;
  voltage = 0;
  temperature = NAN;
  humidity = NAN;
  power_btn_press_millis = 0;
  charge_btn_press_millis = 0;
  power_press_duration = 0;
  charge_press_duration = 0;
  turning_off = false;
  turning_off_start_time = 0;
  power_led_state = 0;
  power_led_repeats_cnt = 0;
  power_led_curr_mode = 255;
  last_voltage_error_time = 0;
  last_fan_update_time = 0;
  last_timer_time = 0;
  silence_mode = false;
  irrecv.enableIRIn();

  turned_on = true;
  turn_on_time = 0;
  digitalWrite(POWER_PIN, HIGH);
  PowerLedSet(POWER_LED_FLASH_NORM);

  current_fan_speed = FAN_MAX_SPEED;
  analogWrite(FAN_IN_PIN, current_fan_speed);
  analogWrite(FAN_OUT_PIN, current_fan_speed);

  byte i = 0;
  voltage = 0;
  while (i<=10 && voltage<VOLTAGE_HAS_SIGNAL) {
    fastBeep(TONE_LEN_SHORT, i>0 ? TONE_FREQ_ERROR : TONE_FREQ_INFO);
    delay(TONE_LEN_SHORT);
    onTimerCheck();
    updateVoltage();
    updateAmperage();
    i++;
  }

  bool is_ready = isfast && voltage>=VOLTAGE_HAS_SIGNAL;
  wire_helper->reset();
  
  if (!is_ready && voltage>=VOLTAGE_HAS_SIGNAL) {
    PowerLedSet(POWER_LED_FLASH_FAST);
    is_ready = wire_helper->waitStartSignal();
    if (is_ready) {
      fastBeep(TONE_LEN_SHORT, TONE_FREQ_MESSAGE);
    }
  }

  if (is_ready) {
    PowerLedSet(POWER_LED_ON);
  } else {
    fastOff();
    fastBeep(TONE_LEN_LONG, TONE_FREQ_ERROR);
  }
  power_btn = false;
  power_btn_press_millis = power_press_duration = 0;
  turn_on_time = is_ready ? millis() : 0;
}

void updateOnOffState()
{
  if (turning_off) {
    fastBeep(TONE_LEN_SHORT, TONE_FREQ_INFO);
    PowerLedSet(POWER_LED_FLASH_SLOW);
    bool external_signal = wire_helper->sendOffPing();
    fastOff();
    fastBeep(TONE_LEN_LONG, external_signal ? TONE_FREQ_MESSAGE : TONE_FREQ_ERROR);
    return;
  }

  if (power_btn) {
    bool long_press = power_press_duration>=BTN_LONG_PRESS;

    if (turned_on) {
      if (long_press) {
        fastOff();
        fastBeep(TONE_LEN_LONG, TONE_FREQ_MESSAGE);
      } else {
        turning_off = true;
        turning_off_start_time = millis();
      }
    } else {
      turnOn(long_press);
    }

    power_btn = false;
    power_btn_press_millis = power_press_duration = 0;
  }
}

void updateChargeState()
{
  unsigned long int cmillis = millis();
  
  if (charge_btn) {
    bool long_press = charge_press_duration>=BTN_LONG_PRESS;

    if (charging) {
      charging = false;
      constant_charging = false;
      charging_start_time = 0;
      fastBeep(TONE_LEN_SHORT, TONE_FREQ_MESSAGE);
      digitalWrite(CHARGE_PIN, LOW);
    } else {
      if (long_press) {
        constant_charging = true;
        charging_start_time = 0;
        fastBeep(TONE_LEN_NORMAL, TONE_FREQ_INFO);
      } else {
        constant_charging = false;
        charging_start_time = cmillis;
        fastBeep(TONE_LEN_SHORT, TONE_FREQ_INFO);
      }
      charging = true;
      digitalWrite(CHARGE_PIN, HIGH);
    }

    charge_btn = false;
    charge_btn_press_millis = charge_press_duration = 0;
  } else {
    if (charging && !constant_charging && cmillis-charging_start_time > BASE_CHARGING_TIME) {
      charging = false;
      constant_charging = false;
      charging_start_time = 0;
      digitalWrite(CHARGE_PIN, LOW);
      fastBeep(TONE_LEN_SHORT, TONE_FREQ_MESSAGE);
    }
  }
}

void setup() {
  last_dht_read = 0;
  voltage = 0;
  current_fan_speed = 0;
  temperature = NAN;
  humidity = NAN;
  power_btn = false;
  charge_btn = false;
  power_btn_press_millis = 0;
  charge_btn_press_millis = 0;
  power_press_duration = 0;
  charge_press_duration = 0;
  turned_on = false;
  turning_off = false;
  charging = false;
  constant_charging = false;
  turn_on_time = 0;
  charging_start_time = 0;
  turning_off_start_time = 0;
  power_led_state = 0;
  power_led_repeats_cnt = 0;
  power_led_curr_mode = 255;
  last_voltage_error_time = 0;
  last_fan_update_time = 0;
  last_timer_time = 0;
  silence_mode = false;

  pinMode(LED_POWER_ALERT_PIN, OUTPUT);
  pinMode(FAN_IN_PIN, OUTPUT);
  pinMode(FAN_OUT_PIN, OUTPUT);
  pinMode(TONE_PIN, OUTPUT);
  pinMode(POWER_PIN, OUTPUT);
  pinMode(CHARGE_PIN, OUTPUT);
  pinMode(BTN_POWER_PIN, INPUT);
  pinMode(BTN_CHARGE_PIN, INPUT);
  pinMode(DHTPIN, INPUT);
  wire_helper = new OneWireHelper(POWER_SIGNAL_MAIN_PIN, onTimerCheck);

  PowerLedSet(POWER_LED_OFF);
  digitalWrite(POWER_PIN, LOW);
  analogWrite(FAN_IN_PIN, FAN_ZERO_SPEED);
  analogWrite(FAN_OUT_PIN, FAN_ZERO_SPEED);
  digitalWrite(TONE_PIN, LOW);
  digitalWrite(CHARGE_PIN, LOW);

  attachInterrupt(BTN_POWER_INT, PowerBTN_Change, CHANGE);
  attachInterrupt(BTN_CHARGE_INT, ChargeBTN_Change, CHANGE);
  dht.begin();
  irrecv.enableIRIn();
  irrecv.blink13(1);
  pinMode(IRECV_PIN, INPUT);
  fastBeep(TONE_LEN_SHORT, TONE_FREQ_MESSAGE);

  TCCR2B = (TCCR2B & 0b11111000) | 0x01;
}

void loop() {
  int onewire_byte = wire_helper->readByte();
  if (onewire_byte >= 0) {
    if (onewire_byte == ONEWIRE_CODE_OFF && !turning_off) {
      turning_off = true;
      turning_off_start_time = millis();
      power_btn = false;
      power_btn_press_millis = power_press_duration = 0;
    } else if (onewire_byte == ONEWIRE_CODE_SILENT_MODE_ON) {
      silence_mode = true;
      last_fan_update_time = 0;
    } else if (onewire_byte == ONEWIRE_CODE_SILENT_MODE_OFF) {
      silence_mode = true;
      last_fan_update_time = 0;
    }
  }
  if (irrecv.decode(&ir_results)) {
    if (ir_results.decode_type == NEC) {
      if (ir_results.value == CM_TOGGLEONOFF) {
        power_btn = true;
        power_press_duration = 0;
      } else if (ir_results.value == CM_TOGGLECHARGE) {
        charge_btn = true;
        charge_press_duration = 0;
      } else if (ir_results.value == 0xFFB847) {
         silence_mode = !silence_mode;
         last_fan_update_time = 0;
      }
    }
    irrecv.resume();
  }
  onTimerCheck();
  updateVoltage();
  updateAmperage();

  if (voltage>VOLTAGE_HAS_SIGNAL) {
    if (!last_dht_read || millis() - last_dht_read > DHT_READ_MS) {
      last_dht_read = millis();
      float h = dht.readHumidity();
      float t = dht.readTemperature();
      if (!isnan(h)) humidity = h;
      if (!isnan(t)) temperature = t;
      onTimerCheck();
      if (!isnan(h) || !isnan(t)) {
        digitalWrite(13, HIGH);
        delay(100);
        digitalWrite(13, LOW);
        onTimerCheck();
      }
    }

    if (turned_on && !turning_off && ((millis() - last_fan_update_time) >= FAN_UPDATE_TIME)) {
      last_fan_update_time = millis();
      updateFanSpeeds();
    }
  }

  updateOnOffState();
  updateChargeState();
}

void onTimerCheck() {
  if (timer_interval && (last_timer_time+timer_interval)<=millis()) {
    if (power_led_repeats_cnt > 0) {
      power_led_repeats_cnt--;
      if (power_led_repeats_cnt == 0) {
        timer_interval = 0;
      }
    }
    power_led_state = !power_led_state;
    digitalWrite(LED_POWER_ALERT_PIN, power_led_state ? HIGH : LOW);
    last_timer_time = millis();
  }
}

