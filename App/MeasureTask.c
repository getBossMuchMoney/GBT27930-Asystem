/*******************************************************************************
 * File Name: MeasurementTask.c
 * Description: The measurement functions and calibration.
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
#include "ProtectTask.h"
#include "TempCal.h"
#include "ShareData.h"
#include "ModbusRtu.h"


MEASURE_ST stMeasure;

void sMeasureTask(void);
void sMeasureTaskInit(void);
void sAcMeasureCalculation(void);
float sTemperatureCal(float uwAdcSample);
void sFanSpeedCtrl(void);

void sMeasureTask(void)
{
    uint16_t event;
    event = suwRTOSGetEvent(cPrioMeasure);     //Get the current task event

    if(event & ((uint16_t)1 << eMeasureACCalEvt))
    {
        sAcMeasureCalculation();
        sRTOSEventSend(cPrioProtect,eProtectAcCheck);
    }

    if(event & ((uint16_t)1 << eMeasureTimerEvt))
    {
        sFanSpeedCtrl();
    }
}

void sMeasureTaskInit(void)
{
    uint16_t i;
    stMeasure.u16_Output_P = 0;
    stMeasure.u16_SC_V_Avg = 0;
    stMeasure.u16_Output_V_Avg = 0;
    stMeasure.u16_Output_I_Avg = 0;

    for(i=0;i<8;i++)
    {
        stMeasure.i16_Temperature[i] = 0;
    }
}

void sAcMeasureCalculation(void)
{
    /*float f32_Temp,f32_Temp1,f32_Temp2,f32_Temp3;
    uint32_t u32_Temp,u32_Temp1;*/
    float f32_Temp;

    if(stSampleCal.u16_SumCntSet != 0)
    {
        f32_Temp = 1.0f/(float)stSampleCal.u16_SumCntSet;
    }
    else
    {
        f32_Temp = 1.0f/1200.0f;
    }

    stMeasure.u16_SC_V_Avg = (uint16_t)((stSampleCal.f32_SC_V_Sum*f32_Temp)*10.0f);
    stMeasure.u16_Output_I_Avg = (uint32_t)((stSampleCal.f32_Output_I_Sum*f32_Temp)*10.0f);
    stMeasure.u16_Output_V_Avg = (uint16_t)((stSampleCal.f32_Output_V_Sum*f32_Temp)*10.0f);
    stMeasure.u16_Output_P = (uint16_t)(stSampleCal.f32_Power_Active_Sum*f32_Temp);
    stMeasure.i16_Temperature[2] = (int16_t)(sTemperatureCal(stSampleCal.f32_T3_Sum * f32_Temp) * 10.0f);
    stMeasure.i16_Temperature[3] = (int16_t)(sTemperatureCal(stSampleCal.f32_T4_Sum * f32_Temp) * 10.0f);
    stMeasure.i16_Temperature[4] = (int16_t)(sTemperatureCal(stSampleCal.f32_T5_Sum * f32_Temp) * 10.0f);




#if   cMeasureTest == 1
    stMeasure.u16_Bus_V_Avg = 3800;
    stMeasure.u16_Output_I_Avg = 55;
    stMeasure.u16_Output_V_Avg = 5988;
    stMeasure.u16_Output_P = 2988;
#endif

}


void sFanSpeedCtrl(void)
{
//    static uint16_t u16_FanCtrlCnt = 0;
//    static uint16_t u16_AfterCloseCnt = 0;
//    static uint16_t FanSpdSet = 1;
//    static uint16_t FanSpdCmp = 0;
//    static uint16_t FanSpdCmpRef = FanLowSpd;
//    uint16_t i = 2; //i = 1; because Temperature channel1 is wrong.
//    int16_t i16_TemperatureMax = 0;
//    int16_t Output_P_Abs = 0;
//    uint16_t FanSpdCmpStep = 20;
//
//    if(u16_FanCtrlCnt++ >= 10) // 200ms
//    {
//        u16_FanCtrlCnt = 0;
//
//        if((stSuper.u16_SysStatus.BIT.Dc1PwmStatus == 1) || (stSuper.u16_SysStatus.BIT.Dc2PwmStatus == 1) || (stSuper.u16_SysStatus.BIT.Dc3PwmStatus == 1))
//        { /*Fan On*/
//
//            Output_P_Abs = abs(stMeasure.u16_Output_P); //stMeasure.u16_Input_P_Total
//            i16_TemperatureMax = stMeasure.i16_Temperature[0];
//            for(; i<5; i++)
//            {
//              if(i16_TemperatureMax < stMeasure.i16_Temperature[i])
//              {
//                  i16_TemperatureMax = stMeasure.i16_Temperature[i];
//              }
//            }
//
//            switch(FanSpdSet)
//            {
//                case 1:
//                    if((Output_P_Abs > 250) || (i16_TemperatureMax > 400))      //Greater than 25kW or 40бу ShiftUp
//                        FanSpdSet = 2;
//                    break;
//                case 2:
//                    if((Output_P_Abs <= 200) && (i16_TemperatureMax < 380))     //Less than 20kW and 38бу ShiftDown
//                        FanSpdSet = 1;
//                    else if((Output_P_Abs > 625) || (i16_TemperatureMax > 450)) //Greater than 62.5kW or 45бу ShiftUp
//                        FanSpdSet = 3;
//                    break;
//                case 3:
//                    if((Output_P_Abs <= 575) && (i16_TemperatureMax < 430))     //Less than 57.5kW and 43бу ShiftDown
//                        FanSpdSet = 2;
//                    break;
//                default:
//                    break;
//            }
//
//            switch(FanSpdSet)
//            {
//                case 1:
//                    //mFanSpdCtrl(FanLowSpd);
//                    FanSpdCmpRef = FanLowSpd;
//                    break;
//                case 2:
//                    //mFanSpdCtrl(FanMidSpd);
//                    FanSpdCmpRef = FanMidSpd;
//                    break;
//                case 3:
//                    //mFanSpdCtrl(FanHighSpd);
//                    FanSpdCmpRef = FanHighSpd;
//                    break;
//                default:
//                    break;
//            }
//
//            u16_FanWorkFlag = 1;
//        }
//        else
//        { /*Fan Off*/
//            if(u16_FanWorkFlag == 1)
//            {
//                if(u16_AfterCloseCnt++ > 100) // 20s
//                {
//                    u16_AfterCloseCnt = 0;
//                    u16_FanWorkFlag = 0;
//                }
//            }
//            else
//            {
//                FanSpdSet = 1;
//                FanSpdCmp = 0;
//                FanSpdCmpRef = FanLowSpd;
//                mFanSpdCtrl(FanOff);
//            }
//        }
//    }
//
//    if(u16_FanWorkFlag == 1)
//    {
//        if(FanSpdCmp < (FanSpdCmpRef - FanSpdCmpStep))
//        {
//            FanSpdCmp += FanSpdCmpStep;
//        }
//        else if(FanSpdCmp > (FanSpdCmpRef + FanSpdCmpStep))
//        {
//            FanSpdCmp -= FanSpdCmpStep;
//        }
//        else
//        {
//            FanSpdCmp = FanSpdCmpRef;
//        }
//
//        mFanSpdCtrl(FanSpdCmp);
//    }

    static uint16_t u16_FanCtrlCnt = 0;
    static uint16_t u16_AfterCloseCnt = 0;
    static uint16_t FanSpdSet = 1;
    static uint16_t FanSpdCmp = 0;
    static uint16_t FanSpdCmpRef = FanLowSpd;
    uint16_t FanSpdCmpStep = 20;

    if(u16_FanCtrlCnt++ >= 10) // 200ms
    {
        u16_FanCtrlCnt = 0;

        if((stSuper.u16_SysStatus.BIT.PwmStatus == 1) && (uModSta.REG.u16_PfcStatus & 0x1))
        { /*Fan On*/

            FanSpdCmpRef = FanHighSpd;

            u16_FanWorkFlag = 1;
        }
        else
        { /*Fan Off*/
            if(u16_FanWorkFlag == 1)
            {
                if(u16_AfterCloseCnt++ > 25) // 5s
                {
                    u16_AfterCloseCnt = 0;
                    u16_FanWorkFlag = 0;
                }
            }
            else
            {
                FanSpdSet = 1;
                FanSpdCmp = 0;
                FanSpdCmpRef = FanLowSpd;
                mFanSpdCtrl(FanOff);
            }

        }
    }

    if(u16_FanWorkFlag == 1)
    {
        if(FanSpdCmp < (FanSpdCmpRef - FanSpdCmpStep))
        {
            FanSpdCmp += FanSpdCmpStep;
        }
        else if(FanSpdCmp > (FanSpdCmpRef + FanSpdCmpStep))
        {
            FanSpdCmp -= FanSpdCmpStep;
        }
        else
        {
            FanSpdCmp = FanSpdCmpRef;
        }

        mFanSpdCtrl(FanSpdCmp);
    }
}
