/*******************************************************************************
 * File Name: Adc_User.c
 * Description: The ADC initialization functions.
 * Designer: Miller Xue
 * Version: 00
 * Copyright: Liyuan Haina Co.Ltd
 *******************************************************************************/
#ifndef _ADC_USER_H
#define _ADC_USER_H

typedef struct {
    int16_t i16_SC_V;
    int16_t i16_L_I;
    int16_t i16_Output_I;
    int16_t i16_Output_V;
    int16_t i16_Adc_24V;

    uint16_t u16_Temperature_1;
    uint16_t u16_Temperature_2;
    uint16_t u16_Temperature_3;
    uint16_t u16_Temperature_4;
    uint16_t u16_Temperature_5;
    uint16_t u16_Temperature_6;

    uint16_t u16_SC_V_Mid;
    uint16_t u16_L_I_Mid;
    uint16_t u16_Output_I_Mid;
    uint16_t u16_Output_V_Mid;

    uint32_t u32_L_I_MidCalSumTemp;
    uint32_t u32_Output_I_MidCalSumTemp;
    uint32_t u32_Output_V_MidCalSumTemp;

    uint16_t u16_MidCalFlag;
    uint16_t u16_MidCalCnt;
}ADC_SAMPLE;


#define   mAdc_SC_V_0()             AdcaResultRegs.ADCRESULT1
#define   mAdc_SC_V_1()             AdcaResultRegs.ADCRESULT7

#define   mAdc_L_I_0()              AdcbResultRegs.ADCRESULT0
#define   mAdc_L_I_1()              AdcbResultRegs.ADCRESULT1
#define   mAdc_L_I_2()              AdcbResultRegs.ADCRESULT2
#define   mAdc_L_I_3()              AdcbResultRegs.ADCRESULT3


#define   mAdc_Output_I_0()         AdcbResultRegs.ADCRESULT0
#define   mAdc_Output_I_1()         AdcbResultRegs.ADCRESULT1
#define   mAdc_Output_I_2()         AdcbResultRegs.ADCRESULT2
#define   mAdc_Output_I_3()         AdcbResultRegs.ADCRESULT3

#define   mAdc_Output_V_0()         AdcaResultRegs.ADCRESULT0
#define   mAdc_Output_V_1()         AdcaResultRegs.ADCRESULT6
#define   mAdc_Output_V_2()         AdcaResultRegs.ADCRESULT12
#define   mAdc_Output_V_3()         AdcaResultRegs.ADCRESULT13

#define   mAdc_Temperature3()       AdcaResultRegs.ADCRESULT3
#define   mAdc_Temperature4()       AdcaResultRegs.ADCRESULT4
#define   mAdc_Temperature5()       AdcaResultRegs.ADCRESULT5

#define   mAdc_24V()                AdcbResultRegs.ADCRESULT2

#define   cAdcMidCalN               15
#define   cAdcMidCalCntSet          ((uint16_t)1<<cAdcMidCalN)



extern void sAdcInit(void);
extern void sAdcCpuSample(void);

extern ADC_SAMPLE stAdcSample;

#endif

