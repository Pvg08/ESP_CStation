#ifndef ONEWIRE_HELPER_H
#define ONEWIRE_HELPER_H

class OneWireHelper 
{
  private:
    unsigned char rpin;
    void (*timerCallback)();
    
  public:

    OneWireHelper(unsigned char pin, void (*callback)()) 
    {
      rpin = pin;
      timerCallback = callback;
      reset();
    }

    void reset() 
    {
      pinMode(rpin, INPUT);
    }

    bool waitStartSignal()
    {
      unsigned long int cmillis = millis();
      bool external_signal = digitalRead(rpin)==HIGH;
      bool is_ready = false;
      byte i = 0;
      while (i<=100 && !external_signal) {
        delay(50);
        if (timerCallback) timerCallback();
        external_signal = digitalRead(rpin)==HIGH;
        i++;
      }
      if (external_signal) {
        delay(500);
        if (digitalRead(rpin)==LOW) {
          is_ready = true;
        }
      }
      return is_ready;
    }

    bool sendOffPing()
    {
      bool external_signal = false;
      pinMode(rpin, OUTPUT);
      digitalWrite(rpin, HIGH);
      for(byte i=0; i<15; i++) {
        if (timerCallback) timerCallback();
        delay(100);
      }
      digitalWrite(rpin, LOW);
      pinMode(rpin, INPUT);
      
      delay(100);
      external_signal = digitalRead(rpin)==HIGH;
      for(byte i=0; i<200 && !external_signal; i++) {
        if (timerCallback) timerCallback();
        delay(100);
        external_signal = digitalRead(rpin)==HIGH;
      }
      return external_signal;
    }

    int readByte()
    {
      // @todo
      return -1;
    }
};

#endif
