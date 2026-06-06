/*
 * Sci_User.h
 *
 *  Created on: 2024��12��17��
 *      Author: dell
 */

#ifndef DRIVER_SCI_USER_H_
#define DRIVER_SCI_USER_H_

#include "App.h"
#include "F28x_Project.h"

typedef enum
{
    BAND9600 = 0,
    BAND19200,
    BAND38400,
    BAND57600,
    BAND115200
}BAND_CONFIG;

#define SCIA_SENDBYTE(x)              SciaRegs.SCITXBUF.bit.TXDT=x
#define SCIB_SENDBYTE(x)              ScibRegs.SCITXBUF.bit.TXDT=x
#define SCIC_SENDBYTE(x)              //ScicRegs.SCITXBUF.bit.TXDT=x

#define SCIA_TXINT_EN()          SciaRegs.SCICTL2.bit.TXINTENA=1
#define SCIA_TXINT_DIS()         SciaRegs.SCICTL2.bit.TXINTENA=0
#define SCIB_TXINT_EN()          ScibRegs.SCICTL2.bit.TXINTENA=1
#define SCIB_TXINT_DIS()         ScibRegs.SCICTL2.bit.TXINTENA=0
#define SCIC_TXINT_EN()          //ScicRegs.SCICTL2.bit.TXINTENA=1
#define SCIC_TXINT_DIS()         //ScicRegs.SCICTL2.bit.TXINTENA=0

void sSciInit(void);
extern uint16_t u16SciBandConfigList[5][2];


#endif /* DRIVER_SCI_USER_H_ */
