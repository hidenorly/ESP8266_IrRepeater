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

class CTrackerParam{
};

void setOutputAndValue(int port, int initialVal) {
  digitalWrite(port, initialVal); // Output data should be set first (before direction setting)
  pinMode(port, OUTPUT);
}

int StringFind(String& in, String key, int nStartPos)
{
  int pos = -1;
  int keyLen = key.length();
  int maxSearchLen = in.length() - keyLen;
  char searchKey = key.charAt(0);

  for(int i=nStartPos; i<maxSearchLen; i++){
    if( in.charAt(i) == searchKey ) {
      if( in.substring(i, i+keyLen) == key ){ 
        pos = i;
        break;
      }
    }
  }

  return pos;
}


AutoDisableInterrupt::AutoDisableInterrupt()
{
  noInterrupts();
}

AutoDisableInterrupt::~AutoDisableInterrupt()
{
  interrupts();
}
