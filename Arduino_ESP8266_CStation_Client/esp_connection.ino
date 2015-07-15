
//#include <SoftwareSerial.h>

#define WIFI_SSID "network"
#define WIFI_PASSWORD "123456"
#define SERVER_ADDR "192.168.1.2"
#define SERVER_PORT 51015

#define MAX_ATTEMPTS 5
#define MAX_CONNECTIONS 4
#define REPLY_BUFFER 512

// FOR ARDUINO UNO
// Arduino Pin 2 to RX
// Arduino Pin 3 to TX
// Need to set baud rate to 9600
// SoftwareSerial espSerial(2, 3);

// FOR ARDUINO MEGA
// Arduino TX2 for RX
// Arduino RX2 for TX
// Baud rate can be up to 38400
#define espSerial Serial2

const char line[] = "-----\n\r";
char reply[REPLY_BUFFER];

bool connected_to_wifi = false;
bool connected_to_server = false;
byte connection_id = 0;
char temp[5];

void initESP() {
  espSerial.begin(BAUD_RATE);
}

void StartConnection(bool reconnect) 
{
  bool rok = true;
  byte attempts = 0;

  if (reconnect || !connected_to_wifi) 
  {
    do {
      DEBUG_WRITE("Reset the module");
      setLCDText("Reset");
      attempts = 0;
      do {
        espSerial.print("AT+RST\r\n");
        getReply( 4000 );
        rok = replyIsOK();
        attempts++;
      } while (!rok && attempts<MAX_ATTEMPTS);
      if (!rok) continue;
      
      DEBUG_WRITE("Change to station mode");
      setLCDText("Station mode ->");
      attempts = 0;
      do {
        espSerial.print("AT+CWMODE=1\r\n");
        getReply( 2500 );
        rok = replyIsOK();
        attempts++;
      } while (!rok && attempts<MAX_ATTEMPTS);
      if (!rok) continue;
  
      DEBUG_WRITE("Connect to a network");
      setLCDText("WIFI Network ->");
      attempts = 0;
      do {
        espSerial.print("AT+CWJAP=\"");
        espSerial.print(WIFI_SSID);
        espSerial.print("\",\"");
        espSerial.print(WIFI_PASSWORD);
        espSerial.print("\"\r\n");
        getReply( 6000 );
        rok = replyIsOK();
        attempts++;
      } while (!rok && attempts<MAX_ATTEMPTS);
      if (!rok) continue;
  
      DEBUG_WRITE("Get ip address assigned by the router");
      setLCDLines("Getting IP","address");
      attempts = 0;
      do {
        espSerial.print("AT+CIFSR\r\n");
        getReply( 1000 );
        rok = replyIsOK();
        attempts++;
      } while (!rok && attempts<MAX_ATTEMPTS);
      if (!rok) continue;
  
      DEBUG_WRITE("Set for multiple connections");
      setLCDLines("Configuring","the connection");
      attempts = 0;
      do {
        espSerial.print("AT+CIPMUX=1\r\n");
        getReply( 1500 );
        rok = replyIsOK();
        attempts++;
      } while (!rok && attempts<MAX_ATTEMPTS);

    } while (!rok);

    connected_to_wifi = true;
    connected_to_server = false;
  }
  if (reconnect && connected_to_wifi && !connected_to_server) 
  {
    do {
      DEBUG_WRITE("Connect to the server");
      setLCDLines("Connect to", "server");
      connection_id++;
      if (connection_id > MAX_CONNECTIONS) connection_id = 1;
      attempts = 0;
      do {
        espSerial.print("AT+CIPSTART=");
        espSerial.print(connection_id);
        espSerial.print(",\"TCP\",\"");
        espSerial.print(SERVER_ADDR);
        espSerial.print("\",");
        espSerial.print(SERVER_PORT);
        espSerial.print("\r\n");
        getReply( 2000 );
        rok = replyIsOK();
        attempts++;
      } while (!rok && attempts<MAX_ATTEMPTS);
      if (!rok) {
        DEBUG_WRITE("Can't connect to server. Let's try again");
        setLCDText("Error: No server");
        delay(10000);
        continue;
      }

      DEBUG_WRITE("Start the server");
      setLCDLines("Start local", "server");
      attempts = 0;
      do {
        espSerial.print("AT+CIPSERVER=1,51015\r\n");
        getReply( 1500 );
        rok = replyIsOK();
        attempts++;
      } while (!rok && attempts<MAX_ATTEMPTS);
      if (!rok) {
        DEBUG_WRITE("Can't start the server. Let's try again");
        setLCDLines("Error: Can't", "start server");
        delay(5000);
        continue;
      }

      DEBUG_WRITE("Send identification Number");
      setLCDText("Identification");
      attempts = 0;
      do {
        espSerial.print("AT+CIPSEND=");
        espSerial.print(connection_id);
        espSerial.print(",3");
        espSerial.print("\r\n");
        getReply( 2000 );
        espSerial.print(IDENTIFICATION_CODE);
        espSerial.print("\r\n");
        getReply( 2000 );
        rok = replyIsOK();
        attempts++;
      } while (!rok && attempts<MAX_ATTEMPTS);

    } while (!rok);
    
    connected_to_server = true;
  }
  
  DEBUG_WRITE("Connected successfully!");
  setLCDText("Connected!");
}

void sendMessage(String message)
{
  DEBUG_WRITE("Sending message");
  DEBUG_WRITE(message.c_str());
  espSerial.print("AT+CIPSEND=");
  espSerial.print(connection_id);
  espSerial.print(",");
  espSerial.print(message.length());
  espSerial.print("\r\n");
  getReply( 500 );
  if (replyIsOK()) {
    espSerial.print(message);
    espSerial.print("\r\n");
    getReply( 1000 );
    if (!replyIsOK()) errors_count++;
  } else {
    errors_count++;
  }
}

char* readReply(int wait)
{
  char* result = NULL;
  if(espSerial.available())
  {
    char *result = getReply( wait );
    if (!replyIsOK()) errors_count++;
    millis_sum_delay += wait;
  }
  return result;
}

char* getReply(int wait)
{
  int tempPos = 0;
  long int time = millis();
  while( (time + wait) > millis())
  {
    while(espSerial.available())
    {
      char c = espSerial.read(); 
      if (tempPos < REPLY_BUFFER) { reply[tempPos] = c; tempPos++; }
    }
    reply[tempPos] = 0;
  }
  DEBUG_WRITE(reply); DEBUG_WRITE(line);
  return reply;
}

bool replyIsOK()
{
  int i;
  int reply_len = strlen(reply);
  bool foundOK = false;

  for (i=0; i<reply_len-2; i++)
  {
    if ( (reply[i]=='O') && (reply[i+1]=='K') ) { 
      foundOK = true;
      break;
    }
  }

  return foundOK;
}
