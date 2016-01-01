#include "DHT.h"

/* fan-control */
#define TEMP_LOWLEVEL 18
#define TEMP_HIGHLEVEL 30
#define TEMP_CRITICALLEVEL 60
#define HUMIDITY_BASELEVEL 50
#define FAN_MIN_SPEED 150
#define FAN_MAX_SPEED 255
/* /fan-control */

/* pins */
#define VOLTAGE_ANALOG_PIN 3
#define VOLTAGE_R1 59850.0
#define VOLTAGE_R2 10000.0
#define BASE_VOLTAGE 5.0
#define VOLTAGE_K (BASE_VOLTAGE * (VOLTAGE_R1+VOLTAGE_R2) / (1024.0 * VOLTAGE_R2))

#define FAN_IN_PIN 10
#define FAN_OUT_PIN 11
#define TONE_PIN 5

#define POWER_PIN 4
#define CHARGE_PIN 8
#define POWER_SIGNAL_MAIN_PIN 9

#define BASE_CHARGING_TIME 14400000

#define BTN_SHORTEST_PRESS 20
#define BTN_LONG_PRESS 2000
#define BTN_POWER_PIN 2
#define BTN_POWER_INT 0
#define BTN_CHARGE_PIN 3
#define BTN_CHARGE_INT 1

#define LED_POWER_ALERT_PIN 7

#define DHTPIN 6
/* /pins */

#define VOLTAGE_HAS_SIGNAL 3.0
#define VOLTAGE_LOWLEVEL 11.0
#define VOLTAGE_HIGHLEVEL 12.4
#define VOLTAGE_CRITICALLEVEL 13

/* dht-sensor */
#define DHT_READ_MS 20000
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
/* /dht-sensor */

unsigned long int last_dht_read;
int current_fan_speed;
float voltage;
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
volatile bool charge_btn;
volatile unsigned long int charge_btn_press_millis, charge_press_duration;

void updateVoltage() {
  int value = analogRead(VOLTAGE_ANALOG_PIN);
  voltage = value * VOLTAGE_K;
  if (voltage<0.09) {
    voltage=0.0;
  }
}

void updateFanSpeeds() {
  int calc_speed = 0;

  if (turn_on_time && millis()-turn_on_time<30000) calc_speed+=(30000-millis()+turn_on_time)/200;
  
  if (voltage>VOLTAGE_HIGHLEVEL) calc_speed+=(int) (voltage-VOLTAGE_HIGHLEVEL)*80;
  if (voltage>VOLTAGE_CRITICALLEVEL) calc_speed+=255;

  if (!isnan(humidity)) {
    calc_speed+=(int) (humidity-HUMIDITY_BASELEVEL);
  }
  if (!isnan(temperature)) {
    if (temperature < TEMP_LOWLEVEL) calc_speed+=10;
    else if (temperature > TEMP_HIGHLEVEL) calc_speed+=(int) (temperature-TEMP_HIGHLEVEL)*5;
  }

  if (calc_speed>0) {
    calc_speed+=FAN_MIN_SPEED;
    if (calc_speed < FAN_MIN_SPEED) calc_speed = FAN_MIN_SPEED;
  } else if (calc_speed<0) {
    calc_speed = 0;
  }

  if (calc_speed > FAN_MAX_SPEED) calc_speed = FAN_MAX_SPEED;

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

void fastBeep(unsigned msec) {
  digitalWrite(TONE_PIN, HIGH);
  delay(msec);
  digitalWrite(TONE_PIN, LOW);
}

void fastOff()
{
  turned_on = turning_off = false;
  turning_off_start_time = 0;
  digitalWrite(POWER_PIN, LOW);
  power_btn = false;
  power_btn_press_millis = power_press_duration = turn_on_time = 0;
}

void turnOn(bool isfast)
{
  turned_on = true;
  digitalWrite(POWER_PIN, HIGH);
  digitalWrite(LED_POWER_ALERT_PIN, HIGH);

  byte i = 0;
  voltage = 0;
  while (i<=10 && voltage<VOLTAGE_HAS_SIGNAL) {
    fastBeep(500);
    delay(500);
    updateVoltage();
    i++;
  }

  bool is_ready = isfast && voltage>=VOLTAGE_HAS_SIGNAL;
  pinMode(POWER_SIGNAL_MAIN_PIN, INPUT);
  
  if (!is_ready && voltage>=VOLTAGE_HAS_SIGNAL) {
    unsigned long int cmillis = millis();
    bool external_signal = digitalRead(POWER_SIGNAL_MAIN_PIN)==HIGH;
    i = 0;
    while (i<=100 && !external_signal) {
      delay(50);
      external_signal = digitalRead(POWER_SIGNAL_MAIN_PIN)==HIGH;
      i++;
    }
    if (external_signal) {
      delay(500);
      if (digitalRead(POWER_SIGNAL_MAIN_PIN)==LOW) {
        fastBeep(50);
        is_ready = true;
      }
    }
  }

  if (!is_ready) {
    fastOff();
    fastBeep(1000);
  }
  power_btn = false;
  power_btn_press_millis = power_press_duration = 0;
  turn_on_time = is_ready ? millis() : 0;
  digitalWrite(LED_POWER_ALERT_PIN, LOW);
}

void updateOnOffState()
{
  if (turning_off) {
    digitalWrite(LED_POWER_ALERT_PIN, HIGH);
    pinMode(POWER_SIGNAL_MAIN_PIN, OUTPUT);
    digitalWrite(POWER_SIGNAL_MAIN_PIN, HIGH);
    delay(1000);
    digitalWrite(POWER_SIGNAL_MAIN_PIN, LOW);
    pinMode(POWER_SIGNAL_MAIN_PIN, INPUT);
    
    delay(100);
    bool external_signal = digitalRead(POWER_SIGNAL_MAIN_PIN)==HIGH;
    byte i = 0;
    while (i<=100 && !external_signal) {
      delay(100);
      external_signal = digitalRead(POWER_SIGNAL_MAIN_PIN)==HIGH;
      i++;
    }

    fastOff();

    digitalWrite(LED_POWER_ALERT_PIN, LOW);
    return;
  }

  if (power_btn) {
    bool long_press = power_press_duration>=BTN_LONG_PRESS;

    if (turned_on) {
      if (long_press) {
        fastOff();
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
      digitalWrite(CHARGE_PIN, LOW);
    } else {
      if (long_press) {
        constant_charging = true;
        charging_start_time = 0;
      } else {
        constant_charging = false;
        charging_start_time = cmillis;
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
  charging_start_time = 0;
  turning_off_start_time = 0;

  pinMode(LED_POWER_ALERT_PIN, OUTPUT);
  pinMode(FAN_IN_PIN, OUTPUT);
  pinMode(FAN_OUT_PIN, OUTPUT);
  pinMode(TONE_PIN, OUTPUT);
  pinMode(POWER_PIN, OUTPUT);
  pinMode(CHARGE_PIN, OUTPUT);
  pinMode(BTN_POWER_PIN, INPUT);
  pinMode(BTN_CHARGE_PIN, INPUT);
  pinMode(DHTPIN, INPUT);
  pinMode(POWER_SIGNAL_MAIN_PIN, INPUT);

  TCCR1B = TCCR1B & 0b11111000 | 0x01;
  TCCR2B = TCCR2B & 0b11111000 | 0x01;

  attachInterrupt(BTN_POWER_INT, PowerBTN_Change, CHANGE);
  attachInterrupt(BTN_CHARGE_INT, ChargeBTN_Change, CHANGE);
  dht.begin();
  fastBeep(2000);
}

void loop() {
  updateVoltage();

  if (voltage>VOLTAGE_HAS_SIGNAL) {
    if (!last_dht_read || millis() - last_dht_read > DHT_READ_MS) {
      last_dht_read = millis();
      float h = dht.readHumidity();
      float t = dht.readTemperature();
      if (!isnan(h)) humidity = h;
      if (!isnan(t)) temperature = t;

      if (!isnan(h) || !isnan(t)) {
        digitalWrite(13, HIGH);
        delay(100);
        digitalWrite(13, LOW);
      }
    }

    updateFanSpeeds();
  }

  updateOnOffState();
  updateChargeState();

  delay(250);
}

