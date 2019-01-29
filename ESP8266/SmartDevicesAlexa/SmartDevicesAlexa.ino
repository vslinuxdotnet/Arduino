/*
* Alexa Multi Smart Device Emulator
* vasco dot santos at vslinux dot net
* v0.2 - 28/01/2019
*/

#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <WiFiUdp.h>
#include "Switch.h"
#include "UpnpBroadcastResponder.h"

const char* ssid     = "mynetwork";
const char* password = "pass";

UpnpBroadcastResponder upnpBroadcastResponder;

bool ICACHE_FLASH_ATTR officeLights(bool state) {
  if (state)
  {
    Serial.println(F("*** Switch 1 turn on ***"));
    return true;
  }else{
    Serial.println(F("*** Switch 1 turn off ***"));
    return false;
  }
}

bool ICACHE_FLASH_ATTR kitchenLights(bool state) {
  if (state)
  {
    Serial.println(F("*** Switch 2 turn on ***"));
    return true;
  }else{
    Serial.println(F("*** Switch 2 turn off ***"));
    return false;
    }
}

void setup(void){
  Serial.begin(115200);
  Serial.println(F("\nStart ESP Smart Devices...\n"));

  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
   Serial.println("\nWiFi connected");

   upnpBroadcastResponder.beginUdpMulticast();
   Serial.println(F("Adding switches upnp broadcast responder..."));
    
   upnpBroadcastResponder.addDevice(* new Switch(F("my light one"), 81, officeLights));
   upnpBroadcastResponder.addDevice(* new Switch(F("my light two"), 82, kitchenLights));

}

void loop(){
    upnpBroadcastResponder.serverLoop(); 
}
