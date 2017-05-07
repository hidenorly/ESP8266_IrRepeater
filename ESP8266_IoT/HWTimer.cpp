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

#include "base.h"
#include "HWTimer.h"

extern "C" {
#include "ets_sys.h"

//timer dividers
#define TIM_DIV1       0 //80MHz (80 ticks/us - 104857.588 us max)
#define TIM_DIV16      1 //5MHz (5 ticks/us - 1677721.4 us max)
#define TIM_DIV265      3 //312.5Khz (1 tick = 3.2us - 26843542.4 us max)
//timer int_types
#define TIM_EDGE      0 // only this works
#define TIM_LEVEL      1
//timer reload values
#define TIM_SINGLE      0 //on interrupt routine you need to write a new value to start the timer again
#define TIM_LOOP      1 //on interrupt the counter will start with the same value again

void timer1_isr_init(void); 
void timer1_enable(uint8_t divider, uint8_t int_type, uint8_t reload); 
void timer1_disable(void); 
void timer1_attachInterrupt(timercallback userFunc); 
void timer1_detachInterrupt(void); 
void timer1_write(uint32_t ticks);
}

bool HWTimerTicker::mbTimerInitialized=false;
HWTimerTicker* HWTimerTicker::mpThis=NULL;


// --- HWTimerTicker
// Please note that this doesn't require to work with LooperThreadManager.
// We can use this HWTimerTicker alone but the doCallback will be called back as timer context.
HWTimerTicker::HWTimerTicker(CALLBACK_FUNC pFunc, void* pArg, int dutyMicroSec)
{
  mpFunc = pFunc;
  mpArg = pArg;
  mDutyMicroSec = dutyMicroSec;
  mpThis = this;
}

HWTimerTicker::~HWTimerTicker()
{
  unregisterFromTimer();

  mpFunc = NULL;
  mpArg = NULL;
  mDutyMicroSec = 0;
  mpThis = NULL;
}

int HWTimerTicker::getDutyMicroSec(void)
{
  return mDutyMicroSec;
}

void HWTimerTicker::registerToTimer(void)
{
  AutoDisableInterrupt disableInt;

  if( mbTimerInitialized ){
    unregisterFromTimer();
  }

  if( !mbTimerInitialized ) {
    timer1_isr_init();
    timer1_attachInterrupt(HWTimerTicker::_timerCallback);
    timer1_enable(TIM_DIV16, TIM_EDGE, TIM_LOOP);  //5MHz (5 ticks/us - 1677721.4 us max)
    timer1_write(mDutyMicroSec * (clockCyclesPerMicrosecond() / 16) );
  }

  mbTimerInitialized = true;
}

void HWTimerTicker::unregisterFromTimer(void)
{
  AutoDisableInterrupt disableInt;

  if( mbTimerInitialized ) {
    timer1_disable();
    timer1_detachInterrupt();
  }
  mbTimerInitialized = false;
}

void HWTimerTicker::_timerCallback(void)
{
  if( mpThis ){
    mpThis->preCallback();
  }
}

void HWTimerTicker::preCallback(void)
{
  doCallback();
}

void HWTimerTicker::doCallback(void)
{
//  DEBUG_PRINTLN("HWTimerTicker");
  if(mpFunc){
    mpFunc(mpArg);
  }
}
