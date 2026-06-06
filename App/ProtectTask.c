/*******************************************************************************
 * File Name: ProtectTask.c
 * Description: The protection task for the system.
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
#include "MeasureTask.h"
#include "math.h"
#include "stdio.h"
#include "SuperTask.h"
#include "MeasureTask.h"
#include "ProtectTask.h"

OUTRANGE_STRUCT stBusVoltOver = {0,0,0};
OUTRANGE_STRUCT stBusVoltUnder = {0,0,0};
OUTRANGE_STRUCT stOutputVoltOver = {0,0,0};
OUTRANGE_STRUCT stOutputCurrOver = {0,0,0};
OVERLOAD_STRUCT stOverLoad = {0,0,0,0,0,0};
//OUTRANGE_STRUCT stOutputVoltUnder;
OVERTEMP_STRUCT stOverTemp = {0,0,0};
OVERTEMPPOWER_STRUCT stOTPowerCC = {0,0,0,0,0,0,0,0,0,0};
OVERTEMPPOWER_STRUCT stOTPowerCV = {0,0,0,0,0,0,0,0,0,0};

uint16_t u16_FanWorkFlag = 0;

void sProtectTaskInit(void);
void sProtectTask(void);
void sBusVoltCheck(void);
void sOutputVoltCheck(void);
void sOutputCurrCheck(void);
void sOverLoadCheck(void);
void sPwmTripFaultDet(void);
void sTempOverCheck(void);
void sFanDect(void);
uint16_t sOverRangeSingleCheck(uint16_t u16_Va,CHECK_TYPE en_Type,OUTRANGE_STRUCT *stOutRange);
uint16_t sOverRangeThreePhaseCheck(uint16_t u16_Va,
                                   uint16_t u16_Vb,
                                   uint16_t u16_Vc,
                                   CHECK_TYPE en_Type,
                                   OUTRANGE_STRUCT *stOutRange);
uint16_t sOverRangeTwoPhaseCheck(uint16_t u16_Va,
                                 uint16_t u16_Vb,
                                 CHECK_TYPE en_Type,
                                 OUTRANGE_STRUCT *stOutRange);


void sProtectTaskInit(void)
{
    /*stGridVoltOver.u16_Threshold = (uint16_t)(cRatedGridVolt * cOutRangeVoltRatio * 10.0f);
    stGridVoltOver.u16_OutRangeCntSet = cProtect_20ms;//cProtect_100ms;
    stGridVoltOver.u16_OutRangeCnt = 0;

    stGridVoltUnder.u16_Threshold = (uint16_t)(cRatedGridVolt * cOutRangeVoltRatio1 * 10.0f);
    stGridVoltUnder.u16_OutRangeCntSet = cProtect_20ms;//cProtect_100ms;
    stGridVoltUnder.u16_OutRangeCnt = 0;

    stGridVoltUnbalance.u16_Threshold = 200;
    stGridVoltUnbalance.u16_OutRangeCntSet = cProtect_100ms;
    stGridVoltUnbalance.u16_OutRangeCnt = 0;

    stGridCurrOver1.u16_Threshold = (uint16_t)(cRatedGridCurr * cOutRangeCurrRatio * 5.0f);
    stGridCurrOver1.u16_OutRangeCntSet = cProtect_100ms;
    stGridCurrOver1.u16_OutRangeCnt = 0;

    stGridCurrOver2 = stGridCurrOver1;*/

    stBusVoltOver.u16_Threshold = (uint16_t)(cBusOverVolt * 10.0f);
    stBusVoltOver.u16_OutRangeCntSet = cProtect_20ms;
    stBusVoltOver.u16_OutRangeCnt = 0;

    stBusVoltUnder.u16_Threshold = (uint16_t)(cBusUnderVolt * 10.0f);
    stBusVoltUnder.u16_OutRangeCntSet = cProtect_20ms;
    stBusVoltUnder.u16_OutRangeCnt = 0;

    stOutputVoltOver.u16_Threshold = (uint16_t)(cOutputVoltOver * 10.0f);
    stOutputVoltOver.u16_OutRangeCntSet = cProtect_100ms;
    stOutputVoltOver.u16_OutRangeCnt = 0;

    stOutputCurrOver.u16_Threshold = (uint16_t)(cRatedOutputCurr * cOutRangeCurrRatio * 10.0f);
    stOutputCurrOver.u16_OutRangeCntSet = cProtect_100ms;
    stOutputCurrOver.u16_OutRangeCnt = 0;

    stOverLoad.u32_OverLoadLimit1 =  (uint32_t)(cRatedOutputPower * cOverLoadLimit1 * 1.0f);
    stOverLoad.u32_OverLoadLimit2 =  (uint32_t)(cRatedOutputPower * cOverLoadLimit2 * 1.0f);
    stOverLoad.u32_OverLoadCntSet1 = cProtect_1min;
    stOverLoad.u32_OverLoadCntSet2 = cProtect_1s;
    stOverLoad.u32_OverLoadCnt1 = 0;
    stOverLoad.u32_OverLoadCnt2 = 0;

    stOverTemp.u32_OverTempLimit = (uint16_t)(cFaultTempOver * 10.0f);
    stOverTemp.u32_OverTempCntSet = cProtect_100ms;
    stOverTemp.u32_OverTempCnt = 0;

    stOTPowerCC.u16_PowerLimitFlag = 0;
    stOTPowerCC.u16_PowerLimitFirstFlag = 0;
    stOTPowerCC.u32_OverTempPowerLimit1 = (uint16_t)(cPowerTempOver1 * 10.0f);
    stOTPowerCC.u32_OverTempPowerLimit2 = (uint16_t)(cPowerTempOver2 * 10.0f);
    stOTPowerCC.u32_OverTempPowerCntSet = cProtect_90s;
    stOTPowerCC.u32_OverTempPowerCnt = 0;
    stOTPowerCC.f32_PowerLimit = cRatedOutputCurr;
    stOTPowerCC.f32_PowerLimitRef = 0.0f;
    stOTPowerCC.f32_PowerLimitStep = 0.0f;
    stOTPowerCC.f32_PowerLimitLow = cPowerLimitCurrLow;

    stOTPowerCV.u16_PowerLimitFlag = 0;
    stOTPowerCV.u16_PowerLimitFirstFlag = 0;
    stOTPowerCV.u32_OverTempPowerLimit1 = (uint16_t)(cPowerTempOver1 * 10.0f);
    stOTPowerCV.u32_OverTempPowerLimit2 = (uint16_t)(cPowerTempOver2 * 10.0f);
    stOTPowerCV.u32_OverTempPowerCntSet = cProtect_90s;
    stOTPowerCV.u32_OverTempPowerCnt = 0;
    stOTPowerCV.f32_PowerLimit = cRatedOutputVolt;
    stOTPowerCV.f32_PowerLimitRef = 0.0f;
    stOTPowerCV.f32_PowerLimitStep = 0.0f;
    stOTPowerCV.f32_PowerLimitLow = cPowerLimitVoltLow;
}

void sProtectTask(void)
{
    uint16_t event;
    event = suwRTOSGetEvent(cPrioProtect);     //Get the current task event

    if(event & ((uint16_t)1 << eProtectAcCheck))
    {
        if(stSuper.u16_SysStatus.BIT.OpenLoopEn == 0)
        {
            //sBusVoltCheck();
            sOutputVoltCheck();
            sOutputCurrCheck();
        }

    }
    if(event & ((uint16_t)1 << eProtectDcCheck))
    {

    }
    if(event & ((uint16_t)1 << eProtectTimerEvt))
    {
        sOverLoadCheck();
        //sFanDect();
        sPwmTripFaultDet();
        if(stSuper.u16_SysMode != cPowerOnMode)
        {
            sTempOverCheck();
        }
    }
}

void sGridVoltCheck(void)
{

}

void sGridCurrCheck(void)
{


}

void sBusVoltCheck(void)
{
    if(stSuper.u16_SysMode == cRunMode)
    {
        //over voltage check
        if(sOverRangeSingleCheck(stMeasure.u16_SC_V_Avg,OverCheck,&stBusVoltOver) == cCheckOutRange) //stMeasure.u16_PWM_Bus_V_Avg
        {
           sAllPWMOff();
           stSuper.u32_FaultCode.BIT.BusOVP = 1;
           sRTOSEventSend(cPrioSuper,eSuperFaultEvt);
        }
        //under voltage check
        if(sOverRangeSingleCheck(stMeasure.u16_SC_V_Avg,UnderCheck,&stBusVoltUnder) == cCheckOutRange) //stMeasure.u16_PWM_Bus_V_Avg
        {
            sAllPWMOff();
            stSuper.u32_FaultCode.BIT.BusUVP = 1;
            sRTOSEventSend(cPrioSuper,eSuperFaultEvt);
        }
    }
}

void sOutputVoltCheck(void)
{
    if(stSuper.u16_SysMode == cRunMode)
    {
        if(sOverRangeSingleCheck(stMeasure.u16_Output_V_Avg,OverCheck,&stOutputVoltOver) == cCheckOutRange)
        {
           sAllPWMOff();
           stSuper.u32_FaultCode.BIT.OpVoltOVP = 1;
           sRTOSEventSend(cPrioSuper,eSuperFaultEvt);
        }
    }
}
void sOutputCurrCheck(void)
{
    if(stSuper.u16_SysMode == cRunMode)
    {
        if(sOverRangeSingleCheck(stMeasure.u16_Output_I_Avg,OverCheck,&stOutputCurrOver) == cCheckOutRange)
        {
           sAllPWMOff();
           stSuper.u32_FaultCode.BIT.OutputCurrOver = 1;
           sRTOSEventSend(cPrioSuper,eSuperFaultEvt);
        }
    }
}

void sOverLoadCheck(void)
{
    uint32_t u32_Temp1;
    u32_Temp1 = abs(stMeasure.u16_Output_P);
    if(u32_Temp1 > stOverLoad.u32_OverLoadLimit1)
    {
        stOverLoad.u32_OverLoadCnt1++;
        if(stOverLoad.u32_OverLoadCnt1 >= stOverLoad.u32_OverLoadCntSet1)
        {
            sAllPWMOff();
            stSuper.u32_FaultCode.BIT.OverLoad = 1;
            stOverLoad.u32_OverLoadCnt1 = 0;
            sRTOSEventSend(cPrioSuper,eSuperFaultEvt);
        }
    }
    else
    {
        if(stOverLoad.u32_OverLoadCnt1 > 0)
        {
            stOverLoad.u32_OverLoadCnt1--;
        }
    }

    if(u32_Temp1 > stOverLoad.u32_OverLoadLimit2)
    {
        stOverLoad.u32_OverLoadCnt2++;
        if(stOverLoad.u32_OverLoadCnt2 >= stOverLoad.u32_OverLoadCntSet2)
        {
            sAllPWMOff();
            stSuper.u32_FaultCode.BIT.OverLoad = 1;
            stOverLoad.u32_OverLoadCnt2 = 0;
            sRTOSEventSend(cPrioSuper,eSuperFaultEvt);
        }
    }
    else
    {
        if(stOverLoad.u32_OverLoadCnt2 > 0)
        {
            stOverLoad.u32_OverLoadCnt2--;
        }
    }
}

uint16_t sOverRangeSingleCheck(uint16_t u16_Va,CHECK_TYPE en_Type,OUTRANGE_STRUCT *stOutRange)
{
    if(en_Type == OverCheck)
    {
        if(u16_Va > stOutRange->u16_Threshold)
        {
            stOutRange->u16_OutRangeCnt++;
            if(stOutRange->u16_OutRangeCnt >= stOutRange->u16_OutRangeCntSet)
            {
                stOutRange->u16_OutRangeCnt = 0;
                return cCheckOutRange;
            }
        }
        else
        {
            stOutRange->u16_OutRangeCnt = 0;
            return cCheckNormal;
        }
    }
    else
    {
        if(u16_Va < stOutRange->u16_Threshold)
        {
            stOutRange->u16_OutRangeCnt++;
            if(stOutRange->u16_OutRangeCnt >= stOutRange->u16_OutRangeCntSet)
            {
                stOutRange->u16_OutRangeCnt = 0;
                return cCheckOutRange;
            }
        }
        else
        {
            stOutRange->u16_OutRangeCnt = 0;
            return cCheckNormal;
        }
    }
    return cCheckNormal;
}
uint16_t sOverRangeThreePhaseCheck(uint16_t u16_Va,
                                   uint16_t u16_Vb,
                                   uint16_t u16_Vc,
                                   CHECK_TYPE en_Type,
                                   OUTRANGE_STRUCT *stOutRange)
{
    if(en_Type == OverCheck)
    {
        if((u16_Va > stOutRange->u16_Threshold) ||
           (u16_Vb > stOutRange->u16_Threshold) ||
           (u16_Vc > stOutRange->u16_Threshold))
        {
            stOutRange->u16_OutRangeCnt++;
            if(stOutRange->u16_OutRangeCnt >= stOutRange->u16_OutRangeCntSet)
            {
                stOutRange->u16_OutRangeCnt = 0;
                return cCheckOutRange;
            }
        }
        else
        {
            stOutRange->u16_OutRangeCnt = 0;
            return cCheckNormal;
        }
    }
    else
    {
        if((u16_Va < stOutRange->u16_Threshold) ||
           (u16_Vb < stOutRange->u16_Threshold) ||
           (u16_Vc < stOutRange->u16_Threshold))
        {
            stOutRange->u16_OutRangeCnt++;
            if(stOutRange->u16_OutRangeCnt >= stOutRange->u16_OutRangeCntSet)
            {
                stOutRange->u16_OutRangeCnt = 0;
                return cCheckOutRange;
            }
        }
        else
        {
            stOutRange->u16_OutRangeCnt = 0;
            return cCheckNormal;
        }
    }
    return cCheckNormal;
}

uint16_t sOverRangeTwoPhaseCheck(uint16_t u16_Va,
                                 uint16_t u16_Vb,
                                 CHECK_TYPE en_Type,
                                 OUTRANGE_STRUCT *stOutRange)
{
    if(en_Type == OverCheck)
    {
        if((u16_Va > stOutRange->u16_Threshold) ||
           (u16_Vb > stOutRange->u16_Threshold))
        {
            stOutRange->u16_OutRangeCnt++;
            if(stOutRange->u16_OutRangeCnt >= stOutRange->u16_OutRangeCntSet)
            {
                stOutRange->u16_OutRangeCnt = 0;
                return cCheckOutRange;
            }
        }
        else
        {
            stOutRange->u16_OutRangeCnt = 0;
            return cCheckNormal;
        }
    }
    else
    {
        if((u16_Va < stOutRange->u16_Threshold) ||
           (u16_Vb < stOutRange->u16_Threshold))
        {
            stOutRange->u16_OutRangeCnt++;
            if(stOutRange->u16_OutRangeCnt >= stOutRange->u16_OutRangeCntSet)
            {
                stOutRange->u16_OutRangeCnt = 0;
                return cCheckOutRange;
            }
        }
        else
        {
            stOutRange->u16_OutRangeCnt = 0;
            return cCheckNormal;
        }
    }
    return cCheckNormal;
}

/********************************START**********************************************
 * Modified by CPH.
 * For the Fan fault detection function.
 * ***************************************************************************/
void sFanDect(void)
{
    static uint16_t fanErrCnt = 0;

    if(((GpioDataRegs.GPADAT.bit.GPIO28 == 1) || (GpioDataRegs.GPADAT.bit.GPIO29 == 1)) && (u16_FanWorkFlag == 1))
    {
        fanErrCnt ++;
    }
    else
    {
        fanErrCnt = 0;
    }

    if(fanErrCnt >= cFanErrCnt)
    {
        fanErrCnt = 0;
        sAllPWMOff();
        stSuper.u32_FaultCode.BIT.FanFault = 1;
        sRTOSEventSend(cPrioSuper,eSuperFaultEvt);
    }
}

void sPwmTripFaultDet(void)
{
    if((EPwm1Regs.TZOSTFLG.bit.OST1 == 1) ||
       (EPwm2Regs.TZOSTFLG.bit.OST1 == 1) ||
       (EPwm1Regs.TZOSTFLG.bit.OST2 == 1) ||
       (EPwm2Regs.TZOSTFLG.bit.OST2 == 1))
    {
//        sAllPWMOff();
//        if((Cmpss1Regs.COMPSTS.bit.COMPLLATCH == 1) ||
//           (Cmpss4Regs.COMPSTS.bit.COMPLLATCH == 1))
//        {
//            stSuper.u32_FaultCode.BIT.PowerFail = 1;
//            EALLOW;
//            Cmpss1Regs.COMPSTSCLR.bit.LLATCHCLR = 1;
//            Cmpss4Regs.COMPSTSCLR.bit.LLATCHCLR = 1;
//            EDIS;
//        }
//        else
//        {
//            EALLOW;
//            EPwm1Regs.TZOSTCLR.bit.OST1 = 1;
//            EPwm2Regs.TZOSTCLR.bit.OST1 = 1;
//            EPwm1Regs.TZOSTCLR.bit.OST2 = 1;
//            EPwm2Regs.TZOSTCLR.bit.OST2 = 1;
//            EDIS;
//            sRTOSEventSend(cPrioSuper,eSuperFaultEvt);
//        }
//
    }


    /*if((EPwm1Regs.TZOSTFLG.bit.DCAEVT1 == 1) ||
           (EPwm2Regs.TZOSTFLG.bit.DCAEVT1 == 1) ||
           (EPwm1Regs.TZOSTFLG.bit.DCBEVT1 == 1) ||
           (EPwm2Regs.TZOSTFLG.bit.DCBEVT1 == 1))
        {
            sAllPWMOff();
            EALLOW;
            EPwm1Regs.TZOSTCLR.bit.DCAEVT1 = 1;
            EPwm2Regs.TZOSTCLR.bit.DCAEVT1 = 1;
            EPwm1Regs.TZOSTCLR.bit.DCBEVT1 = 1;
            EPwm2Regs.TZOSTCLR.bit.DCBEVT1 = 1;
            EDIS;

            if((Cmpss1Regs.COMPSTS.bit.COMPLLATCH == 1) ||
               (Cmpss4Regs.COMPSTS.bit.COMPLLATCH == 1))
            {
                stSuper.u32_FaultCode.BIT.PowerFail = 1;
                EALLOW;
                Cmpss1Regs.COMPSTSCLR.bit.LLATCHCLR = 1;
                Cmpss4Regs.COMPSTSCLR.bit.LLATCHCLR = 1;
                EDIS;
            }
            else if(stSuper.u16_SysMode == cRunMode)
            {
                sRTOSEventSend(cPrioSuper,eSuperFaultEvt);
                stSuper.u32_FaultCode.BIT.OpCurrOCP = 1;
            }
        }*/

    if((EPwm1Regs.TZOSTFLG.bit.DCAEVT1 == 1) ||
       (EPwm2Regs.TZOSTFLG.bit.DCAEVT1 == 1) ||
       (EPwm1Regs.TZOSTFLG.bit.DCBEVT1 == 1) ||
       (EPwm2Regs.TZOSTFLG.bit.DCBEVT1 == 1))
    {
        sAllPWMOff();
        EALLOW;
        EPwm1Regs.TZOSTCLR.bit.DCAEVT1 = 1;
        EPwm2Regs.TZOSTCLR.bit.DCAEVT1 = 1;
        EPwm1Regs.TZOSTCLR.bit.DCBEVT1 = 1;
        EPwm2Regs.TZOSTCLR.bit.DCBEVT1 = 1;
        EDIS;

        if(stSuper.u16_SysMode == cRunMode)
        {
            sRTOSEventSend(cPrioSuper,eSuperFaultEvt);
            stSuper.u32_FaultCode.BIT.OpCurrOCP = 1;
        }
    }
}
/********************************END**********************************************/

float f32_PowerLimitCurr = cRatedOutputCurr;
float f32_PowerLimitVolt = cRatedOutputVolt;
void sTempOverCheck(void)
{
    //uint16_t i = 0;
    int16_t i16_TemperatureMax = 0;

    //TemperatureMax
    /*for(i = 0; i<5; i++)
    {
        if(i16_TemperatureMax < stMeasure.i16_Temperature[i])
        {
            i16_TemperatureMax = stMeasure.i16_Temperature[i];
        }
    }*/

    i16_TemperatureMax = stMeasure.i16_Temperature[0];
    if(i16_TemperatureMax < stMeasure.i16_Temperature[4])
    {
        i16_TemperatureMax = stMeasure.i16_Temperature[4];
    }

    /******************************Over Temperature Protect********************************/
    if(stSuper.u32_FaultCode.BIT.OverTemp1 == 0)
    {
        if(i16_TemperatureMax > stOverTemp.u32_OverTempLimit)
        {
            stOverTemp.u32_OverTempCnt++;
            if(stOverTemp.u32_OverTempCnt >= stOverTemp.u32_OverTempCntSet)
            {
                sAllPWMOff();
                stSuper.u32_FaultCode.BIT.OverTemp1 = 1;
                stOverTemp.u32_OverTempCnt = 0;
                sRTOSEventSend(cPrioSuper,eSuperFaultEvt);
            }
        }
        else
        {
            if(stOverTemp.u32_OverTempCnt > 0)
            {
                stOverTemp.u32_OverTempCnt--;
            }
        }
    }
    else
    {
        if(i16_TemperatureMax < (stOverTemp.u32_OverTempLimit - 50))
        {
            stOverTemp.u32_OverTempCnt++;
            if(stOverTemp.u32_OverTempCnt >= stOverTemp.u32_OverTempCntSet)
            {
                stSuper.u32_FaultCode.BIT.OverTemp1 = 0;
                stOverTemp.u32_OverTempCnt = 0;
            }
        }
        else
        {
            if(stOverTemp.u32_OverTempCnt > 0)
            {
                stOverTemp.u32_OverTempCnt--;
            }
        }
    }

    /******************************CC Reduced Power********************************/
    stOTPowerCC.u32_OverTempPowerCnt++;
    if((stOTPowerCC.u16_PowerLimitFlag == 0) || (stOTPowerCC.u32_OverTempPowerCnt >= stOTPowerCC.u32_OverTempPowerCntSet))
    {
        stOTPowerCC.u32_OverTempPowerCnt = 0;

        if(i16_TemperatureMax >= stOTPowerCC.u32_OverTempPowerLimit1)
        {
            stOTPowerCC.u16_PowerLimitFlag = 1;
            if(stOTPowerCC.u16_PowerLimitFirstFlag == 0)
            {
                stOTPowerCC.f32_PowerLimitRef = (float)(stMeasure.u16_Output_I_Avg)/10.0f;
                stOTPowerCC.f32_PowerLimitStep = stOTPowerCC.f32_PowerLimitRef * cPowerLimitRatio;
                stOTPowerCC.f32_PowerLimit = stOTPowerCC.f32_PowerLimitRef;
                stOTPowerCC.u16_PowerLimitFirstFlag = 1;
            }

            if(stOTPowerCC.f32_PowerLimit > (stOTPowerCC.f32_PowerLimitLow + stOTPowerCC.f32_PowerLimitStep))
            {
                stOTPowerCC.f32_PowerLimit -= stOTPowerCC.f32_PowerLimitStep;
            }
            else
            {
                stOTPowerCC.f32_PowerLimit = stOTPowerCC.f32_PowerLimitLow;
            }
        }
        else if((i16_TemperatureMax < stOTPowerCC.u32_OverTempPowerLimit2) && (stOTPowerCC.u16_PowerLimitFlag == 1))
        {
            if(stOTPowerCC.u16_PowerLimitFirstFlag == 1)
            {
                stOTPowerCC.f32_PowerLimitStep = stOTPowerCC.f32_PowerLimitRef * cPowerRecovRatio;
                stOTPowerCC.u16_PowerLimitFirstFlag = 0;
            }

            if(stOTPowerCC.f32_PowerLimit < (cRatedOutputCurr - stOTPowerCC.f32_PowerLimitStep))
            {
                stOTPowerCC.f32_PowerLimit += stOTPowerCC.f32_PowerLimitStep;
            }
            else
            {
                stOTPowerCC.f32_PowerLimit = cRatedOutputCurr;
                stOTPowerCC.u16_PowerLimitFlag = 0;
            }
        }

        DINT;
        f32_PowerLimitCurr = stOTPowerCC.f32_PowerLimit;
        EINT;
    }

    /******************************CV Reduced Power********************************/
    stOTPowerCV.u32_OverTempPowerCnt++;
    if((stOTPowerCV.u16_PowerLimitFlag == 0) || (stOTPowerCV.u32_OverTempPowerCnt >= stOTPowerCV.u32_OverTempPowerCntSet))
    {
        stOTPowerCV.u32_OverTempPowerCnt = 0;

        if(i16_TemperatureMax >= stOTPowerCV.u32_OverTempPowerLimit1)
        {
            stOTPowerCV.u16_PowerLimitFlag = 1;
            if(stOTPowerCV.u16_PowerLimitFirstFlag == 0)
            {
                stOTPowerCV.f32_PowerLimitRef = (float)(stMeasure.u16_Output_V_Avg)/10.0f;
                stOTPowerCV.f32_PowerLimitStep = stOTPowerCV.f32_PowerLimitRef * cPowerLimitRatio;
                stOTPowerCV.f32_PowerLimit = stOTPowerCV.f32_PowerLimitRef;
                stOTPowerCV.u16_PowerLimitFirstFlag = 1;
            }

            if(stOTPowerCV.f32_PowerLimit > (stOTPowerCV.f32_PowerLimitLow + stOTPowerCV.f32_PowerLimitStep))
            {
                stOTPowerCV.f32_PowerLimit -= stOTPowerCV.f32_PowerLimitStep;
            }
            else
            {
                stOTPowerCV.f32_PowerLimit = stOTPowerCV.f32_PowerLimitLow;
            }
        }
        else if((i16_TemperatureMax < stOTPowerCV.u32_OverTempPowerLimit2) && (stOTPowerCV.u16_PowerLimitFlag == 1))
        {
            if(stOTPowerCV.u16_PowerLimitFirstFlag == 1)
            {
                stOTPowerCV.f32_PowerLimitStep = stOTPowerCV.f32_PowerLimitRef * cPowerRecovRatio;
                stOTPowerCV.u16_PowerLimitFirstFlag = 0;
            }

            if(stOTPowerCV.f32_PowerLimit < (cRatedOutputVolt - stOTPowerCV.f32_PowerLimitStep))
            {
                stOTPowerCV.f32_PowerLimit += stOTPowerCV.f32_PowerLimitStep;
            }
            else
            {
                stOTPowerCV.f32_PowerLimit = cRatedOutputVolt;
                stOTPowerCV.u16_PowerLimitFlag = 0;
            }
        }

        DINT;
        f32_PowerLimitVolt = stOTPowerCV.f32_PowerLimit;
        EINT;
    }
}
