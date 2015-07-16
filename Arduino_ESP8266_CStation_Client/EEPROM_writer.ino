
#include <EEPROM.h>

void readStringFromEEPROM(int addr, char* string, int string_maxlen) 
{
  int i;
  string_maxlen--;
  for(i = 0; i<string_maxlen; i++) {
    string[i] = EEPROM.read(addr+i);
  }
  string[i] = 0;
}

void writeStringToEEPROM(int addr, char* string, int string_maxlen) 
{
  int i;
  string_maxlen--;
  for(i = 0; i<string_maxlen && string[i]; i++) {
    EEPROM.write(addr+i,string[i]);
  }
  EEPROM.write(addr+i,0);
}

