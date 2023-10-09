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

// Error messages send to STM32
// 1 flash => booting
// 2 flash => no WLAN connection
// 3 flash => no NTP
// 4 flash => ESP OTA update in progress
// 6 flash => ESP OTA update error
// 8 flash => ESP OTA update finished

#define UART_BAUD 115200
#define UART_PORT 22
#define packTimeout 5 // ms (if nothing more on UART, then send packet)
#define bufferSize 8192

#define NTPTimeCount 10000000 // connect about every 3 hours

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
  pinMode(14, OUTPUT);
  digitalWrite(14, HIGH);
  Serial.begin(UART_BAUD);
  delay(5000);                          //wait for STM32 to come up
  Serial.println("");                   //clear startup noise
  Serial.println("statusled 1");
  Serial.println("debug Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("debug Connection Failed! Rebooting...");
    Serial.println("statusled 2");
    WiFi.disconnect();
    delay(5000);
    ESP.restart();
  }

  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("RGBW-Lampe");

  // No authentication by default
  ArduinoOTA.setPassword("admin");

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
    Serial.println("statusled 4");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("debug \nEnd");
    Serial.println("statusled 8");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("debug Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("debug Error[%u]: ", error);
    Serial.println("statusled 6");
    
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
  Serial.println("debug Booting V4 ready.");
  Serial.print("debug IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("debug Starting TCP Server.");
  server.begin(); // start TCP server 

  Serial.println("debug Connect to NTP server.");
  timeClient.forceUpdate();
  ReadAndDecodeTime(); //status led is set or reset here any way
}

void loop() {
  ArduinoOTA.handle();

//  if (WiFi.status() != WL_CONNECTED) {
  //  Serial.println("debug WiFi.disconnect();
    //ESP.restart();
 // }

  if(ntpTimeCounter > ntpTimeCount){
    ntpTimeCounter = 0;
    Serial.println();       //kill any pending transmissions
    ReadAndDecodeTime();
  } else {
    ntpTimeCounter++;
    delay(1);               //create a less random loop timing
  }

  // only serial bridge code below, which will restart the loop, if no client connected
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
      Serial.printf("time %u %u %u\r\n", hour(ThisTime), minute(ThisTime), second(ThisTime));
      Serial.printf("date %u %u %u %u\r\n", ThisYear, ThisMonth, ThisDay, DayOfW);
      Serial.println("statusled 0");
    }
    else
    {
      Serial.println("debug No time received via NTP.");
     //give STM32 time to process command
      Serial.println("statusled 3");
    }
}