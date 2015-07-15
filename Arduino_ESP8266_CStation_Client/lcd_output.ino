#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

void initLCD() {
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.home();
}

void setLCDText(const char* text) {
  lcd.home(); 
  lcd.clear(); 
  lcd.print(text);
}

void setLCDLines(const char* line1, const char* line2) {
  lcd.home(); 
  lcd.clear(); 
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}
