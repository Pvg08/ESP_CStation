//#include <SoftwareSerial.h>
#include <SFE_BMP180.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#define WIFI_SSID "network"
#define WIFI_PASSWORD "123456"
#define SERVER_ADDR "192.168.1.2"
#define SERVER_PORT 51015

// 3 chars station code
#define IDENTIFICATION_CODE "DS1"

#define MAX_CONNECTIONS 4
#define REPLY_BUFFER 512

#define PRINT_REPLY true
//define DEBUG_PRINT(msg, msg2)
#define DEBUG_PRINT(msg, msg2) Serial.println(msg); lcd.home(); lcd.clear(); lcd.print(msg2)
#define DEBUG_PRINTLN(msg) Serial.println(msg)

// FOR ARDUINO UNO
// Arduino Pin 2 to RX
// Arduino Pin 3 to TX
// SoftwareSerial espSerial(2, 3);

// FOR ARDUINO MEGA
// Arduino TX2 for RX
// Arduino RX2 for TX
#define espSerial Serial2

SFE_BMP180 pressure;
LiquidCrystal_I2C lcd(0x27, 16, 2);

const char line[] = "-----\n\r";

char reply[REPLY_BUFFER];

unsigned long int ms_count = 0;
unsigned int rlength = 0;
bool connected_to_wifi = false;
bool connected_to_server = false;
byte connection_id = 0;

char temp[5];

void StartConnection(bool reconnect) 
{
  if (reconnect || !connected_to_wifi) {
    DEBUG_PRINT("Reset the module","Reset");
    espSerial.print("AT+RST\r\n");
    getReply( 4000 );
    DEBUG_PRINT("Change to station mode","Station mode ->");
    espSerial.print("AT+CWMODE=1\r\n");
    getReply( 2500 );
    DEBUG_PRINT("Connect to a network","WIFI Network ->");
    espSerial.print("AT+CWJAP=\"");
    espSerial.print(WIFI_SSID);
    espSerial.print("\",\"");
    espSerial.print(WIFI_PASSWORD);
    espSerial.print("\"\r\n");
    getReply( 6000 );
    DEBUG_PRINT("Get ip address assigned by the router","Getting IP");
    espSerial.print("AT+CIFSR\r\n");
    getReply( 1000 );
    DEBUG_PRINT("Set for multiple connections","");
    espSerial.print("AT+CIPMUX=1\r\n");
    getReply( 1500 );
    connected_to_wifi = true;
    connected_to_server = false;
  }
  if (reconnect && connected_to_wifi && !connected_to_server) {

    DEBUG_PRINT("Connect to the server","Server ->");
    connection_id++;
    if (connection_id > MAX_CONNECTIONS) connection_id = 1;
    espSerial.print("AT+CIPSTART=");
    espSerial.print(connection_id);
    espSerial.print(",\"TCP\",\"");
    espSerial.print(SERVER_ADDR);
    espSerial.print("\",");
    espSerial.print(SERVER_PORT);
    espSerial.print("\r\n");
    getReply( 2000 );

    DEBUG_PRINT("Start the server","Identification");
    espSerial.print("AT+CIPSERVER=1,51015\r\n");
    getReply( 1500 );
    connected_to_server = true;

    DEBUG_PRINT("Send identification Number","Identification");
    espSerial.print("AT+CIPSEND=");
    espSerial.print(connection_id);
    espSerial.print(",3");
    espSerial.print("\r\n");
    getReply( 2000 );
    
    espSerial.print(IDENTIFICATION_CODE);
    espSerial.print("\r\n");
    getReply( 2000 );
  }
}

void setup()
{
    Serial.begin(38400);
    espSerial.begin(38400);
  
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.home();
    
    delay(100);
    
    if (!pressure.begin()) {
      DEBUG_PRINTLN("Error with bmp180/bmp085 connection\r\n");
    }
    
    delay(100);
    
    DEBUG_PRINTLN("Start\r\n\r\n");

    delay(100);
    StartConnection(true);
}

void loop()
{
    char status;
    double T,P;
  
    if(espSerial.available())
    {
        getReply( 1000 );
        ms_count += 1000;

        bool foundConnect = false;
        int i;
        int reply_len = strlen(reply);
        char* message = reply;
        
        for (i=0; i<reply_len-5; i++)
        {
             if ( (reply[i]=='+') && (reply[i+1]=='I') && (reply[i+2]=='P') && (reply[i+3]=='D') && (reply[i+4]==',') ) { 
               foundConnect = true;
               break;
             }
        }

        if (foundConnect)
        {
          for (; i<reply_len && reply[i]!=':'; i++);
          i++;

          if (i<reply_len) {
            message = reply+i;
            DEBUG_PRINTLN(message);

            if (strncmp(message, "RST\r\n\r\n", 7)==0) {
              StartConnection(true);
              delay (1000);
              return;
            }
            
          }
        }
    }

    ms_count += 100;
    delay (100);
    
    if (ms_count > 20000) {
      
      String send_str;
      
      status = pressure.startTemperature();
      if (status != 0)
      {
        delay(status);
        status = pressure.getTemperature(T);
        if (status != 0)
        {
          send_str = send_str + "T(" + String(T, 2) + ")";
          
          status = pressure.startPressure(3);
          if (status != 0)
          {
            delay(status);
            status = pressure.getPressure(P, T);
            if (status != 0)
            {
              send_str = send_str + "P(" + String(P, 4) + ")";
            }
          }
          
          lcd.home();
          lcd.clear();
          lcd.print("T: ");
          lcd.print(T, 2);
          lcd.print(" *C");
          lcd.setCursor(0, 1);
          lcd.print("P: ");
          lcd.print(P, 2);
          lcd.print(" mb");
          
          DEBUG_PRINTLN("Sending T");
          espSerial.print("AT+CIPSEND=");
          espSerial.print(connection_id);
          espSerial.print(",");
          espSerial.print(5+send_str.length());
          espSerial.print("\r\n");
          getReply( 2000 );
          
          espSerial.print("SV:{");
          espSerial.print(send_str);
          espSerial.print("}\r\n");
          getReply( 2000 );
        }
      }
      ms_count = ms_count % 20000;
    }
}

void getReply(int wait)
{
    int tempPos = 0;
    long int time = millis();
    while( (time + wait) > millis())
    {
        while(espSerial.available())
        {
            char c = espSerial.read(); 
            if (tempPos < REPLY_BUFFER) { reply[tempPos] = c; tempPos++;   }
        }
        reply[tempPos] = 0;
    }
    if (PRINT_REPLY) { DEBUG_PRINTLN( reply );  DEBUG_PRINTLN(line);     }
}


