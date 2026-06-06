/*******************************************************************************
 * File Name: Initial.c
 * Description: The MCU initialization functions.
 * Designer: Miller Xue
 * Version: 00
 * Copyright: Liyuan Haina Co.Ltd
 *******************************************************************************/
#include "App.h"
#include "F28x_Project.h"
#include "Gpio_User.h"
#include "Sci_User.h"
#include "UartFunction.h"
#include "PWM_User.h"
#include "Adc_User.h"
#include "Cmpss_User.h"
#include "Can_User.h"

//static void setupTimer0( void );
void sMcuInitial(void);
void sEnableINT(void);
void sCpuTimerInit(void);
void sInitECap1(void);
void sInitECap2(void);


void sMcuInitial(void)
{
   //Config the PLL, FLASH,
   InitSysCtrl();
   InitPieCtrl();

   DELAY_US(1000);
   sGpioInit();
   sCmpssInit();
   sPWMInit();
   sAdcInit();
   sCpuTimerInit();
   sSciInit();
   sCanAInit();
   sSpiInit();
   sFanPwmInit();
   //sInitECap1();
   //sInitECap2();
   sEnableINT();

}


/******************************************************************************
 * Function Name: sCpuTimerInit
 * Description: The CPU timer0 initialization for system time-ticker
 * Input: None
 * Output: None
 * Return: None
 * ***************************************************************************/
void sCpuTimerInit(void)
{
    CpuTimer0Regs.PRD.all =  120000;     // A tasks
    CpuTimer0Regs.TCR.bit.TIF = 0x01;   //Clear the CpuTimer0 Interrupt Flag
    CpuTimer0Regs.TCR.bit.TIE = 0x01;   //Enable the CpuTimer0 Interrupt
}


void sInitECap1(void)
{
    EALLOW;
    InputXbarRegs.INPUT7SELECT = 28;        // Set eCAP1 source to GPIO-pin
    GpioCtrlRegs.GPAMUX2.bit.GPIO28 = 0;    //GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO28 = 0;     //INPUT
    GpioCtrlRegs.GPAQSEL2.bit.GPIO28 = 3;   //ASYNC
    EDIS;

    ECap1Regs.ECEINT.all = 0x0000;          // Disable all capture __interrupts
    ECap1Regs.ECCLR.all = 0xFFFF;           // Clear all CAP __interrupt flags
    ECap1Regs.ECCTL1.bit.CAPLDEN = 0;       // Disable CAP1-CAP4 register loads
    ECap1Regs.ECCTL2.bit.TSCTRSTOP = 0;     // Make sure the counter is stopped

    // Configure peripheral registers
    ECap1Regs.ECCTL2.bit.CONT_ONESHT = 1;   // One-shot
    ECap1Regs.ECCTL2.bit.STOP_WRAP = 2;     // Stop at 3 events
    ECap1Regs.ECCTL1.bit.CAP1POL = 1;       // Falling edge
    ECap1Regs.ECCTL1.bit.CAP2POL = 0;       // Rising edge
    ECap1Regs.ECCTL1.bit.CAP3POL = 1;       // Falling edge
    //ECap1Regs.ECCTL1.bit.CAP4POL = 0;       // Rising edge
    ECap1Regs.ECCTL1.bit.CTRRST1 = 1;       // Difference operation
    ECap1Regs.ECCTL1.bit.CTRRST2 = 1;       // Difference operation
    ECap1Regs.ECCTL1.bit.CTRRST3 = 1;       // Difference operation
    //ECap1Regs.ECCTL1.bit.CTRRST4 = 1;       // Difference operation
    ECap1Regs.ECCTL2.bit.SYNCI_EN = 0;//1;      // Disable sync in
    ECap1Regs.ECCTL2.bit.SYNCO_SEL = 0;     // Pass through
    ECap1Regs.ECCTL1.bit.CAPLDEN = 1;       // Enable capture units

    ECap1Regs.ECCTL2.bit.TSCTRSTOP = 1;     // Start Counter
    ECap1Regs.ECCTL2.bit.REARM = 1;         // arm one-shot
    ECap1Regs.ECCTL1.bit.CAPLDEN = 1;       // Enable CAP1-CAP4 register loads
    ECap1Regs.ECEINT.bit.CEVT3 = 1;         // 3 events = __interrupt
}

__interrupt void ecap1_isr(void)
{
    //u32_CAP2_Cnt = ECap1Regs.CAP2;
    //u32_CAP3_Cnt = ECap1Regs.CAP3;
    //u32_Duty_Cnt = u32_CAP3_Cnt / (u32_CAP2_Cnt + u32_CAP3_Cnt);

    ECap1Regs.ECCLR.bit.CEVT3 = 1;
    ECap1Regs.ECCLR.bit.INT = 1;
    ECap1Regs.ECCTL2.bit.REARM = 1;

    // Acknowledge this __interrupt to receive more __interrupts from group 4
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP4;
}


void sInitECap2(void)
{
    EALLOW;
    InputXbarRegs.INPUT8SELECT = 29;        // Set eCAP2 source to GPIO-pin
    GpioCtrlRegs.GPAMUX2.bit.GPIO29 = 0;    //GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO29 = 0;     //INPUT
    GpioCtrlRegs.GPAQSEL2.bit.GPIO29 = 3;   //ASYNC
    EDIS;

    ECap2Regs.ECEINT.all = 0x0000;          // Disable all capture __interrupts
    ECap2Regs.ECCLR.all = 0xFFFF;           // Clear all CAP __interrupt flags
    ECap2Regs.ECCTL1.bit.CAPLDEN = 0;       // Disable CAP1-CAP4 register loads
    ECap2Regs.ECCTL2.bit.TSCTRSTOP = 0;     // Make sure the counter is stopped

    // Configure peripheral registers
    ECap2Regs.ECCTL2.bit.CONT_ONESHT = 1;   // One-shot
    ECap2Regs.ECCTL2.bit.STOP_WRAP = 2;     // Stop at 3 events
    ECap2Regs.ECCTL1.bit.CAP1POL = 1;       // Falling edge
    ECap2Regs.ECCTL1.bit.CAP2POL = 0;       // Rising edge
    ECap2Regs.ECCTL1.bit.CAP3POL = 1;       // Falling edge
    //ECap2Regs.ECCTL1.bit.CAP4POL = 0;       // Rising edge
    ECap2Regs.ECCTL1.bit.CTRRST1 = 1;       // Difference operation
    ECap2Regs.ECCTL1.bit.CTRRST2 = 1;       // Difference operation
    ECap2Regs.ECCTL1.bit.CTRRST3 = 1;       // Difference operation
    //ECap2Regs.ECCTL1.bit.CTRRST4 = 1;       // Difference operation
    ECap2Regs.ECCTL2.bit.SYNCI_EN = 0;//1;      // Disable sync in
    ECap2Regs.ECCTL2.bit.SYNCO_SEL = 0;     // Pass through
    ECap2Regs.ECCTL1.bit.CAPLDEN = 1;       // Enable capture units

    ECap2Regs.ECCTL2.bit.TSCTRSTOP = 1;     // Start Counter
    ECap2Regs.ECCTL2.bit.REARM = 1;         // arm one-shot
    ECap2Regs.ECCTL1.bit.CAPLDEN = 1;       // Enable CAP1-CAP4 register loads
    ECap2Regs.ECEINT.bit.CEVT3 = 1;         // 3 events = __interrupt
}

__interrupt void ecap2_isr(void)
{

    ECap2Regs.ECCLR.bit.CEVT3 = 1;
    ECap2Regs.ECCLR.bit.INT = 1;
    ECap2Regs.ECCTL2.bit.REARM = 1;

    // Acknowledge this __interrupt to receive more __interrupts from group 4
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP4;
}




void sEnableINT(void)
{
    IER = 0x0000;
    IFR = 0x0000;

    InitPieVectTable();

    //Interrupt vector table initialization
    EALLOW;
    PieVectTable.ADCA1_INT = &sADCA1_isr; //function for ADCA interrupt 1
    PieVectTable.TIMER0_INT = &sRtosTimerInterruptIsr; //function for TIMER0_INT
    PieVectTable.ECAP1_INT = &ecap1_isr;
    PieVectTable.ECAP2_INT = &ecap2_isr;
    PieVectTable.SCIA_RX_INT = &sSciaRxIsr;
    PieVectTable.SCIA_TX_INT = &sSciaTxIsr;
    PieVectTable.SCIB_RX_INT = &sScibRxIsr;
    PieVectTable.SCIB_TX_INT = &sScibTxIsr;
    PieVectTable.CANA0_INT = &sCanAIsr;
    EDIS;

    PieCtrlRegs.PIEIER1.bit.INTx1 = 1;      //ADCA1 interrupt
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;      //TIMER0 interrupt
    PieCtrlRegs.PIEIER4.bit.INTx1 = 1;      //ECap1 interrupt
    PieCtrlRegs.PIEIER4.bit.INTx2 = 1;      //ECap2 interrupt
    PieCtrlRegs.PIEIER9.bit.INTx1 = 1;      //SCIA RX
    PieCtrlRegs.PIEIER9.bit.INTx2 = 1;      //SCIA TX
    PieCtrlRegs.PIEIER9.bit.INTx3 = 1;      //SCIB RX
    PieCtrlRegs.PIEIER9.bit.INTx4 = 1;      //SCIB TX
    PieCtrlRegs.PIEIER9.bit.INTx5 = 0;      //CANA INTERR0
    PieCtrlRegs.PIEACK.all = 0xFFFF;


    IER |= (M_INT1 | M_INT11 | M_INT9 | M_INT4); //Enable group 1 interrupts
//    EINT;          //Enable global interrupt
//    ERTM;
}



