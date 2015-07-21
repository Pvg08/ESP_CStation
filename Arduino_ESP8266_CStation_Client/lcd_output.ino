#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

bool fixed_text = false;
char line1_fixed[17];
char line2_fixed[17];
char line1_dyn[17];
char line2_dyn[17];

void initLCD() 
{
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.home();
  if (fixed_text) {
    lcd.print(line1_fixed);
    lcd.setCursor(0, 1);
    lcd.print(line2_fixed);
  }
}

void setLCDFixed(const char* text) 
{
  strncpy(line1_fixed, text, 17);
  if (strlen(text)>16) {
    strncpy(line2_fixed, text+16, 17);
  } else {
    line2_fixed[0] = 0;
  }
  fixed_text = true;
  lcd.home(); 
  lcd.clear();
  lcd.print(line1_fixed);
  lcd.setCursor(0, 1);
  lcd.print(line2_fixed);
}

void resetLCDFixed()
{
  fixed_text = false;
  lcd.home(); 
  lcd.clear(); 
  lcd.print(line1_dyn);
  lcd.setCursor(0, 1);
  lcd.print(line2_dyn);
}

void setLCDText(const char* text) 
{
  if (fixed_text) return;
  lcd.home(); 
  lcd.clear(); 
  lcd.print(text);
  strncpy(line1_dyn, text, 17);
  line2_dyn[0] = 0;
}

void setLCDLines(const char* line1, const char* line2) 
{
  if (fixed_text) return;
  lcd.home(); 
  lcd.clear(); 
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
  strncpy(line1_dyn, line1, 17);
  strncpy(line2_dyn, line2, 17);
}
