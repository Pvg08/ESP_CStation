
#include <math.h>
#include <Servo.h>
#include <IRremote.h>
#include <LedControl.h>
#include <TM1637Display.h>
#include <Time.h>
#include <DS1302RTC.h>

// Clock Display pins
#define TM_CLK 22
#define TM_DIO 24

// RTC pins
#define DS1302_CLK_PIN 34
#define DS1302_DAT_PIN 32
#define DS1302_RST_PIN 30
#define DS1302_GND_PIN 26
#define DS1302_VCC_PIN 28

// Laser & Servo pins
#define LASER_GND_PIN 46
#define LASER_VCC_PIN 48
#define SERVO1_PIN 50
#define SERVO2_PIN 52

// LED Strip pins
#define RECV_PIN 13
#define PIN_W 5
#define PIN_B 4
#define PIN_G 3
#define PIN_R 2
// Yellow Orange Green Blue White

// Servo restrictions
#define SERVO1_MIN_ANGLE 45
#define SERVO1_MAX_ANGLE 135
#define SERVO2_MIN_ANGLE 15
#define SERVO2_MAX_ANGLE 100
#define SERVO1_FACE_DIR_ANGLE 100
#define SERVO2_FACE_DIR_ANGLE 65

// Button codes
#define CM_ON 0xFFB04F
#define CM_OFF 0xFFF807
#define CM_B_DOWN 0xFFB847
#define CM_B_DOWN2 0xA23C94BF
#define CM_B_UP 0xFF906F
#define CM_B_UP2 0xE5CFBD7F
#define CM_R 0xFF9867
#define CM_G 0xFFD827
#define CM_B 0xFF8877
#define CM_W 0xFFA857
#define CM_COL11 0xFFE817
#define CM_COL21 0xFF02FD
#define CM_COL31 0xFF50AF
#define CM_COL41 0xFF38C7
#define CM_COL12 0xFF48B7
#define CM_COL22 0xFF32CD
#define CM_COL32 0xFF7887
#define CM_COL42 0xFF28D7
#define CM_COL13 0xFF6897
#define CM_COL23 0xFF20DF
#define CM_COL33 0xFF708F
#define CM_COL43 0xFFF00F
#define CM_MODE_FLASH 0xFFB24D
#define CM_MODE_STROBE 0xFF00FF
#define CM_MODE_FADE 0xFF58A7
#define CM_MODE_SMOOTH 0xFF30CF
#define CM_REPEAT 0xFFFFFFFF

#define DEFAULT_MAX_COUNTER 255

typedef enum {MD_NONE, MD_FLASH, MD_STROBE, MD_FADE, MD_SMOOTH} led_mode;
led_mode mode = MD_NONE;
byte mode_speed = 1;
byte mode_step = 0;
float mode_k = 0;
unsigned long int mode_max_counter;
unsigned long int mode_counter;

byte cr = 0, cg = 0, cb = 0, cw = 255;
bool rbright = false, gbright = false, bbright = false, wbright = false;
int nr, ng, nb, nw;
byte onstate = 0;

IRrecv irrecv(RECV_PIN);
decode_results ir_results;
LedControl lc=LedControl(12, 11, 10, 5);
TM1637Display display(TM_CLK, TM_DIO);
DS1302RTC RTC(DS1302_RST_PIN, DS1302_DAT_PIN, DS1302_CLK_PIN);

bool show_dots = false;
time_t last_rtc = 0;

void showTime() {
  time_t c_rtc = RTC.get();
  if (c_rtc == last_rtc) return;
  last_rtc = c_rtc;
  tmElements_t tm;
  if (! RTC.read(tm)) {
    unsigned int cont = tm.Hour*100 + tm.Minute;
    if (show_dots) {
      uint8_t segto;
      segto = 0x80 | display.encodeDigit((cont / 100)%10);
      display.setSegments(&segto, 1, 1);
    } else {
      display.showNumberDec(cont, true);
    }
    show_dots = !show_dots;
  }
}

void writeColor(byte rr, byte gg, byte bb, byte ww) {
  //Serial.println(ww);
  analogWrite(PIN_R, rr*onstate);
  analogWrite(PIN_G, gg*onstate);
  analogWrite(PIN_B, bb*onstate);
  analogWrite(PIN_W, ww*onstate);
  return;
}

void writeNColor(byte rr, byte gg, byte bb, byte ww) {
  cr = rr;
  cg = gg;
  cb = bb;
  cw = ww;
  writeColor(cr, cg, cb, cw);
  return;
}

void turnOff() {
  onstate = 0;
  writeColor(cr, cg, cb, cw);
  return;
}

void turnOn() {
  onstate = 1;
  writeColor(cr, cg, cb, cw);
  return;
}

Servo myservo1;
Servo myservo2;
void servoMovement(int angle1, int angle2) {
  myservo1.attach(SERVO1_PIN);
  myservo2.attach(SERVO2_PIN);
  if (angle1<SERVO1_MIN_ANGLE) angle1 = SERVO1_MIN_ANGLE;
  if (angle1>SERVO1_MAX_ANGLE) angle1 = SERVO1_MAX_ANGLE;
  if (angle2<SERVO2_MIN_ANGLE) angle2 = SERVO2_MIN_ANGLE;
  if (angle2>SERVO2_MAX_ANGLE) angle2 = SERVO2_MAX_ANGLE;
  myservo1.write(angle1);
  myservo2.write(angle2);
  delay(20);
  myservo1.detach();
  myservo2.detach();
}

void setLaser(bool state) {
  digitalWrite(LASER_VCC_PIN, state ? HIGH : LOW);
}

void setup() {
  //Serial.begin(9600);
  
  int myEraser = 7;
  int myPrescaler = 1;
  
  TCCR3B &= ~myEraser;
  TCCR3B |= myPrescaler;
  TCCR0B &= ~myEraser;
  TCCR0B |= myPrescaler;
  
  irrecv.enableIRIn();
  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  pinMode(PIN_W, OUTPUT);
  turnOff();

  lc.shutdown(0,false);
  lc.shutdown(1,false);
  lc.shutdown(2,false);
  lc.shutdown(3,false);
  lc.shutdown(4,false);
  lc.shutdown(5,false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0,8);
  lc.setIntensity(1,1);
  lc.setIntensity(2,4);
  lc.setIntensity(3,1);
  lc.setIntensity(4,2);
  lc.setIntensity(5,4);
  /* and clear the display */
  lc.clearDisplay(0);
  lc.clearDisplay(1);
  lc.clearDisplay(2);
  lc.clearDisplay(3);
  lc.clearDisplay(4);
  lc.clearDisplay(5);

  display.setBrightness(0x0f);

  // Init laser
  pinMode(LASER_GND_PIN, OUTPUT);
  pinMode(LASER_VCC_PIN, OUTPUT);
  digitalWrite(LASER_GND_PIN, LOW);
  digitalWrite(LASER_VCC_PIN, LOW);

  // Activate RTC module
  pinMode(DS1302_GND_PIN, OUTPUT);
  pinMode(DS1302_VCC_PIN, OUTPUT);
  digitalWrite(DS1302_GND_PIN, LOW);
  digitalWrite(DS1302_VCC_PIN, HIGH);

  if (RTC.haltRTC()) {
    //Serial.println("The DS1302 is stopped.  Please run the SetTime");
    //Serial.println("example to initialize the time and begin running.");
    //Serial.println();
  }
  if (!RTC.writeEN()) {
    //Serial.println("The DS1302 is write protected. This normal.");
    //Serial.println();
  }
}

unsigned long int current_com = 0;
unsigned long int last_com = 0;
unsigned long int active_com = 0;

void loop() {
  if (irrecv.decode(&ir_results)) {
    if (ir_results.decode_type == NEC) {
      current_com = ir_results.value;
      //Serial.println(ccom, HEX);
      
      if (current_com == CM_REPEAT) {
        active_com = last_com;
      } else {
        active_com = current_com;
      }

      switch(active_com) {
        case CM_ON:
        case CM_OFF:
           rbright = gbright = bbright = wbright = false;
        case CM_B_DOWN:
        case CM_B_UP:
          break;
        case CM_MODE_FLASH:
        case CM_MODE_STROBE:
        case CM_MODE_FADE:
        case CM_MODE_SMOOTH:
           rbright = gbright = bbright = wbright = false;
          if (last_com == active_com) {
            mode_speed = (mode_speed + 3) % 17 + 1;
          } else {
            mode_speed = 8;
            mode_counter = 0;
            mode_step = 0;
            mode_max_counter = DEFAULT_MAX_COUNTER;
            switch(active_com) {
              case CM_MODE_FLASH:  
                mode = MD_FLASH; 
                break;
              case CM_MODE_STROBE: 
                mode = MD_STROBE; 
                if (!cr && !cg && !cb && !cw) cw=255;
                break;
              case CM_MODE_FADE:   
                mode = MD_FADE; 
                if (!cr && !cg && !cb && !cw) cw=255;
                break;
              case CM_MODE_SMOOTH: 
                mode = MD_SMOOTH; 
                break;
            }
          }
          break;
         default:
           mode = MD_NONE;
      }

      switch(active_com) {
        case CM_ON:
          turnOn();
          break;
        case CM_OFF:
          turnOff();
          break;
        case CM_B_UP:
          if (!gbright && !bbright && !wbright) {
            if (cr<45) nr = cr + 5; else nr = round(cr * 1.1);
            if (nr>255) nr = 255;
          } else {
            nr = cr; 
          }
          if (!rbright && !bbright && !wbright) {
            if (cg<45) ng = cg + 5; else ng = round(cg * 1.1);
            if (ng>255) ng = 255;
          } else {
            ng = cg; 
          }
          if (!rbright && !gbright && !wbright) {
            if (cb<45) nb = cb + 5; else nb = round(cb * 1.1);
            if (nb>255) nb = 255;
          } else {
            nb = cb; 
          }
          if (!rbright && !gbright && !bbright) {
            if (cw<45) nw = cw + 5; else nw = round(cw * 1.1);
            if (nw>255) nw = 255;
          } else {
            nw = cw; 
          }
          writeNColor(nr, ng, nb, nw);
          break;
        case CM_B_DOWN:
          if (!gbright && !bbright && !wbright) {
            if (cr<=46 && cr>=5) nr = cr - 5; else nr = round(cr * 0.9); 
          } else { 
            nr = cr;
          }
          if (!rbright && !bbright && !wbright) {
            if (cg<=46 && cg>=5) ng = cg - 5; else ng = round(cg * 0.9);
          } else {
            ng = cg;
          }
          if (!rbright && !gbright && !wbright) {
            if (cb<=46 && cb>=5) nb = cb - 5; else nb = round(cb * 0.9);
          } else {
            nb = cb;
          }
          if (!rbright && !gbright && !bbright) {
            if (cw<=46 && cr>=5) nw = cw - 5; else nw = round(cw * 0.9);
          } else {
            nw = cw;
          }
          writeNColor(nr, ng, nb, nw);
          break;
        case CM_R:
          rbright = true;
          gbright = bbright = wbright = false;
          if (active_com == last_com) {
            writeNColor(255, 0, 0, 0);
            if (current_com!=active_com) rbright = false;
          } else {
            writeColor(0, cg, cb, cw);
            delay(10000);
            writeColor(255, cg, cb, cw);
            delay(10000);
            writeColor(0, cg, cb, cw);
            delay(10000);
            writeColor(255, cg, cb, cw);
            delay(10000);
            writeColor(cr, cg, cb, cw);
          }
          break;
        case CM_G:
          gbright = true;
          rbright = bbright = wbright = false;
          if (active_com == last_com) {
            writeNColor(0, 255, 0, 0);
            if (current_com!=active_com) gbright = false;
          } else {
            writeColor(cr, 0, cb, cw);
            delay(10000);
            writeColor(cr, 255, cb, cw);
            delay(10000);
            writeColor(cr, 0, cb, cw);
            delay(10000);
            writeColor(cr, 255, cb, cw);
            delay(10000);
            writeColor(cr, cg, cb, cw);
          }
          break;
        case CM_B:
          bbright = true;
          rbright = gbright = wbright = false;
          if (active_com == last_com) {
            writeNColor(0, 0, 255, 0);
            if (current_com!=active_com) bbright = false;
          } else {
            writeColor(cr, cg, 0, cw);
            delay(10000);
            writeColor(cr, cg, 255, cw);
            delay(10000);
            writeColor(cr, cg, 0, cw);
            delay(10000);
            writeColor(cr, cg, 255, cw);
            delay(10000);
            writeColor(cr, cg, cb, cw);
          }
          break;
        case CM_W:
          wbright = true;
          rbright = gbright = bbright = false;
          if (active_com == last_com) {
            writeNColor(0, 0, 0, 255);
            if (current_com!=active_com) wbright = false;
          } else {
            writeColor(cr, cg, cb, 0);
            delay(10000);
            writeColor(cr, cg, cb, 255);
            delay(10000);
            writeColor(cr, cg, cb, 0);
            delay(10000);
            writeColor(cr, cg, cb, 255);
            delay(10000);
            writeColor(cr, cg, cb, cw);
          }
          break;
        case CM_COL11:
          writeNColor(235, 100, 0, 0);
          break;
        case CM_COL21:
          writeNColor(215, 64, 0, 0);
          break;
        case CM_COL31:
          writeNColor(255, 128, 0, 0);
          break;
        case CM_COL41:
          writeNColor(255, 215, 0, 0);
          break;
        case CM_COL12:
          writeNColor(145, 255, 100, 0);
          break;
        case CM_COL22:
          writeNColor(20, 255, 255, 0);
          break;
        case CM_COL32:
          writeNColor(5, 128, 192, 0);
          break;
        case CM_COL42:
          writeNColor(0, 72, 106, 0);
          break;
        case CM_COL13:
          writeNColor(0, 80, 255, 0);
          break;
        case CM_COL23:
          writeNColor(64, 0, 128, 0);
          break;
        case CM_COL33:
          writeNColor(128, 0, 128, 0);
          break;
        case CM_COL43:
          writeNColor(255, 0, 255, 0);
          break;
      }
      
      if (current_com != CM_REPEAT) {
        last_com = current_com;
      }
    }
    irrecv.resume();
  }

  if (mode!=MD_NONE && onstate) {
    switch(mode) {
      case MD_FLASH:
        mode_counter++;
        if (mode_counter >= mode_max_counter*mode_speed) {
          mode_counter = 0;
          mode_step = (mode_step + 1) % 8;
          switch (mode_step) {
            case 0: writeColor(255, 0, 0, 0); break;
            case 1: writeColor(0, 255, 0, 0); break;
            case 2: writeColor(0, 0, 255, 0); break;
            case 3: writeColor(0, 0, 0, 255); break;
            case 4: writeColor(255, 255, 0, 0); break;
            case 5: writeColor(0, 255, 255, 0); break;
            case 6: writeColor(255, 0, 255, 0); break;
            case 7: writeColor(255, 255, 255, 0); break;
          }
        }
        delay(10);
      break;
      case MD_STROBE:
        mode_counter++;
        if (mode_counter >= mode_max_counter*mode_speed) {
          mode_counter = 0;
          mode_step = (mode_step + 1) % 2;
          switch (mode_step) {
            case 0: writeColor(cr, cg, cb, cw); break;
            case 1: writeColor(0, 0, 0, 0); break;
          }
        }
        delay(10);
      break;
      case MD_FADE:
        mode_counter++;
        mode_k = (float) mode_counter / (mode_max_counter*mode_speed);
        if (!mode_step) {
          mode_k = 1-mode_k;
        }
        nr = round(cr * mode_k); if(nr>255) nr=255;
        ng = round(cg * mode_k); if(ng>255) ng=255;
        nb = round(cb * mode_k); if(nb>255) nb=255;
        nw = round(cw * mode_k); if(nw>255) nw=255;
        writeColor(nr, ng, nb, nw);
        if (mode_counter >= mode_max_counter*mode_speed) {
          mode_counter = 0;
          mode_step = (mode_step + 1) % 2;
          delay(20000);
        }
        delay(10);
      break;
      case MD_SMOOTH:
        mode_counter++;
        if (mode_counter >= mode_max_counter*mode_speed) {
          mode_counter = 0;
          mode_step = (mode_step + 1) % 4;
          switch (mode_step) {
            case 0: writeColor(255, 0, 0, 0); break;
            case 1: writeColor(0, 255, 0, 0); break;
            case 2: writeColor(0, 0, 255, 0); break;
            case 3: writeColor(0, 0, 0, 255); break;
          }
        }
        delay(10);
      break;
    }
  }

  showTime();
}
