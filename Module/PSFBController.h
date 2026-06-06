/*******************************************************************************
 * File Name: InvController.h
 * Description: The PFC controller functions.
 * Designer: Miller Xue
 * Version: 00
 * Copyright: Liyuan Haina Co.Ltd
 *******************************************************************************/
#ifndef _INVCONTROLLER_H
#define _INVCONTROLLER_H
#include "F28x_Project.h"
#include "stdlib.h"

#define  cSC_V_Ratio            0.200244f
#define  cL_I_Ratio              0.6628855f
#define  cOutput_I_Ratio         0.003663f
#define  cOutput_V_Ratio         0.200244f

#define     DSP_CPU_CLK          120.0                 //MHz
#define     PSFB_Fs              60                   //kHz
#define     Fs_Ctrl              60                    //kHz

#define   PI                3.14159265358979f
#define   PI2               6.28318530717958f
#define   TAN60             1.73205080756888f
#define   COS30             0.86602540378444f
//#define   COS30             14189
#define   COS60             0.5f
//#define   COS60             8192

#define   INV_SQRT3         0.57735026918963f   // 1/sqrt(3) = 0.57735026918963
#define   SQRT2             1.414213562373f     // sqrt(2) = 1.4142135623730950488016887242097
#define   INV_SQRT2         0.7071067812f   // 1/sqrt(2) = 0.70710678118654752440084436210485
#define   ONE_THIRD         0.33333333333333f
#define   TWO_THIRD         0.66666666666667f
#define   PFC_L             90.0f              //uH
#define   GRID_L            16.0f               //uH
#define   FLT_C             15.0f               //uF
#define   DUMMY_R           2.0f

#define   cSPS              0
#define   cDPS              1
#define   cTPS              2

#define   cDefault_Vout     712.0f
#define   cDefault_Iout     5.0f
#define   cDefault_Pout     3000.0f

#define   cPhaseShiftMax    0.95f

//#define   cBusRefCmd        f32_PfcBusRefCmd

/**************************************************************************
 *                      Variable Declaration
 *************************************************************************/

typedef struct {
    float f32_Ref;
    float f32_Fbk;
    float f32_A0;
    float f32_A1;
    float f32_B0;
    float f32_B1;
    float f32_B2;
    float f32_x0;
    float f32_x1;
    float f32_x2;
    float f32_y;
    float f32_y0;
    float f32_y1;
    float f32_Max;
    float f32_Min;
    float f32_Pid_Err_Max;
    float f32_Pid_Err_Min;
    float f32_Temp;
}CTRL_2P2Z_ST;

typedef struct {
    float f32_Ref;
    float f32_Fbk;

    float f32_Kp;
    float f32_Ki;

    float f32_Err;
    float f32_Err0;

    float f32_Up;
    float f32_Ui;

    float f32_Uo;
    float f32_Out;
    float f32_Max;
    float f32_Min;
}CTRL_PI_ST;

typedef struct {
    int16_t i16_Td;
    int16_t i16_Td1;
    int16_t i16_Td2;
    int16_t i16_Tdd;
    uint16_t u16_PeriodCnt;
    float f32_Winner;
    float f32_Td;
    float f32_Td1;
    float f32_Td2;
    float f32_Tdd;
    float f32_Td_OpenLoop;
    float f32_Td1_OpenLoop;
    float f32_Td2_OpenLoop;
    uint16_t u16_WinnerFlag1;
    uint16_t u16_PsType;
}CMPRPSFB_ST;

typedef struct {

    float f32_SC_V;
    float f32_L_I;
    float f32_Output_I;
    float f32_Output_V;
    float f32_T3;
    float f32_T4;
    float f32_T5;

    float f32_SC_V_SumTemp;
    float f32_L_I_SumTemp;
    float f32_Output_I_SumTemp;
    float f32_Output_V_SumTemp;
    float f32_Power_Active_SumTemp;
    float f32_T3_SumTemp;
    float f32_T4_SumTemp;
    float f32_T5_SumTemp;

    float f32_SC_V_Sum;
    float f32_L_I_Sum;
    float f32_Output_I_Sum;
    float f32_Output_V_Sum;
    float f32_Power_Active_Sum;
    float f32_T3_Sum;
    float f32_T4_Sum;
    float f32_T5_Sum;


    float f32_Temperature;
    float f32_Temperature_SumTemp;
    float f32_Temperature_Sum;

    uint16_t u16_SumCnt;
    uint16_t u16_SumCntSet;

}SAMPLE_CAL;



#define CTRL_2P2Z(v)                     \
    v.f32_Temp =  v.f32_B0 * v.f32_x0;   \
    v.f32_Temp += v.f32_B1  * v.f32_x1;  \
    v.f32_Temp += v.f32_B2 * v.f32_x2;   \
    v.f32_Temp += v.f32_A0 * v.f32_y0;   \
    v.f32_Temp += v.f32_A1 * v.f32_y1;   \
    v.f32_x2 = v.f32_x1;     \
    v.f32_y1 = v.f32_y0;     \
    v.f32_y0 = v.f32_Temp;   \
    if(v.f32_y0 >= v.f32_Max)     \
    {v.f32_y0 = v.f32_Max;}  \
    else if(v.f32_y0 <= v.f32_Min)                  \
    {v.f32_y0 = v.f32_Min;}

#define CTRL_PI(v)          \
    v.f32_Up =  (v.f32_Err - v.f32_Err0) * v.f32_Kp; \
    v.f32_Ui = v.f32_Err * v.f32_Ki;                 \
    v.f32_Uo += v.f32_Up + v.f32_Ui;                 \
    v.f32_Err0 = v.f32_Err;                          \
    if(v.f32_Uo >= v.f32_Max)                        \
    {v.f32_Uo = v.f32_Max;}                          \
    else if(v.f32_Uo <= v.f32_Min)                   \
      {v.f32_Uo = v.f32_Min;}







//extern void sNotchFilterParaUpdate(int32 dwFreqAvg);
void sSampelCalculation(void);
extern void sPSFBControllerInit(void);
extern void sPSFBControllerReset(void);
extern void sInvGridTieController(void);
extern void sPSFBController(void);

extern SAMPLE_CAL stSampleCal;
extern uint16_t u16_OpenLoop;
extern uint16_t u16_PwmNeedOn;
extern uint16_t u16_PwmOnWaitCnt;
extern float f32_OpenLoopDuty;
extern CTRL_PI_ST   stCtrlPiVout;
extern CTRL_PI_ST   stCtrlPiIdc;
extern CMPRPSFB_ST stPSFBCmpr;
extern uint16_t u16_TurnOffEn;

#endif
