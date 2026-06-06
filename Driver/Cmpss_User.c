/*******************************************************************************
 * File Name: Cmpss_User.c
 * Description: The CMPSS initialization functions.
 * Designer: Miller Xue
 * Version: 00
 * Copyright: Liyuan Haina Co.Ltd
 *******************************************************************************/
#include "F28x_Project.h"
#include "Cmpss_User.h"


void sCmpssInit(void);

//CMPSS1 for 1st AC-DC R current
/*******************************************************************************
 * Function Name: sCmpssInit
 * Description: Initialize the CMPSS1~8.
 *              CMPSS1: 1ST AC-DC r current; CMPSS2: 2ND AC-DC r current
 *              CMPSS3: 1ST AC-DC S current; CMPSS4: 2ND AC-DC S current
 *              CMPSS5: 1ST AC-DC T current; CMPSS6: 2ND AC-DC T current
 *              CMPSS7: 1ST DC-DC current;   CMPSS8: 2ND DC-DC current
 *              When the P input >= DACHVAL, OUTPUT high
 *              When the P input <= DACNVAL, OUTPUT high as it is inverted
 *******************************************************************************/
void sCmpssInit(void)
{
    uint16_t n;
    volatile struct CMPSS_REGS *CmpssReg[4] =
                      {&Cmpss1Regs, &Cmpss2Regs, &Cmpss3Regs, &Cmpss4Regs};

    EALLOW;
    for(n=0;n<4;n++)
    {
        (*CmpssReg[n]).COMPCTL.bit.COMPDACE = 1;        //Enable CMPSS
        (*CmpssReg[n]).COMPCTL.bit.COMPHSOURCE = NEGIN_DAC; //NEG signal comes from DAC
        (*CmpssReg[n]).COMPDACCTL.bit.SELREF = REFERENCE_VDDA; //Use VDDA as the reference

        if(n == 0)
        {
            //(*CmpssReg[n]).DACHVALS.bit.DACVAL = AC_CURRENT_DAC_HIGH;            //Set P COMP DAC
            (*CmpssReg[n]).DACLVALS.bit.DACVAL = CMPSS1_18V_LOW;             //Set N COMP DAC
            (*CmpssReg[n]).COMPSTSCLR.bit.LLATCHCLR = 1;
        }
        else if(n == 1)
        {
            (*CmpssReg[n]).DACHVALS.bit.DACVAL = CMPSS2_V2_HIGH;            //Set P COMP DAC
            //(*CmpssReg[n]).DACLVALS.bit.DACVAL = AC_CURRENT_DAC_LOW;             //Set N COMP DAC
            //(*CmpssReg[n]).COMPSTSCLR.bit.LLATCHCLR = 1;
        }
        else if(n == 2)
        {
            (*CmpssReg[n]).DACHVALS.bit.DACVAL = CMPSS3_IPEAK_HIGH;            //Set P COMP DAC

        }
        else
        {
            (*CmpssReg[n]).DACLVALS.bit.DACVAL = CMPSS4_5V_LOW;             //Set N COMP DAC
            (*CmpssReg[n]).COMPSTSCLR.bit.LLATCHCLR = 1;
        }


        // Configure Digital Filter
        //Maximum CLKPRESCALE value provides the most time between samples
        (*CmpssReg[n]).CTRIPHFILCLKCTL = 0x1;   //SYS_CLK/2
        (*CmpssReg[n]).CTRIPLFILCLKCTL = 0x1;   //SYS_CLK/2
        //Maximum SAMPWIN value provides largest number of samples
        (*CmpssReg[n]).CTRIPHFILCTL.bit.SAMPWIN = 0xF;         //16 clocks
        (*CmpssReg[n]).CTRIPLFILCTL.bit.SAMPWIN = 0xF;         //16 clocks
        //Maximum THRESH value requires static value for entire window
        //  THRESH should be GREATER than half of SAMPWIN
        (*CmpssReg[n]).CTRIPHFILCTL.bit.THRESH = 0xF;          //greater than 10 samples,200ns
        (*CmpssReg[n]).CTRIPHFILCTL.bit.FILINIT = 1; //Reset filter logic & start filtering
        (*CmpssReg[n]).CTRIPLFILCTL.bit.THRESH = 0xF;          //greater than 10 samples,200ns
        (*CmpssReg[n]).CTRIPLFILCTL.bit.FILINIT = 1; //Reset filter logic & start filtering

        // Configure CTRIPOUT path
        // Digital filter output feeds CTRIPH and CTRIPOUTH
        (*CmpssReg[n]).COMPCTL.bit.CTRIPHSEL = CTRIP_FILTER;
        (*CmpssReg[n]).COMPCTL.bit.CTRIPOUTHSEL = CTRIP_FILTER;
        (*CmpssReg[n]).COMPCTL.bit.CTRIPLSEL = CTRIP_FILTER;
        (*CmpssReg[n]).COMPCTL.bit.CTRIPOUTLSEL = CTRIP_FILTER;
        (*CmpssReg[n]).COMPCTL.bit.COMPLINV = 1;                 //invert the N COMP output
    }
    EDIS;
}







