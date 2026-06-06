/*
 * UartTask.h
 *
 *  Created on: 2024��12��17��
 *      Author: dell
 */

#ifndef APP_UARTTASK_H_
#define APP_UARTTASK_H_
#include "ModbusRtu.h"

#define PFC_REG_END         134

typedef struct
{
    uint16_t u16_Grid_V;
    uint16_t u16_Grid_I;
    int16_t  u16_Vbus;
    uint16_t u16_ActPower;
    int16_t  i16_Temp1;
    uint16_t u16_PfcWorkMode;
    uint16_t u16_PfcFwVersion;
    uint16_t u16_PfcFaultCode;
    uint16_t u16_PfcStatus;
} PFC_STA;

void sSciTask(void);

extern ModbusOpt MasterOpt;

#endif /* APP_UARTTASK_H_ */
