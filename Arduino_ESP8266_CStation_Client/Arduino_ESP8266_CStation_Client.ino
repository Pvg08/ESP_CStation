#include <SoftwareSerial.h>

// Arduino pin 2 for RX
// Arduino Pin 3 for TX
SoftwareSerial espSerial(2, 3);

#define WIFI_SSID "network"
#define WIFI_PASSWORD "123456"
#define SERVER_ADDR "192.168.1.2"
#define SERVER_PORT 51015
#define MAX_CONNECTIONS 4

// 3 chars station code
#define IDENTIFICATION_CODE "DS0"

#define REPLY_BUFFER 500

#define PRINT_REPLY true
//define DEBUG_PRINT(msg)
#define DEBUG_PRINT(msg) Serial.println(msg)

const char line[] = "-----\n\r";

char html[50];
char reply[REPLY_BUFFER];
 
unsigned int rlength = 0;
bool connected_to_wifi = false;
bool connected_to_server = false;
byte connection_id = 0;

char temp[5];

void StartConnection(bool reconnect) 
{
  if (reconnect || !connected_to_wifi) {
    DEBUG_PRINT("Reset the module");
    espSerial.print("AT+RST\r\n");
    getReply( 2000 );
    DEBUG_PRINT("Change to station mode");
    espSerial.print("AT+CWMODE=1\r\n");
    getReply( 1500 );
    DEBUG_PRINT("Connect to a network");
    espSerial.print("AT+CWJAP=\"");
    espSerial.print(WIFI_SSID);
    espSerial.print("\",\"");
    espSerial.print(WIFI_PASSWORD);
    espSerial.print("\"\r\n");
    getReply( 6000 );
    DEBUG_PRINT("Get the ip address assigned by the router");
    espSerial.print("AT+CIFSR\r\n");
    getReply( 1000 );
    DEBUG_PRINT("Set for multiple connections");
    espSerial.print("AT+CIPMUX=1\r\n");
    getReply( 1500 );
    connected_to_wifi = true;
    connected_to_server = false;
  }
  if (reconnect && connected_to_wifi && !connected_to_server) {

    DEBUG_PRINT("Connect to the server");
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

    DEBUG_PRINT("Start the server");
    espSerial.print("AT+CIPSERVER=1,51015\r\n");
    getReply( 1500 );
    connected_to_server = true;

    DEBUG_PRINT("Send identification Number");
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
    Serial.begin(9600);
    DEBUG_PRINT("Start\r\n\r\n");
    espSerial.begin(9600);

    StartConnection(true);
}

void loop()
{
    if(espSerial.available())
    {
        getReply( 1000 );

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
            DEBUG_PRINT(message);

            if (strncmp(message, "RST\r\n\r\n", 7)==0) {
              StartConnection(true);
              delay (1000);
              return;
            }
            
          }
        }
    }

    delay (100);
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
 
    if (PRINT_REPLY) { DEBUG_PRINT( reply );  DEBUG_PRINT(line);     }
}


