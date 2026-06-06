/*******************************************************************************
 * File Name: SuperTask.c
 * Description: The system status machine.
 * Designer: Miller Xue
 * Version: 00
 * Copyright: Liyuan Haina Co.Ltd
 *******************************************************************************/
#include "PSFBController.h"
#include "App.h"
#include "F28x_Project.h"
#include "Adc_User.h"
#include "PWM_User.h"
#include "Gpio_User.h"
#include "RTOS_APP.h"
#include "RTOS_Define.h"
#include "math.h"
#include "stdio.h"
#include "SuperTask.h"
#include "MeasureTask.h"
#include "ModbusRtu.h"

SUPER_ST stSuper;
OUTPUT_CTRL_ST stOutputCtrl = {0};
void sSuperTaskInit(void);
void sSuperTask(void);
void sPowerOnMode(void);
void sSoftStartMode(void);
void sStandbyMode(void);
void sRunMode(void);
void sCVMode(void);
void sDischageMode(void);
void sFaultMode(void);
void sAllPWMOff(void);
void sOutputTimerEvt(void);

void sSuperTaskInit(void)
{
    stSuper.u16_SysMode = cPowerOnMode;
    stSuper.u16_PowerOnModeCnt = 0;
    stSuper.u16_PreModeCnt = 0;
    stSuper.u16_DischageModeCnt = 0;
    stSuper.u32_FaultCode.all = 0;
    stSuper.u16_SysStatus.all = 0;
    stSuper.u16_OutputMode = OP_CC;
    stSuper.u16_SoftStartModeCnt = 0;
    stSuper.u16_FaultModeCnt = 0;
    stSuper.u32_SwVersion = cSW_VERSION;
    stSuper.u16_PreChargeDelay = 0;
    stSuper.u16_SysStatus.BIT.OpenLoopEn = cPWM_OpenLoop_Test;
}

void sSuperTask(void)
{
    switch(stSuper.u16_SysMode)
    {
        case cPowerOnMode:
            sPowerOnMode();
        break;

        case cStandbyMode:
            sStandbyMode();
        break;

        case cSoftStartMode:
            sSoftStartMode();
        break;

        case cRunMode:
            sRunMode();
        break;

        case cDischageMode:
            sDischageMode();
        break;

        case cFaultMode:
            sFaultMode();
        break;

        default:
        break;
    }
}

void sPowerOnMode(void)
{
    uint16_t event;
    event = suwRTOSGetEvent(cPrioSuper);     //Get the current task event

    if(event & ((uint16_t)1 << eSuperTurnOnEvt))
    {
    }
    if(event & ((uint16_t)1 << eSuperTurnOffEvt))
    {
    }
    if(event & ((uint16_t)1 << eSuperFaultEvt))
    {
        sAllPWMOff();
        stSuper.u16_SysMode = cFaultMode;
        return;
    }
    if(event & ((uint16_t)1 << eSuperTimerEvt))
    {
        //Wait for the ADC offset calculation ready
        if(stAdcSample.u16_MidCalFlag == 0)
        {
            return;
        }
        //Wait for sysInit
        if(stSuper.u16_PowerOnModeCnt++ >= cSuper2s)
        {
            stSuper.u16_PowerOnModeCnt = 0;
            stSuper.u16_SysMode = cStandbyMode;
            return;
        }
        else
        {
            return;
        }
    }
}

void sStandbyMode(void)
{
    uint16_t event;
    event = suwRTOSGetEvent(cPrioSuper);     //Get the current task event

    if(event & ((uint16_t)1 << eSuperTurnOnEvt))
    {
        if(stSuper.u16_SysStatus.BIT.FaultBack == 0)
        {
            if(stSuper.u16_SysStatus.BIT.OpenLoopEn == 1)
            {
                stSuper.u16_SysMode = cSoftStartMode;
                return;
            }
            else
            {
                if((uModSta.REG.u16_Vbus <= V_TurnOnBusVoltMax) &&
                   (uModSta.REG.u16_Vbus >= V_TurnOnBusVoltMin) &&
                   (uModSta.REG.u16_PfcStatus & 0x4))
                {
                    /*if(stSuper.u16_SysStatus.BIT.OpRly == 1)
                    {
                        stSuper.u16_SysStatus.BIT.OpPreCharge = 0;
                        mOpRelay_Off();
                    }*/
                    stSuper.u16_SysMode = cSoftStartMode;
                    return;
                }
            }
        }
    }
    if(event & ((uint16_t)1 << eSuperTurnOffEvt))
    {
        sAllPWMOff();
        stSuper.u16_SysStatus.BIT.FaultBack = 0;
    }
    if(event & ((uint16_t)1 << eSuperFaultEvt))
    {
        sAllPWMOff();
        stSuper.u16_SysMode = cFaultMode;
        return;
    }
    if(event & ((uint16_t)1 << eSuperTimerEvt))
    {
        if(stSuper.u16_SysStatus.BIT.OpRly == 1)
        {
            stSuper.u16_SysStatus.BIT.OpPreCharge = 0;
            mOpRelay_Off();
        }

    }
}

void sSoftStartMode(void)
{
    uint16_t event;
    event = suwRTOSGetEvent(cPrioSuper);     //Get the current task event

    if(event & ((uint16_t)1 << eSuperTurnOnEvt))
    {

    }
    if(event & ((uint16_t)1 << eSuperTurnOffEvt))
    {
        sAllPWMOff();
        stSuper.u16_SysMode = cStandbyMode;
        stSuper.u16_SysStatus.BIT.FaultBack = 0;
        stSuper.u16_SoftStartModeCnt = 0;
        stSuper.u16_PreChargeDelay = 0;
        stSuper.u16_SysStatus.BIT.OpPreChargeEn = 0;
        return;
    }
    if(event & ((uint16_t)1 << eSuperFaultEvt))
    {
        sAllPWMOff();
        stSuper.u16_SysMode = cFaultMode;
        stSuper.u16_SoftStartModeCnt = 0;
        stSuper.u16_PreChargeDelay = 0;
        stSuper.u16_SysStatus.BIT.OpPreChargeEn = 0;
        return;
    }
    if(event & ((uint16_t)1 << eSuperTimerEvt))
    {
        if(stSuper.u16_SysStatus.BIT.OpenLoopEn == 0)
        {
            if((stMeasure.u16_Output_V_Avg < (stMeasure.u16_SC_V_Avg + 20)) &&
               (stSuper.u16_SysStatus.BIT.OpPreCharge == 0))
            {
                //Output PreCharge
                //Open PWM...
                stSuper.u16_SysStatus.BIT.OpPreChargeEn = 1;
                sOutputTimerEvt();

                stSuper.u16_SoftStartModeCnt++;
                if(stSuper.u16_SoftStartModeCnt >= cSuper20s)
                {
                    stSuper.u16_SysStatus.BIT.OpPreChargeEn = 0;
                    sAllPWMOff();
                    stSuper.u16_SoftStartModeCnt = 0;
                    stSuper.u32_FaultCode.BIT.PreChargeFail = 1;
                    stSuper.u16_SysMode = cFaultMode;
                    return;
                }
            }
            else
            {
                //Close PWM...
                sAllPWMOff();
                stSuper.u16_SysStatus.BIT.OpPreChargeEn = 0;
                stSuper.u16_SysStatus.BIT.OpPreCharge = 1;

//                if(stMeasure.u16_Output_V_Avg < (stMeasure.u16_SC_V_Avg + 10))
//                {
//                     if(stSuper.u16_SysStatus.BIT.OpRly == 0)
//                     {
//                         mOpRelay_On();
//                     }
//
//                     /*stSuper.u16_PreChargeDelay++;
//                     if(stSuper.u16_PreChargeDelay >= 5) // Delay 100ms
//                     {
//                         stSuper.u16_PreChargeDelay = 0;
//                         stSuper.u16_SysMode = cRunMode;
//                         return;
//                     }*/
//                }

                if(stSuper.u16_SysStatus.BIT.OpRly == 1)
                {
                    stSuper.u16_PreChargeDelay++;
                    if(stSuper.u16_PreChargeDelay >= 5) // Delay 100ms
                    {
                        stSuper.u16_PreChargeDelay = 0;
                        stSuper.u16_SysMode = cRunMode;
                        return;
                    }
                }

            }
        }
        else
        {
            stSuper.u16_SysMode = cRunMode;
            return;
        }
    }
}

void sRunMode(void)
{
    uint16_t event;
    event = suwRTOSGetEvent(cPrioSuper);     //Get the current task event

    if(event & ((uint16_t)1 << eSuperTurnOnEvt))
    {

    }
    if(event & ((uint16_t)1 << eSuperTurnOffEvt))
    {
        if(u16_TurnOffEn == 1)
        {
            sAllPWMOff();
            stSuper.u16_SysMode = cStandbyMode;
            return;
        }
    }
    if(event & ((uint16_t)1 << eSuperFaultEvt))
    {
        sAllPWMOff();
        stSuper.u16_SysMode = cFaultMode;
        return;
    }
    if(event & ((uint16_t)1 << eSuperTimerEvt))
    {
        stSuper.u16_SysStatus.BIT.OpPreChargeEn = 0;
        sOutputTimerEvt();

        if(stSuper.u16_OutputMode == 0)  //CC Mode
        {

        }
        else if(stSuper.u16_OutputMode == 1) //CV Mode
        {

        }
    }
}

void sCVMode(void)
{
    uint16_t event;
    event = suwRTOSGetEvent(cPrioSuper);     //Get the current task event

    if(event & ((uint16_t)1 << eSuperTurnOnEvt))
    {

    }
    if(event & ((uint16_t)1 << eSuperTurnOffEvt))
    {
        sAllPWMOff();
        stSuper.u16_SysMode = cStandbyMode;
        return;
    }
    if(event & ((uint16_t)1 << eSuperFaultEvt))
    {
        sAllPWMOff();
        stSuper.u16_SysMode = cFaultMode;
        return;
    }
    if(event & ((uint16_t)1 << eSuperTimerEvt))
    {
        sOutputTimerEvt();
    }
}

void sDischageMode(void)
{
    uint16_t event;
    event = suwRTOSGetEvent(cPrioSuper);     //Get the current task event

    if(event & ((uint16_t)1 << eSuperTurnOnEvt))
    {
    }
    if(event & ((uint16_t)1 << eSuperTurnOffEvt))
    {
    }
    if(event & ((uint16_t)1 << eSuperFaultEvt))
    {
        sAllPWMOff();
        stSuper.u16_SysMode = cFaultMode;
        return;
    }
    if(event & ((uint16_t)1 << eSuperTimerEvt))
    {
        /*if(CTRL.DischargeBit == Lowbit)
        {
            sAllPWMOff();
            stSuper.u16_SysMode = cPreMode;
            return;
        }
        else if((stSuper.u16_DischageModeCnt++ >= cSuper30s) && (stMeasure.u16_Bus_V_Avg > V_SoftStart_PROT))
        {
            sAllPWMOff();
            stSuper.u16_SysMode = cFaultMode;
            stSuper.u16_DischageModeCnt = 0;
            return;
        }
        else if(stMeasure.u16_Bus_V_Avg > (V_SoftStart_PROT - 1.0f))
        {
            sAllPWMOff();
        }
        else if(stMeasure.u16_Bus_V_Avg < V_SoftStart_PROT)
        {
            sAllPWMOff();
            stSuper.u16_SysMode = cPreMode;
            return;
        }
        else
        {
        }*/
    }
}

void sFaultMode(void)
{
    uint16_t event;
    event = suwRTOSGetEvent(cPrioSuper);     //Get the current task event

    if(event & ((uint16_t)1 << eSuperTurnOnEvt))
    {

    }
    if(event & ((uint16_t)1 << eSuperTurnOffEvt))
    {

    }
    if(event & ((uint16_t)1 << eSuperFaultEvt))
    {

    }
    if(event & ((uint16_t)1 << eSuperClearFaultEvt))
    {
        DINT;
        stSuper.u16_FaultModeCnt = 0;
        stSuper.u32_FaultCode.all = 0;
        stSuper.u16_SysStatus.BIT.FaultBack = 1;
        stSuper.u16_SysMode = cStandbyMode;
        EINT;
        return;
    }
    if(event & ((uint16_t)1 << eSuperTimerEvt))
    {
        if(stSuper.u16_SysStatus.BIT.OpRly == 1)
        {
            mOpRelay_Off();
        }

        stSuper.u16_FaultModeCnt++;
        if(stSuper.u16_FaultModeCnt > cSuper10s)
        {
            stSuper.u16_FaultModeCnt = 0;
            DINT;
            stSuper.u32_FaultCode.all = 0;
            stSuper.u16_SysStatus.BIT.FaultBack = 1;
            stSuper.u16_SysMode = cStandbyMode;
            EINT;
            return;
        }
    }
}

void sAllPWMOff(void)
{
    DINT;
    mPSFB_PwmOff();
    u16_PwmNeedOn = 0;
    EINT;
}

void sOutputTimerEvt(void)
{
    if(stSuper.u16_SysStatus.BIT.PwmStatus == 0)
    {
        DINT;
        sPSFBControllerReset();
        u16_PwmNeedOn = 1;
        EINT;
    }
}

