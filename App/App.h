/*******************************************************************************
 * File Name: app.h
 * Description: The system setting and the system macros definition
 * Designer: Miller Xue
 * Version: 00
 * Copyright: Liyuan Haina Co.Ltd
 *******************************************************************************/
#ifndef _APP_H
#define _APP_H
#include "F28x_Project.h"
#include "hw_types.h"
#define    cPWM_OpenLoop_Test   0
#define    cSW_VERSION          10010

//typedef uint16_t uint8_t;
//typedef int16_t int8_t;

#define    cPrioJ1939                     5
#define    cPrioSuper                     1
#define    cPrioProtect                   2
#define    cPrioMeasure                   3
#define    cPrioCharge                    4
#define    cPrioDebug                     6
#define    cPrioUart                      7
#define    cPrioDataProc                  8
#define    cPrioPara                      9
#define    cPrioEEprom                    11
#define    cPrioLed                       10
#define    eLedTimerEvt                 0

//#define TRUE 1
//#define FALSE 0


extern void sMcuInitial(void);
extern interrupt void sADCA1_isr(void);
extern interrupt void sRtosTimerInterruptIsr(void);
extern interrupt void sSciaRxIsr(void);
extern interrupt void sSciaTxIsr(void);
extern interrupt void sScibRxIsr(void);
extern interrupt void sScibTxIsr(void);
extern interrupt void sCanAIsr(void);
void CheckErrSingal(uint8_t data);
#endif


