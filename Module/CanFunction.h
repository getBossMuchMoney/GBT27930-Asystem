/*******************************************************************************
 * File Name: CanFunction.h
 * Description:
 * Designer: Dell
 * Version: 00
 * Copyright: Liyuan Haina Co.Ltd
 *******************************************************************************/
#ifndef MODULE_CANFUNCTION_H_
#define MODULE_CANFUNCTION_H_

#include "can.h"
#include "interrupt.h"
#include "string.h"
#include "hw_can.h"
#include "Can_User.h"

#define MSG_DATA_LENGTH 8
#define CAN_QUEUE_DEPTH 20

#define cCanTaskTimerEvt     0
#define cCanTaskActEvt       1
#define cCanTaskRcvErrEvt    2
#define cCanTaskForwEvt      3

typedef enum
{
    CTRL = 0,
    SET,
    QUERY,
} PriorityType;

typedef enum
{
    CAN_OK = 0,
    CAN_CODE_ERR,
    CAN_ADDR_ERR,
    CAN_CMD_ERR,
    CAN_VALUE_ERR,
    CAN_SEND_TIMEOUT,
    CAN_RECV_TIMEOUT,
    CAN_PARAM_ERR,

} CAN_ERRCODE;

// unit:ms
typedef enum
{
    NO_TIMEOUT = 0,
    TIME_LEVEL1 = 2,
    TIME_LEVEL2 = 3,
    TIMR_LEVEL3 = 4,

} TIMEOUT_SET;

typedef struct
{
    uint32_t         msgID;
    CAN_MsgFrameType frameType;
    uint8_t          size;
    uint8_t          buff[8];
} CAN_BUFF;


typedef struct
{
    CAN_MsgFrameType frameType;
    uint32_t rxMsgID;
    uint32_t RXobjID;
    uint32_t TXobjID;
    uint8_t *ptxdata;
    uint8_t rxmsgData[8];
    PriorityType rxMsgType;
    uint8_t rxSize;
    CAN_MODULE RXmod;
    CAN_MODULE TXmod;
    uint32_t txMsgID;
    uint8_t txLen;
}CanPara_OP;


typedef struct
{
    uint16_t head;
    uint16_t tail;
    uint16_t length;
    CAN_BUFF data[CAN_QUEUE_DEPTH];
}CAN_QUEUE;


extern uint32_t timercnt;
extern CanPara_OP ParaCan;

extern CAN_MsgFrameType can_frame_type;

uint32_t get_timer(void);

uint8_t enCanRXQueue(void);
uint8_t deCanRXQueue(CAN_MODULE mod, PriorityType cmdType, CAN_BUFF *pmbuff);
uint8_t enCanForwdQueue(void);
uint8_t deCanForwdQueue(CAN_MODULE mod, PriorityType cmdType, CAN_BUFF *pmbuff);


bool CAN_readMesID(CanPara_OP *pPara);

void CAN_sendMes(CanPara_OP *pPara);

CAN_ERRCODE CAN_SendData(CanPara_OP *pPara);

#endif /* MODULE_CANFUNCTION_H_ */
