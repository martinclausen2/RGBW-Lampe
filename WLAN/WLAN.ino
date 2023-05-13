/*

 Sends time from NTP to UART
 
 Uses Time library to facilitate time computation

 
 Known issue:
 -the exact "second" precision is not guaranteed because of the semplicity of the NTP imnplementation
  normally the packet transit delay would be taken into account, but here is not

   
 Based upon:
 Udp NTP Client

 Get the time from a Network Time Protocol (NTP) time server
 Demonstrates use of UDP sendPacket and ReceivePacket
 For more on NTP time servers and the messages needed to communicate with them,
 see http://en.wikipedia.org/wiki/Network_Time_Protocol

 created 4 Sep 2010
 by Michael Margolis
 modified 9 Apr 2012
 by Tom Igoe
 updated for the ESP8266 12 Apr 2015 
 by Ivan Grokhotkov
 modified
 by Martin Clausen

 This code is in the public domain.

 */

#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Ticker.h>
#include <TimeLib.h>

// Auto Light Sleep https://blog.creations.de/?p=149
extern "C" {  
  #include "user_interface.h"
}

WiFiUDP ntpUDP;
// more generic NTP server "europe.pool.ntp.org"
// time offset 3600 => 1 hour => Central European Time
// refresh time frequency 600000 => 10 minutes
NTPClient timeClient(ntpUDP, "fritz.box", 3600, 600000);

// sleep duration between blocks of active transmission of time data to DCF77 receiver
#define sleepcount 48

// define pinout
#define LedPin 2
#define WPS 0

int ThisHour,ThisMinute,ThisSecond,ThisDay,ThisMonth,ThisYear,DayOfW;

int Dls;                    //DayLightSaving

void setup()
{
  Serial.begin(115200);
  Serial.println();
 
  pinMode(LedPin, OUTPUT);
  digitalWrite(LedPin, LOW);
  
  wifi_set_sleep_type(LIGHT_SLEEP_T);
  
  //first connect to wifi network
  //NOTE testing WiFi.status() BEFThisHour the FIRST WiFi.begin() seems to hang the system
  //so we attempt a first connection BEFORE the main loop
  
  Serial.printf("\nVersuche Verbindung mit gespeicherter SSID '%s'\n", WiFi.SSID().c_str());
  pinMode(WPS, INPUT_PULLUP); //Taster Eingang aktivieren
  ConnectToWiFi();
  CheckConnection();
  // wait 10 seconds for clock to come up
  delay(10000);
  timeClient.begin();
}

void loop(){
  int n;
  //check the WLAN status
  if (WiFi.status() == WL_CONNECTED)
  {
    // word clock reads every hour. so we present a stretch of ~75 minutes signal in every activity block
    for (n=0;n<25;n++) {
      ReadAndDecodeTime();
      delay(6000);
      if (WiFi.status() != WL_CONNECTED)
        break;
    };
    WiFi.mode( WIFI_OFF );
    WiFi.forceSleepBegin();
    delay(1);
    Serial.println("Beginne Ruhezustand.");
    // wait between blocks of active transmission
    for (n=0;n<sleepcount;n++) {
      Serial.printf("Aufwachen in %d Minuten.\n", (sleepcount - n) * 10);
      delay(600000);   //10 Minuten
    }
    Serial.println("Ende Ruhezustand.");
    Serial.printf("\nVersuche Verbindung mit gespeicherter SSID '%s'\n", WiFi.SSID().c_str());
    WiFi.forceSleepWake();
    delay(1);
    WiFi.mode( WIFI_STA );
    WiFi.begin();
    AwaitConnection();
    Serial.println();
  }
  else
  {
    // wait 1 Minute
    delay(60000);
    ReconnectToWiFi();
  }
}

void ConnectToWiFi(){
  WiFi.mode(WIFI_STA);
  String ssid = WiFi.SSID();
  if (ssid.length() > 0)
  {
    Serial.print("Connecting to ");
    Serial.println(WiFi.SSID().c_str());
    WiFi.begin(); // letzte gespeicherte Zugangsdaten
    if (!AwaitConnection())
      return;
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.print("Cannot connect, no SSID yet set.");
  }
}

void ReconnectToWiFi(){
  Serial.println("\nReconnecting to WLAN");
  WiFi.reconnect();
  AwaitConnection();
}

bool AwaitConnection(){
  int Timeout = 60;
  int timeoutCounter = 0;
  bool success = true;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (timeoutCounter++ > Timeout){
      Serial.println("\nImpossible to connect to WLAN!");
      success = false;
      break;
      }
    };
  return success;
}

void CheckConnection(){
  wl_status_t status = WiFi.status();
  if(status == WL_CONNECTED) {
    Serial.printf("\nErfolgreich angemeldet an SSID '%s'\n", WiFi.SSID().c_str());
  } else {
    //Wir waren nicht erfolgreich starten daher WPS
    while (WiFi.status() != WL_CONNECTED) {
      Serial.printf("\nKann keine WiFi Verbindung herstellen. Status ='%d'\n", status); 
      Serial.println("WPS Taste am Router drücken.\n Dann WPS Taste am ESP drücken!");
      while (digitalRead(WPS)!=0) {yield();}   
      if(!startWPS())
        Serial.println("Keine Verbindung über WPS herstellbar");  
    }
  } 
}

//Startet die WPS Konfiguration
bool startWPS() {
  Serial.println("WPS Konfiguration gestartet.");
  bool wpsSuccess = WiFi.beginWPSConfig();
  if(wpsSuccess) {
      // Muss nicht immer erfolgreich heißen! Nach einem Timeout ist die SSID leer
      String newSSID = WiFi.SSID();
      if(newSSID.length() > 0) {
        // Nur wenn eine SSID gefunden wurde waren wir erfolgreich 
        Serial.printf("WPS fertig. Erfolgreich angemeldet an SSID '%s'\n", newSSID.c_str());
        wpsSuccess = AwaitConnection();
      } else {
        wpsSuccess = false;
      }
  }
  return wpsSuccess; 
}

void ReadAndDecodeTime() {
    int DayToEndOfMonth,DayOfWeekToEnd,DayOfWeekToSunday;

    //get a random server from the pool
    timeClient.update();

    // avoid sending the wrong time
    if(timeClient.isTimeSet()) {
     
      // now convert NTP time into everyday time:
      Serial.print("Unix time = ");
      //note: we add two minutes because the dcf protocol send the time of the FOLLOWING minute
      //and our transmission begins the next minute more
      time_t ThisTime = timeClient.getEpochTime() + 120;
      // print Unix time:
      Serial.println(ThisTime);
  
      //calculate actual day to evaluate the summer/winter time of day ligh saving
      DayOfW = weekday(ThisTime);
      ThisDay = day(ThisTime);
      ThisMonth = month(ThisTime);
      ThisYear = year(ThisTime);
      Serial.print("date ");       // UTC is the time at Greenwich Meridian (GMT)
      Serial.print(ThisYear);
      Serial.print(' ');
      Serial.print(ThisMonth);
      Serial.print(' ');
      Serial.print(ThisDay);
      Serial.print(' ');
      Serial.print(DayOfW+1);
      Serial.print(' ');
    
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
      ThisHour = hour(ThisTime);
      ThisMinute = minute(ThisTime);
      ThisSecond = second(ThisTime);
      Serial.print("time "); 
      Serial.print(ThisHour); // print the hour
      Serial.print(' ');
      Serial.print(ThisMinute); // print the minute
      Serial.print(' ');
      Serial.println(ThisSecond); // print the second
    }
    else
    {
      Serial.println("No time received via NTP. Sending no DCF signal.");
    }
}