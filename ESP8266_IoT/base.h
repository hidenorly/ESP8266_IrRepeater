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

#ifndef __BASE_H__
#define __BASE_H__

#include "Arduino.h"

// --- debug macro
#define ENABLE_DEBUG 1

#if ENABLE_DEBUG
  #define DEBUG_PRINT(...) Serial.print(__VA_ARGS__)
  #define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)
#else // ENABLE_DEBUG
  #define DEBUG_PRINT(...)
  #define DEBUG_PRINTLN(...)
#endif // ENABLE_DEBUG

class CTrackerParam;
void setOutputAndValue(int port, int initialVal);
class AutoDisableInterrupt {
  public:
    AutoDisableInterrupt();
    ~AutoDisableInterrupt();
};

int StringFind(String& in, String key, int nStartPos=0);

#endif // __BASE_H__
