#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include "EspMQTTClient.h"
#include <WiFiClient.h>
#include <Ticker.h>
#include <TimeLib.h>
#include <NTPClient.h>
#include "credentials.h" // not part of the git repository due to confidential information, please create you own file to inject the WIFI credetials or exclude this line and add them delow

#ifndef STASSID
#define STASSID "myWLAN"
#define STAPSK "secret"
#endif

// please adjust the following lines for your environment
#define NTPserver "fritz.box"
#define MQTTbroker "192.168.178.123"
#define nodename "RGBW-Lampe Node"
#define publishtopicstatus "RGBW-Lampe/node/status"
#define publishtopicNTP "RGBW-Lampe/node/NTP"
#define publishtopicswitch "RGBW-Lampe/node/switch"
#define subscribetopicswitch "RGBW-Lampe/switch"
#define subscribetopic "RGBW-Lampe"
#define version "5"

#define switchGPIO 12

// Error messages send to STM32
// 1 flash => booting
// 2 flash => no WLAN connection
// 3 flash => no NTP

#define UART_BAUD 115200
#define UART_PORT 22
#define packTimeout 5 // ms (if nothing more on UART, then send packet)
#define bufferSize 8192

#define NTPTimeCount 10000000 // connect about every 3 hours

const int uart_port = UART_PORT;

EspMQTTClient client(
  STASSID,
  STAPSK,
  MQTTbroker, // MQTT Broker server ip
  nodename    // Client name that uniquely identify your device
);

WiFiServer server(uart_port);
WiFiClient UARTclient;

uint8_t buf1[bufferSize];
uint16_t i1=0;

uint8_t buf2[bufferSize];
uint16_t i2=0;

WiFiUDP ntpUDP;
// more generic NTP server "europe.pool.ntp.org"
// time offset 3600 => 1 hour => Central European Time
// refresh time frequency 600000 => 10 minutes
NTPClient timeClient(ntpUDP, NTPserver, 3600, 600000);
const long ntpTimeCount = NTPTimeCount;
long ntpTimeCounter = 0;

void setup() {
  Serial.begin(UART_BAUD);
  delay(5000);                          //wait for STM32 to come up
  Serial.println("");                   //clear startup noise
  Serial.println("statusled 1");
  Serial.println("debug Booting");
  pinMode(switchGPIO, OUTPUT);
  client.enableOTA("admin", 8266); // Enable OTA (Over The Air) updates. Password defaults to MQTTPassword. Port is the default OTA port. Can be overridden with enableOTA("password", port).
  }

void onConnectionEstablished() {

  client.subscribe(subscribetopic, [] (const String &payload)  {
    Serial.println(""); // clear queue
    Serial.println(payload);
  });

  client.subscribe(subscribetopicswitch, [] (const String &payload)  {
    if(payload == "ON")
    {
      digitalWrite(switchGPIO, HIGH);
      client.publish(publishtopicswitch, "switch on");
    }
    else if(payload == "OFF")
    {
      digitalWrite(switchGPIO, LOW);
      client.publish(publishtopicswitch, "switch off");
    }
  });

  IPAddress ip = WiFi.localIP();
  char buffer[60];
  sprintf(buffer, "%s started version %s with IP %d.%d.%d.%d", client.getMqttClientName(), version, ip[0], ip[1], ip[2], ip[3]);
  client.publish(publishtopicstatus, buffer);

  Serial.println("debug Booting done.");
  Serial.print("debug ");
  Serial.println(buffer);
  server.begin(); // start TCP server 

  Serial.println("debug Connect to NTP server.");
  timeClient.forceUpdate();
  ReadAndDecodeTime(); //status led is set or reset here any way
}

void loop() {
  client.loop();    // WIFI and MQTT client

  if(ntpTimeCounter > ntpTimeCount){
    ntpTimeCounter = 0;
    ReadAndDecodeTime();
  } else {
    ntpTimeCounter++;
    delay(1);               //create a less random loop timing
  }

  // only serial bridge code below, which will restart the loop, if no client connected
  if(!UARTclient.connected()) { // if client not connected
    UARTclient = server.available(); // wait for it to connect
    return;
  }
  
  // here we have a connected client
  if(UARTclient.available()) {
    while(UARTclient.available()) {
      buf1[i1] = (uint8_t)UARTclient.read(); // read char from client
      if(i1<bufferSize-1) i1++;
    }
    // now send to UART:
    Serial.write(buf1, i1);
    i1 = 0;
  }

  if(Serial.available()) {

    // read the data until pause:
   
    while(1) {
      if(Serial.available()) {
        buf2[i2] = (char)Serial.read(); // read char from UART
        if(i2<bufferSize-1) i2++;
      } else {
        //delayMicroseconds(packTimeoutMicros);
        delay(packTimeout);
        if(!Serial.available()) {
          break;
        }
      }
    }
    
    // now send to WiFi:
    UARTclient.write((char*)buf2, i2);
    i2 = 0;
  }
}

void ReadAndDecodeTime() {
    int DayToEndOfMonth,DayOfWeekToEnd,DayOfWeekToSunday;
    int ThisDay,ThisMonth,ThisYear,DayOfW;
    int Dls;                    //DayLightSaving
    char buffer[60];

    //get a random server from the pool
    timeClient.update();

    // avoid sending the wrong time
    if(timeClient.isTimeSet()) {
     
      // now convert NTP time into everyday time:
      time_t ThisTime = timeClient.getEpochTime();
  
      //calculate actual day to evaluate the summer/winter time of day ligh saving
      DayOfW = weekday(ThisTime);
      ThisDay = day(ThisTime);
      ThisMonth = month(ThisTime);
    
      //check daylight saving
      Dls = 0;    //default winter time
      //From April to september we are surely on summer time
      if (ThisMonth > 3 && ThisMonth < 10) {
        Dls = 1;
      };
      //March, month of change winter->summer time, last Sunday of the month
      //March has 31 days so from 25 included on Sunday we can be in summer time
      if (ThisMonth == 3 && ThisDay > 24) {
        DayToEndOfMonth = 31 - ThisDay;
        DayOfWeekToSunday = 7 - DayOfW;
        if (DayOfWeekToSunday >= DayToEndOfMonth)
          Dls = 1;
      };
      //October, month of change summer->winter time, last Sunday of the month
      //October has 31 days so from 25 included on Sunday we can be in winter time
      if (ThisMonth == 10) {
        Dls = 1;
        if (ThisDay > 24) {
          DayToEndOfMonth = 31 - ThisDay;
          DayOfWeekToEnd = 7 - DayOfW;
          if (DayOfWeekToEnd >= DayToEndOfMonth)
          Dls = 0;
        };
      };
  
      //add one hour if we are in summer time
      if (Dls == 1)
        ThisTime += 3600;

      // calculate date again from ThisTime as this might change due to a change in the time
      DayOfW = weekday(ThisTime);
      ThisDay = day(ThisTime);
      ThisMonth = month(ThisTime);
      ThisYear = year(ThisTime) - 2000;
    
      // translate american weekday numbering (first day of week = sunday) to european weekday numbering (first day of week = monday)
      DayOfW--;
      if (DayOfW == 0) {
        DayOfW = 7;
      };

      //now that we know the dls state, we can calculate the time to
      // print the hour, minutes and seconds:
      Serial.println(""); // clear queue
      Serial.printf("time %u %u %u\r\n", hour(ThisTime), minute(ThisTime), second(ThisTime));
      Serial.printf("date %u %u %u %u\r\n", ThisYear, ThisMonth, ThisDay, DayOfW);
      Serial.println("statusled 0");
      sprintf(buffer, "received timestamp %02u:%02u:%02u %02u-%02u-%02u day %u", hour(ThisTime), minute(ThisTime), second(ThisTime), ThisYear, ThisMonth, ThisDay, DayOfW);
      client.publish(publishtopicNTP, buffer);
    }
    else
    {
     //give STM32 time to process command
      Serial.println(""); // clear queue
      Serial.println("statusled 3");
      Serial.println("debug No time received via NTP.");
      client.publish(publishtopicNTP, "No time received.");
    }
}