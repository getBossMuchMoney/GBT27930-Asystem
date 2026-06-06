/*******************************************************************************
 * File Name: Gpio_User.c
 * Description: The GPIO initialization functions.
 * Designer: Miller Xue
 * Version: 00
 * Copyright: Liyuan Haina Co.Ltd
 *******************************************************************************/
#include "F28x_Project.h"
#include "Gpio_User.h"


void sGpioInit(void);

void sGpioInit(void)
{
    InitGpio(); //Reset the GPIO

    EALLOW;

    //ON_OFF_EN
    GpioCtrlRegs.GPAMUX1.bit.GPIO8 = 0; //GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO8 = 0;  //INPUT

    //PowerFault2
    //GpioCtrlRegs.GPAMUX1.bit.GPIO9 = 0; //GPIO
    //GpioCtrlRegs.GPADIR.bit.GPIO9 = 0;  //INPUT

    //PowerFault1
    GpioCtrlRegs.GPAMUX2.bit.GPIO31 = 0; //GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO31 = 0;  //INPUT

    //CTRL2
    //GpioCtrlRegs.GPAMUX2.bit.GPIO29 = 0; //GPIO
    //GpioCtrlRegs.GPADIR.bit.GPIO29 = 0;  //INPUT

    //TZ1
    GpioCtrlRegs.GPAMUX1.bit.GPIO10 = 0; //GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO10 = 0;  //INPUT

    //Desat-C IGBT short fault
    //GpioCtrlRegs.GPAMUX1.bit.GPIO14 = 0; //GPIO
    //GpioCtrlRegs.GPADIR.bit.GPIO14 = 0;  //INPUT

    //RLY EN
    GpioCtrlRegs.GPAMUX1.bit.GPIO12 = 0; //GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO12 = 1;  //OUTPUT
    GpioDataRegs.GPACLEAR.bit.GPIO12 = 1; //SET LOW

    //Desat-A IGBT short fault
    //GpioCtrlRegs.GPAMUX2.bit.GPIO22 = 0; //GPIO
    //GpioCtrlRegs.GPADIR.bit.GPIO22 = 0;  //INPUT

    //On board LED1 Normal
    GpioCtrlRegs.GPBMUX2.bit.GPIO54 = 0; //GPIO
    GpioCtrlRegs.GPBDIR.bit.GPIO54 = 1;  //OUTPUT
    GpioDataRegs.GPBCLEAR.bit.GPIO54 = 1; //SET LOW

    //Relay1
    GpioCtrlRegs.GPBMUX1.bit.GPIO34 = 0; //GPIO
    GpioCtrlRegs.GPBDIR.bit.GPIO34 = 1;  //OUTPUT
    GpioDataRegs.GPBSET.bit.GPIO34 = 1; //SET HIGH

    //IN2 External stop
    //GpioCtrlRegs.GPBMUX2.bit.GPIO59 = 0; //GPIO
    //GpioCtrlRegs.GPBDIR.bit.GPIO59 = 0;  //INPUT

    //485_EN2
    GpioCtrlRegs.GPBMUX2.bit.GPIO58 = 0; //GPIO
    GpioCtrlRegs.GPBDIR.bit.GPIO58 = 1;  //OUTPUT
    GpioDataRegs.GPBCLEAR.bit.GPIO58 = 1; //SET LOW

    //DO1 AC RELAY
    //GpioCtrlRegs.GPBMUX1.bit.GPIO32 = 0; //GPIO
    //GpioCtrlRegs.GPBDIR.bit.GPIO32 = 1;  //OUTPUT
    //GpioDataRegs.GPBSET.bit.GPIO32 = 1; //SET HIGH

    //ID2
    GpioCtrlRegs.GPBMUX1.bit.GPIO33 = 0; //GPIO
    GpioCtrlRegs.GPBDIR.bit.GPIO33 = 0;  //INPUT

    //OUT2_Dsp
    //GpioCtrlRegs.GPAMUX1.bit.GPIO7 = 0; //GPIO
    //GpioCtrlRegs.GPADIR.bit.GPIO7 = 1;  //OUTPUT
    //GpioDataRegs.GPASET.bit.GPIO7 = 1; //SET HIGH

    //OUT1_Dsp
    //GpioCtrlRegs.GPAMUX1.bit.GPIO6 = 0; //GPIO
    //GpioCtrlRegs.GPADIR.bit.GPIO6 = 1;  //OUTPUT
    //GpioDataRegs.GPASET.bit.GPIO6 = 1; //SET HIGH


    //On board LED2 Alarm
    GpioCtrlRegs.GPAMUX1.bit.GPIO13 = 0; //GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO13 = 1;  //OUTPUT
    GpioDataRegs.GPASET.bit.GPIO13 = 1; //SET HIGH

    EDIS;
}
