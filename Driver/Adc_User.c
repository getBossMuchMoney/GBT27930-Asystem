/*******************************************************************************
 * File Name: Adc_User.c
 * Description: The ADC initialization functions.
 * Designer: Miller Xue
 * Version: 00
 * Copyright: Liyuan Haina Co.Ltd
 *******************************************************************************/
#include "F28x_Project.h"
#include "Adc_User.h"

ADC_SAMPLE stAdcSample;

void sAdcInit(void);
void sAdcSampleInit(void);
void sAdcCpuSample(void);
void sAdcChannelSetup(uint16_t u16_Adc,
                      uint16_t u16_ChSel[],
                      uint16_t u16_Acqps[],
                      uint16_t u16_Trigger[],
                      uint16_t u16_ContinousMode,
                      uint16_t u16_IntEn,
                      uint16_t u16_IntSel[]);


/*******************************************************************************
 * Function Name: sAdcInit
 * Description: Initialize the ADC A/B/C.
 *******************************************************************************/
void sAdcInit(void)
{
    uint16_t u16_ChSel[16];
    uint16_t u16_Acqps[16];
    uint16_t u16_Trigger[16];
    uint16_t u16_IntSel[3];

    EALLOW;
    //The maximum ADCCLK of the ADC is 50MHz, the minimum is 5MHz
    AdcaRegs.ADCCTL2.bit.PRESCALE = 6; //set ADCCLK = 120MHz/4 = 30MHz
    AdcbRegs.ADCCTL2.bit.PRESCALE = 6; //set ADCCLK divider to /4
    AdccRegs.ADCCTL2.bit.PRESCALE = 6; //set ADCCLK divider to /4

    //Set the ADC A/B/C/D working mode
    //The ADC in this DSP is external reference only.
    /*AdcSetMode(ADC_ADCA, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
    AdcSetMode(ADC_ADCB, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
    AdcSetMode(ADC_ADCC, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);*/

    //The minimum sample time is 75ns, 15*5ns = 75ns
    u16_Acqps[0] = 9;
    u16_Acqps[1] = 9;
    u16_Acqps[2] = 9;
    u16_Acqps[3] = 9;
    u16_Acqps[4] = 9;
    u16_Acqps[5] = 9;
    u16_Acqps[6] = 9;
    u16_Acqps[7] = 9;
    u16_Acqps[8] = 9;
    u16_Acqps[9] = 9;
    u16_Acqps[10] = 9;
    u16_Acqps[11] = 9;
    u16_Acqps[12] = 9;
    u16_Acqps[13] = 9;
    u16_Acqps[14] = 9;
    u16_Acqps[15] = 9;

    //Interrupt trigger source
    u16_IntSel[0] = 5;
    u16_IntSel[1] = 0;
    u16_IntSel[2] = 0;


    //ADC A setting
    AdcaRegs.ADCCTL1.bit.INTPULSEPOS = 1;           //The interrupt is generated at the end of conversion
    AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1;              //Power up the analog power
    DELAY_US(1000);

    //ADC A Channel select SOCA
    u16_ChSel[0] = 5;              //ADCINA5, Output voltage,V1_AD
    u16_ChSel[1] = 4;              //ADCINA4, Super capacity voltage, V2_AD
    u16_ChSel[2] = 8;//6;          //ADCINA6, Iac_AD
    u16_ChSel[3] = 11;              //ADCINA9, TEMP3
    u16_ChSel[4] = 10;              //ADCINA10,TEMP4
    u16_ChSel[5] = 9;              //ADCINA11, TEMP5

    u16_ChSel[6] = 5;              //ADCINA5, Output voltage,V1_AD
    u16_ChSel[7] = 4;              //ADCINA4, Super capacity voltage, V2_AD
    u16_ChSel[8] = 8;//6;          //ADCINA6, Iac_AD
    u16_ChSel[9] = 11;              //ADCINA9, TEMP3
    u16_ChSel[10] = 10;              //ADCINA10,TEMP4
    u16_ChSel[11] = 9;              //ADCINA11, TEMP5
    u16_ChSel[12] = 5;              //ADCINA5, Output voltage,V1_AD
    u16_ChSel[13] = 5;              //ADCINA5, Output voltage,V1_AD

    u16_Trigger[0] = 5;             //EPWM1SOCA
    u16_Trigger[1] = 5;             //EPWM1SOCA
    u16_Trigger[2] = 5;             //EPWM1SOCA
    u16_Trigger[3] = 5;             //EPWM1SOCA
    u16_Trigger[4] = 5;             //EPWM1SOCA
    u16_Trigger[5] = 5;             //EPWM1SOCA
    u16_Trigger[6] = 6;             //EPWM1SOCB
    u16_Trigger[7] = 6;             //EPWM1SOCB
    u16_Trigger[8] = 6;             //EPWM1SOCB
    u16_Trigger[9] = 6;             //EPWM1SOCB
    u16_Trigger[10] = 6;             //EPWM1SOCB
    u16_Trigger[11] = 6;             //EPWM1SOCB
    u16_Trigger[12] = 9;             //EPWM3SOCA
    u16_Trigger[13] = 10;             //EPWM3SOCB
    u16_Trigger[14] = 0;             //
    u16_Trigger[15] = 0;             //
    sAdcChannelSetup(ADC_ADCA,u16_ChSel,u16_Acqps,u16_Trigger,0,1,u16_IntSel);

    //ADCB setting
    AdcbRegs.ADCCTL1.bit.INTPULSEPOS = 1;           //The interrupt is generated at the end of conversion
    AdcbRegs.ADCCTL1.bit.ADCPWDNZ = 1;              //Power up the analog power
    DELAY_US(1000);
    //ADC B Channel select SOCA
    u16_ChSel[0] = 2;              //ADCINB2, L current
    u16_ChSel[1] = 2;              //ADCINB0, TEMP5
    u16_ChSel[2] = 2;              //ADCINB4, ADC_24V
    u16_ChSel[3] = 2;              //ADCINB2, L current

    u16_Trigger[0] = 5;             //EPWM1SOCA
    u16_Trigger[1] = 6;             //EPWM1SOCB
    u16_Trigger[2] = 9;             //EPWM3SOCA
    u16_Trigger[3] = 10;             //EPWM3SOCB
    u16_Trigger[4] = 0;             //
    u16_Trigger[5] = 0;             //
    u16_Trigger[6] = 0;             //SW trigger
    u16_Trigger[7] = 0;             //
    u16_Trigger[8] = 0;             //
    u16_Trigger[9] = 0;             //
    u16_Trigger[10] = 0;             //
    u16_Trigger[11] = 0;             //
    u16_Trigger[12] = 0;             //
    u16_Trigger[13] = 0;             //
    u16_Trigger[14] = 0;             //
    u16_Trigger[15] = 0;             //
    sAdcChannelSetup(ADC_ADCB,u16_ChSel,u16_Acqps,u16_Trigger,0,0,u16_IntSel);

    //ADC C setting
    AdccRegs.ADCCTL1.bit.INTPULSEPOS = 1;           //The interrupt is generated at the end of conversion
    AdccRegs.ADCCTL1.bit.ADCPWDNZ = 1;              //Power up the analog power
    DELAY_US(1000);
    //ADC C Channel Select SOCA
    //u16_ChSel[0] = 1;              //ADCINC1,
    //SOCB
    //u16_ChSel[1] = 1;              //ADCINC1,

    u16_Trigger[0] = 0;//5             //EPWM1SOCA
    u16_Trigger[1] = 0;//6             //EPWM1SOCB
    u16_Trigger[2] = 0;             //SW trigger
    u16_Trigger[3] = 0;             //
    u16_Trigger[4] = 0;             //
    u16_Trigger[5] = 0;             //
    u16_Trigger[6] = 0;             //
    u16_Trigger[7] = 0;             //
    u16_Trigger[8] = 0;             //
    u16_Trigger[9] = 0;             //
    u16_Trigger[10] = 0;             //
    u16_Trigger[11] = 0;             //
    u16_Trigger[12] = 0;             //
    u16_Trigger[13] = 0;             //
    u16_Trigger[14] = 0;             //
    u16_Trigger[15] = 0;             //
    sAdcChannelSetup(ADC_ADCC,u16_ChSel,u16_Acqps,u16_Trigger,0,0,u16_IntSel);

    EDIS;

    sAdcSampleInit();
}

void sAdcChannelSetup(uint16_t u16_Adc,
                      uint16_t u16_ChSel[],
                      uint16_t u16_Acqps[],
                      uint16_t u16_Trigger[],
                      uint16_t u16_ContinousMode,
                      uint16_t u16_IntEn,
                      uint16_t u16_IntSel[])
{
    volatile struct ADC_REGS *AdcRegsSel[3] = {&AdcaRegs,&AdcbRegs,&AdccRegs};

    //EALLOW;

    //SOC channel select
    (*AdcRegsSel[u16_Adc]).ADCSOC0CTL.bit.CHSEL = u16_ChSel[0];
    (*AdcRegsSel[u16_Adc]).ADCSOC1CTL.bit.CHSEL = u16_ChSel[1];
    (*AdcRegsSel[u16_Adc]).ADCSOC2CTL.bit.CHSEL = u16_ChSel[2];
    (*AdcRegsSel[u16_Adc]).ADCSOC3CTL.bit.CHSEL = u16_ChSel[3];
    (*AdcRegsSel[u16_Adc]).ADCSOC4CTL.bit.CHSEL = u16_ChSel[4];
    (*AdcRegsSel[u16_Adc]).ADCSOC5CTL.bit.CHSEL = u16_ChSel[5];
    (*AdcRegsSel[u16_Adc]).ADCSOC6CTL.bit.CHSEL = u16_ChSel[6];
    (*AdcRegsSel[u16_Adc]).ADCSOC7CTL.bit.CHSEL = u16_ChSel[7];
    (*AdcRegsSel[u16_Adc]).ADCSOC8CTL.bit.CHSEL = u16_ChSel[8];
    (*AdcRegsSel[u16_Adc]).ADCSOC9CTL.bit.CHSEL = u16_ChSel[9];
    (*AdcRegsSel[u16_Adc]).ADCSOC10CTL.bit.CHSEL = u16_ChSel[10];
    (*AdcRegsSel[u16_Adc]).ADCSOC11CTL.bit.CHSEL = u16_ChSel[11];
    (*AdcRegsSel[u16_Adc]).ADCSOC12CTL.bit.CHSEL = u16_ChSel[12];
    (*AdcRegsSel[u16_Adc]).ADCSOC13CTL.bit.CHSEL = u16_ChSel[13];
    (*AdcRegsSel[u16_Adc]).ADCSOC14CTL.bit.CHSEL = u16_ChSel[14];
    (*AdcRegsSel[u16_Adc]).ADCSOC15CTL.bit.CHSEL = u16_ChSel[15];

    //SOC acquisition select
    (*AdcRegsSel[u16_Adc]).ADCSOC0CTL.bit.ACQPS = u16_Acqps[0];
    (*AdcRegsSel[u16_Adc]).ADCSOC1CTL.bit.ACQPS = u16_Acqps[1];
    (*AdcRegsSel[u16_Adc]).ADCSOC2CTL.bit.ACQPS = u16_Acqps[2];
    (*AdcRegsSel[u16_Adc]).ADCSOC3CTL.bit.ACQPS = u16_Acqps[3];
    (*AdcRegsSel[u16_Adc]).ADCSOC4CTL.bit.ACQPS = u16_Acqps[4];
    (*AdcRegsSel[u16_Adc]).ADCSOC5CTL.bit.ACQPS = u16_Acqps[5];
    (*AdcRegsSel[u16_Adc]).ADCSOC6CTL.bit.ACQPS = u16_Acqps[6];
    (*AdcRegsSel[u16_Adc]).ADCSOC7CTL.bit.ACQPS = u16_Acqps[7];
    (*AdcRegsSel[u16_Adc]).ADCSOC8CTL.bit.ACQPS = u16_Acqps[8];
    (*AdcRegsSel[u16_Adc]).ADCSOC9CTL.bit.ACQPS = u16_Acqps[9];
    (*AdcRegsSel[u16_Adc]).ADCSOC10CTL.bit.ACQPS = u16_Acqps[10];
    (*AdcRegsSel[u16_Adc]).ADCSOC11CTL.bit.ACQPS = u16_Acqps[11];
    (*AdcRegsSel[u16_Adc]).ADCSOC12CTL.bit.ACQPS = u16_Acqps[12];
    (*AdcRegsSel[u16_Adc]).ADCSOC13CTL.bit.ACQPS = u16_Acqps[13];
    (*AdcRegsSel[u16_Adc]).ADCSOC14CTL.bit.ACQPS = u16_Acqps[14];
    (*AdcRegsSel[u16_Adc]).ADCSOC15CTL.bit.ACQPS = u16_Acqps[15];

    //SOC trigger select
    (*AdcRegsSel[u16_Adc]).ADCSOC0CTL.bit.TRIGSEL = u16_Trigger[0];
    (*AdcRegsSel[u16_Adc]).ADCSOC1CTL.bit.TRIGSEL = u16_Trigger[1];
    (*AdcRegsSel[u16_Adc]).ADCSOC2CTL.bit.TRIGSEL = u16_Trigger[2];
    (*AdcRegsSel[u16_Adc]).ADCSOC3CTL.bit.TRIGSEL = u16_Trigger[3];
    (*AdcRegsSel[u16_Adc]).ADCSOC4CTL.bit.TRIGSEL = u16_Trigger[4];
    (*AdcRegsSel[u16_Adc]).ADCSOC5CTL.bit.TRIGSEL = u16_Trigger[5];
    (*AdcRegsSel[u16_Adc]).ADCSOC6CTL.bit.TRIGSEL = u16_Trigger[6];
    (*AdcRegsSel[u16_Adc]).ADCSOC7CTL.bit.TRIGSEL = u16_Trigger[7];
    (*AdcRegsSel[u16_Adc]).ADCSOC8CTL.bit.TRIGSEL = u16_Trigger[8];
    (*AdcRegsSel[u16_Adc]).ADCSOC9CTL.bit.TRIGSEL = u16_Trigger[9];
    (*AdcRegsSel[u16_Adc]).ADCSOC10CTL.bit.TRIGSEL = u16_Trigger[10];
    (*AdcRegsSel[u16_Adc]).ADCSOC11CTL.bit.TRIGSEL = u16_Trigger[11];
    (*AdcRegsSel[u16_Adc]).ADCSOC12CTL.bit.TRIGSEL = u16_Trigger[12];
    (*AdcRegsSel[u16_Adc]).ADCSOC13CTL.bit.TRIGSEL = u16_Trigger[13];
    (*AdcRegsSel[u16_Adc]).ADCSOC14CTL.bit.TRIGSEL = u16_Trigger[14];
    (*AdcRegsSel[u16_Adc]).ADCSOC15CTL.bit.TRIGSEL = u16_Trigger[15];

    //Continuous mode setting
    if((u16_ContinousMode & 0x1) != 0)
    {
        (*AdcRegsSel[u16_Adc]).ADCINTSEL1N2.bit.INT1CONT = 1;
    }
    else
    {
        (*AdcRegsSel[u16_Adc]).ADCINTSEL1N2.bit.INT1CONT = 0;
    }
    if((u16_ContinousMode & 0x2) != 0)
    {
        (*AdcRegsSel[u16_Adc]).ADCINTSEL1N2.bit.INT2CONT = 1;
    }
    else
    {
        (*AdcRegsSel[u16_Adc]).ADCINTSEL1N2.bit.INT2CONT = 0;
    }
    if((u16_ContinousMode & 0x4) != 0)
    {
        (*AdcRegsSel[u16_Adc]).ADCINTSEL3N4.bit.INT3CONT = 1;
    }
    else
    {
        (*AdcRegsSel[u16_Adc]).ADCINTSEL3N4.bit.INT3CONT = 0;
    }
    if((u16_ContinousMode & 0x8) != 0)
    {
        (*AdcRegsSel[u16_Adc]).ADCINTSEL3N4.bit.INT4CONT = 1;
    }
    else
    {
        (*AdcRegsSel[u16_Adc]).ADCINTSEL3N4.bit.INT4CONT = 0;
    }

    //Interrupt Select
    if((u16_IntEn & 0x1) != 0)
    {
        (*AdcRegsSel[u16_Adc]).ADCINTSEL1N2.bit.INT1E = 1;
        //(*AdcRegsSel[u16_Adc]).ADCINTSEL1N2.bit.INT1SEL = u16_IntSel[0];
    }
    else
    {
        (*AdcRegsSel[u16_Adc]).ADCINTSEL1N2.bit.INT1E = 0;
    }

    if((u16_IntEn & 0x2) != 0)
    {
        (*AdcRegsSel[u16_Adc]).ADCINTSEL1N2.bit.INT2E = 1;
        //(*AdcRegsSel[u16_Adc]).ADCINTSEL1N2.bit.INT2SEL = u16_IntSel[1];
    }
    else
    {
        (*AdcRegsSel[u16_Adc]).ADCINTSEL1N2.bit.INT2E = 0;
    }

    if((u16_IntEn & 0x4) != 0)
    {
        (*AdcRegsSel[u16_Adc]).ADCINTSEL3N4.bit.INT3E = 1;
        //(*AdcRegsSel[u16_Adc]).ADCINTSEL3N4.bit.INT3SEL = u16_IntSel[2];
    }
    else
    {
        (*AdcRegsSel[u16_Adc]).ADCINTSEL3N4.bit.INT3E = 0;
    }

    if((u16_IntEn & 0x8) != 0)
    {
        (*AdcRegsSel[u16_Adc]).ADCINTSEL3N4.bit.INT4E = 1;
        //(*AdcRegsSel[u16_Adc]).ADCINTSEL3N4.bit.INT4SEL = u16_IntSel[3];
    }
    else
    {
        (*AdcRegsSel[u16_Adc]).ADCINTSEL3N4.bit.INT4E = 0;
    }

    if(u16_Adc == ADC_ADCA)
    {
        (*AdcRegsSel[u16_Adc]).ADCINTSEL1N2.bit.INT1SEL = u16_IntSel[0];
    }
    else if(u16_Adc == ADC_ADCB)
    {
        (*AdcRegsSel[u16_Adc]).ADCINTSEL1N2.bit.INT1SEL = u16_IntSel[1];
    }
    else if(u16_Adc == ADC_ADCC)
    {
        (*AdcRegsSel[u16_Adc]).ADCINTSEL1N2.bit.INT1SEL = u16_IntSel[2];
    }

    (*AdcRegsSel[u16_Adc]).ADCINTFLGCLR.all = 0x000F;       //Clear all the interrupt flags

    //EDIS;
}

void sAdcSampleInit(void)
{
    stAdcSample.u16_L_I_Mid = 2048;
    stAdcSample.u16_Output_I_Mid = 2048;
    stAdcSample.u16_Output_V_Mid = 0;
    stAdcSample.u16_SC_V_Mid = 0;

    stAdcSample.u32_L_I_MidCalSumTemp = 0;
    stAdcSample.u32_Output_I_MidCalSumTemp = 0;
    stAdcSample.u32_Output_V_MidCalSumTemp = 0;

    stAdcSample.u16_MidCalFlag = 0;
    stAdcSample.u16_MidCalCnt = 0;
}

void sAdcCpuSample(void)
{
    if(stAdcSample.u16_MidCalFlag == 0)
    {
        //stAdcSample.u32_L_I_MidCalSumTemp += mAdc_L_I_0();
        stAdcSample.u32_Output_I_MidCalSumTemp += mAdc_Output_I_0();
        stAdcSample.u32_Output_V_MidCalSumTemp += mAdc_Output_V_0();

        stAdcSample.u16_MidCalCnt++;
        if(stAdcSample.u16_MidCalCnt >= cAdcMidCalCntSet)
        {
            //stAdcSample.u16_L_I_Mid = (uint16_t)(stAdcSample.u32_L_I_MidCalSumTemp>>cAdcMidCalN);
            stAdcSample.u16_Output_I_Mid = (uint16_t)(stAdcSample.u32_Output_I_MidCalSumTemp>>cAdcMidCalN);
            //stAdcSample.u16_Output_V_Mid = (uint16_t)(stAdcSample.u32_Output_V_MidCalSumTemp>>cAdcMidCalN);
            stAdcSample.u16_MidCalCnt = 0;
            stAdcSample.u16_MidCalFlag = 1;
        }
    }
    else
    {
        //stAdcSample.i16_L_I = (int16_t)(((mAdc_L_I_0() + mAdc_L_I_1())>>1) - stAdcSample.u16_L_I_Mid);
        stAdcSample.i16_Output_I = (int16_t)(((mAdc_Output_I_0() + mAdc_Output_I_1() + mAdc_Output_I_2() + mAdc_Output_I_3())>>2) - stAdcSample.u16_Output_I_Mid);
        stAdcSample.i16_Output_V = (int16_t)(((mAdc_Output_V_0() + mAdc_Output_V_1() + mAdc_Output_V_2() + mAdc_Output_V_3())>>2) - stAdcSample.u16_Output_V_Mid);

        stAdcSample.i16_SC_V = (int16_t)(((mAdc_SC_V_0() + mAdc_SC_V_1())>>1) - stAdcSample.u16_SC_V_Mid);

        stAdcSample.u16_Temperature_3 = (uint16_t)(mAdc_Temperature3());
        stAdcSample.u16_Temperature_4 = (uint16_t)(mAdc_Temperature4());
        stAdcSample.u16_Temperature_5 = (uint16_t)(mAdc_Temperature5());
        stAdcSample.i16_Adc_24V = (int16_t)(mAdc_24V());
    }
}




