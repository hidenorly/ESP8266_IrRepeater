/* 
 Copyright (C) 2017 hidenorly

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

extern "C" {
#include "user_interface.h"
}

#include "base.h"
#include "config.h"

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include "WiFiUtil.h"
#include "WebConfig.h"
#include "NtpUtil.h"
#include "LooperThreadTicker.h"

#include <FS.h>
#include <Time.h>
#include <TimeLib.h>
#include <NTP.h>

#include "PWM.h"

#define DEBUG_PRINT(...) Serial.print(__VA_ARGS__)
#define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)

// --- mode changer
bool initializeProperMode(){
  if( (digitalRead(MODE_PIN) == 0) || (!SPIFFS.exists(WIFI_CONFIG)) ){
    // setup because WiFi AP mode is specified or WIFI_CONFIG is not found.
    setupWiFiAP();
    setup_httpd();
    return false;
  } else {
    setupWiFiClient();
    setup_httpd();  // comment this out if you don't need to have httpd on WiFi client mode
  }
  return true;
}

// --- handler for WiFi client enabled
void onWiFiClientConnected(){
  DEBUG_PRINTLN("WiFi connected.");
  DEBUG_PRINT("IP address: ");
  DEBUG_PRINTLN(WiFi.localIP());
  start_NTP(); // socket related is need to be executed in main loop.
}

bool g_bEnableIrRelay = true;

// -- setup Ir
#define IR_CYCLE_MICRO_SEC (1000.0f/38.0f) // 38KHz
#define IR_DUTY (33.3f) // 33%

IPWM* g_irLED = new PWM(IR_OUTPUT, IR_CYCLE_MICRO_SEC, IR_DUTY, false);

void setupIrPWM(void)
{
  PWMManager* pPWMManager = PWMManager::getInstance();
  pPWMManager->addPWM(g_irLED);
  DEBUG_PRINT("PWM Cycle : ");
  DEBUG_PRINTLN(g_irLED->getCycleMicroSec());
}

void terminateIrPWM(void)
{
  PWMManager* pPWMManager = PWMManager::getInstance();
  pPWMManager->removePWM(g_irLED); // irLED is removed by this
  pPWMManager->terminate();
}


// --- General setup() function
void setup() {
  // Initialize GPIO
  initializeGPIO();
  
  // Initialize SerialPort
  Serial.begin(115200);

  // Initialize SPI File System
  SPIFFS.begin();

  DEBUG_PRINT("setup");

  // Check mode
  delay(1000);
  if(initializeProperMode()){
//    static Poller* sPoll=new Poller(1000);
//    g_LooperThreadManager.add(sPoll);
  }

  // Ir
  setupIrPWM();
}

void handleIrRelay(void)
{
  if( !g_bEnableIrRelay || digitalRead(IR_INPUT) == 1 ) return;

  // something code is received
  DEBUG_PRINT("handleIrRelay 0");
  wdt_disable();

  g_irLED->setEnableOutput(true);
  int current = digitalRead(IR_INPUT);
  int nReceived = 0;

  unsigned long last_time = micros();
  bool bLoop = true;
  do {
    if( current ^ digitalRead(IR_INPUT) ){
      current = digitalRead(IR_INPUT);
      g_irLED->setEnableOutput(!current);
      nReceived++;
      bLoop = true;
      last_time = micros();
    } else {
      if( micros() > (last_time+500000) ){
        bLoop = false;
      }
    }
  } while (bLoop);

#if 1
  if( nReceived ){
      DEBUG_PRINT(": ");
      DEBUG_PRINT(nReceived);
      DEBUG_PRINT(" codes received.");
  }
#endif // _DEBUG
  DEBUG_PRINTLN(": time out");

  g_irLED->setEnableOutput(false);

  wdt_enable(WDTO_8S);
}

void loop() {
  // put your main code here, to run repeatedly:
  handleWiFiClientStatus();
  handleWebServer();
  g_LooperThreadManager.handleLooperThread();
  handleIrRelay();
}
