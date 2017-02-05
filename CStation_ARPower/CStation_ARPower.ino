#include "DHT.h"
#include <IRremote.h>
#include <TimerFreeTone.h>
#include "onewire_helper.h"

#define CSTATION_DEBUG_MODE

/* fan-control */
#define FAN_ZERO_SPEED 0
#define FAN_MIN_SPEED 75
#define FAN_MAX_SPEED 255
#define FAN_UPDATE_TIME 1120

#define TEMP_LOWLEVEL 15
#define TEMP_NORMLEVEL 30
#define TEMP_HIGHLEVEL 45
#define TEMP_CRITICALLEVEL 60
#define HUMIDITY_BASELEVEL 50
/* /fan-control */

/* pins */
#define AMPERAGE_ANALOG_PIN A0
#define VOLTAGE_ANALOG_PIN A3
#define FAN_IN_PIN 10
#define FAN_OUT_PIN 9
#define TONE_PIN 5
#define IRECV_PIN 12
#define POWER_PIN 4
#define CHARGE_PIN 8
#define DHTPIN 6
#define LED_POWER_ALERT_PIN 11
#define BTN_POWER_PIN 2
#define BTN_CHARGE_PIN 3
/* /pins */

/* amperage-voltage levels */
#define VOLTAGE_HAS_SIGNAL 7.0
#define VOLTAGE_LOWLEVEL 10.5
#define VOLTAGE_NORMALLEVEL 12.0
#define VOLTAGE_HIGHLEVEL 12.8
#define VOLTAGE_CRITICALLEVEL 13

#define AMPERAGE_NORMALLEVEL 0.5
#define AMPERAGE_HIGHLEVEL 1
#define AMPERAGE_CRITICALLEVEL 2
/* /amperage-voltage levels */

/* amperage-voltage constants */
#define AMPERAGE_MVPERAMP 66
#define AMPERAGE_ACSOFFSET 2500
#define AMPERAGE_VK (5000 / 1024.0)

#define VOLTAGE_R1 59850.0
#define VOLTAGE_R2 10000.0
#define BASE_VOLTAGE 5.0
#define VOLTAGE_K (BASE_VOLTAGE * (VOLTAGE_R1+VOLTAGE_R2) / (1024.0 * VOLTAGE_R2))
/* /amperage-voltage constants */

/* OneWire codes */
#define POWER_SIGNAL_MAIN_PIN 7
#define ONEWIRE_CODE_NOOP 0x00
#define ONEWIRE_CODE_OFF 0b11011001
#define ONEWIRE_CODE_SILENT_MODE_OFF 0b11100000
#define ONEWIRE_CODE_SILENT_MODE_MID 0b10100001
#define ONEWIRE_CODE_SILENT_MODE_ON 0b01100111
/* /OneWire codes */

/* LED codes & intervals */
#define POWER_LED_OFF 0
#define POWER_LED_ON 1
#define POWER_LED_FLASH_FAST 2
#define POWER_LED_FLASH_NORM 3
#define POWER_LED_FLASH_SLOW 4

#define POWER_LED_FLASH_INTERVAL_FAST 100
#define POWER_LED_FLASH_INTERVAL_NORM 400
#define POWER_LED_FLASH_INTERVAL_SLOW 1000
/* /LED codes & intervals */

/* button params */
#define BTN_SHORTEST_PRESS 20
#define BTN_LONG_PRESS 2000
#define BTN_POWER_INT 0
#define BTN_CHARGE_INT 1
/* /button params */

/* tone signal params */
#define TONE_LEN_SHORT 250
#define TONE_LEN_NORMAL 900
#define TONE_LEN_LONG 2000

#define TONE_FREQ_INFO 2200
#define TONE_FREQ_MESSAGE 600
#define TONE_FREQ_ERROR 300
/* /tone signal params */

/* Command codes */
#define CM_TOGGLEONOFF 0xFFB04F
#define CM_TOGGLECHARGE 0xFFF807
#define CM_TOGGLESILENCE 0xFFB847
#define CM_TOGGLESPEEDTEST 0xFF9867
/* /Command codes */

/* dht-sensor params */
#define DHT_READ_MS 20000
#define DHTTYPE DHT11
/* /dht-sensor params */

/* other consts */
#define BASE_CHARGING_TIME 14400000
#define BASE_VOLTAGE_ERROR_TIME 45000
/* /other consts */

DHT dht(DHTPIN, DHTTYPE);
IRrecv irrecv(IRECV_PIN);
decode_results ir_results;
OneWireHelper *wire_helper;

unsigned long int last_dht_read;
byte current_fan_speed;
float voltage, voltage_old;
float amperage, amperage_old;
float temperature;
float humidity;

bool turned_on;
bool turning_off;
unsigned long int turn_on_time;
volatile bool power_btn;
volatile unsigned long int power_btn_press_millis, power_press_duration;

bool charging, constant_charging;
unsigned long int charging_start_time;
unsigned long int last_voltage_error_time;
unsigned long int last_fan_update_time;
volatile bool charge_btn;
volatile unsigned long int charge_btn_press_millis, charge_press_duration;
volatile byte power_led_repeats_cnt;
volatile bool power_led_state;
volatile byte power_led_curr_mode;

unsigned long int last_timer_time;
unsigned timer_interval;

bool silence_mode, silence_ismid;

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
    timer_interval = POWER_LED_FLASH_INTERVAL_SLOW;
    power_led_repeats_cnt = repeats ? repeats : 20;
  } else if (mode == POWER_LED_FLASH_FAST) {
    power_led_state = true;
    digitalWrite(LED_POWER_ALERT_PIN, HIGH);
    timer_interval = POWER_LED_FLASH_INTERVAL_FAST;
    power_led_repeats_cnt = repeats;
  } else if (mode == POWER_LED_FLASH_NORM) {
    power_led_state = true;
    digitalWrite(LED_POWER_ALERT_PIN, HIGH);
    timer_interval = POWER_LED_FLASH_INTERVAL_NORM;
    power_led_repeats_cnt = repeats;
  } else if (mode == POWER_LED_FLASH_SLOW) {
    power_led_state = true;
    digitalWrite(LED_POWER_ALERT_PIN, HIGH);
    timer_interval = POWER_LED_FLASH_INTERVAL_SLOW;
    power_led_repeats_cnt = repeats;
  }
}

void fastBeep(unsigned msec, unsigned frequency = TONE_FREQ_MESSAGE) {
  if (silence_mode) {
    if ((frequency == TONE_FREQ_INFO) || (silence_ismid && (frequency == TONE_FREQ_MESSAGE))) {
      wire_helper->sleepDelay(msec, 50);
      return;
    }
  }
  TimerFreeTone(TONE_PIN, frequency, msec);
  onTimerCheck();
}

void updateVoltage() {
  voltage_old = voltage;
  int value = analogRead(VOLTAGE_ANALOG_PIN);
  voltage = value * VOLTAGE_K;
  if (voltage<0.09) {
    voltage=0.0;
  }
  voltage = (voltage + voltage_old) * 0.5;
}

void updateAmperage() {
  amperage_old = amperage;
  int value = analogRead(AMPERAGE_ANALOG_PIN);
  amperage = value * AMPERAGE_VK;
  amperage = (amperage - AMPERAGE_ACSOFFSET) / AMPERAGE_MVPERAMP;
  if (amperage<0.001) {
    amperage=0.0;
  }
  amperage = (amperage + amperage_old) * 0.5;
}

void updateFanSpeeds() {
  int calc_speed = FAN_MIN_SPEED;
  bool is_feed_error = false;
  unsigned long int cmillis = millis();

  if (turn_on_time && (cmillis > turn_on_time) && (cmillis-turn_on_time)<60000) calc_speed+=(FAN_MAX_SPEED - FAN_MIN_SPEED)*(60000+turn_on_time-cmillis)/60000.0;
  
  if (voltage > VOLTAGE_HIGHLEVEL) calc_speed+=(int) (voltage-VOLTAGE_HIGHLEVEL)*120;
  if (amperage > AMPERAGE_HIGHLEVEL) calc_speed-=(int) (amperage-AMPERAGE_HIGHLEVEL)*120;

  if (amperage > AMPERAGE_CRITICALLEVEL || voltage > VOLTAGE_CRITICALLEVEL) {
    PowerLedSet(POWER_LED_FLASH_FAST);
    turning_off = true;
    current_fan_speed = (amperage > AMPERAGE_CRITICALLEVEL) ? FAN_ZERO_SPEED : FAN_MAX_SPEED;
    analogWrite(FAN_IN_PIN, current_fan_speed);
    analogWrite(FAN_OUT_PIN, current_fan_speed);
    fastBeep(TONE_LEN_LONG, TONE_FREQ_ERROR);
    is_feed_error = true;
    return;
  }
  if (amperage > AMPERAGE_HIGHLEVEL || voltage > VOLTAGE_HIGHLEVEL) {
    PowerLedSet(POWER_LED_FLASH_NORM);
    is_feed_error = true;
  }

  if (!isnan(humidity)) {
    calc_speed+=(int) (humidity-HUMIDITY_BASELEVEL);
  }
  if (!isnan(temperature)) {
    if (temperature < TEMP_LOWLEVEL) calc_speed+=10;
    else if (temperature > TEMP_NORMLEVEL) {
      calc_speed+=(int) (temperature-TEMP_HIGHLEVEL)*5;
      if (temperature > TEMP_HIGHLEVEL) {
        is_feed_error = true;
      }
    }
  }

  if (silence_mode && !is_feed_error) {
    calc_speed = silence_ismid ? FAN_MIN_SPEED : FAN_ZERO_SPEED;
  }

  if (calc_speed>FAN_ZERO_SPEED) {
    calc_speed+=FAN_MIN_SPEED;
    if (calc_speed < FAN_MIN_SPEED) calc_speed = FAN_MIN_SPEED;
  } else if (calc_speed<=FAN_ZERO_SPEED) {
    calc_speed = FAN_ZERO_SPEED;
  }

  if (calc_speed > FAN_MAX_SPEED) calc_speed = FAN_MAX_SPEED;

  if (silence_mode && !is_feed_error && calc_speed <= FAN_MIN_SPEED) {
    calc_speed = silence_ismid ? FAN_MIN_SPEED : FAN_ZERO_SPEED;
  }

  if (is_feed_error && (cmillis > last_voltage_error_time) && (cmillis - last_voltage_error_time) > BASE_VOLTAGE_ERROR_TIME) {
    last_voltage_error_time = cmillis;
    fastBeep(TONE_LEN_NORMAL, TONE_FREQ_ERROR);
  } else if (!is_feed_error && power_led_curr_mode != POWER_LED_OFF && power_led_curr_mode != POWER_LED_ON) {
    PowerLedSet(POWER_LED_ON);
  }

  if (current_fan_speed != calc_speed) {
    if (calc_speed > current_fan_speed && current_fan_speed <= FAN_MIN_SPEED) {
      analogWrite(FAN_IN_PIN, FAN_MAX_SPEED);
      analogWrite(FAN_OUT_PIN, FAN_MAX_SPEED);
      wire_helper->sleepDelay(1000, 100);
    }
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
  PowerLedSet(POWER_LED_OFF);
  digitalWrite(POWER_PIN, LOW);
  reset_onoffvars();
  irrecv.enableIRIn();
}

void reset_chargevars() {
  digitalWrite(CHARGE_PIN, LOW);
  charge_btn = false;
  charge_btn_press_millis = 0;
  charge_press_duration = 0;
  charging = false;
  constant_charging = false;
  charging_start_time = 0;
}

void reset_onoffvars() {
  digitalWrite(POWER_PIN, LOW);
  analogWrite(FAN_IN_PIN, FAN_ZERO_SPEED);
  analogWrite(FAN_OUT_PIN, FAN_ZERO_SPEED);
  digitalWrite(TONE_PIN, LOW);
  last_dht_read = 0;
  temperature = NAN;
  humidity = NAN;
  current_fan_speed = 0;
  power_btn = false;
  power_btn_press_millis = 0;
  power_press_duration = 0;
  turn_on_time = 0;
  turned_on = false;
  turning_off = false;
  power_led_state = false;
  power_led_repeats_cnt = 0;
  power_led_curr_mode = 255;
  last_voltage_error_time = 0;
  last_fan_update_time = 0;
  last_timer_time = 0;
  silence_mode = false;
  silence_ismid = false;
  voltage = 0.0;
  amperage = 0.0;
  PowerLedSet(POWER_LED_OFF);
}

void turnOn(bool isfast)
{
  reset_onoffvars();

  turned_on = true;
  digitalWrite(POWER_PIN, HIGH);
  PowerLedSet(POWER_LED_FLASH_NORM);

  amperage = AMPERAGE_NORMALLEVEL;

  for(byte i=0; i<10 && (voltage < VOLTAGE_HAS_SIGNAL); i++) {
    fastBeep(TONE_LEN_SHORT, i>0 ? TONE_FREQ_ERROR : TONE_FREQ_INFO);
    wire_helper->sleepDelay(500, 100);
    updateVoltage();
    updateAmperage();
  }

  bool is_ready = isfast && (voltage>=VOLTAGE_HAS_SIGNAL);
  wire_helper->read_mode();
  
  if (!is_ready && voltage>=VOLTAGE_HAS_SIGNAL) {
    PowerLedSet(POWER_LED_FLASH_FAST);
    is_ready = wire_helper->waitStartSignal();
    if (is_ready) {
      fastBeep(TONE_LEN_SHORT, TONE_FREQ_MESSAGE);
    }
  }

  if (is_ready) {
    current_fan_speed = FAN_MAX_SPEED;
    analogWrite(FAN_IN_PIN, current_fan_speed);
    analogWrite(FAN_OUT_PIN, current_fan_speed);
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
      reset_chargevars();
      fastBeep(TONE_LEN_SHORT, TONE_FREQ_MESSAGE);
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
    if (charging && !constant_charging && (cmillis-charging_start_time) > BASE_CHARGING_TIME) {
      reset_chargevars();
      fastBeep(TONE_LEN_SHORT, TONE_FREQ_MESSAGE);
    }
  }
}

void setup() {
  TCCR1B = (TCCR1B & 0b11111000) | 0x01;
  pinMode(LED_POWER_ALERT_PIN, OUTPUT);
  pinMode(FAN_IN_PIN, OUTPUT);
  pinMode(FAN_OUT_PIN, OUTPUT);
  pinMode(TONE_PIN, OUTPUT);
  pinMode(POWER_PIN, OUTPUT);
  pinMode(CHARGE_PIN, OUTPUT);
  pinMode(BTN_POWER_PIN, INPUT);
  pinMode(BTN_CHARGE_PIN, INPUT);
  pinMode(DHTPIN, INPUT);

  reset_onoffvars();
  reset_chargevars();

  wire_helper = new OneWireHelper(POWER_SIGNAL_MAIN_PIN, onTimerCheck);

  attachInterrupt(BTN_POWER_INT, PowerBTN_Change, CHANGE);
  attachInterrupt(BTN_CHARGE_INT, ChargeBTN_Change, CHANGE);
  dht.begin();
  irrecv.enableIRIn();
  fastBeep(TONE_LEN_SHORT, TONE_FREQ_MESSAGE);
}

void loop() {
  int onewire_byte = wire_helper->tryReadByteCommand();
  if (onewire_byte != ONEWIRE_CODE_NOOP) {
    if (onewire_byte == ONEWIRE_CODE_OFF && !turning_off) {
      turning_off = true;
      power_btn = false;
      power_btn_press_millis = power_press_duration = 0;
    } else if (onewire_byte == ONEWIRE_CODE_SILENT_MODE_ON) {
      silence_mode = true;
      silence_ismid = false;
      last_fan_update_time = 0;
    } else if (onewire_byte == ONEWIRE_CODE_SILENT_MODE_MID) {
      silence_mode = true;
      silence_ismid = true;
      last_fan_update_time = 0;
    } else if (onewire_byte == ONEWIRE_CODE_SILENT_MODE_OFF) {
      silence_mode = false;
      silence_ismid = false;
      last_fan_update_time = 0;
    }
  }
  if (irrecv.decode(&ir_results)) {
    if (ir_results.decode_type == NEC) {
      if (ir_results.value == CM_TOGGLEONOFF) {
        power_btn = true;
        power_press_duration = BTN_SHORTEST_PRESS;
        #ifdef CSTATION_DEBUG_MODE
            fastBeep(TONE_LEN_SHORT, 100);
        #endif
      } else if (ir_results.value == CM_TOGGLECHARGE) {
        charge_btn = true;
        charge_press_duration = BTN_SHORTEST_PRESS;
        #ifdef CSTATION_DEBUG_MODE
            fastBeep(TONE_LEN_SHORT, 150);
        #endif
      } 
      #ifdef CSTATION_DEBUG_MODE
      else if (ir_results.value == CM_TOGGLESILENCE) {
        if (silence_mode && !silence_ismid) {
          silence_ismid = true;
        } else {
          silence_mode = !silence_mode;
          silence_ismid = false;
        }
        last_fan_update_time = 0;
      } else if (ir_results.value == CM_TOGGLESPEEDTEST) {
        current_fan_speed = 255;
        fastBeep(TONE_LEN_SHORT, current_fan_speed * 2);
        analogWrite(FAN_IN_PIN, current_fan_speed);
        analogWrite(FAN_OUT_PIN, current_fan_speed);
        delay(10000);
        current_fan_speed = 240;
        fastBeep(TONE_LEN_SHORT, current_fan_speed * 2);
        analogWrite(FAN_IN_PIN, current_fan_speed);
        analogWrite(FAN_OUT_PIN, current_fan_speed);
        delay(10000);
        current_fan_speed = 210;
        fastBeep(TONE_LEN_SHORT, current_fan_speed * 2);
        analogWrite(FAN_IN_PIN, current_fan_speed);
        analogWrite(FAN_OUT_PIN, current_fan_speed);
        delay(10000);
        current_fan_speed = 180;
        fastBeep(TONE_LEN_SHORT, current_fan_speed * 2);
        analogWrite(FAN_IN_PIN, current_fan_speed);
        analogWrite(FAN_OUT_PIN, current_fan_speed);
        delay(10000);
        current_fan_speed = 150;
        fastBeep(TONE_LEN_SHORT, current_fan_speed * 2);
        analogWrite(FAN_IN_PIN, current_fan_speed);
        analogWrite(FAN_OUT_PIN, current_fan_speed);
        delay(10000);
        current_fan_speed = 128;
        fastBeep(TONE_LEN_SHORT, current_fan_speed * 2);
        analogWrite(FAN_IN_PIN, current_fan_speed);
        analogWrite(FAN_OUT_PIN, current_fan_speed);
        delay(10000);
        current_fan_speed = 100;
        fastBeep(TONE_LEN_SHORT, current_fan_speed * 2);
        analogWrite(FAN_IN_PIN, current_fan_speed);
        analogWrite(FAN_OUT_PIN, current_fan_speed);
        delay(10000);
        current_fan_speed = 75;
        fastBeep(TONE_LEN_SHORT, current_fan_speed * 2);
        analogWrite(FAN_IN_PIN, current_fan_speed);
        analogWrite(FAN_OUT_PIN, current_fan_speed);
        delay(10000);
        current_fan_speed = 50;
        fastBeep(TONE_LEN_SHORT, current_fan_speed * 2);
        analogWrite(FAN_IN_PIN, current_fan_speed);
        analogWrite(FAN_OUT_PIN, current_fan_speed);
        delay(10000);
      }
      #endif
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
        if (power_led_curr_mode == POWER_LED_ON) {
          PowerLedSet(POWER_LED_OFF);
        } else {
          timer_interval = 0;
        }
      }
    }
    if (power_led_curr_mode != POWER_LED_ON) {
      power_led_state = !power_led_state;
      digitalWrite(LED_POWER_ALERT_PIN, power_led_state ? HIGH : LOW);
    }
    last_timer_time = millis();
  }
}

