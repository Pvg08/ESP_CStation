#include <Wire.h>
#include <SFE_BMP180.h>
#include <DHT.h>
#include <BH1750.h>

#define MAX_ERRORS 4

#define SENDING_INTERVAL 30000
#define ERROR_CHECK_INTERVAL 120000

#define DHTPIN 5
#define DHTTYPE DHT11

#define HC_PIN 18
#define HC_INTERRUPT 5
#define HC_INTERRUPT_MODE CHANGE
#define HC_SIGNAL_PIN 49

#define NS_PIN 19
#define NS_INTERRUPT 4
#define NS_INTERRUPT_MODE RISING

#define ACTION_SIGNAL_PIN 48

unsigned long int last_sending_millis, last_reset_millis;

SFE_BMP180 pressure;
DHT dht(DHTPIN, DHTTYPE);
BH1750 lightMeter;

bool setH = false;
float oldH;
volatile bool hc_info_sended = false;
volatile bool hc_state = false;
volatile bool ns_info_sended = false;
volatile bool ns_state = false;

void HC_State_Changed() 
{
  hc_state = digitalRead(HC_PIN) == HIGH;
  if (hc_state) {
    hc_info_sended = false;
    digitalWrite(ACTION_SIGNAL_PIN, HIGH);
  }
  digitalWrite(HC_SIGNAL_PIN, hc_state ? HIGH : LOW);
}

void NS_State_Rising()
{
  ns_state = true;
  ns_info_sended = false;
  digitalWrite(ACTION_SIGNAL_PIN, HIGH);
}

void initSensors() 
{
  pinMode(HC_PIN, INPUT);
  pinMode(NS_PIN, INPUT);
  pinMode(ACTION_SIGNAL_PIN, OUTPUT);
  pinMode(HC_SIGNAL_PIN, OUTPUT);
  if (!pressure.begin()) 
  {
    DEBUG_WRITELN("Error with bmp180/bmp085 connection\r\n");
    setLCDText("BMP Sensor Error");
  }
  dht.begin();
  lightMeter.begin();
  attachInterrupt(HC_INTERRUPT, HC_State_Changed, HC_INTERRUPT_MODE);
  attachInterrupt(NS_INTERRUPT, NS_State_Rising, NS_INTERRUPT_MODE);
  last_sending_millis = last_reset_millis = millis();
}

bool sendSensorsInfo(unsigned connection_id) 
{
  char* reply;
  bool rok;
  
  reply = sendMessage(connection_id, "DS_INFO=A:enum(off,on)[60]|Activity", MAX_ATTEMPTS);
  rok = replyIsOK(reply);
  if (!rok) return rok;

  reply = sendMessage(connection_id, "DS_INFO=E:int(0..100000)[]|Errors", MAX_ATTEMPTS);
  rok = replyIsOK(reply);
  if (!rok) return rok;

  reply = sendMessage(connection_id, "DS_INFO=T:float(-100..100)[]C|Temperature", MAX_ATTEMPTS);
  rok = replyIsOK(reply);
  if (!rok) return rok;
  
  reply = sendMessage(connection_id, "DS_INFO=P:float(500..1000)[]mm|Pressure", MAX_ATTEMPTS);
  rok = replyIsOK(reply);
  if (!rok) return rok;
  
  reply = sendMessage(connection_id, "DS_INFO=H:int(0..100)[]%|Humidity", MAX_ATTEMPTS);
  rok = replyIsOK(reply);
  if (!rok) return rok;
  
  reply = sendMessage(connection_id, "DS_INFO=L:float(0..200000)[]lux|Illuminance", MAX_ATTEMPTS);
  rok = replyIsOK(reply);
  if (!rok) return rok;
  
  reply = sendMessage(connection_id, "DS_INFO=R:enum(no,yes)[10]|Presence", MAX_ATTEMPTS);
  rok = replyIsOK(reply);
  if (!rok) return rok;

  reply = sendMessage(connection_id, "DS_INFO=N:enum(no,yes)[5]|Noise", MAX_ATTEMPTS);
  rok = replyIsOK(reply);

  return rok;
}

bool sensorsSending() 
{
  if ((hc_state && !hc_info_sended) || (ns_state && !ns_info_sended)) {
    digitalWrite(ACTION_SIGNAL_PIN, HIGH);
  }
  
  bool result = false;
  unsigned long int curr_millis = millis();
  unsigned long int millis_sum_delay = 0;
  unsigned long int millis_sum_reset_delay = 0;
  
  if (curr_millis > last_reset_millis) {
    millis_sum_reset_delay = curr_millis - last_reset_millis;
  }

  if (millis_sum_reset_delay > ERROR_CHECK_INTERVAL) 
  {
    if (errors_count>MAX_ERRORS) 
    {
      DEBUG_WRITELN("Too much errors. Restarting...");
      StartConnection(true);
      last_reset_millis = last_sending_millis = millis();
      return false;
    }
    errors_count = 0;
    last_reset_millis = curr_millis;
  }

  if (curr_millis > last_sending_millis) 
  {
    millis_sum_delay = curr_millis - last_sending_millis;
  }

  if (millis_sum_delay > SENDING_INTERVAL) 
  {
    char status;
    double T,P;

    String send_str = "A(on)";
    String lcd1 = "";
    String lcd2 = "";
    
    if (send_str.length()>0) send_str = send_str + ";";
    send_str = send_str + "E(" + String(errors_count) + ")";
    
    status = pressure.startTemperature();
    if (status != 0)
    {
      delay(status);
      status = pressure.getTemperature(T);
      if (status != 0)
      {
        if (send_str.length()>0) send_str = send_str + ";";
        send_str = send_str + "T(" + String(T, 2) + ")";
        lcd1 = "T="+String(T, 1)+"C  ";
        
        status = pressure.startPressure(3);
        if (status != 0)
        {
          delay(status);
          status = pressure.getPressure(P, T);
          if (status != 0)
          {
            P = P*0.750063755;
            if (send_str.length()>0) send_str = send_str + ";";
            send_str = send_str + "P(" + String(P, 3) + ")";
            lcd2 = "P="+String(P, 2)+"mm";
          }
        }
      }
    }
    
    float H = dht.readHumidity();
    if (!isnan(H)) {
      setH = true;
      oldH = H;
      if (send_str.length()>0) send_str = send_str + ";";
      send_str = send_str + "H(" + String(H, 0) + ")";
      lcd1 = lcd1 + "H="+String(H, 0)+"%";
    } else if (setH) {
      lcd1 = lcd1 + "H="+String(oldH, 0)+"%";
    }

    uint16_t lux = lightMeter.readLightLevel();
    if (send_str.length()>0) send_str = send_str + ";";
    send_str = send_str + "L(" + String(lux) + ")";

    if (send_str.length()>0) send_str = send_str + ";";
    send_str = send_str + "R(" + (digitalRead(HC_PIN) == HIGH ? "yes" : "no") + ")";

    if (ns_state && !ns_info_sended) {
      if (send_str.length()>0) send_str = send_str + ";";
      send_str = send_str + "N(yes)";
    }

    setLCDLines(lcd1.c_str(), lcd2.c_str());
    
    if (send_str.length()>0) {
      send_str = "DS_V={" + send_str + "}";
      sendMessage(connection_id, send_str, 0);
      result = true;
    }
    
    if (!hc_info_sended || !ns_info_sended) {
      digitalWrite(ACTION_SIGNAL_PIN, LOW);
    }
    
    hc_info_sended = true;
    ns_info_sended = true;

    last_sending_millis = millis();

  } else if ((hc_state && !hc_info_sended) || (ns_state && !ns_info_sended)) {
    String send_str = "A(on)";
    
    if (hc_state && !hc_info_sended) {
      if (send_str.length()>0) send_str = send_str + ";";
      send_str = send_str + "R(yes)";
    }
    if (ns_state && !ns_info_sended) {
      if (send_str.length()>0) send_str = send_str + ";";
      send_str = send_str + "N(yes)";
    }
    
    char* reply = sendMessage(connection_id, "DS_V={"+send_str+"}", MAX_ATTEMPTS);
    bool info_sended = replyIsOK(reply);

    if (hc_state && !hc_info_sended) hc_info_sended = info_sended;
    if (ns_state && !ns_info_sended) ns_info_sended = info_sended;
    delay(500);
    digitalWrite(ACTION_SIGNAL_PIN, LOW);
  }

  return result;
}
