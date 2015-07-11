#include <SoftwareSerial.h>

// Arduino pin 2 for RX
// Arduino Pin 3 for TX
SoftwareSerial espSerial(2,3);
  
const bool printReply = true;
const char line[] = "-----\n\r";
int loopCount=0;
 
char html[50];
char command[20];
char reply[500];
 
char ipAddress [20];
char name[30];
unsigned int rlength = 0;
char temp[5];

void setup()
{
    Serial.begin(9600);
    Serial.println("Start\r\n\r\n");
    espSerial.begin(9600);
    Serial.println("reset the module");
    espSerial.print("AT+RST\r\n");
    getReply( 2000 );
    Serial.println("Change to station mode");
    espSerial.print("AT+CWMODE=1\r\n");
    getReply( 1500 );
    Serial.println("Connect to a network ");
    espSerial.print("AT+CWJAP=\"PWRT_SDP__m11\",\"tUqq0Oo_OMRT102qD_b1\"\r\n");
    getReply( 6000 );
    Serial.println("Get the ip address assigned ny the router");
    espSerial.print("AT+CIFSR\r\n");
    getReply( 1000 );

    int len = strlen( reply );
    bool done=false;
    bool error = false;
    int pos = 0;
    while (!done)
    {
        if ( reply[pos] == 10) { done = true;}
        pos++;
        if (pos > len) { done = true;  error = true;}
    }
 
    if (!error)
    {
        int buffpos = 0;
        done = false;
        while (!done)
        {
           if ( reply[pos] == 13 ) { done = true; }
           else { ipAddress[buffpos] = reply[pos];    buffpos++; pos++;   }
        }
        ipAddress[buffpos] = 0;
    }
    else { strcpy(ipAddress,"ERROR"); }
 
    Serial.println("Set for multiple connections");
    espSerial.print("AT+CIPMUX=1\r\n");
    getReply( 1500 );
 
    Serial.println("Start the server");
    espSerial.print("AT+CIPSERVER=1,51015\r\n");
    getReply( 1500 );
 
    Serial.println("");
  
    Serial.println("Waiting for requests");
    Serial.print("Connect to "); Serial.println(ipAddress);
    Serial.println("");
}


void loop()
{
    if(espSerial.available())
    {
        getReply( 2000 );

        bool foundConnect = false;
        for (int i=0; i<strlen(reply)-6; i++)
        {
             if (  (reply[i]=='C') && (reply[i+1]=='O') && (reply[i+2]=='N') && (reply[i+3]=='N') && (reply[i+4]=='E') && (reply[i+5]=='C') && (reply[i+6]=='T') ) { foundConnect = true;    }
        }
  
        if (foundConnect)
        {

            loopCount++;
            Serial.print( "Have a connection.  Loop = ");  
            Serial.println(loopCount); 
            Serial.println("");

            strncpy(html, "RESULT", sizeof(html));
            rlength = strlen(html);
            
            espSerial.print("AT+CIPSEND=0,");
            espSerial.print(rlength);
            espSerial.print("\r\n");
            getReply( 1000 );
            espSerial.print(html);
            getReply( 1000 );

            espSerial.print( "AT+CIPCLOSE=0\r\n" );
            getReply( 500 );

            Serial.println("last getReply 1 ");
            getReply( 500 );
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
            if (tempPos < 500) { reply[tempPos] = c; tempPos++;   }
        }
        reply[tempPos] = 0;
    } 
 
    if (printReply) { Serial.println( reply );  Serial.println(line);     }
}
