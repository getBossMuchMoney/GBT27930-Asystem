/*******************************************************************************
 * File Name: MeasurementTask.h
 * Description: The measurement functions and calibration.
 * Designer: Miller Xue
 * Version: 00
 * Copyright: Liyuan Haina Co.Ltd
 *******************************************************************************/
#ifndef _MASUREMENTTASK_H
#define _MASUREMENTTASK_H



//Measurment task definition

#define    eMeasureTimerEvt                 0
#define    eMeasureACCalEvt                 1

#define    cMeasureTest                     0

#define     FanOff                          0
#define     FanLowSpd                       4000
#define     FanMidSpd                       5200
#define     FanHighSpd                      6000


typedef struct {
    uint16_t u16_SC_V_Avg;
    uint16_t u16_Output_I_Avg;
    uint16_t u16_Output_V_Avg;
    uint16_t u16_Output_P;

    int16_t  i16_Temperature[8];
}MEASURE_ST;

extern void sMeasureTask(void);
extern void sMeasureTaskInit(void);
extern MEASURE_ST stMeasure;

#endif




