/*******************************************************************************
 * File Name: SuperTask.c
 * Description: The system status machine.
 * Designer: Miller Xue
 * Version: 00
 * Copyright: Liyuan Haina Co.Ltd
 *******************************************************************************/
#ifndef APP_SUPERTASK_H_
#define APP_SUPERTASK_H_

//Super task definition

#define    eSuperTimerEvt                 0
#define    eSuperTurnOnEvt                1
#define    eSuperTurnOffEvt               2
#define    eSuperFaultEvt                 3
#define    eSuperClearFaultEvt            4

#define    cSuper1s                       50
#define    cSuper2s                       100
#define    cSuper5s                       250
#define    cSuper10s                      500
#define    cSuper20s                      1000
#define    cSuper30s                      1500
#define    cSuper60s                      3000

#define    cPowerOnMode       0
#define    cStandbyMode       1
#define    cSoftStartMode     2
#define    cRunMode           3
#define    cFaultMode         4
#define    cDischageMode      5

#define    cPwmOn             1
#define    cPwmOff            0
#define    cRlyOn             1
#define    cRlyOff            0
#define    cFanOn             1
#define    cFanOff            0

#define    OP_CC              0
#define    OP_CV              1

#define    cPreChargeVolt     4000
#define    cSoftStartVolt     (uint16_t)(f32_PfcBusRefCmd * 10.0f)
#define    cBusNormalStep     50
//#define    cBusRefCmd         200.0f
#define    cGridVoltPreCharge  400         //40v rms

#define    V_SoftStart_TARGET  5800     //软起动电压值 600V
#define    V_SoftStart_PROT    500      //软起动电压值 50V
#define    V_TurnOnBusVoltMax  4100     //410V
#define    V_TurnOnBusVoltMin  3900     //390V

typedef union {
    uint16_t all;
    struct {
        uint16_t BusOVP:1;
        uint16_t BusUVP:1;
        uint16_t OpVoltOVP:1;
        uint16_t OutputCurrOver:1;
        uint16_t PowerFail:1;
        uint16_t PfcFault:1;
        uint16_t PfcCommFault:1;
        uint16_t OpCurrOCP:1;
        uint16_t OverLoad:1;
        uint16_t OverTemp1:1;
        uint16_t FanFault:1;
        uint16_t PreChargeFail:1;
        uint16_t PfcLossCom:1;
        uint16_t Resv:3;
    }BIT;
}FAULT_BIT;

typedef union{
    uint16_t all;
    struct {
        uint16_t OpRly:1;
        uint16_t OpenLoopEn:1;
        uint16_t PwmStatus:1;
        uint16_t PfcRunStatus:1;
        uint16_t FaultBack:1;
        uint16_t BusPreCharge:1;
        uint16_t OpPreChargeEn:1;
        uint16_t OpPreCharge:1;
        uint16_t Rsrv:8;
    }BIT;
}SYS_BIT;

typedef struct {
    uint16_t    u16_SysMode;
    uint16_t    u16_PowerOnModeCnt;
    uint16_t    u16_PreModeCnt;
    uint16_t    u16_PreChargeDelay;
    uint16_t    u16_StandbyCnt;
    uint16_t    u16_SoftStartModeCnt;
    uint16_t    u16_DischageModeCnt;
    uint16_t    u16_FaultModeCnt;
    uint16_t    u16_OutputMode;
    uint16_t    u16_PhysicalAddr;
    SYS_BIT     u16_SysStatus;
    FAULT_BIT   u32_FaultCode;
    uint16_t    u32_SwVersion;
}SUPER_ST;

typedef struct {
    float f32_VoutRefCmd;
    float f32_VoutRef;
    float f32_VoutStep;

    float f32_IoutRefCmd;
    float f32_IoutRef;
    float f32_IoutStep;

    float f32_PoutRefCmd;

    uint16_t u16_OutputMode;
    uint16_t u16_DcLoopCnt;
}OUTPUT_CTRL_ST;

extern SUPER_ST stSuper;
extern OUTPUT_CTRL_ST stOutputCtrl;
extern void sSuperTaskInit(void);
extern void sSuperTask(void);
extern void sAllPWMOff(void);

#endif /* APP_SUPERTASK_H_ */
