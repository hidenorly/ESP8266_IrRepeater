/* 
 Copyright (C) 2016 hidenorly

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
*/

#include "base.h"
#include "config.h"
#include "WiFiUtil.h"

#include <FS.h>
#include <Ticker.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiType.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>

String getDefaultSSID(){
  byte mac[6];
  WiFi.macAddress(mac);
  String ssid = "";
  for (int i = 0; i < 6; i++) {
    ssid += String(mac[i], HEX);
  }
  return ssid;
}

void setupWiFiAP(){
  String ssid = getDefaultSSID();
  DEBUG_PRINTLN("SSID: " + ssid);
  DEBUG_PRINT("PASS: ");
  DEBUG_PRINTLN(WIFIAP_PASSWORD);

  WiFi.softAP(ssid.c_str(), WIFIAP_PASSWORD);
}

void saveWiFiConfig(String ssid, String pass){
  if( ssid!="" && pass!="" ) {
    if ( SPIFFS.exists(WIFI_CONFIG) ) {
      SPIFFS.remove(WIFI_CONFIG);
    }

    File f = SPIFFS.open(WIFI_CONFIG, "w");
    f.println(ssid);
    f.println(pass);
    f.close();
  }
}

void loadWiFiConfig(String& ssid, String& pass){
  File f = SPIFFS.open(WIFI_CONFIG, "r");
  ssid = f.readStringUntil('\n');
  pass = f.readStringUntil('\n');
  f.close();

  ssid.trim();
  pass.trim();
}

volatile int g_bNetworkConnected = false;
void setupWiFiStatusTracker();

void setupWiFiClient() {
  String ssid="";
  String pass="";
  loadWiFiConfig(ssid, pass);

  DEBUG_PRINTLN("SSID: " + ssid);
  DEBUG_PRINTLN("PASS: " + pass);

  g_bNetworkConnected = false;

  WiFi.begin(ssid.c_str(), pass.c_str());
  WiFi.mode(WIFI_STA);
  setupWiFiStatusTracker();
}

void onWiFiClientConnected();
// this function should be called in loop()
void handleWiFiClientStatus(){
  volatile static int bPreviousNetworkConnected = false;
  if( bPreviousNetworkConnected != g_bNetworkConnected ){
    bPreviousNetworkConnected = g_bNetworkConnected;
    if ( g_bNetworkConnected ) {
      // network is connected!
      onWiFiClientConnected();
    }
  }
}

Ticker g_WifiStatusTracker;

void checkWiFiStatus(CTrackerParam* p){
  static int previousStatus = WL_IDLE_STATUS;
  int curStatus = WiFi.status();
  if( previousStatus == curStatus) {
    return;
  } else {
    previousStatus = curStatus;
    switch (curStatus) {
      case WL_CONNECTED:
        g_bNetworkConnected = true;
        break;
      case WL_CONNECT_FAILED:
      case WL_CONNECTION_LOST:
      case WL_DISCONNECTED:
        setupWiFiClient();  // retry...
        break;
      default:;
        break;
    }
  }
}

void setupWiFiStatusTracker(){
  static int bInitialized = false;
  if( !bInitialized ) {
    g_WifiStatusTracker.attach_ms<CTrackerParam*>(500, checkWiFiStatus, NULL);
  }
}
