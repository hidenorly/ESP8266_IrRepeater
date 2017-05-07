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

#ifndef __HWTIMER_H__
#define __HWTIMER_H__

#include "base.h"
#include "TemplateArray.h"


class HWTimerTicker
{
  public:
    typedef void (*CALLBACK_FUNC)(void*);
    HWTimerTicker(CALLBACK_FUNC pFunc=NULL, void* pArg=NULL, int dutyMicroSec=0);
    virtual ~HWTimerTicker();
    int getDutyMicroSec(void);

  protected:
    static void _timerCallback(void);
    virtual void preCallback(void);

  public:
    void registerToTimer(void);
    void unregisterFromTimer(void);

    virtual void doCallback(void);

  protected:
    CALLBACK_FUNC mpFunc;
    void* mpArg;
    int mDutyMicroSec;
    static bool mbTimerInitialized;
    static HWTimerTicker* mpThis;
};

#endif // __HWTIMER_H__
