/*
 * ShareData.h
 *
 *  Created on: 2024Äê10ÔÂ2ÈÕ
 *      Author: Miller
 */

#ifndef MODULE_SHAREDATA_H_
#define MODULE_SHAREDATA_H_



typedef struct {
    int16_t i16_Buck_I1;
    int16_t i16_Buck_I2;
    int16_t i16_Buck_I3;
    int16_t i16_Output_I;
    int16_t i16_Output_V;

    uint16_t u16_Temperature_1;
    uint16_t u16_Temperature_2;
    uint16_t u16_Temperature_3;
    uint16_t u16_Temperature_4;
    uint16_t u16_Temperature_5;
    uint16_t u16_Temperature_6;

    uint16_t u16_Buck_I1_Mid;
    uint16_t u16_Buck_I2_Mid;
    uint16_t u16_Buck_I3_Mid;
    uint16_t u16_Output_I_Mid;
    uint32_t u32_Buck_I1_MidCalSumTemp;
    uint32_t u32_Buck_I2_MidCalSumTemp;
    uint32_t u32_Buck_I3_MidCalSumTemp;
    uint32_t u32_Output_I_MidCalSumTemp;

    uint16_t u16_MidCalFlag;
    uint16_t u16_MidCalCnt;
}ADC_SAMPLE_CLA;

typedef struct {
    float f32_Buck_I1;
    float f32_Buck_I2;
    float f32_Buck_I3;
    float f32_Output_I;
    float f32_Output_V;

    float f32_Buck_I1_SumTemp;
    float f32_Buck_I2_SumTemp;
    float f32_Buck_I3_SumTemp;
    float f32_Output_I_SumTemp;
    float f32_Output_V_SumTemp;
    float f32_Buck_I1_Sum;
    float f32_Buck_I2_Sum;
    float f32_Buck_I3_Sum;
    float f32_Output_I_Sum;
    float f32_Output_V_Sum;

    uint16_t u16_SumCntCla;
    uint16_t u16_SumCntSetCla;
    uint16_t u16_DcSumStatus;

}SAMPLE_CAL_CLA;

typedef struct {
    uint16_t u16_PwmCmpr1;
    uint16_t u16_PwmCmpr2;
    uint16_t u16_PwmCmpr3;
    uint16_t u16_PeriodCnt;
    float f32_DcDuty1;
    float f32_DcDuty2;
    float f32_DcDuty3;
    float f32_DcDutyDiff;
}CMPRDC_ST;


typedef struct {
    float f32_PfcOpenloopDuty;
    float f32_DcOpenloopDuty;
}SYS_SETTING;

typedef struct {
    float f32_Single_Total_I;
    float f32_Cabinet_Total_I;
    uint32_t u32_Single_Total_I_SumpTemp;
    uint32_t u32_Cabinet_Total_I_SumpTemp;
}SDFM_SAMPLE_CLA;

/********************************START**********************************************
 * Modified by CPH.
 * For the calibration function.
 * ***************************************************************************/
typedef struct {
    float f32_K;
    float f32_b;
}CALCOEFF_ST;

typedef struct {
    uint16_t CalibrateFlag;
    uint16_t CalibrateNum;
    uint16_t CalibratePointFlag;
    CALCOEFF_ST CalCoff[4];
    float    CalRef[4];
}CAL_ST;
/********************************END**********************************************/


extern SAMPLE_CAL_CLA stSampleCalCla;
extern ADC_SAMPLE_CLA stSampleCla;
extern SDFM_SAMPLE_CLA stSDFMSampleCla;

extern CMPRDC_ST    stDcPwmCmpr;
extern uint16_t u16_DcOpenLoop;
extern float f32_DcOpenLoopDuty;
extern float f32_DcOpenLoopDutyCmd;
extern uint16_t u16_DcPwmNeedOn;
extern uint16_t u16_DcPwmOnWaitCnt;
extern uint16_t u16_TempChannel;
extern uint16_t u16_TempChannel_Last;
extern CAL_ST stVoutCal;
extern CAL_ST stIoutCal;
extern float f32_DroopRatio;
extern float f32_IoutAvgDroop;
extern uint16_t u16_ClearErrFlag;

extern void sShareDataInit(void);
extern void sDcControllerReset(void);
extern void sCalCoeffCalculate(CAL_ST *stCalSt,uint16_t u16_Num, uint32_t *u32_CalTab);
extern void sVoutCalReset(void);
extern void sIoutCalReset(void);


#endif /* MODULE_SHAREDATA_H_ */
