
//#include <SoftwareSerial.h>

#define HOST_WIFI_SSID "ESP_CONF_HOST"
#define HOST_WIFI_PASSWORD ""
#define HOST_WIFI_CHANNEL 1
#define HOST_WIFI_ECN 0

#define SERVER_PORT 51015

#define MAX_ATTEMPTS 5
#define MAX_CONNECTIONS 4
#define REPLY_BUFFER 512

#define EEPROM_START_ADDR 120
#define WIFI_SSID_MAXLEN 40
#define WIFI_PASSWORD_MAXLEN 40
#define WIFI_SERVER_ADDRESS_MAXLEN 16

#define CONNECTIONS_ALL 5

#define CONNECTION_ESP_PIN 48

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

const char line[] = "-----\r\n";

char reply[REPLY_BUFFER];

char wifi_ssid[WIFI_SSID_MAXLEN];
char wifi_passw[WIFI_PASSWORD_MAXLEN];
char server_ip_addr[WIFI_SERVER_ADDRESS_MAXLEN];
byte station_id = 0;

bool connected_to_wifi = false;
bool connected_to_server = false;
bool in_configuration_mode = false;
bool transmittion_mode = false;
byte connection_id = 0;
char temp[5];

void initESP() {
  espSerial.begin(BAUD_RATE);
  pinMode(CONNECTION_ESP_PIN, OUTPUT);

  wifi_ssid[0] = 0;
  wifi_passw[0] = 0;
  server_ip_addr[0] = 0;
  station_id = EEPROM.read(EEPROM_START_ADDR);
  if (station_id && station_id<255) {
    readStringFromEEPROM(EEPROM_START_ADDR+1, wifi_ssid, WIFI_SSID_MAXLEN);
    readStringFromEEPROM(EEPROM_START_ADDR+WIFI_SSID_MAXLEN+2, wifi_passw, WIFI_PASSWORD_MAXLEN);
    readStringFromEEPROM(EEPROM_START_ADDR+WIFI_SSID_MAXLEN+WIFI_PASSWORD_MAXLEN+3, server_ip_addr, WIFI_SERVER_ADDRESS_MAXLEN);
  }
}

void StartConfiguringMode()
{
  bool rok = true;
  byte attempts = 0;
  char* reply = NULL;

  digitalWrite(CONNECTION_ESP_PIN, HIGH);

  if (connected_to_wifi && connected_to_server) closeConnection(CONNECTIONS_ALL);
  connected_to_wifi = false;
  connected_to_server = false;
  in_configuration_mode = false;

  DEBUG_WRITELN("Starting configuration mode");
  setLCDLines("Configuration", "      MODE");
  delay(1000);
  do {
      DEBUG_WRITELN("Reset the module");
      setLCDText("Reset");
      attempts = 0;
      do {
        espSerial.print("AT+RST\r\n");
        reply = getReply( 4000, false );
        rok = replyIsOK(reply);
        attempts++;
      } while (!rok && attempts<MAX_ATTEMPTS);
      if (!rok) continue;
      
      DEBUG_WRITELN("Change to host mode");
      setLCDText("Host mode ->");
      attempts = 0;
      do {
        espSerial.print("AT+CWMODE=3\r\n");
        reply = getReply( 1500, true );
        rok = replyIsOK(reply);
        attempts++;
      } while (!rok && attempts<MAX_ATTEMPTS);
      if (!rok) continue;
      
      DEBUG_WRITELN("Configuring a network");
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
        reply = getReply( 2000, true );
        rok = replyIsOK(reply);
        attempts++;
      } while (!rok && attempts<MAX_ATTEMPTS);
      if (!rok) continue;
      
      if (strlen(wifi_ssid)>0 || strlen(wifi_passw)>0) {
        DEBUG_WRITELN("Connect to a network");
        setLCDText("WIFI Network ->");
        espSerial.print("AT+CWJAP=\"");
        espSerial.print(wifi_ssid);
        espSerial.print("\",\"");
        espSerial.print(wifi_passw);
        espSerial.print("\"\r\n");
        getReply( 6000, true );
      }

      DEBUG_WRITELN("Get ip address of the esp");
      setLCDLines("Getting IP","address");
      attempts = 0;
      do {
        espSerial.print("AT+CIFSR\r\n");
        reply = getReply( 1000, true );
        rok = replyIsOK(reply);
        attempts++;
      } while (!rok && attempts<MAX_ATTEMPTS);
      if (!rok || !reply) continue;

      char ipAddress [WIFI_SERVER_ADDRESS_MAXLEN+1];
      int buffpos = 0;
      int pos = 0;
      bool start = false;
      bool done = false;
      while (!done && buffpos<WIFI_SERVER_ADDRESS_MAXLEN && pos<REPLY_BUFFER)
      {
        if (reply[pos] == '"') {
          done = start;
          start = true;
        } else if (start) {
          ipAddress[buffpos] = reply[pos];
          buffpos++;
        }
        pos++;
      }
      ipAddress[buffpos] = 0;

      DEBUG_WRITELN("Set for multiple connections");
      setLCDLines("Configuring","the connection");
      attempts = 0;
      do {
        espSerial.print("AT+CIPMUX=1\r\n");
        reply = getReply( 1500, true );
        rok = replyIsOK(reply);
        attempts++;
      } while (!rok && attempts<MAX_ATTEMPTS);
      if (!rok) continue;

      rok = startServer(1, SERVER_PORT);
      if (!rok) {
        DEBUG_WRITELN("Can't start the server. Let's try again");
        setLCDLines("Error: Can't", "start server");
        delay(5000);
        continue;
      }

      DEBUG_WRITE("You need to connect to wifi-host"); DEBUG_WRITELN(HOST_WIFI_SSID);
      DEBUG_WRITE("Then use configuration programm to connect to"); DEBUG_WRITELN(ipAddress);
      DEBUG_WRITELN("and perform setup operations...");
      
      setLCDLines("Waiting at host:", HOST_WIFI_SSID);
      delay(4000);
      setLCDLines("Server IP:", ipAddress);

    } while (!rok);

    in_configuration_mode = true;
    while (!reset_btn_pressed) {
      unsigned tcp_connection_id = 0;
      char* message = readTCPMessage( 1000, &tcp_connection_id, false );
      char* param;
      if (message) {
       
        if ((param = getMessageParam(message, "DS_SETUP:\r\n", false))) {
          unsigned line_pos = 0;
          
          rok = readLineToStr(param, wifi_ssid, WIFI_SSID_MAXLEN, line_pos, &line_pos);
          writeStringToEEPROM(EEPROM_START_ADDR+1, wifi_ssid, WIFI_SSID_MAXLEN);
          DEBUG_WRITE("SSID written to EEPROM:"); DEBUG_WRITELN(wifi_ssid);
          
          rok = readLineToStr(param, wifi_passw, WIFI_PASSWORD_MAXLEN, line_pos, &line_pos);
          writeStringToEEPROM(EEPROM_START_ADDR+WIFI_SSID_MAXLEN+2, wifi_passw, WIFI_PASSWORD_MAXLEN);
          DEBUG_WRITE("PASSW written to EEPROM:"); DEBUG_WRITELN(wifi_passw);
          
          rok = readLineToStr(param, server_ip_addr, WIFI_SERVER_ADDRESS_MAXLEN, line_pos, &line_pos);
          writeStringToEEPROM(EEPROM_START_ADDR+WIFI_SSID_MAXLEN+WIFI_PASSWORD_MAXLEN+3, server_ip_addr, WIFI_SERVER_ADDRESS_MAXLEN);
          DEBUG_WRITE("Server address written to EEPROM:"); DEBUG_WRITELN(server_ip_addr);
          
          station_id = readIntFromString(param, line_pos);
          EEPROM.write(EEPROM_START_ADDR, station_id);
          DEBUG_WRITE("Station ID written to EEPROM:"); DEBUG_WRITELN(station_id);
          break;
          
        } else if ((param = getMessageParam(message, "SERV_RST=1", true))) {
          break;
        }
        
        closeConnection(5);
        startServer(1, SERVER_PORT);
      }
    }
    
    digitalWrite(CONNECTION_ESP_PIN, LOW);
}

void StartConnection(bool reconnect) 
{
  bool rok = true;
  byte attempts = 0;
  char* reply;
  
  digitalWrite(CONNECTION_ESP_PIN, HIGH);
  
  if (connected_to_wifi && connected_to_server) closeConnection(CONNECTIONS_ALL);
  in_configuration_mode = false;

  if (reconnect || !connected_to_wifi) 
  {
    connected_to_wifi = false;
    connected_to_server = false;
    do {
      if (strlen(wifi_ssid)==0 || strlen(wifi_passw)==0 || strlen(server_ip_addr)==0 || !station_id) {
        DEBUG_WRITELN("Need to set SSID, password and server ip");
        setLCDLines("Need ", "SSID, PWD, SRVIP");
        delay(5000);
        StartConfiguringMode();
        delay(1000);
        digitalWrite(CONNECTION_ESP_PIN, HIGH);
      }

      DEBUG_WRITELN("Reset the module");
      setLCDText("Reset");
      attempts = 0;
      do {
        espSerial.print("AT+RST\r\n");
        reply = getReply( 4000, false );
        rok = replyIsOK(reply);
        attempts++;
      } while (!rok && attempts<MAX_ATTEMPTS);
      if (!rok) continue;
      
      DEBUG_WRITELN("Change to client mode");
      setLCDText("Client mode ->");
      attempts = 0;
      do {
        espSerial.print("AT+CWMODE=1\r\n");
        reply = getReply( 1500, true );
        rok = replyIsOK(reply);
        attempts++;
      } while (!rok && attempts<MAX_ATTEMPTS);
      if (!rok) continue;
  
      DEBUG_WRITELN("Connect to a network");
      setLCDText("WIFI Network ->");
      attempts = 0;
      do {
        espSerial.print("AT+CWJAP=\"");
        espSerial.print(wifi_ssid);
        espSerial.print("\",\"");
        espSerial.print(wifi_passw);
        espSerial.print("\"\r\n");
        reply = getReply( 6000, true );
        rok = replyIsOK(reply);
        attempts++;
      } while (!rok && attempts<MAX_ATTEMPTS);
      if (!rok) continue;
  
      DEBUG_WRITELN("Get ip address assigned by the router");
      setLCDLines("Getting IP","address");
      attempts = 0;
      do {
        espSerial.print("AT+CIFSR\r\n");
        reply = getReply( 1000, true );
        rok = replyIsOK(reply);
        attempts++;
      } while (!rok && attempts<MAX_ATTEMPTS);
      if (!rok) continue;
  
      DEBUG_WRITELN("Set for multiple connections");
      setLCDLines("Configuring","the connection");
      attempts = 0;
      do {
        espSerial.print("AT+CIPMUX=1\r\n");
        reply = getReply( 750, true );
        rok = replyIsOK(reply);
        attempts++;
      } while (!rok && attempts<MAX_ATTEMPTS);

    } while (!rok);

    connected_to_wifi = true;
    connected_to_server = false;
  }
  if (reconnect && connected_to_wifi && !connected_to_server) 
  {
    do {
      DEBUG_WRITELN("Connect to the server");
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
        reply = getReply( 2000, true );
        rok = replyIsOK(reply);
        attempts++;
      } while (!rok && attempts<MAX_ATTEMPTS);
      if (!rok) {
        DEBUG_WRITELN("Can't connect to server. Let's try again");
        setLCDText("Error: No server");
        delay(10000);
        continue;
      }

      rok = startServer(1, SERVER_PORT);
      if (!rok) {
        DEBUG_WRITELN("Can't start the server. Let's try again");
        setLCDLines("Error: Can't", "start server");
        delay(5000);
        continue;
      }

      DEBUG_WRITELN("Send identification Number");
      setLCDText("Identification");
      
      reply = sendMessage(connection_id, "DS="+String(station_id), MAX_ATTEMPTS);
      rok = replyIsOK(reply);
      
      DEBUG_WRITELN("Send sensors info");
      setLCDLines("Send semnsors", "info");
      rok = sendSensorsInfo(connection_id);
    } while (!rok);
    
    connected_to_server = true;
  }
  
  DEBUG_WRITELN("Connected successfully!");
  setLCDText("Connected!");
  digitalWrite(CONNECTION_ESP_PIN, LOW);
}

bool startServer(unsigned connection, unsigned port)
{
  DEBUG_WRITELN("Start the server");
  setLCDLines("Start local", "server");
  unsigned attempts = 0;
  bool rok = false;
  char* reply;
  do {
    espSerial.print("AT+CIPSERVER=");
    espSerial.print(connection);
    espSerial.print(",");
    espSerial.print(port);
    espSerial.print("\r\n");
    reply = getReply( 750, true );
    rok = replyIsOK(reply);
    attempts++;
  } while (!rok && attempts<MAX_ATTEMPTS);
  return rok;
}

bool closeConnection(unsigned connection) {
  DEBUG_WRITELN("Closing the server");
  setLCDLines("Close local", "server");
  unsigned attempts = 0;
  bool rok = false;
  char* reply;
  do {
    espSerial.print("AT+CIPCLOSE=");
    espSerial.print(connection);
    espSerial.print("\r\n");
    reply = getReply( 800, true );
    rok = replyIsOK(reply);
    attempts++;
  } while (!rok && attempts<MAX_ATTEMPTS);
  return rok;
}

char* sendMessage(unsigned connection_id, String message, unsigned max_attempts)
{
  DEBUG_WRITE("Sending to "); DEBUG_WRITE(connection_id);  DEBUG_WRITELN(" message:");
  DEBUG_WRITELN(message.c_str());
  DEBUG_WRITELN(line);
  
  unsigned attempts = 0;
  unsigned written = 0;
  unsigned str_length = message.length();
  unsigned buffer_len = SERIAL_RX_BUFFER_SIZE;
  String spart;
  char* reply = NULL;
  
  transmittion_mode = true;
  
  while (written<str_length) {
    spart = message.substring(written, written+buffer_len);
    
    espSerial.print("AT+CIPSEND=");
    espSerial.print(connection_id, DEC);
    espSerial.print(",");
    espSerial.print(spart.length(), DEC);
    espSerial.print("\r\n");
    reply = getReply( 5000, true );
    if (!replyIsOK(reply) && max_attempts && attempts<max_attempts) {
      errors_count++;
      attempts++;
      DEBUG_WRITELN("Sending Error: Retry");
      continue;
    }
    
    espSerial.print(spart.c_str());
    espSerial.print("\r\n");
    reply = getReply( 5000, true );
    if (!replyIsOK(reply) && max_attempts && attempts<max_attempts) {
      errors_count++;
      attempts++;
      DEBUG_WRITELN("Sending Error: Retry");
      continue;
    }

    written += buffer_len;
    attempts = 0;
  }
  
  transmittion_mode = false;
  
  return reply;
}

char* readReply(unsigned int wait, bool skip_on_ok)
{
  char* result = NULL;
  if(espSerial.available())
  {
    result = getReply( wait, skip_on_ok );
    if (!replyIsOK(result)) errors_count++;
  }
  return result;
}

bool checkReplyQuery(char* message, unsigned* connect_start)
{
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

  if (foundConnect) {
    *connect_start = i;
  }

  return foundConnect;
}

char* readTCPMessage(unsigned int wait, unsigned* tcp_connection_id, bool from_reply_buffer)
{
  char *message = readReply( wait, false );

  if (!message && from_reply_buffer) {
    message = reply;
  }

  if (message) {
    unsigned i;
    int reply_len = strlen(message);
    bool foundConnect = checkReplyQuery(message, &i);

    if (foundConnect)
    {
      bool start = false;
      bool done = false;
      if (tcp_connection_id) *tcp_connection_id = 0;
      for (; i<reply_len && message[i]!=':'; i++) {
        if (tcp_connection_id && !done) {
          if (!start && message[i] == ',') {
            start = true;
          } else if (start && message[i]>='0' && message[i]<='9') {
            *tcp_connection_id = readIntFromString(message, i);
            done = true;
          }
        }
      };
      i++;
      
      if (i<reply_len) 
      {
        message = message+i;
        DEBUG_WRITE("TCP message:"); DEBUG_WRITELN(message);
        if (from_reply_buffer) {
          reply[0] = 0;
        }
        return message;
      }
    }
  }

  return NULL;
}

char* getReply(unsigned int wait, bool skip_on_ok)
{
  char c;
  int tempPos = 0;
  int lastPos = 0;
  unsigned long int rtime = millis()+wait;
  bool foundOK = false;
  while( rtime > millis() && (!skip_on_ok || !foundOK))
  {
    lastPos = tempPos;
    while(espSerial.available())
    {
      c = espSerial.read(); 
      if (tempPos < REPLY_BUFFER) { reply[tempPos] = c; tempPos++; }
    }
    reply[tempPos] = 0;
    if (skip_on_ok && tempPos>lastPos) {
      if (lastPos>1) lastPos-=2;
      for (; lastPos<tempPos-1; lastPos++)
      {
        if ( (reply[lastPos]=='O') && (reply[lastPos+1]=='K') ) { 
          foundOK = true;
          delay(100);
          while(espSerial.available()) { espSerial.read(); }
          DEBUG_WRITELN("Found OK");
          break;
        }
      }
    }
  }
  DEBUG_WRITELN(line); DEBUG_WRITELN(reply); DEBUG_WRITELN(line);
  return reply;
}

