/* 
 Copyright (C) 2016,2017 hidenorly

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
#include "PWM.h"
#include "limits.h"

PWM::PWM(int nGPO, int nCycleMicroSec, float dutyPercent, bool bEnable):mGPO(nGPO),mCycleMicroSec(nCycleMicroSec),mEnable(bEnable)
{
  setOutputAndValue(nGPO, LOW);
  updateDutyMicroSec(dutyPercent);
  PWMManager* pManager = PWMManager::getInstance();
  pManager->addPWM(this);
}

PWM::~PWM()
{
  PWMManager* pManager = PWMManager::getInstance();
  pManager->removePWM(this);
}

void PWM::setDuty(float dutyPercent)
{
  updateDutyMicroSec(dutyPercent);
#if 0
  DEBUG_PRINT("PWM::setDuty(");
  DEBUG_PRINT(dutyPercent);
  DEBUG_PRINT(") on ");
  DEBUG_PRINT(mGPO);
  DEBUG_PRINT(mDutyMicroSec/1000.0f);
  DEBUG_PRINT(" / ");
  DEBUG_PRINTLN(mCycleMicroSec);
#endif
}

void PWM::setDuty(int dutyPecent)
{
  setDuty((float)dutyPecent);
}

int PWM::getCycleMicroSec(void)
{
  return mCycleMicroSec;
}

float PWM::getDuty(void)
{
  return mDutyMicroSec/mCycleMicroSec*100.0f;
}

int PWM::getPort(void)
{
  return mGPO;
}

int PWM::getDutyMicroSec(void)
{
  return mDutyMicroSec; 
}

void PWM::updateDutyMicroSec(float dutyPercent)
{
  mDutyMicroSec = (mCycleMicroSec * dutyPercent / 100.0f);
}

void PWM::setEnableOutput(bool enable)
{
  if( mEnable != enable){
    mEnable=enable;
    PWMManager* pManager = PWMManager::getInstance();
    pManager->setPWMCycle();
  }
}

bool PWM::getEnableOutput(void)
{
  return mEnable;
}



// --- PWM Data Manager
PWMManager* PWMManager::mpThis = NULL;

template class TemplateArray<IPWM>;
TemplateArray<IPWM> PWMManager::mpPWMs(MAX_PWMS);

PWMManager::PWMManager():mpPoller(NULL)
{
}

PWMManager::~PWMManager()
{
  cleanUp();
}


PWMManager* PWMManager::getInstance(void)
{
  if( NULL==mpThis ){
    mpThis = new PWMManager();
  }
  return mpThis;
}

void PWMManager::cleanUp(void)
{
  PWMManager* pThis = mpThis;
  mpThis = NULL;
  delete mpThis;
}

void PWMManager::terminate(void)
{
  cleanUp();
}

void PWMManager::addPWM(IPWM* pPWM)
{
  mpPWMs.add(pPWM);
  setPWMCycle();
}

void PWMManager::removePWM(IPWM* pPWM)
{
  mpPWMs.remove(pPWM);
}

int PWMManager::getOptimalCycle(void)
{
  int ret = INT_MAX;

  // TODO: improvement is required, such as GCD method etc.
  for(int i=0, c=mpPWMs.size(); i<c; i++){
    IPWM* pPWM = mpPWMs.getPtr(i);
    if( NULL != pPWM && pPWM->getEnableOutput() ){
      int nCycle = pPWM->getCycleMicroSec();
      if( nCycle < ret ){
        ret = nCycle;
      }
    }
  }
  
  return ret;
}

void PWMManager::setPWMCycle(void)
{
  int cycle = getOptimalCycle();
  if( INT_MAX == cycle ){
    // No enabled PWM
    if( NULL != mpPoller ){
      mpPoller->unregisterFromTimer();
      delete mpPoller;
      mpPoller = NULL;
    }
  } else {
    bool bCreate = false;
    if( NULL == mpPoller ){
      mpPoller = new Poller(cycle, this);
      bCreate = true;
    } else if( mpPoller->getDutyMicroSec() != cycle ){
      mpPoller->unregisterFromTimer();
      delete mpPoller;
      mpPoller = new Poller(cycle, this);
      bCreate = true;
    }
    if(bCreate){
      mpPoller->registerToTimer();
    }
  }
}

PWMManager::Poller::Poller(int dutyMicroSec, PWMManager* pThis):HWTimerTicker(NULL, reinterpret_cast<void*>(pThis), dutyMicroSec)
{
}

void PWMManager::Poller::doCallback(void)
{
  if( mpArg != NULL){
    PWMManager* pManager = reinterpret_cast<PWMManager*>(mpArg);
    pManager->tick();
  }
}

void PWMManager::tick(void)
{
  if( mpPoller == NULL ) return;

  AutoDisableInterrupt();

  unsigned long start_time = micros();
  int cycle = mpPoller->getDutyMicroSec();

  // set HIGH first
  int nextDutyMicroSec = INT_MAX;
  for(int i=0, c=mpPWMs.size(); i<c; i++){
    IPWM* pPWM = mpPWMs.getPtr(i);
    if( NULL != pPWM && pPWM->getEnableOutput() ){
      digitalWrite(pPWM->getPort(), HIGH);
      int candidate = pPWM->getDutyMicroSec();
      if( candidate < nextDutyMicroSec ){
        nextDutyMicroSec = candidate;
      }
    }
  }
  
  // TODO: wait with ticker if the next is more than 1 MicroSec (nextDutyMicroSec > 1000), etc.
  // Otherwise, if we set the duty=100%, always MCU will be occupied by this loop.
  int nPWM = mpPWMs.size();
  bool bFound=false;
  do {
    bFound=false;
    unsigned long nowDelta = micros() - start_time;
    for(int i=0; i<nPWM; i++){
      IPWM* pPWM = mpPWMs.getPtr(i);
      if( NULL != pPWM && pPWM->getEnableOutput() ){
        if( pPWM->getDutyMicroSec() < nowDelta ){
          digitalWrite(pPWM->getPort(), LOW);
        } else {
          bFound = true;
        }
      }
    }
  } while(bFound);
}


