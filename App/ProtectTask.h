/*******************************************************************************
 * File Name: ProtectTask.h
 * Description: The protection task for the system.
 * Designer: Miller Xue
 * Version: 00
 * Copyright: Liyuan Haina Co.Ltd
 *******************************************************************************/

#ifndef APP_PROTECTTASK_H_
#define APP_PROTECTTASK_H_

#define     eProtectTimerEvt            0
#define     eProtectAcCheck             1
#define     eProtectDcCheck             2

#define     cCheckNormal                0
#define     cCheckOutRange              1

#define     cProtect_20ms               1
#define     cProtect_40ms               2
#define     cProtect_60ms               3
#define     cProtect_100ms              5
#define     cProtect_1s                 50
#define     cProtect_10s                500
#define     cProtect_1min               3000
#define     cProtect_90s                4500
#define     cProtect_5min               15000


#define     cRatedGridVolt              380.0f
#define     cOutRangeVoltRatio          1.2f
#define     cOutRangeVoltRatio1         0.9f
#define     cRatedGridCurr              220.0f
#define     cOutRangeCurrRatio          1.2f
#define     cBusOverVolt                420.0f
#define     cBusUnderVolt               310.0f
#define     cOutputVoltOver             720.0f
#define     cRatedOutputCurr            5.0f
#define     cRatedOutputVolt            720.0f
#define     cRatedOutputPower           3000.0f
#define     cOverLoadLimit1             1.1f
#define     cOverLoadLimit2             1.2f

#define     cFaultTempOver              107.0f
#define     cPowerTempOver1             105.0f
#define     cPowerTempOver2             100.0f
#define     cPowerLimitCurrLow          500.0f
#define     cPowerLimitVoltLow          2.0f
#define     cPowerLimitRatio            0.05f
#define     cPowerRecovRatio            0.05f

#define     cVoltUnbalanceRatio         0.1f

#define     cFanErrCnt                  1000

typedef enum {
    OverCheck = 0,
    UnderCheck = 1,
}CHECK_TYPE;

typedef struct {
    uint16_t    u16_Threshold;
    uint16_t    u16_OutRangeCntSet;
    uint16_t    u16_OutRangeCnt;
}OUTRANGE_STRUCT;

typedef struct {
    uint16_t    u16_Threshold;
    uint16_t    u16_RecoverCntSet;
    uint16_t    u16_RecoverCnt;
}RECOVER_STRUCT;

typedef struct {
    uint32_t    u32_OverLoadLimit1;
    uint32_t    u32_OverLoadLimit2;
    uint32_t    u32_OverLoadCntSet1;
    uint32_t    u32_OverLoadCntSet2;
    uint32_t    u32_OverLoadCnt1;
    uint32_t    u32_OverLoadCnt2;
}OVERLOAD_STRUCT;

typedef struct {
uint32_t    u32_OverTempLimit;
uint32_t    u32_OverTempCntSet;
uint32_t    u32_OverTempCnt;
}OVERTEMP_STRUCT;

typedef struct {
    uint16_t    u16_PowerLimitFlag;
    uint16_t    u16_PowerLimitFirstFlag;
    uint32_t    u32_OverTempPowerLimit1;
    uint32_t    u32_OverTempPowerLimit2;
    uint32_t    u32_OverTempPowerCntSet;
    uint32_t    u32_OverTempPowerCnt;
    float       f32_PowerLimit;
    float       f32_PowerLimitRef;
    float       f32_PowerLimitStep;
    float       f32_PowerLimitLow;
}OVERTEMPPOWER_STRUCT;

extern void sProtectTaskInit(void);
extern void sProtectTask(void);

extern uint16_t u16_FanWorkFlag;
extern float f32_PowerLimitCurr;
extern float f32_PowerLimitVolt;

#endif /* APP_PROTECTTASK_H_ */
