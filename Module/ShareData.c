

#include "F28x_Project.h"
#include "RTOS_APP.h"
#include "RTOS_Define.h"
#include "ShareData.h"
#include "Adc_User.h"
#include "app.h"
#include "SuperTask.h"

//#pragma DATA_SECTION(stSampleCal,"CLADataLS1");
//SAMPLE_CAL stSampleCal;
//#pragma DATA_SECTION(stSampleCla,"CLADataLS1");
//ADC_SAMPLE_CLA stSampleCla;
//#pragma DATA_SECTION(stSampleCalCla,"CLADataLS1");
//SAMPLE_CAL_CLA stSampleCalCla;
//#pragma DATA_SECTION(stSDFMSampleCla,"CLADataLS1");
//SDFM_SAMPLE_CLA stSDFMSampleCla;
//
//#pragma DATA_SECTION(stCtrlPiVout,"CLADataLS1");
//CTRL_PI_ST   stCtrlPiVout = {0};
//#pragma DATA_SECTION(stCtrlPiIdc,"CLADataLS1");
//CTRL_PI_ST   stCtrlPiIdc = {0};
//#pragma DATA_SECTION(stCtrlPiIdc1,"CLADataLS1");
//CTRL_PI_ST   stCtrlPiIdc1 = {0};
//#pragma DATA_SECTION(stCtrlPiIdc2,"CLADataLS1");
//CTRL_PI_ST   stCtrlPiIdc2 = {0};
//#pragma DATA_SECTION(stCtrlPiPout,"CLADataLS1");
//CTRL_PI_ST   stCtrlPiPout = {0};
//#pragma DATA_SECTION(stCtrlPiIdcDiff,"CLADataLS1");
//CTRL_PI_ST   stCtrlPiIdcDiff = {0};
//#pragma DATA_SECTION(stOutputCtrl,"CLADataLS1");
//OUTPUT_CTRL_ST stOutputCtrl = {0};
//#pragma DATA_SECTION(stDcPwmCmpr,"CLADataLS1");
//CMPRDC_ST    stDcPwmCmpr = {0};
//#pragma DATA_SECTION(u16_DcOpenLoop,"CLADataLS1");
//uint16_t u16_DcOpenLoop;
//#pragma DATA_SECTION(f32_DcOpenLoopDuty,"CLADataLS1");
//float f32_DcOpenLoopDuty;
//#pragma DATA_SECTION(f32_DcOpenLoopDutyCmd,"CLADataLS1");
//float f32_DcOpenLoopDutyCmd;
//#pragma DATA_SECTION(u16_DcPwmNeedOn,"CLADataLS1");
//uint16_t u16_DcPwmNeedOn;
//#pragma DATA_SECTION(u16_DcPwmOnWaitCnt,"CLADataLS1");
//uint16_t u16_DcPwmOnWaitCnt;
//#pragma DATA_SECTION(u16_TempChannel,"CLADataLS1");
//uint16_t u16_TempChannel;
//#pragma DATA_SECTION(u16_TempChannel_Last,"CLADataLS1");
//uint16_t u16_TempChannel_Last;
CAL_ST stVoutCal;
CAL_ST stIoutCal;
//#pragma DATA_SECTION(f32_DroopRatio,"CLADataLS1");
//float f32_DroopRatio;
//#pragma DATA_SECTION(f32_IoutAvgDroop,"CLADataLS1");
//float f32_IoutAvgDroop;

void sShareDataInit(void);
void sDcControllerReset(void);
void sVoutCalReset(void);
void sIoutCalReset(void);
void sCalCoeffCalculate(CAL_ST *stCalSt,uint16_t u16_Num, uint32_t *u32_CalTab);

void sShareDataInit(void)
{
    uint16_t i;


    //Default output control setting
//    stOutputCtrl.f32_VoutRefCmd = 600.0f;
//    stOutputCtrl.f32_VoutRef = 0.0f;
//    stOutputCtrl.f32_VoutStep = 0.1f;       //100V/s
//
//    stOutputCtrl.f32_IoutRefCmd = 210.0f;
//    stOutputCtrl.f32_IoutRef = 0.0f;
//    stOutputCtrl.f32_IoutStep = 0.05;       //50A/s
//
//    stOutputCtrl.f32_PoutRefCmd = 125000.0f;
//    stOutputCtrl.f32_PoutRef = 0.0f;
//    stOutputCtrl.f32_PoutStep = 1000.0f;       //1000W/ms
//
//    stOutputCtrl.u16_OutputMode = 0;          //CC
//    stOutputCtrl.u16_DcLoopCnt = 0;
//
//    stOutputCtrl.f32_DroopValue = 0.0f;
//
//    stDcPwmCmpr.u16_PeriodCnt = (uint16_t)((int32_t)CPU_CLK*1000L/Fs)>>2;
//    stDcPwmCmpr.u16_PwmCmpr1 = 0;
//    stDcPwmCmpr.u16_PwmCmpr2 = 0;
//    stDcPwmCmpr.f32_DcDuty1 = 0.0f;
//    stDcPwmCmpr.f32_DcDuty2 = 0.0f;
//    stDcPwmCmpr.f32_DcDutyDiff = 0.0f;

//    u16_DcOpenLoop = cPWM_OpenLoop_Test;
//    stSuper.u16_SysStatus.BIT.DcOpenLoopEn = u16_DcOpenLoop;
//    f32_DcOpenLoopDuty = 0.0f;
//    f32_DcOpenLoopDutyCmd = 0.5f;

//    u16_DcPwmNeedOn = 0;
//    u16_DcPwmOnWaitCnt = 0;
//    u16_TempChannel = 0;
//    u16_TempChannel_Last = 0;
//
//    stSDFMSampleCla.f32_Single_Total_I = 0.0f;
//    stSDFMSampleCla.f32_Cabinet_Total_I = 0.0f;
//    stSDFMSampleCla.u32_Single_Total_I_SumpTemp = 0;
//    stSDFMSampleCla.u32_Cabinet_Total_I_SumpTemp = 0;

/********************************START**********************************************
 * Modified by CPH.
 * For the calibration function.
 * ***************************************************************************/
    for(i=0;i<8;i++)
    {
        stVoutCal.CalCoff[i].f32_K = 1.0f;
        stVoutCal.CalCoff[i].f32_b = 0.0f;
        stIoutCal.CalCoff[i].f32_K = 1.0f;
        stIoutCal.CalCoff[i].f32_b = 0.0f;
    }
    stVoutCal.CalibrateFlag = 0;
    stVoutCal.CalibrateNum = 0;
    stVoutCal.CalibratePointFlag = 0;
    stIoutCal.CalibrateFlag = 0;
    stIoutCal.CalibrateNum = 0;
    stIoutCal.CalibratePointFlag = 0;
/********************************END**********************************************/

//    f32_DroopRatio = 0.0f;
//    f32_IoutAvgDroop = 0.0f;
}

void sDcControllerReset(void)
{
//    stCtrlPiIdc.f32_Ref = 0.0f;
//    stCtrlPiIdc.f32_Err = 0.0f;
//    stCtrlPiIdc.f32_Err0 = 0.0f;
//    stCtrlPiIdc.f32_Out = 0.0f;
//    stCtrlPiIdc.f32_Ui = 0.0f;
//    stCtrlPiIdc.f32_Uo = 0.0f;
//    stCtrlPiIdc.f32_Up = 0.0f;
//
//    stCtrlPiIdc1.f32_Ref = 0.0f;
//    stCtrlPiIdc1.f32_Err = 0.0f;
//    stCtrlPiIdc1.f32_Err0 = 0.0f;
//    stCtrlPiIdc1.f32_Out = 0.0f;
//    stCtrlPiIdc1.f32_Ui = 0.0f;
//    stCtrlPiIdc1.f32_Uo = 0.0f;
//    stCtrlPiIdc1.f32_Up = 0.0f;
//
//    stCtrlPiIdc2.f32_Ref = 0.0f;
//    stCtrlPiIdc2.f32_Err = 0.0f;
//    stCtrlPiIdc2.f32_Err0 = 0.0f;
//    stCtrlPiIdc2.f32_Out = 0.0f;
//    stCtrlPiIdc2.f32_Ui = 0.0f;
//    stCtrlPiIdc2.f32_Uo = 0.0f;
//    stCtrlPiIdc2.f32_Up = 0.0f;
//
//    stCtrlPiIdcDiff.f32_Err = 0.0f;
//    stCtrlPiIdcDiff.f32_Err0 = 0.0f;
//    stCtrlPiIdcDiff.f32_Out = 0.0f;
//    stCtrlPiIdcDiff.f32_Ui = 0.0f;
//    stCtrlPiIdcDiff.f32_Uo = 0.0f;
//    stCtrlPiIdcDiff.f32_Up = 0.0f;
//
//    stCtrlPiVout.f32_Max = 210.0f;
//    stCtrlPiVout.f32_Min = -210.0f;
//    stCtrlPiVout.f32_Ref = 0.0f;
//    stCtrlPiVout.f32_Err = 0.0f;
//    stCtrlPiVout.f32_Err0 = 0.0f;
//    stCtrlPiVout.f32_Out = 0.0f;
//    stCtrlPiVout.f32_Ui = 0.0f;
//    stCtrlPiVout.f32_Uo = 0.0f;
//    stCtrlPiVout.f32_Up = 0.0f;
//
//    stCtrlPiPout.f32_Ref = 0.0f;
//    stCtrlPiPout.f32_Err = 0.0f;
//    stCtrlPiPout.f32_Err0 = 0.0f;
//    stCtrlPiPout.f32_Out = 0.0f;
//    stCtrlPiPout.f32_Ui = 0.0f;
//    stCtrlPiPout.f32_Uo = 0.0f;
//    stCtrlPiPout.f32_Up = 0.0f;
//
//    stOutputCtrl.f32_IoutRef = 0.0f;
//    stOutputCtrl.f32_VoutRef = stSampleCalCla.f32_Output_V;
//    stOutputCtrl.f32_PoutRef = 0.0f;
//
//    f32_DcOpenLoopDuty = 0.0f;
//
//    u16_DcPwmNeedOn = 0;
//    u16_DcPwmOnWaitCnt = 0;
//
//    stOutputCtrl.f32_DroopValue = 0.0f;
}

/********************************START**********************************************
 * Modified by CPH.
 * For the calibration function.
 * ***************************************************************************/
void sVoutCalReset(void)
{
    uint16_t i;
    for(i=0;i<4;i++)
    {
        stVoutCal.CalCoff[i].f32_K = 1.0f;
        stVoutCal.CalCoff[i].f32_b = 0.0f;
    }
    stVoutCal.CalibrateFlag = 0;
    stVoutCal.CalibrateNum = 0;
    stVoutCal.CalibratePointFlag = 0;
}
void sIoutCalReset(void)
{
    uint16_t i;
    for(i=0;i<4;i++)
    {
        stIoutCal.CalCoff[i].f32_K = 1.0f;
        stIoutCal.CalCoff[i].f32_b = 0.0f;
    }
    stIoutCal.CalibrateFlag = 0;
    stIoutCal.CalibrateNum = 0;
    stIoutCal.CalibratePointFlag = 0;
}

void sCalCoeffCalculate(CAL_ST *stCalSt,uint16_t u16_Num, uint32_t *u32_CalTab)
{
    float f32_x0,f32_x1;
    float f32_y0,f32_y1;
    uint16_t i;

    if(u16_Num == 1)
    {
        /*f32_x0 = 0.0f;
        f32_y0 = 0.0f;
        f32_x1 = u16_CalTab[0] * 0.1f;
        f32_y1 = u16_CalTab[1] * 0.1f;

        if((f32_x1 != f32_x0) && (f32_y1 != f32_y0))
        {
            stCalSt->CalibrateNum = 1;
            stCalSt->CalCoff[0].f32_K = (f32_y1 - f32_y0)/(f32_x1 - f32_x0);
            stCalSt->CalCoff[0].f32_b = f32_y0 - stCalSt->CalCoff[0].f32_K * f32_x0;
            stCalSt->CalRef[0] = f32_x0;
            stCalSt->CalRef[1] = f32_x1;
            stCalSt->CalibrateFlag = 1;
        }*/
    }
    else if(u16_Num > 1)
    {
        stCalSt->CalibrateNum = 0;
        for(i=0;i<(u16_Num-1)*2;i+=2)
        {
            f32_x0 = u32_CalTab[i] * 0.01f;
            f32_y0 = u32_CalTab[i+1] * 0.01f;
            f32_x1 = u32_CalTab[i+2] * 0.01f;
            f32_y1 = u32_CalTab[i+3] * 0.01f;

            if((f32_x1 != f32_x0) && (f32_y1 != f32_y0))
            {
                stCalSt->CalCoff[i>>1].f32_K = (f32_y1 - f32_y0)/(f32_x1 - f32_x0);
                stCalSt->CalCoff[i>>1].f32_b = f32_y0 - stCalSt->CalCoff[i>>1].f32_K * f32_x0;
                stCalSt->CalRef[i>>1] = f32_x0;
                stCalSt->CalRef[(i>>1) + 1] = f32_x1;
                stCalSt->CalibrateNum += 1;
            }
            else
            {
                break;
            }
        }

        if(stCalSt->CalibrateNum >= 1)
        {
            stCalSt->CalibrateFlag = 1;
        }
    }
}
/********************************END**********************************************/
