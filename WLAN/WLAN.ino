#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <WiFiClient.h>
#include <Ticker.h>
#include <TimeLib.h>
#include <NTPClient.h>
#include "credentials.h"

#ifndef STASSID
#define STASSID "myWLAN"
#define STAPSK "secret"
#endif

#define UART_BAUD 115200
#define UART_PORT 22
#define packTimeout 5 // ms (if nothing more on UART, then send packet)
#define bufferSize 8192

#define NTPTimeCount 10000000

const char* ssid = STASSID;
const char* password = STAPSK;
const int uart_port = UART_PORT;

WiFiServer server(uart_port);
WiFiClient client;

uint8_t buf1[bufferSize];
uint16_t i1=0;

uint8_t buf2[bufferSize];
uint16_t i2=0;

WiFiUDP ntpUDP;
// more generic NTP server "europe.pool.ntp.org"
// time offset 3600 => 1 hour => Central European Time
// refresh time frequency 600000 => 10 minutes
NTPClient timeClient(ntpUDP, "fritz.box", 3600, 600000);
const long ntpTimeCount = NTPTimeCount;
long ntpTimeCounter = 0;

void setup() {
  Serial.begin(UART_BAUD);
  Serial.println("debug Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("debug Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {  // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("debug Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("debug \nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("debug Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("debug Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("debug Booting ready.");
  Serial.print("debug IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("debug Starting TCP Server.");
  server.begin(); // start TCP server 

  Serial.println("debug Connect to NTP server.");
  timeClient.forceUpdate();
  ReadAndDecodeTime();
}

void loop() {
  ArduinoOTA.handle();

  if(!client.connected()) { // if client not connected
    client = server.available(); // wait for it to connect
    return;
  }

  // here we have a connected client

  if(client.available()) {
    while(client.available()) {
      buf1[i1] = (uint8_t)client.read(); // read char from client
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
    client.write((char*)buf2, i2);
    i2 = 0;
  }
  
  if(ntpTimeCounter > ntpTimeCount){
    ntpTimeCounter = 0;
    ReadAndDecodeTime();
  } else {
    ntpTimeCounter++;
  }
}

void ReadAndDecodeTime() {
    int DayToEndOfMonth,DayOfWeekToEnd,DayOfWeekToSunday;
    int ThisDay,ThisMonth,ThisYear,DayOfW;
    int Dls;                    //DayLightSaving
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
      ThisYear = year(ThisTime) - 2000;
    
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
  
      //now that we know the dls state, we can calculate the time to
      // print the hour, minutes and seconds:
      Serial.printf("time %u %u %u\n", hour(ThisTime), minute(ThisTime), second(ThisTime));
      Serial.printf("date %u %u %u %u\n", ThisYear, ThisMonth, ThisDay, DayOfW);
    }
    else
    {
      Serial.println("debug No time received via NTP.");
    }
}