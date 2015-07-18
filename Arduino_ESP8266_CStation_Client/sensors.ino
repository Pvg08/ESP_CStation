#include <Wire.h>
#include <SFE_BMP180.h>
#include <DHT.h>

#define MAX_ERRORS 4

#define DHTPIN 2
#define DHTTYPE DHT11

SFE_BMP180 pressure;
DHT dht(DHTPIN, DHTTYPE);
unsigned long int r_ms_count = 0;
bool setH = false;
float oldH;

void initSensors() {
  if (!pressure.begin()) 
  {
    DEBUG_WRITELN("Error with bmp180/bmp085 connection\r\n");
    setLCDText("BMP Sensor Error");
  }
  dht.begin();
}

bool sendSensorsInfo(unsigned connection_id) {
  char* reply;
  bool rok;
  
  reply = sendMessage(connection_id, "DS_INFO=A:enum(on,off)[off-60]|Activity", MAX_ATTEMPTS);
  rok = replyIsOK(reply);
  if (!rok) return rok;
  
  reply = sendMessage(connection_id, "DS_INFO=T:float(-100..100)C|Temperature", MAX_ATTEMPTS);
  rok = replyIsOK(reply);
  if (!rok) return rok;
  
  reply = sendMessage(connection_id, "DS_INFO=P:float(500..1000)mm|Pressure", MAX_ATTEMPTS);
  rok = replyIsOK(reply);
  if (!rok) return rok;
  
  reply = sendMessage(connection_id, "DS_INFO=H:int(0..100)%|Humidity", MAX_ATTEMPTS);
  rok = replyIsOK(reply);
  if (!rok) return rok;
  
  reply = sendMessage(connection_id, "DS_INFO=L:float(0..200000)lux|Illuminance", MAX_ATTEMPTS);
  rok = replyIsOK(reply);
  if (!rok) return rok;
  
  reply = sendMessage(connection_id, "DS_INFO=R:enum(no,yes)[no-10]|Presence", MAX_ATTEMPTS);
  rok = replyIsOK(reply);

  return rok;
}

bool sensorsSending() 
{
  bool result = false;
  if (millis_sum_delay > 30000) 
  {
    char status;
    double T,P;
    float H;

    String send_str = "A(on)";
    String lcd1 = "";
    String lcd2 = "";
    
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
    
    H = dht.readHumidity();
    if (!isnan(H)) {
      setH = true;
      oldH = H;
      if (send_str.length()>0) send_str = send_str + ";";
      send_str = send_str + "H(" + String(H, 0) + ")";
      lcd1 = lcd1 + "H="+String(H, 0)+"%";
    } else if (setH) {
      lcd1 = lcd1 + "H="+String(oldH, 0)+"%";
    }
    
    setLCDLines(lcd1.c_str(), lcd2.c_str());
    
    if (send_str.length()>0) {
      send_str = "DS_V={" + send_str + "}";
      sendMessage(connection_id, send_str, 0);
      result = true;
    }
    
    millis_sum_delay = millis_sum_delay % 30000;
    r_ms_count = r_ms_count+30000;
    if (r_ms_count>120000) {
      r_ms_count = 0;
      if (errors_count>MAX_ERRORS) {
        DEBUG_WRITELN("Too much errors. Restarting...");
        StartConnection(true);
      }
      errors_count = 0;
    }
  }
  
  return result;
}
