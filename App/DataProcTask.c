/*******************************************************************************
 * File Name: DataProcTask.c
 * Description: The communication data processing in database.
 * Designer: Miller Xue
 * Version: 00
 * Copyright: Liyuan Haina Co.Ltd
 *******************************************************************************/
#include "PSFBController.h"
#include "App.h"
#include "F28x_Project.h"
#include "Gpio_User.h"
#include "RTOS_APP.h"
#include "RTOS_Define.h"
#include "math.h"
#include "stdio.h"
#include "DataProcTask.h"
#include "SpiEeprom_User.h"
#include "ModbusRtu.h"
#include "SuperTask.h"
#include "MeasureTask.h"
#include "ShareData.h"

void sDataProcTask(void);
void sExtCtrlCmdProc(void);
void sUpdateStatus(void);
void sExtCtrlSettingProc(void);
void sExtCalCmdProc(void);
void sUpdateCtrlParaEeprom(void);
void sUpdateCtrlPara(void);
void sResetCtrlPara(void);

uint32_t u32_FaultFlag = 0;
uint16_t u16_ClearErrFlag = 0;
void sDataProcTask(void)
{
    uint16_t event;

    event = suwRTOSGetEvent(cPrioDataProc);     //Get the current task event

    if(event & ((uint16_t)1 << eDataProcTimerEvt))
    {
        sUpdateStatus();
        sExtCtrlCmdProc();
        sExtCtrlSettingProc();
        //sExtCalCmdProc();
        sUpdateCtrlPara();

    }

    if(event & ((uint16_t)1 << cBootEvt))
    {
        if ((stSuper.u16_SysMode != cRunMode) &&
            (stSuper.u16_SysMode != cSoftStartMode))
        {
            DINT;
            DRTM;
            //CanbRegs.CAN_CTL.bit.Init = 1;
            //Interrupt_initModule();
            //((void (*)(void))0x80000)();
            EALLOW;
            WdRegs.WDKEY.all = 0x0055;
            EDIS;
        }
    }

}


void sExtCtrlCmdProc(void)
{
    //int32_t i32_Temp = 0;

    if(1)
    {
        if(uModSet.REG.U16_CmdWd.BIT.u16_TurnOnOff == 1)
        {
            if(stSuper.u16_SysMode == cStandbyMode)
            {
               sRTOSEventSend(cPrioSuper,eSuperTurnOnEvt);   //send the turn on cmd
            }
        }
        else
        {
            sRTOSEventSend(cPrioSuper,eSuperTurnOffEvt);
        }

        if(uModSet.REG.U16_CmdWd.BIT.u16_ClearErr == 1)
        {
            uModSet.REG.U16_CmdWd.BIT.u16_ClearErr = 0;
            u16_ClearErrFlag = 1;
            if(stSuper.u16_SysMode == cFaultMode)
            {
                uModSet.REG.U16_CmdWd.BIT.u16_TurnOnOff = 0;
                sRTOSEventSend(cPrioSuper,eSuperClearFaultEvt);   //send the turn on cmd
            }
        }

       /* if(uModSet.REG.U16_CmdWd.BIT.u16_IapEN == 1)
        {
            uModSet.REG.U16_CmdWd.BIT.u16_IapEN = 0;
            if(stSuper.u16_SysMode == cStandbyMode)
            {
                DINT;
                DRTM;
                //Interrupt_initModule();
                ((void (*)(void))0x80000)();
            }
        }

        if(uModSet.REG.U16_CmdWd.BIT.u16_ResetCtrlPara == 1)
        {
            uModSet.REG.U16_CmdWd.BIT.u16_ResetCtrlPara = 0;
            if(stSuper.u16_SysMode == cStandbyMode)
            {
                sResetCtrlPara();
            }
        }*/

        if(stSuper.u16_SysStatus.BIT.PwmStatus == 0)
        {
            DINT;
            stSuper.u16_SysStatus.BIT.OpenLoopEn = uModSet.REG.U16_CmdWd.BIT.u16_DcOpenLoopEn;
            EINT;
        }

        /*if(uModSet.REG.U16_CmdWd.BIT.u16_DataSave == 1)
        {
            uModSet.REG.U16_CmdWd.BIT.u16_DataSave = 0;
            if(1)
            {
                stEepromUnitData.Ctrl_Ratio_Flag = 1;
                stEepromUnitData.u32_CurrLoopK1 = uModSet.REG.u32_CurrLoopK1.value;
                stEepromUnitData.u32_CurrLoopK2 = uModSet.REG.u32_CurrLoopK2.value;
                stEepromUnitData.u32_CurrLoopK3 = uModSet.REG.u32_CurrLoopK3.value;
                stEepromUnitData.u32_CurrLoopK4 = uModSet.REG.u32_CurrLoopK4.value;

                stEepromUnitData.u32_VoltLoopK1 = uModSet.REG.u32_VoltLoopK1.value;
                stEepromUnitData.u32_VoltLoopK2 = uModSet.REG.u32_VoltLoopK2.value;
                stEepromUnitData.u32_VoltLoopK3 = uModSet.REG.u32_VoltLoopK3.value;
                stEepromUnitData.u32_VoltLoopK4 = uModSet.REG.u32_VoltLoopK4.value;

                stEepromUnitData.Ctrl_Ratio_K1 = uModSet.REG.u32_KResv[0].value;
                stEepromUnitData.Ctrl_Ratio_K2 = uModSet.REG.u32_KResv[1].value;
                stEepromUnitData.Ctrl_Ratio_K3 = uModSet.REG.u32_KResv[2].value;
                stEepromUnitData.Ctrl_Ratio_K4 = uModSet.REG.u32_KResv[3].value;
                stEepromUnitData.Ctrl_Ratio_K5 = uModSet.REG.u32_KResv[4].value;
                stEepromUnitData.Ctrl_Ratio_K6 = uModSet.REG.u32_KResv[5].value;
                stEepromUnitData.Ctrl_Ratio_K7 = uModSet.REG.u32_KResv[6].value;
                stEepromUnitData.Ctrl_Ratio_K8 = uModSet.REG.u32_KResv[7].value;
                stEepromUnitData.Ctrl_Ratio_K9 = uModSet.REG.u32_KResv[8].value;
                stEepromUnitData.Ctrl_Ratio_K10 = uModSet.REG.u32_KResv[9].value;
            }
            WriteAllData();
        }*/
    }

}

void sUpdateStatus(void)
{
    //uModSta.REG.u16_Vbus = stMeasure.u16_Bus_V_Avg;
    uModSta.REG.u16_Vout = stMeasure.u16_Output_V_Avg;
    uModSta.REG.u16_DcFaultCode = stSuper.u32_FaultCode.all;
    uModSta.REG.u16_DcFwVersion = cSW_VERSION;
    uModSta.REG.u16_DcStatus = stSuper.u16_SysStatus.all;
    uModSta.REG.u16_DcWorkMode = stSuper.u16_SysMode;
    uModSta.REG.u16_Iout = stMeasure.u16_Output_I_Avg;
    uModSta.REG.i16_Temp2 = stMeasure.u16_SC_V_Avg;//stMeasure.i16_Temperature[2];

    //Check DCDC Fault
    if((uModSta.REG.u16_DcFaultCode != 0) && (uModSet.REG.U16_CmdWd.BIT.u16_TurnOnOff == 1))
    {
        uModSet.REG.U16_CmdWd.BIT.u16_TurnOnOff = 0;
    }
}

void sExtCtrlSettingProc(void)
{
    float f32_Temp1,f32_Temp2;
    DINT;
    stOutputCtrl.f32_VoutRefCmd = uModSet.REG.u16_VoutSet * 1.0f;
    //stOutputCtrl.f32_IoutRefCmd = uModSet.REG.u16_IoutSet * 1.0f;
    stPSFBCmpr.f32_Td_OpenLoop = uModSet.REG.u16_DcOpenPs * 0.01f;
    stCtrlPiVout.f32_Kp = uModSet.REG.u32_CtrlK1.value * 0.00001f;
    stCtrlPiVout.f32_Ki = uModSet.REG.u32_CtrlK2.value * 0.00001f;
    stCtrlPiIdc.f32_Kp = uModSet.REG.u32_CtrlK3.value * 0.00001f;
    stCtrlPiIdc.f32_Ki = uModSet.REG.u32_CtrlK4.value * 0.00001f;
    EINT;

    f32_Temp1 = stMeasure.u16_Output_V_Avg * 0.1f;
    f32_Temp2 = uModSet.REG.u16_IoutSet * 1.0f;
    if(f32_Temp2 > cDefault_Iout)
    {
        f32_Temp2 = cDefault_Iout;
    }
    if(f32_Temp1 > 0)
    {
        f32_Temp1 = 3000.0f / f32_Temp1;
        if(f32_Temp1 >= cDefault_Iout)
        {
            DINT;
            stOutputCtrl.f32_IoutRefCmd = f32_Temp2;
            EINT;
        }
        else
        {
            DINT;
            stOutputCtrl.f32_IoutRefCmd = f32_Temp1;
            EINT;
        }
    }
    else
    {
        DINT;
        stOutputCtrl.f32_IoutRefCmd = f32_Temp2;
        EINT;
    }
}

void sExtCalCmdProc(void)
{
    //uint16_t u16_Temp,i;
    //uint16_t u16_CalPointNum;

    /*if(uModSet.REG.u16_EnCalType == 1)
    {
        if(stSuper.u16_SysMode == cStandbyMode)
        {
            stOutputCtrl.u16_OutputMode = 0;        //CC mode
        }
        u16_Temp = (uModSet.REG.u16_RunCal>>8) &0xFF;
        if(u16_Temp != 0)
        {
            stIoutCal.CalibratePointFlag |= ((uint16_t)0x1<<(u16_Temp - 1));
            if((uModSet.REG.u16_RunCal & 0x0F) == 0x01)
            {
                DINT;
                stIoutCal.CalibrateFlag = 0;
                //stOutputCtrl.f32_IoutStep = 0.1f;
                stOutputCtrl.f32_IoutRefCmd = (float)(*(uint32_t *)(&uModSet.REG.u32_RefValCCal1.value + (u16_Temp - 1)*2)) * 0.01f;
                EINT;
                if(stSuper.u16_SysMode == cStandbyMode)
                {
                    sRTOSEventSend(cPrioSuper,eSuperTurnOnEvt);   //send the turn on cmd
                    sIoutCalReset();
                }

            }
        }
        if((uModSet.REG.u16_RunCal & 0x0F) == 0x02)        //calibrate finished
        {
            uModSet.REG.u16_RunCal = 0;
            uModSet.REG.u16_EnCalType = 0;
            u16_CalPointNum = 0;
            for(i=0;i<16;i++)
            {
                if(stIoutCal.CalibratePointFlag & ((uint16_t)0x1<<i))
                {
                    u16_CalPointNum++;
                }
                else
                {
                    break;
                }
            }
            sCalCoeffCalculate(&stIoutCal,u16_CalPointNum,&uModSet.REG.u32_RefValCCal1.value);
            if(stIoutCal.CalibrateFlag == 1)
            {
                stEepromUnitData.Cal_RefValue_I1 = uModSet.REG.u32_RefValCCal1.value;
                stEepromUnitData.Cal_ActValue_I1 = uModSet.REG.u32_ActValCCal1.value;
                stEepromUnitData.Cal_RefValue_I2 = uModSet.REG.u32_RefValCCal2.value;
                stEepromUnitData.Cal_ActValue_I2 = uModSet.REG.u32_ActValCCal2.value;
                stEepromUnitData.Cal_RefValue_I3 = uModSet.REG.u32_RefValCCal3.value;
                stEepromUnitData.Cal_ActValue_I3 = uModSet.REG.u32_ActValCCal3.value;
                stEepromUnitData.Cal_RefValue_I4 = uModSet.REG.u32_RefValCCal4.value;
                stEepromUnitData.Cal_ActValue_I4 = uModSet.REG.u32_ActValCCal4.value;
                stEepromUnitData.Cal_I_Num_Points = stIoutCal.CalibrateNum + 1;
                stEepromUnitData.Cal_Parm_Flag = 1;
                uModSet.REG.U16_CmdWd.BIT.u16_DataSave = 1;
            }

    }
    else if(uModSet.REG.u16_EnCalType == 2)
    {
        if(stSuper.u16_SysMode == cStandbyMode)
        {
            stOutputCtrl.u16_OutputMode = 1;        //CV mode
        }
        u16_Temp = (uModSet.REG.u16_RunCal>>8) &0xFF;
        if(u16_Temp != 0)
        {
            stVoutCal.CalibratePointFlag |= ((uint16_t)0x1<<(u16_Temp - 1));
            if((uModSet.REG.u16_RunCal & 0x0F) == 0x01)
            {
                DINT;
                stVoutCal.CalibrateFlag = 0;
                //stOutputCtrl.f32_VoutStep = 1.0f;
                stOutputCtrl.f32_VoutRefCmd = (float)(*(uint32_t *)(&uModSet.REG.u32_RefValUCal1.value + (u16_Temp - 1)*2)) * 0.01f;
                EINT;
                if(stSuper.u16_SysMode == cStandbyMode)
                {
                    sRTOSEventSend(cPrioSuper,eSuperTurnOnEvt);   //send the turn on cmd
                    sVoutCalReset();
                }
            }
        }
        if((uModSet.REG.u16_RunCal & 0x0F) == 0x02)
        {
            uModSet.REG.u16_RunCal = 0;
            uModSet.REG.u16_EnCalType = 0;
            u16_CalPointNum = 0;
            for(i=0;i<16;i++)
            {
                if(stVoutCal.CalibratePointFlag & ((uint16_t)0x1<<i))
                {
                    u16_CalPointNum++;
                }
                else
                {
                    break;
                }
            }
            sCalCoeffCalculate(&stVoutCal,u16_CalPointNum,&uModSet.REG.u32_RefValUCal1.value);

            if(stVoutCal.CalibrateFlag == 1)
            {
                stEepromUnitData.Cal_RefValue_V1 = uModSet.REG.u32_RefValUCal1.value;
                stEepromUnitData.Cal_ActValue_V1 = uModSet.REG.u32_ActValUCal1.value;
                stEepromUnitData.Cal_RefValue_V2 = uModSet.REG.u32_RefValUCal2.value;
                stEepromUnitData.Cal_ActValue_V2 = uModSet.REG.u32_ActValUCal2.value;
                stEepromUnitData.Cal_RefValue_V3 = uModSet.REG.u32_RefValUCal3.value;
                stEepromUnitData.Cal_ActValue_V3 = uModSet.REG.u32_ActValUCal3.value;
                stEepromUnitData.Cal_RefValue_V4 = uModSet.REG.u32_RefValUCal4.value;
                stEepromUnitData.Cal_ActValue_V4 = uModSet.REG.u32_ActValUCal4.value;
                stEepromUnitData.Cal_V_Num_Points = stVoutCal.CalibrateNum + 1;
                stEepromUnitData.u16_Recv1 = 0;
                stEepromUnitData.u16_Recv2 = 0;
                stEepromUnitData.u16_Recv3 = 0;
                stEepromUnitData.u16_Recv4 = 0;
                stEepromUnitData.Cal_Parm_Flag1 = 1;
                uModSet.REG.U16_CmdWd.BIT.u16_DataSave = 1;
            }
        }
    }


    if((uModSet.REG.u16_RunCal & 0x0F) == 0x03)
    {
        uModSet.REG.u16_EnCalType = 0;
        uModSet.REG.u16_RunCal = 0;
        sRTOSEventSend(cPrioSuper,eSuperTurnOffEvt);
        sVoutCalReset();
        sIoutCalReset();
        stEepromUnitData.Cal_Parm_Flag = 0;
        stEepromUnitData.Cal_Parm_Flag1 = 0;
        uModSet.REG.U16_CmdWd.BIT.u16_DataSave = 1;
    }
    */
}

void sUpdateCtrlParaEeprom(void)
{
    /*uModSet.REG.u32_CurrLoopK1.value = stEepromUnitData.u32_CurrLoopK1;
    uModSet.REG.u32_CurrLoopK2.value = stEepromUnitData.u32_CurrLoopK2;
    uModSet.REG.u32_CurrLoopK3.value = stEepromUnitData.u32_CurrLoopK3;
    uModSet.REG.u32_CurrLoopK4.value = stEepromUnitData.u32_CurrLoopK4;

    uModSet.REG.u32_VoltLoopK1.value = stEepromUnitData.u32_VoltLoopK1;
    uModSet.REG.u32_VoltLoopK2.value = stEepromUnitData.u32_VoltLoopK2;
    uModSet.REG.u32_VoltLoopK3.value = stEepromUnitData.u32_VoltLoopK3;
    uModSet.REG.u32_VoltLoopK4.value = stEepromUnitData.u32_VoltLoopK4;

    uModSet.REG.u32_KResv[0].value = stEepromUnitData.Ctrl_Ratio_K1;
    uModSet.REG.u32_KResv[1].value = stEepromUnitData.Ctrl_Ratio_K2;
    uModSet.REG.u32_KResv[2].value = stEepromUnitData.Ctrl_Ratio_K3;
    uModSet.REG.u32_KResv[3].value = stEepromUnitData.Ctrl_Ratio_K4;
    uModSet.REG.u32_KResv[4].value = stEepromUnitData.Ctrl_Ratio_K5;
    uModSet.REG.u32_KResv[5].value = stEepromUnitData.Ctrl_Ratio_K6;
    uModSet.REG.u32_KResv[6].value = stEepromUnitData.Ctrl_Ratio_K7;
    uModSet.REG.u32_KResv[7].value = stEepromUnitData.Ctrl_Ratio_K8;
    uModSet.REG.u32_KResv[8].value = stEepromUnitData.Ctrl_Ratio_K9;
    uModSet.REG.u32_KResv[9].value = stEepromUnitData.Ctrl_Ratio_K10;*/

    //以下更新到对应的控制参数
    DINT;
    /*stInvCtrl.KvSet = uModSet.REG.u32_VoltLoopK1.value * float_Para_Ratio;
    stInvCtrl.Ki_1 = uModSet.REG.u32_CurrLoopK1.value * float_Para_Ratio;
    stInvCtrl.stInvCurrRmsPid.f32_Kp = uModSet.REG.u32_KResv[0].value * float_Para_Ratio;
    stInvCtrl.stInvCurrRmsPid.f32_Ki = uModSet.REG.u32_KResv[1].value * float_Para_Ratio;
    stInvCtrl.stInvVoltRmsPid.f32_Kp = uModSet.REG.u32_KResv[2].value * float_Para_Ratio;
    stInvCtrl.stInvVoltRmsPid.f32_Ki = uModSet.REG.u32_KResv[3].value * float_Para_Ratio;
    stInvCtrl.KRctrl = uModSet.REG.u32_KResv[4].value * float_Para_Ratio;;
    stInvCtrl.KRctrl1 = uModSet.REG.u32_KResv[5].value * float_Para_Ratio;
    stInvCtrl.KRctrl2 = uModSet.REG.u32_KResv[6].value * float_Para_Ratio;
    stInvCtrl.InvDummyRes = uModSet.REG.u32_KResv[7].value * float_Para_Ratio;
    stInvCtrl.stInvCurrPid.f32_Ki = uModSet.REG.u32_CurrLoopK2.value * float_Para_Ratio;
    stInvCtrl.stInvCurrPid.f32_Kp = stInvCtrl.Ki_1;*/
    EINT;


}

void sUpdateCtrlPara(void)
{
    //uint16_t u16_Temp;
    //stCtrlPiPll.f32_Kp = uModSet.REG.i32_CtrlK1.value * 0.0001f;
    //stCtrlPiPll.f32_Ki = uModSet.REG.i32_CtrlK2.value * 0.0001f;
    DINT;
    /*stInvCtrl.KvSet = uModSet.REG.u32_VoltLoopK1.value * float_Para_Ratio;
    stInvCtrl.Ki_1 = uModSet.REG.u32_CurrLoopK1.value * float_Para_Ratio;
    stInvCtrl.stInvCurrRmsPid.f32_Kp = uModSet.REG.u32_KResv[0].value * float_Para_Ratio;
    stInvCtrl.stInvCurrRmsPid.f32_Ki = uModSet.REG.u32_KResv[1].value * float_Para_Ratio;
    stInvCtrl.stInvVoltRmsPid.f32_Kp = uModSet.REG.u32_KResv[2].value * float_Para_Ratio;
    stInvCtrl.stInvVoltRmsPid.f32_Ki = uModSet.REG.u32_KResv[3].value * float_Para_Ratio;
    stInvCtrl.stInvCurrPid.f32_Ki = uModSet.REG.u32_CurrLoopK2.value * float_Para_Ratio;
    stInvCtrl.stInvCurrPid.f32_Kp = stInvCtrl.Ki_1;
    stInvCtrl.KRctrl = uModSet.REG.u32_KResv[4].value * float_Para_Ratio;;
    stInvCtrl.KRctrl1 = uModSet.REG.u32_KResv[5].value * float_Para_Ratio;
    stInvCtrl.KRctrl2 = uModSet.REG.u32_KResv[6].value * float_Para_Ratio;
    stInvCtrl.InvDummyRes = uModSet.REG.u32_KResv[7].value * float_Para_Ratio;
    f32_OpenLoopDuty = uModSet.REG.u32_CurrLoopK3.value * float_Para_Ratio;

    u16_Temp = uModSet.REG.u32_KResv[9].value * float_Para_Ratio;*/
    EINT;
}

void sResetCtrlPara(void)
{
    uModSet.REG.u32_CtrlK1.value = 10000;
    uModSet.REG.u32_CtrlK2.value = 1000;
    uModSet.REG.u32_CtrlK3.value = 2000;
    uModSet.REG.u32_CtrlK4.value = 100;

//    DINT;
//    stCtrlPiVout.f32_Kp = uModSet.REG.u32_CtrlK1.value * 0.00001f;
//    stCtrlPiVout.f32_Ki = uModSet.REG.u32_CtrlK2.value * 0.00001f;
//    stCtrlPiIdc.f32_Kp = uModSet.REG.u32_CtrlK3.value * 0.00001f;
//    stCtrlPiIdc.f32_Ki = uModSet.REG.u32_CtrlK4.value * 0.00001f;
//    EINT;

}



