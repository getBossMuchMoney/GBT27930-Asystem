/*******************************************************************************
 * File Name: ParallelTask.h
 * Description: The Parallel task functions.
 * Designer: Miller Xue
 * Version: 00
 * Copyright: Liyuan Haina Co.Ltd
 *******************************************************************************/

#ifndef APP_PARALLELTASK_H_
#define APP_PARALLELTASK_H_

#define     cParaCanBase             0
#define     cParaInqMboxId           11
#define     cParaCtrlMboxId          9
#define     cParaSetMboxId           10
#define     cParaErrMboxId           8

#define     cParaInqRxMboxId         4
#define     cParaCtrlRxMboxId        5
#define     cParaSetRxMboxId         6


#define     cParaTimerEvt           0
#define     cParaSynCtrlEvt         1
#define     cParaSingleCtrlEvt      2
#define     cParaSynSettingEvt      3
#define     cParaSingleSettingEvt   4

#define     cParaSlave                  0
#define     cParaMaster                 1

#define     cParaSignInState            0
#define     cParaOnlineState            1
#define     cParaFaultState             2

#define     cParaSynNone                0
#define     cParaSynTurnOn              1
#define     cParaSynSetting             2

#define     cParaBroad               0

#define     cParaAckNormal           0
#define     cParaAckCodeErr          1
#define     cParaAckAddrErr          2
#define     cParaAckDataErr          3

#define     cParaForceAck            0x0
#define     cParaNormalAck           0x1


#define     cPara_20ms               1
#define     cPara_40ms               2
#define     cPara_100ms              5
#define     cPara_200ms              10
#define     cPara_400ms              20
#define     cPara_500ms              25
#define     cPara_800ms              40
#define     cPara_1s                 50
#define     cParaInqTimerOut         cPara_200ms
#define     cParaResponseTimerOut    cPara_40ms

//Flag
#define     cTxSuccess          0
#define     cTxFail             1
#define     cRxSuccess          0
#define     cRxEmpty            1

#define     cListEmpty          0
#define     cListNotEmpty       1

#define     cParaInqCmdNum            12
#define     cParaInqSetNum            27
#define     cWriteListDepth           20
#define     cReadListDepth            30


#define   cStaRegStart     0
#define   cStaRegEnd       41
#define   cSetRegStart     128
#define   cSetRegEnd       249


typedef union {
    uint32_t IdFrame;
    struct {
        uint32_t SrcId:8;
        uint32_t DesId:8;
        uint32_t FuncCode:8;
        uint32_t AckSts:2;
        uint32_t CtrlFlag:1;
        uint32_t SlaveForceAck:1;
        uint32_t Resv:4;
    }BIT;
}CANID_UNION;

typedef struct {
    CANID_UNION CanId;
    uint16_t    u8_CanData[8];
}CAN_FRAME;


typedef struct {
    uint16_t u16_Role;
    uint16_t u16_PhyId;
    uint32_t u32_McuId;
    uint16_t u16_SlaveNum;
    uint16_t u16_SlaveExistFlag;
    uint32_t u32_SlaveMcuId[16];
    uint16_t u16_MasterExistFlag;
    uint16_t u16_SlaveResponseFlag;
    uint16_t u16_SlaveReadResponseFlag;
    uint16_t u16_SlaveCtrlResponseFlag;
    uint16_t u16_SlaveSetResponsFlag;
    uint16_t u16_ParaState;
    uint16_t u16_SyncMode;
    uint16_t u16_QueryTimeOutCnt;
    uint16_t u16_ModeCnt;
    uint16_t u16_ModeDelayCnt;
    uint16_t u16_InqCmdIndex;
    CAN_FRAME stCanFrameTx;
    CAN_FRAME stCanFrameRx;
}PARA_STRUCT;

extern void sParallelTaskInit(void);
extern void sParallelTask(void);
#endif /* APP_PARALLELTASK_H_ */
