/*******************************************************************************
 * File Name: InvController.c
 * Description: The Inv controller functions.
 * Designer: Miller Xue
 * Version: 00
 * Copyright: Liyuan Haina Co.Ltd
 *******************************************************************************/
#include "PSFBController.h"
#include "F28x_Project.h"
#include "math.h"
#include "stdio.h"
#include "SuperTask.h"
#include "PWM_User.h"
#include "Adc_User.h"
#include "MeasureTask.h"
#include "app.h"
#include "RTOS_APP.h"
#include "RTOS_Define.h"
#include "ProtectTask.h"
#include "ModbusRtu.h"


SAMPLE_CAL stSampleCal;
CTRL_PI_ST   stCtrlPiVout = {0};
CTRL_PI_ST   stCtrlPiIdc = {0};
CMPRPSFB_ST stPSFBCmpr;
float    f32_OpenLoopDuty;
uint16_t wSamplePointer;
uint16_t u16_OpenLoop = 0;
uint16_t u16_PwmNeedOn = 0;
uint16_t u16_PwmOnWaitCnt = 0;
float f32_VoutRefLimit = 0.0f;
float f32_IoutRefLimit = 0.0f;
float f32_VoutRefCmd_Last = 0.0f;
uint16_t u16_TurnOffEn = 0;

/****************************************************************************
*                   Function   Declaration  		   					*
*****************************************************************************/
void sSampelCalculation(void);
void sPSFBControllerInit(void);
void sPSFBControllerReset(void);
void sPSFBController(void);
void sEpsVoltRmsRegu(void);
void sEpsCurrRmsRegu(void);

#ifdef _FLASH
#pragma CODE_SECTION(sSampelCalculation, ".TI.ramfunc");
#endif
void sSampelCalculation(void)
{
    //float f32_Temp;

    stSampleCal.f32_SC_V = (float)stAdcSample.i16_SC_V * cSC_V_Ratio;
    stSampleCal.f32_Output_I = (float)stAdcSample.i16_Output_I * cOutput_I_Ratio;
    stSampleCal.f32_Output_V = (float)stAdcSample.i16_Output_V * cOutput_V_Ratio;

    stSampleCal.f32_SC_V_SumTemp += stSampleCal.f32_SC_V;
    stSampleCal.f32_Output_I_SumTemp += stSampleCal.f32_Output_I;
    stSampleCal.f32_Output_V_SumTemp += stSampleCal.f32_Output_V;
    stSampleCal.f32_Power_Active_SumTemp += stSampleCal.f32_Output_V * stSampleCal.f32_Output_I;
    stSampleCal.f32_T3_SumTemp += stAdcSample.u16_Temperature_3;
    stSampleCal.f32_T4_SumTemp += stAdcSample.u16_Temperature_4;
    stSampleCal.f32_T5_SumTemp += stAdcSample.u16_Temperature_5;

    stSampleCal.u16_SumCnt++;
    if(stSampleCal.u16_SumCnt >= stSampleCal.u16_SumCntSet)
    {
        stSampleCal.u16_SumCnt = 0;
        stSampleCal.f32_SC_V_Sum = stSampleCal.f32_SC_V_SumTemp;
        stSampleCal.f32_Output_I_Sum = stSampleCal.f32_Output_I_SumTemp;
        stSampleCal.f32_Output_V_Sum = stSampleCal.f32_Output_V_SumTemp;
        stSampleCal.f32_Power_Active_Sum = stSampleCal.f32_Power_Active_SumTemp;
        stSampleCal.f32_T3_Sum = stSampleCal.f32_T3_SumTemp;
        stSampleCal.f32_T4_Sum = stSampleCal.f32_T4_SumTemp;
        stSampleCal.f32_T5_Sum = stSampleCal.f32_T5_SumTemp;

        stSampleCal.f32_SC_V_SumTemp = 0;
        stSampleCal.f32_L_I_SumTemp = 0;
        stSampleCal.f32_Output_I_SumTemp = 0;
        stSampleCal.f32_Output_V_SumTemp = 0;
        stSampleCal.f32_Power_Active_SumTemp = 0;
        stSampleCal.f32_T3_SumTemp = 0;
        stSampleCal.f32_T4_SumTemp = 0;
        stSampleCal.f32_T5_SumTemp = 0;
        sRTOSEventSendIsr(cPrioMeasure,eMeasureACCalEvt);
    }
}

void sPSFBControllerInit(void)
{
    stSampleCal.f32_SC_V_SumTemp = 0;
    stSampleCal.f32_L_I_SumTemp = 0;
    stSampleCal.f32_Output_I_SumTemp = 0;
    stSampleCal.f32_Output_V_SumTemp = 0;
    stSampleCal.f32_Temperature_SumTemp = 0;
    stSampleCal.u16_SumCnt = 0;
    stSampleCal.u16_SumCntSet = (uint16_t)(PSFB_Fs * 1000.0f/50.0f);

    stCtrlPiIdc.f32_Ref = 0.0f;
    stCtrlPiIdc.f32_Fbk = 0.0f;
    stCtrlPiIdc.f32_Kp = 0.02f;//1.0f
    stCtrlPiIdc.f32_Ki = 0.001f;//0.2f
    stCtrlPiIdc.f32_Max = cPhaseShiftMax;
    stCtrlPiIdc.f32_Min = 0.0f;
    stCtrlPiIdc.f32_Err = 0.0f;
    stCtrlPiIdc.f32_Err0 = 0.0f;
    stCtrlPiIdc.f32_Out = 0.0f;
    stCtrlPiIdc.f32_Ui = 0.0f;
    stCtrlPiIdc.f32_Uo = 0.0f;
    stCtrlPiIdc.f32_Up = 0.0f;

    stCtrlPiVout.f32_Ref = 0.0f;
    stCtrlPiVout.f32_Fbk = 0.0f;
    stCtrlPiVout.f32_Kp = 0.1f;//1.0f
    stCtrlPiVout.f32_Ki = 0.01f;//0.2f
    stCtrlPiVout.f32_Max = cPhaseShiftMax;
    stCtrlPiVout.f32_Min = 0.0f;
    stCtrlPiVout.f32_Err = 0.0f;
    stCtrlPiVout.f32_Err0 = 0.0f;
    stCtrlPiVout.f32_Out = 0.0f;
    stCtrlPiVout.f32_Ui = 0.0f;
    stCtrlPiVout.f32_Uo = 0.0f;
    stCtrlPiVout.f32_Up = 0.0f;

    stPSFBCmpr.f32_Td = 0.0f;
    stPSFBCmpr.f32_Td1 = 0.0f;
    stPSFBCmpr.f32_Td2 = 0.0f;
    stPSFBCmpr.f32_Tdd = 0.0f;
    stPSFBCmpr.f32_Winner = 0.0f;
    stPSFBCmpr.u16_PeriodCnt = (int16_t)((int32_t)DSP_CPU_CLK*1000.0f/PSFB_Fs);
    stPSFBCmpr.u16_WinnerFlag1 = 0;
    stPSFBCmpr.u16_PsType = 0;

    stSuper.u16_SysStatus.BIT.OpenLoopEn = cPWM_OpenLoop_Test;
    //f32_DabOpenLoopPhase = 1.0f;

    stOutputCtrl.f32_VoutRefCmd = cDefault_Vout;
    stOutputCtrl.f32_IoutRefCmd = cDefault_Iout;
    stOutputCtrl.f32_PoutRefCmd = cDefault_Pout;
    stOutputCtrl.f32_VoutStep = 0.1f;
    stOutputCtrl.f32_IoutStep = 0.001f;
}

void sPSFBControllerReset(void)
{
    stCtrlPiIdc.f32_Ref = 0.0f;
    stCtrlPiIdc.f32_Fbk = 0.0f;
    stCtrlPiIdc.f32_Err = 0.0f;
    stCtrlPiIdc.f32_Err0 = 0.0f;
    stCtrlPiIdc.f32_Out = 0.0f;
    stCtrlPiIdc.f32_Ui = 0.0f;
    stCtrlPiIdc.f32_Uo = 0.0f;
    stCtrlPiIdc.f32_Up = 0.0f;

    stCtrlPiVout.f32_Ref = 0.0f;
    stCtrlPiVout.f32_Fbk = 0.0f;
    stCtrlPiVout.f32_Err = 0.0f;
    stCtrlPiVout.f32_Err0 = 0.0f;
    stCtrlPiVout.f32_Out = 0.0f;
    stCtrlPiVout.f32_Ui = 0.0f;
    stCtrlPiVout.f32_Uo = 0.0f;
    stCtrlPiVout.f32_Up = 0.0f;

    stPSFBCmpr.f32_Td = 0.0f;
    stPSFBCmpr.f32_Td1 = 0.0f;
    stPSFBCmpr.f32_Td2 = 0.0f;
    stPSFBCmpr.f32_Tdd = 0.0f;
    stPSFBCmpr.f32_Winner = 0.0f;
    //stPSFBCmpr.u16_PeriodCnt = (int16_t)((int32_t)CPU_CLK*1000L/Fs)>>1;
    stPSFBCmpr.u16_WinnerFlag1 = 0;

    //u16_CtrlLoopCnt = 0;
    //u16_DabPwmOnWaitCnt = 0;

    stOutputCtrl.f32_IoutRef = 0.0f;
    stOutputCtrl.f32_VoutRef = stSampleCal.f32_Output_V;//0;
    stOutputCtrl.u16_DcLoopCnt = 0;

    f32_VoutRefLimit = 0.0f;
    f32_IoutRefLimit = 0.0f;
    f32_VoutRefCmd_Last = 0.0f;
    u16_TurnOffEn = 0;
}

#ifdef _FLASH
#pragma CODE_SECTION(sPSFBController, ".TI.ramfunc");
#endif
void sPSFBController(void)
{	
	
    if((stSuper.u16_SysStatus.BIT.PwmStatus == 1) ||
        (u16_PwmNeedOn != 0))
    {
        if(u16_PwmNeedOn != 0)
        {
            u16_PwmOnWaitCnt++;
            if(u16_PwmOnWaitCnt >= 3)
            {
                u16_PwmNeedOn = 0;
                u16_PwmOnWaitCnt = 0;
                mPSFB_PwmOn();
            }
        }
        stCtrlPiVout.f32_Fbk = stSampleCal.f32_Output_V;
        stOutputCtrl.u16_DcLoopCnt++;
        if(stOutputCtrl.u16_DcLoopCnt >= Fs_Ctrl)
        {
            stOutputCtrl.u16_DcLoopCnt = 0;

            if(stSuper.u16_SysStatus.BIT.OpPreChargeEn == 1) //OutPut Voltage PreCharge
            {
                f32_VoutRefLimit = ((float)stMeasure.u16_SC_V_Avg * 0.1f) + 5.0f;
                if(f32_VoutRefLimit > 720.0f)
                {
                    f32_VoutRefLimit = 720.0f;
                }
                f32_IoutRefLimit = 1.0f; //1A
            }
            else
            {
                if(uModSet.REG.U16_CmdWd.BIT.u16_TurnOnOff == 0)
                {
                    f32_IoutRefLimit = 0.0f; //0A
                    if(stOutputCtrl.f32_IoutRef <= f32_IoutRefLimit)
                    {
                        u16_TurnOffEn = 1;
                    }
                }
                else
                {
                    f32_IoutRefLimit = stOutputCtrl.f32_IoutRefCmd;
                }
                f32_VoutRefLimit = stOutputCtrl.f32_VoutRefCmd;
            }

            if(stOutputCtrl.f32_VoutRef < (f32_VoutRefLimit - stOutputCtrl.f32_VoutStep))
            {
               stOutputCtrl.f32_VoutRef += stOutputCtrl.f32_VoutStep;
            }
            else if(stOutputCtrl.f32_VoutRef > (f32_VoutRefLimit + stOutputCtrl.f32_VoutStep))
            {
               stOutputCtrl.f32_VoutRef -= stOutputCtrl.f32_VoutStep;
            }
            else
            {
               stOutputCtrl.f32_VoutRef = f32_VoutRefLimit;
            }

            if((stOutputCtrl.f32_IoutRef < (f32_IoutRefLimit - stOutputCtrl.f32_IoutStep)))
            {
                stOutputCtrl.f32_IoutRef += stOutputCtrl.f32_IoutStep;
            }
            else if(stOutputCtrl.f32_IoutRef > (f32_IoutRefLimit + stOutputCtrl.f32_IoutStep))
            {
                stOutputCtrl.f32_IoutRef -= stOutputCtrl.f32_IoutStep;
            }
            else
            {
                stOutputCtrl.f32_IoutRef = f32_IoutRefLimit;
            }

        }


        stCtrlPiVout.f32_Ref = stOutputCtrl.f32_VoutRef;
        stCtrlPiVout.f32_Err = stCtrlPiVout.f32_Ref - stCtrlPiVout.f32_Fbk;
        stCtrlPiIdc.f32_Ref = stOutputCtrl.f32_IoutRef;
        stCtrlPiIdc.f32_Fbk = stSampleCal.f32_Output_I;
        stCtrlPiIdc.f32_Err = stCtrlPiIdc.f32_Ref - stCtrlPiIdc.f32_Fbk;

        CTRL_PI(stCtrlPiVout);
        CTRL_PI(stCtrlPiIdc);

        if(stCtrlPiVout.f32_Uo <= stCtrlPiIdc.f32_Uo)
        {
            stPSFBCmpr.f32_Winner = stCtrlPiVout.f32_Uo;
            if(stOutputCtrl.f32_VoutRefCmd != f32_VoutRefCmd_Last)
            {
                if(stSuper.u16_SysStatus.BIT.OpPreChargeEn == 0)
                {
                    stOutputCtrl.f32_IoutRef = 0.0f;
                    stCtrlPiIdc.f32_Ref = 0.0f;
                    stCtrlPiIdc.f32_Fbk = 0.0f;
                    stCtrlPiIdc.f32_Err = 0.0f;
                    stCtrlPiIdc.f32_Err0 = 0.0f;
                    stCtrlPiIdc.f32_Out = 0.0f;
                    stCtrlPiIdc.f32_Ui = 0.0f;
                    stCtrlPiIdc.f32_Uo = 0.0f;
                    stCtrlPiIdc.f32_Up = 0.0f;
                }
            }
        }
        else
        {
            stPSFBCmpr.f32_Winner = stCtrlPiIdc.f32_Uo;
        }
        stPSFBCmpr.f32_Td = stPSFBCmpr.f32_Winner;
        stPSFBCmpr.i16_Td = ((int16_t)(stPSFBCmpr.f32_Td * stPSFBCmpr.u16_PeriodCnt))>>1;

        //Open loop
        if(stSuper.u16_SysStatus.BIT.OpenLoopEn == 1)
        {
            if(stPSFBCmpr.f32_Td_OpenLoop > cPhaseShiftMax)
            {
                stPSFBCmpr.f32_Td_OpenLoop = cPhaseShiftMax;
            }
            stPSFBCmpr.i16_Td = ((int16_t)(stPSFBCmpr.f32_Td_OpenLoop * stPSFBCmpr.u16_PeriodCnt))>>1;
        }

        mSetPhase(stPSFBCmpr.i16_Td);
        f32_VoutRefCmd_Last = stOutputCtrl.f32_VoutRefCmd;
    }
}




