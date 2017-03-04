#ifndef ONEWIRE_HELPER_H
#define ONEWIRE_HELPER_H

#define ONE_BIT_DELAY_MS 50
#define DEF_TIMER_DELAY 90

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
      read_mode();
    }

    void read_mode() 
    {
      pinMode(rpin, INPUT);
    }
    void write_mode() 
    {
      pinMode(rpin, OUTPUT);
    }

    bool waitStartSignal()
    {
      bool is_ready = false;
      bool external_signal = waitForSignal(HIGH, 5000, 50);
      if (external_signal) {
        sleepDelay(500, 50);
        if (digitalRead(rpin)==LOW) {
          is_ready = true;
        }
      }
      return is_ready;
    }

    bool sendOffPing()
    {
      write_mode();
      digitalWrite(rpin, HIGH);
      sleepDelay(1500, 50);
      digitalWrite(rpin, LOW);
      sleepDelay(100, 50);
      read_mode();
      return waitForSignal(HIGH, 20000, 50);
    }

    void sleepDelay(unsigned int msdelay, unsigned int rdelay)
    {
      for(unsigned int i=0; i<=msdelay; i+=rdelay) {
        if (timerCallback && ((rdelay>1) || (i % DEF_TIMER_DELAY == 0))) timerCallback();
        delay(rdelay);
      }
    }

    bool waitForSignal(byte state, unsigned int maxmsdelay, unsigned int rdelay)
    {
      bool external_signal = digitalRead(rpin)==state;
      for(unsigned int i=0; (i<=maxmsdelay) && !external_signal; i+=rdelay) {
        delay(rdelay);
        if (timerCallback && ((rdelay>1) || (i % DEF_TIMER_DELAY == 0))) timerCallback();
        external_signal = digitalRead(rpin)==state;
        i+=rdelay;
      }
      return external_signal;
    }

    bool tryReadOffSignal()
    {
      if (digitalRead(rpin)==HIGH) {
        delay(DEF_TIMER_DELAY);
        if (digitalRead(rpin)==HIGH) {
          return true;
        }
      }
      return false;
    }

    byte tryReadByteCommand()
    {
      byte result = 0;
      if (tryReadOffSignal()) {
        bool external_signal;
        external_signal = waitForSignal(LOW, 10000, 1);
        if (external_signal) {
          for(byte i = 0; i < 8; i++) {
            delay(ONE_BIT_DELAY_MS);
            result = (result << 1) | ((digitalRead(rpin)==HIGH) & 0b00000001);
          }
        }
      }
      return result;
    }

    void writeSignal(byte wsignal, unsigned wdelay)
    {
      write_mode();
      digitalWrite(rpin, wsignal);
      delay(wdelay);
      read_mode();
    }

    void writeByteCommand(byte wbyte)
    {
      write_mode();
      digitalWrite(rpin, HIGH);
      delay(2000);
      digitalWrite(rpin, LOW);
      for(byte i = 0; i < 8; i++) {
        delay(ONE_BIT_DELAY_MS);
        digitalWrite(rpin, (wbyte & 0b10000000) ? HIGH : LOW);
        wbyte = wbyte << 1;
      }
      delay(ONE_BIT_DELAY_MS);
      digitalWrite(rpin, LOW);
      read_mode();
    }
};

#endif
