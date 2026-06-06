/*
 * Sci_User.c
 *
 *  Created on: 2024��12��17��
 *      Author: dell
 */


#include "Sci_User.h"
#include "App.h"
//#include "Crc16.h"
#include "F28x_Project.h"
#include "Gpio_User.h"
#include "RTOS_APP.h"
#include "RTOS_Define.h"
#include "UartTask.h"
#include "string.h"
//#include "pin_map.h"
//#include "gpio.h"

uint16_t u16SciBandConfigList[5][2] = {
       {0x02,0x8A},
       {0x00,0xC2},
       {0x00,0xA2},
       {0x00,0x6C},
       {0x00,0x35}
};

void sSciInit(void)
{

#ifdef _LAUNCHXL_F28379D

    GPIO_setPinConfig(GPIO_42_SCITXDA);
    GPIO_setPinConfig(GPIO_43_SCIRXDA);

#else
    EALLOW;
    GpioCtrlRegs.GPBGMUX2.bit.GPIO48 = 1;
    GpioCtrlRegs.GPBMUX2.bit.GPIO48 = 2;  // SCIA TX
    GpioCtrlRegs.GPBGMUX2.bit.GPIO49 = 1;  // SCIA RX
    GpioCtrlRegs.GPBMUX2.bit.GPIO49 = 2;  // SCIA RX


    GpioCtrlRegs.GPBGMUX1.bit.GPIO40 = 2;  //
    GpioCtrlRegs.GPBMUX1.bit.GPIO40 = 1;  // SCIB TX
    GpioCtrlRegs.GPBGMUX1.bit.GPIO41 = 2;  //
    GpioCtrlRegs.GPBMUX1.bit.GPIO41 = 1;  // SCIB RX
    GpioCtrlRegs.GPBMUX1.bit.GPIO47 = 0;  // GPIO
    GpioCtrlRegs.GPBDIR.bit.GPIO47 = 1;   // OUTPUT
    GpioDataRegs.GPBCLEAR.bit.GPIO47 = 1; // SET LOW
    EDIS;
#endif

    //disable scia
    SciaRegs.SCICTL1.bit.RXENA = 0;
    SciaRegs.SCICTL1.bit.TXENA = 0;
    SciaRegs.SCIFFTX.bit.SCIFFENA = 0;

    SciaRegs.SCIFFTX.bit.SCIRST = 0;
    SciaRegs.SCIFFTX.bit.SCIRST = 1;


    // One stop bit, no parity, 8-bit character length
    SciaRegs.SCICCR.bit.SCICHAR = 0x7;

    // Enable transmit and receive
    SciaRegs.SCICTL1.bit.RXENA = 1;
    SciaRegs.SCICTL1.bit.TXENA = 1;

    // The default of the LOSPSP is 50MHz
    // BRR = (30MHz/(baudrate*8)) - 1 = 0x144
    SciaRegs.SCIHBAUD.bit.BAUD = u16SciBandConfigList[BAND19200][0];
    SciaRegs.SCILBAUD.bit.BAUD = u16SciBandConfigList[BAND19200][1];

    // Enable Receive interrupt
    SciaRegs.SCICTL2.bit.RXBKINTENA = 1;
    SciaRegs.SCICTL1.bit.RXERRINTENA = 1;
    SciaRegs.SCICTL1.bit.SWRESET = 1; // Relinquish SCI from Reset

    //disable scib
    ScibRegs.SCICTL1.bit.RXENA = 0;
    ScibRegs.SCICTL1.bit.TXENA = 0;
    ScibRegs.SCIFFTX.bit.SCIFFENA = 0;

    ScibRegs.SCIFFTX.bit.SCIRST = 0;
    ScibRegs.SCIFFTX.bit.SCIRST = 1;


    // One stop bit, no parity, 8-bit character length
    ScibRegs.SCICCR.bit.SCICHAR = 0x7;

    // Enable transmit and receive
    ScibRegs.SCICTL1.bit.RXENA = 1;
    ScibRegs.SCICTL1.bit.TXENA = 1;

    // The default of the LOSPSP is 50MHz
    // BRR = (30MHz/(baudrate*8)) - 1 = 0x144
    ScibRegs.SCIHBAUD.bit.BAUD = u16SciBandConfigList[BAND19200][0];
    ScibRegs.SCILBAUD.bit.BAUD = u16SciBandConfigList[BAND19200][1];

    // Enable Receive interrupt
    ScibRegs.SCICTL2.bit.RXBKINTENA = 1;
    ScibRegs.SCICTL1.bit.RXERRINTENA = 1;
    ScibRegs.SCICTL1.bit.SWRESET = 1; // Relinquish SCI from Reset


}



