/* 
 Copyright (C) 2016, 2017 hidenorly

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

#ifndef __PWM_H__
#define __PWM_H__

#include "TemplateArray.h"
#include "HWTimer.h"

#define MAX_PWMS  1

class IPWM
{
public:
    virtual void setDuty(float dutyPercent)=0;
    virtual void setDuty(int dutyPecent)=0;
    virtual int getCycleMicroSec(void)=0;
    virtual float getDuty(void)=0;
    virtual int getPort(void)=0;
    virtual int getDutyMicroSec(void)=0;
    virtual void setEnableOutput(bool enable)=0;
    virtual bool getEnableOutput(void)=0;
};

class PWM : public IPWM
{
  public:
    PWM(int nGPO, int nCycleMicroSec, float dutyPercent=0.0f, bool bEnable=false);
    ~PWM();
    virtual void setDuty(float dutyPercent);
    virtual void setDuty(int dutyPecent);
    virtual int getCycleMicroSec(void);
    virtual float getDuty(void);
    virtual int getPort(void);
    virtual int getDutyMicroSec(void);
    virtual void setEnableOutput(bool enable);
    virtual bool getEnableOutput(void);
    
  protected:
    void updateDutyMicroSec(float dutyPercent);
    int mGPO;
    int mCycleMicroSec;
    int mDutyMicroSec;
    bool mEnable;
};

class PWMManager
{
  public:
    static PWMManager* getInstance(void);
    void addPWM(IPWM* pPWM);
    void removePWM(IPWM* pPWM);
    void terminate(void);
    void setPWMCycle(void);

  protected:
    PWMManager();
    ~PWMManager();
    static void cleanUp(void);

    static TemplateArray<IPWM> mpPWMs;
    static PWMManager* mpThis;

    class Poller:public HWTimerTicker
    {
      public:
        Poller(int dutyMicroSec=20, PWMManager* pThis=NULL);
      protected:
        virtual void doCallback(void);
    };
    Poller* mpPoller;

  int getOptimalCycle(void);
public:
  void tick(void);
};
#endif // __PWM_H__
