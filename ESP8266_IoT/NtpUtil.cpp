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
#include "NtpUtil.h"
#include <NTP.h>
#include "LooperThreadTicker.h"


class NtpStatusTracker:public LooperThreadTicker
{
  public:
    NtpStatusTracker(int dutyMSec=500);
  protected:
    virtual void doCallback(void);
    int mbInitialized1;
    int mbInitialized2;
};

NtpStatusTracker::NtpStatusTracker(int dutyMSec):LooperThreadTicker(NULL, NULL, dutyMSec),mbInitialized1(false),mbInitialized2(false)
{
}

void NtpStatusTracker::doCallback(void)
{
  time_t n = now();
  if( year(n) == 1970 ) {
    // NTP is not succeeded
    if( !mbInitialized1 ){
      setSyncInterval(10);
      mbInitialized1 = true;
    }
  } else {
    // after successful to set NTPed time on system
    if( !mbInitialized2 ){
      setSyncInterval(300);
      mbInitialized2 = true;
    }
  }
}

static NtpStatusTracker* g_pNtpStatusTracker = NULL;

void start_NTP(){
  static int bInitialized=false;
  if(!bInitialized){
    setTimeServer(NTP_SERVER);
    ntp_begin(2390);
    g_pNtpStatusTracker = new NtpStatusTracker();
    g_LooperThreadManager.add(g_pNtpStatusTracker);
    bInitialized = true;
  }
}
