
//#include <SoftwareSerial.h>

#define HOST_WIFI_SSID "ESP_CONF_HOST"
#define HOST_WIFI_PASSWORD ""
#define HOST_WIFI_CHANNEL 1
#define HOST_WIFI_ECN 4

#define SERVER_PORT 51015

#define MAX_ATTEMPTS 5
#define MAX_CONNECTIONS 4
#define REPLY_BUFFER 512

#define EEPROM_START_ADDR 120
#define WIFI_SSID_MAXLEN 40
#define WIFI_PASSWORD_MAXLEN 40
#define WIFI_SERVER_ADDRESS_MAXLEN 16

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

char wifi_ssid[WIFI_SSID_MAXLEN];
char wifi_passw[WIFI_PASSWORD_MAXLEN];
char server_ip_addr[WIFI_SERVER_ADDRESS_MAXLEN];
byte station_id = 0;

bool connected_to_wifi = false;
bool connected_to_server = false;
byte connection_id = 0;
char temp[5];

void initESP() {
  espSerial.begin(BAUD_RATE);

  wifi_ssid[0] = 0;
  wifi_passw[0] = 0;
  server_ip_addr[0] = 0;
  station_id = EEPROM.read(EEPROM_START_ADDR);
  readStringFromEEPROM(EEPROM_START_ADDR+1, wifi_ssid, WIFI_SSID_MAXLEN);
  readStringFromEEPROM(EEPROM_START_ADDR+WIFI_SSID_MAXLEN+2, wifi_passw, WIFI_PASSWORD_MAXLEN);
  readStringFromEEPROM(EEPROM_START_ADDR+WIFI_SSID_MAXLEN+WIFI_PASSWORD_MAXLEN+3, server_ip_addr, WIFI_SERVER_ADDRESS_MAXLEN);
}

void StartConfiguringMode()
{
  bool rok = true;
  byte attempts = 0;

  DEBUG_WRITE("Starting configuration mode");
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
      
      DEBUG_WRITE("Configuring a network");
      setLCDLines("Set Network","Parameters");
      attempts = 0;
      do {
        espSerial.print("AT+CWSAP=\"");
        espSerial.print(HOST_WIFI_SSID);
        espSerial.print("\",\"");
        espSerial.print(HOST_WIFI_PASSWORD);
        espSerial.print("\",");
        espSerial.print(HOST_WIFI_CHANNEL);
        espSerial.print(",");
        espSerial.print(HOST_WIFI_ECN);
        espSerial.print("\r\n");
        getReply( 2000 );
        rok = replyIsOK();
        attempts++;
      } while (!rok && attempts<MAX_ATTEMPTS);
      if (!rok) continue;

      DEBUG_WRITE("Change to host mode");
      setLCDText("Host mode ->");
      attempts = 0;
      do {
        espSerial.print("AT+CWMODE=2\r\n");
        getReply( 1500 );
        rok = replyIsOK();
        attempts++;
      } while (!rok && attempts<MAX_ATTEMPTS);
      if (!rok) continue;

      char* reply = NULL;
      DEBUG_WRITE("Get ip address of the esp");
      setLCDLines("Getting IP","address");
      attempts = 0;
      do {
        espSerial.print("AT+CIFSR\r\n");
        reply = getReply( 1000 );
        rok = replyIsOK();
        attempts++;
      } while (!rok && attempts<MAX_ATTEMPTS);
      if (!rok || !reply) continue;

      char ipAddress [20];
      int buffpos = 0;
      int pos = 0;
      bool done = false;
      while (!done)
      {
         if (reply[pos] == 13) { 
           done = true; 
         } else { 
           ipAddress[buffpos] = reply[pos];
           buffpos++;
           pos++;
         }
      }
      ipAddress[buffpos] = 0;

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

      DEBUG_WRITE("You need to connect to wifi-host");
      DEBUG_WRITE(HOST_WIFI_SSID);
      DEBUG_WRITE("Then use configuration programm to connect to");
      DEBUG_WRITE(ipAddress);
      DEBUG_WRITE("and perform setup operations...");
      
      setLCDLines("Waiting at host:", HOST_WIFI_SSID);
      delay(4000);
      setLCDLines("Server IP:", ipAddress);

    } while (!rok);

    while (1) {
      char* message = readTCPMessage( 1000 );
      char* param;
      if (message) {
        if ((param = getMessageParam(message, "SET_SSID="))) {
          strncpy(wifi_ssid, param, WIFI_SSID_MAXLEN);
          writeStringToEEPROM(EEPROM_START_ADDR+1, wifi_ssid, WIFI_SSID_MAXLEN);
          DEBUG_WRITE("SSID written to EEPROM:");
          DEBUG_WRITE(wifi_ssid);
        } else if ((param = getMessageParam(message, "SET_PSWD="))) {
          strncpy(wifi_passw, param, WIFI_PASSWORD_MAXLEN);
          writeStringToEEPROM(EEPROM_START_ADDR+WIFI_SSID_MAXLEN+2, wifi_passw, WIFI_PASSWORD_MAXLEN);
          DEBUG_WRITE("PASSW written to EEPROM:");
          DEBUG_WRITE(wifi_passw);
        } else if ((param = getMessageParam(message, "SET_SERV="))) {
          strncpy(server_ip_addr, param, WIFI_SERVER_ADDRESS_MAXLEN);
          writeStringToEEPROM(EEPROM_START_ADDR+WIFI_SSID_MAXLEN+WIFI_PASSWORD_MAXLEN+3, server_ip_addr, WIFI_SERVER_ADDRESS_MAXLEN);
          DEBUG_WRITE("Server address written to EEPROM:");
          DEBUG_WRITE(server_ip_addr);
        } else if ((param = getMessageParam(message, "SET_STID="))) {
          station_id = String(param).toInt();
          EEPROM.write(EEPROM_START_ADDR, station_id);
          DEBUG_WRITE("Station ID written to EEPROM:");
          DEBUG_WRITE(station_id);
        } else if ((param = getMessageParam(message, "SERV_RST=1"))) {
          return;
        }
      }
    }
}

char* getMessageParam(char* message, char const * param_name)
{
  int param_len = strlen(param_name);
  if (strncmp(message, param_name, param_len)==0) {
      message += param_len;
      int message_len = strlen(message);
      int i;
      for(i = 0; i<message_len && message[i]!='\r'; i++);
      message[i]=0;
      return message;
  }
  return NULL;
}

void StartConnection(bool reconnect) 
{
  bool rok = true;
  byte attempts = 0;

  if (reconnect || !connected_to_wifi) 
  {
    do {
      if (strlen(wifi_ssid)==0 || strlen(wifi_passw)==0 || strlen(server_ip_addr)==0 || !station_id) {
        DEBUG_WRITE("Need to set SSID, password and server ip");
        setLCDLines("Need ", "SSID, PWD, SRVIP");
        delay(5000);
        StartConfiguringMode();
      }

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
      
      DEBUG_WRITE("Change to client mode");
      setLCDText("Client mode ->");
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
        espSerial.print(wifi_ssid);
        espSerial.print("\",\"");
        espSerial.print(wifi_passw);
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
        espSerial.print(server_ip_addr);
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
        espSerial.print(",");
        espSerial.print(String(station_id).length()+2);
        espSerial.print("\r\n");
        getReply( 2000 );
        espSerial.print("DS");
        espSerial.print(station_id);
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

char* readReply(unsigned int wait)
{
  char* result = NULL;
  if(espSerial.available())
  {
    result = getReply( wait );
    if (!replyIsOK()) errors_count++;
    millis_sum_delay += wait;
  }
  return result;
}

char* readTCPMessage(unsigned int wait)
{
  char *message = readReply( wait );

  if (message) {
    bool foundConnect = false;
    int i;
    int reply_len = strlen(message);
    
    for (i=0; i<reply_len-5; i++)
    {
      if ( (message[i]=='+') && (message[i+1]=='I') && (message[i+2]=='P') && (message[i+3]=='D') && (message[i+4]==',') ) { 
        foundConnect = true;
        break;
      }
    }

    if (foundConnect)
    {
      for (; i<reply_len && message[i]!=':'; i++);
      i++;

      if (i<reply_len) 
      {
        message = message+i;
        DEBUG_WRITE(message);
        return message;
      }
    }
  }

  return NULL;
}

char* getReply(unsigned int wait)
{
  int tempPos = 0;
  unsigned int rtime = millis();
  while( (rtime + wait) > millis())
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
