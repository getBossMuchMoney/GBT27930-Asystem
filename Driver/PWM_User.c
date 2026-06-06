/*******************************************************************************
 * File Name: PWM_User.c
 * Description: The ePWM initialization functions.
 * Designer: Miller Xue
 * Version: 00
 * Copyright: Liyuan Haina Co.Ltd
 *******************************************************************************/
#include "F28x_Project.h"
#include "PWM_User.h"


void sPWMInit(void);
void sPwmUserInit(uint16_t n, uint16_t Period, uint16_t coutdir, uint16_t phase,uint16_t deadtime);
void sPwmXbarInit(void);
void sPwmTzDc_CBCInit(uint16_t n,uint16_t TripSrc,uint16_t ActA,uint16_t ActB);
void sFanPwmInit(void);


/*******************************************************************************
 * Function Name: sPWMInit
 * Description: Initialize the ePWM 1~3.
 *              EPWM1: ;
 *              EPWM2: ;
 *              EPWM3: ;
 *              All the PWM carrier frequency is 3kHz
 *******************************************************************************/
void sPWMInit(void)
{
    EALLOW;

    //Stop the TBCLK
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;

    //Set the EPWMSYNCIN signal for EPWM1, GPIO70 is the SYN-RX
    //GpioCtrlRegs.GPCPUD.bit.GPIO70 = 0;    // Enable pull-up on GPIO70
    //GpioCtrlRegs.GPCDIR.bit.GPIO70 = 0;    // Input
    //InputXbarRegs.INPUT5SELECT = 70;
    //SyncSocRegs.SYNCSELECT.bit.EPWM4SYNCIN = 0;   //EPWM4,5,6 use the EPWM1 synout
    //SyncSocRegs.SYNCSELECT.bit.EPWM7SYNCIN = 0;   //EPWM7,8,9 use the EPWM1 synout

    //InputXbarRegs.INPUT1SELECT = 22;   //TZ1 for Desat-A
    //InputXbarRegs.INPUT2SELECT = 14;   //TZ2 for Desat-C
    //InputXbarRegs.INPUT1SELECT = 31;   //TZ1 for power fault1
    //InputXbarRegs.INPUT2SELECT = 9;    //TZ2 for power fault2

    sPwmUserInit(cEPWM1,Fs_PSFB,TB_COUNT_UP,0,cDeadtime);
    sPwmUserInit(cEPWM2,Fs_PSFB,TB_COUNT_UP,0,cDeadtime2);
    sPwmUserInit(cEPWM3,Fs_PSFB,TB_COUNT_UP,Fs_PSFB>>1,cDeadtime);

    //Set the ADC SOCA/B
    EPwm1Regs.ETSEL.bit.SOCAEN = 1;
    EPwm1Regs.ETSEL.bit.SOCASEL = ET_CTR_ZERO;
    EPwm1Regs.ETPS.bit.SOCAPRD = ET_1ST;

    EPwm1Regs.ETSEL.bit.SOCBEN = 1;
    EPwm1Regs.ETSEL.bit.SOCBSEL = ET_CTR_PRD;
    EPwm1Regs.ETPS.bit.SOCBPRD = ET_1ST;

    EPwm3Regs.ETSEL.bit.SOCAEN = 1;
    EPwm3Regs.ETSEL.bit.SOCASEL = ET_CTR_ZERO;
    EPwm3Regs.ETPS.bit.SOCAPRD = ET_1ST;

    EPwm3Regs.ETSEL.bit.SOCBEN = 1;
    EPwm3Regs.ETSEL.bit.SOCBSEL = ET_CTR_PRD;
    EPwm3Regs.ETPS.bit.SOCBPRD = ET_1ST;

    sPwmXbarInit();
    sPwmTzDc_CBCInit(cEPWM1,DC_TRIPIN4,TZ_DCAH_LOW,TZ_DCBH_LOW);
    sPwmTzDc_CBCInit(cEPWM2,DC_TRIPIN4,TZ_DCAH_LOW,TZ_DCBH_LOW);
    //sPwmTzDc_CBCInit(cEPWM3,DC_TRIPIN4,TZ_DCAH_LOW,TZ_DCBH_LOW);

    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;

    //PWM pins for Interleaved AC-DC PWM rectifier
    GpioCtrlRegs.GPAPUD.bit.GPIO0 = 1;    // Disable pull-up on GPIO0 (EPWM1A)
    GpioCtrlRegs.GPAPUD.bit.GPIO1 = 1;    // Disable pull-up on GPIO1 (EPWM1B)
    GpioCtrlRegs.GPAPUD.bit.GPIO2 = 1;    // Disable pull-up on GPIO2 (EPWM2A)
    GpioCtrlRegs.GPAPUD.bit.GPIO3 = 1;    // Disable pull-up on GPIO3 (EPWM2B)
    //
    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 1;   // Configure GPIO0 as EPWM1A
    GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 1;   // Configure GPIO1 as EPWM1B
    GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 1;   // Configure GPIO2 as EPWM2A
    GpioCtrlRegs.GPAMUX1.bit.GPIO3 = 1;   // Configure GPIO3 as EPWM2B
    EDIS;
}

void sPwmXbarInit(void)
{
    //all the EPWM X-bar registers are EALLOW protected
    //EALLOW;            //uncoment if no EALLOW protected before calling
    //trip4 for EPWM1 which is for 1st AC-DC R phase current control
    /*EPwmXbarRegs.TRIP4MUX0TO15CFG.bit.MUX0 = 0;   //CMPSS1.CTRIPH
    EPwmXbarRegs.TRIP4MUX0TO15CFG.bit.MUX1 = 0;   //CMPSS1.CTRIPL
    EPwmXbarRegs.TRIP4MUXENABLE.bit.MUX0 = 1;     //CMPSS1.MUX0 enable
    EPwmXbarRegs.TRIP4MUXENABLE.bit.MUX1 = 1;     //CMPSS1.MUX1 enable
    EPwmXbarRegs.TRIPOUTINV.bit.TRIP4 = 1;        //invert the output, active low*/

    EPwmXbarRegs.TRIP4MUX0TO15CFG.all = 0;       //CMPSS:CTRIPH,CTRIPL all selected
    //EPwmXbarRegs.TRIP4MUXENABLE.all = 0x0096;     //Enable mux1,2,3,5
    EPwmXbarRegs.TRIP4MUXENABLE.all = 0x0014;
    EPwmXbarRegs.TRIPOUTINV.bit.TRIP4 = 1;        //invert the output

    //trip5 for EPWM4 which is 2nd AC-DC R phase current control
    /*EPwmXbarRegs.TRIP5MUX0TO15CFG.bit.MUX2 = 0;   //CMPSS2.CTRIPH
    EPwmXbarRegs.TRIP5MUX0TO15CFG.bit.MUX3 = 0;   //CMPSS2.CTRIPL
    EPwmXbarRegs.TRIP5MUXENABLE.bit.MUX2 = 1;     //CMPSS2.MUX0 enable
    EPwmXbarRegs.TRIP5MUXENABLE.bit.MUX3 = 1;     //CMPSS2.MUX1 enable
    EPwmXbarRegs.TRIPOUTINV.bit.TRIP5 = 1;        //invert the output, active low

    //trip7 for EPWM2 which is 1st AC-DC s phase current control
    EPwmXbarRegs.TRIP7MUX0TO15CFG.bit.MUX4 = 0;   //CMPSS3.CTRIPH
    EPwmXbarRegs.TRIP7MUX0TO15CFG.bit.MUX5 = 0;   //CMPSS3.CTRIPL
    EPwmXbarRegs.TRIP7MUXENABLE.bit.MUX4 = 1;     //CMPSS3.MUX0 enable
    EPwmXbarRegs.TRIP7MUXENABLE.bit.MUX5 = 1;     //CMPSS3.MUX1 enable
    EPwmXbarRegs.TRIPOUTINV.bit.TRIP7 = 1;        //invert the output, active low

    //trip8 for EPWM5 which is for 2nd AC-DC S phase current control
    EPwmXbarRegs.TRIP8MUX0TO15CFG.bit.MUX6 = 0;   //CMPSS4.CTRIPH
    EPwmXbarRegs.TRIP8MUX0TO15CFG.bit.MUX7 = 0;   //CMPSS4.CTRIPL
    EPwmXbarRegs.TRIP8MUXENABLE.bit.MUX6 = 1;     //CMPSS4.MUX0 enable
    EPwmXbarRegs.TRIP8MUXENABLE.bit.MUX7 = 1;     //CMPSS4.MUX1 enable
    EPwmXbarRegs.TRIPOUTINV.bit.TRIP8 = 1;        //invert the output, active low

    //trip9 for EPWM3 which is 1st AC-DC T phase current control
    EPwmXbarRegs.TRIP9MUX0TO15CFG.bit.MUX8 = 0;   //CMPSS5.CTRIPH
    EPwmXbarRegs.TRIP9MUX0TO15CFG.bit.MUX9 = 0;   //CMPSS5.CTRIPL
    EPwmXbarRegs.TRIP9MUXENABLE.bit.MUX8 = 1;     //CMPSS5.MUX0 enable
    EPwmXbarRegs.TRIP9MUXENABLE.bit.MUX9 = 1;     //CMPSS5.MUX1 enable
    EPwmXbarRegs.TRIPOUTINV.bit.TRIP9 = 1;        //invert the output, active low

    //trip10 for EPWM6 which is 2ND AC-DC T phase current control
    EPwmXbarRegs.TRIP10MUX0TO15CFG.bit.MUX10 = 0;   //CMPSS6.CTRIPH
    EPwmXbarRegs.TRIP10MUX0TO15CFG.bit.MUX11 = 0;   //CMPSS6.CTRIPL
    EPwmXbarRegs.TRIP10MUXENABLE.bit.MUX10 = 1;     //CMPSS6.MUX0 enable
    EPwmXbarRegs.TRIP10MUXENABLE.bit.MUX11 = 1;     //CMPSS6.MUX1 enable
    EPwmXbarRegs.TRIPOUTINV.bit.TRIP10 = 1;        //invert the output, active low*/

    //trip11 for EPWM7 which is 1st DC-DC current control
    /*EPwmXbarRegs.TRIP11MUX0TO15CFG.bit.MUX12 = 0;   //CMPSS7.CTRIPH
    EPwmXbarRegs.TRIP11MUX0TO15CFG.bit.MUX13 = 0;   //CMPSS7.CTRIPL
    EPwmXbarRegs.TRIP11MUXENABLE.bit.MUX12 = 1;     //CMPSS7.MUX0 enable
    EPwmXbarRegs.TRIP11MUXENABLE.bit.MUX13 = 1;     //CMPSS7.MUX1 enable
    EPwmXbarRegs.TRIPOUTINV.bit.TRIP11 = 1;        //invert the output, active low

    //trip12 for EPWM8 which is 1st DC-DC current control
    EPwmXbarRegs.TRIP12MUX0TO15CFG.bit.MUX14 = 0;   //CMPSS8.CTRIPH
    EPwmXbarRegs.TRIP12MUX0TO15CFG.bit.MUX15 = 0;   //CMPSS8.CTRIPL
    EPwmXbarRegs.TRIP12MUXENABLE.bit.MUX14 = 1;     //CMPSS8.MUX0 enable
    EPwmXbarRegs.TRIP12MUXENABLE.bit.MUX15 = 1;     //CMPSS8.MUX1 enable
    EPwmXbarRegs.TRIPOUTINV.bit.TRIP12 = 1;        //invert the output, active low*/
    //EDIS;
}


void sPwmUserInit(uint16_t n, uint16_t Period, uint16_t coutdir, uint16_t phase,uint16_t deadtime)
{
    volatile struct EPWM_REGS *ePWM[8] =
                  {  &EPwm1Regs, &EPwm2Regs, &EPwm3Regs, &EPwm4Regs,
                     &EPwm5Regs, &EPwm6Regs, &EPwm7Regs, &EPwm8Regs};

    //Time Base SubModule Register
    (*ePWM[n]).TBCTL.bit.PRDLD = TB_IMMEDIATE;    // set Immediate load
    (*ePWM[n]).TBPRD = Period;
    (*ePWM[n]).TBPHS.bit.TBPHS = phase;
    (*ePWM[n]).TBCTR = 0;
    (*ePWM[n]).TBCTL.bit.CTRMODE = coutdir;
    (*ePWM[n]).TBCTL.bit.HSPCLKDIV = TB_DIV1;
    (*ePWM[n]).TBCTL.bit.CLKDIV = TB_DIV1;

    (*ePWM[n]).TBCTL.bit.FREE_SOFT = 3;            //free-run
    (*ePWM[n]).TBCTL.bit.PHSEN = TB_ENABLE;         //Enable SYNCIN
    //(*ePWM[n]).TBCTL.bit.SYNCOSEL = 0x1;            //SYNCOUT when CTR = 0;
    (*ePWM[n]).EPWMSYNCOUTEN.bit.ZEROEN = 0x1;            //SYNCOUT when CTR = 0;

    // Counter compare submodule registers
    (*ePWM[n]).CMPA.bit.CMPA = 0;           //Initialize the CMP register
    (*ePWM[n]).CMPA.bit.CMPAHR = 0;
    (*ePWM[n]).CMPB.bit.CMPB = Period>>1;
    (*ePWM[n]).CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    (*ePWM[n]).CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;
    (*ePWM[n]).CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    (*ePWM[n]).CMPCTL.bit.SHDWBMODE = CC_SHADOW;

    (*ePWM[n]).AQCTLA.bit.CAU = AQ_SET;
    (*ePWM[n]).AQCTLA.bit.CBU = AQ_CLEAR;
    (*ePWM[n]).AQCTLA.bit.PRD = AQ_NO_ACTION;
    (*ePWM[n]).AQCTLA.bit.ZRO = AQ_NO_ACTION;

    //In current control mode, the HiResPwm is disabled, the MCU generate the deadtime automatically
    (*ePWM[n]).DBCTL.bit.OUT_MODE = DB_FULL_ENABLE; //Both rising and falling edge
    (*ePWM[n]).DBCTL.bit.POLSEL = DB_ACTV_HIC;  // Active high complementary

    (*ePWM[n]).DBFED.bit.DBFED = deadtime;                      //UP switch off to low switch on delay
    (*ePWM[n]).DBRED.bit.DBRED = deadtime;                       //LOW switch off to up switch on delay

    /*EALLOW;
    (*ePWM[n]).HRCNFG.all = 0x0;
    (*ePWM[n]).HRCNFG.bit.EDGMODE = HR_FEP;          // MEP control on both edges
    (*ePWM[n]).HRCNFG.bit.CTLMODE = HR_CMP;          // CMPAHR and TBPRDHR HR control
    (*ePWM[n]).HRCNFG.bit.HRLOAD  = HR_CTR_ZERO;     // load on CTR = 0 and PRD
    (*ePWM[n]).HRCNFG.bit.AUTOCONV = 1;              // Enable autoconversion for HR period
    (*ePWM[n]).HRPCTL.bit.TBPHSHRLOADE = 0;          // Enable TBPHSHR sync (required for updwn count HR control)
    (*ePWM[n]).HRPCTL.bit.HRPE = 0;                  // Turn on high-resolution period control.
    EDIS;*/

//===========================================================================
// TZA events can force EPWMxA
// TZB events can force EPWMxB
    //(*ePWM[n]).TZSEL.bit.CBC1 = TZ_ENABLE; // EPWM1A will go low
    (*ePWM[n]).TZCTL.bit.TZA = TZ_FORCE_LO; // EPWMxA will go low
    (*ePWM[n]).TZCTL.bit.TZB = TZ_FORCE_LO; // EPWMxB will go low
    (*ePWM[n]).TZCTL.bit.DCBEVT2 = TZ_FORCE_LO; // EPWMxB will go low
    (*ePWM[n]).TZCTL.bit.DCAEVT2 = TZ_FORCE_LO; // EPWMxB will go low
    (*ePWM[n]).TZCTL.bit.DCBEVT1 = TZ_FORCE_LO; // EPWMxB will go low
    (*ePWM[n]).TZCTL.bit.DCAEVT1 = TZ_FORCE_LO; // EPWMxB will go low
    (*ePWM[n]).TZFRC.bit.OST = 1;           // Turn off the PWM
    //EDIS;
}

void sPwmTzDc_CBCInit(uint16_t n,uint16_t TripSrc,uint16_t ActA,uint16_t ActB)
{
    volatile struct EPWM_REGS *ePWM[8] =
                      {  &EPwm1Regs, &EPwm2Regs, &EPwm3Regs, &EPwm4Regs,
                         &EPwm5Regs, &EPwm6Regs, &EPwm7Regs, &EPwm8Regs};

    //(*ePWM[n]).TZDCSEL.bit.DCAEVT2 = TZ_DCAH_LOW;  //EVT2 is CBC source,when goes low
    //(*ePWM[n]).TZDCSEL.bit.DCBEVT2 = TZ_DCBH_LOW;   //EVT2 is CBC source,when goes low
    //(*ePWM[n]).TZDCSEL.bit.DCAEVT2 = ActA;   //EVT2 is CBC source,when goes low
    //(*ePWM[n]).TZDCSEL.bit.DCBEVT2 = ActB;   //EVT2 is CBC source,when goes low
    //(*ePWM[n]).TZSEL.bit.DCBEVT2 = 1;               //CBC enalbed
    //(*ePWM[n]).TZSEL.bit.DCAEVT2 = 1;               //CBC enalbed
    (*ePWM[n]).TZDCSEL.bit.DCAEVT1 = ActA;   //EVT1 is OST source,when goes low
    (*ePWM[n]).TZDCSEL.bit.DCBEVT1 = ActB;   //EVT1 is OST source,when goes low
    (*ePWM[n]).TZSEL.bit.DCBEVT1 = 1;               //OST enalbed
    (*ePWM[n]).TZSEL.bit.DCAEVT1 = 1;

    //(*ePWM[n]).TZSEL.bit.OSHT1 = 1;                 //TZ1 enable
    //(*ePWM[n]).TZSEL.bit.OSHT2 = 1;                 //TZ2 enable
    //(*ePWM[n]).TZSEL.bit.OSHT3 = 1;                 //TZ3 enable

    (*ePWM[n]).DCTRIPSEL.bit.DCBHCOMPSEL = TripSrc;     //Select TRIP4 source of the DCBH
    (*ePWM[n]).DCTRIPSEL.bit.DCAHCOMPSEL = TripSrc;     //Select TRIP4 source of the DCAH
    (*ePWM[n]).DCACTL.bit.EVT2SRCSEL = 0;           //Select DCAEVT2
    (*ePWM[n]).DCACTL.bit.EVT2FRCSYNCSEL = 0;       //SYN with TBCLK
    (*ePWM[n]).DCBCTL.bit.EVT2SRCSEL = 0;           //Select DCBEVT2
    (*ePWM[n]).DCBCTL.bit.EVT2FRCSYNCSEL = 0;       //SYN with TBCLK
}

void sFanPwmInit(void)
{
    EALLOW;
    GpioCtrlRegs.GPAGMUX1.bit.GPIO6 = 0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO6 = 1;   // Configure GPIO6 as EPWM4A

    EPwm4Regs.TBCTL.bit.PRDLD = TB_IMMEDIATE;    // set Immediate load
    EPwm4Regs.TBPRD = 6000;                     //10k
    EPwm4Regs.TBPHS.bit.TBPHS = 0;
    EPwm4Regs.TBCTR = 0;
    EPwm4Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;
    EPwm4Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
    EPwm4Regs.TBCTL.bit.CLKDIV = TB_DIV1;

    EPwm4Regs.TBCTL.bit.FREE_SOFT = 3;            //free-run
    EPwm4Regs.TBCTL.bit.PHSEN = TB_DISABLE;         //Enable SYNCIN
    //EPwm4Regs.TBCTL.bit.SYNCOSEL = 0x1;            //SYNCOUT when CTR = 0;
    EPwm4Regs.EPWMSYNCOUTEN.bit.ZEROEN = 0x1;            //SYNCOUT when CTR = 0;

    // Counter compare submodule registers
    EPwm4Regs.CMPA.bit.CMPA = 0;           //Initialize the CMP register
    EPwm4Regs.CMPA.bit.CMPAHR = 0;
    EPwm4Regs.CMPB.bit.CMPB = 0;
    EPwm4Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    EPwm4Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;
    EPwm4Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm4Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;

    EPwm4Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    EPwm4Regs.AQCTLA.bit.CAD = AQ_SET;
    //EPwm4Regs.AQCTLB.bit.CBU = AQ_CLEAR;
    //EPwm4Regs.AQCTLB.bit.CBD = AQ_SET;
    EPwm4Regs.AQCTLA.bit.PRD = AQ_NO_ACTION;
    EPwm4Regs.AQCTLA.bit.ZRO = AQ_NO_ACTION;

    //In current control mode, the HiResPwm is disabled, the MCU generate the deadtime automatically
    EPwm4Regs.DBCTL.bit.OUT_MODE = DB_DISABLE;  //DB_DISABLE

    //EPwm12Regs.TZFRC.bit.OST = 1;           // Turn off the PWM
    EDIS;
}

