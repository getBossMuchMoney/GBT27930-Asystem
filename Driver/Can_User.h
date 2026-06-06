/*
 * CAN_User.h
 *
 *  Created on:
 *      Author: RUI_HENG
 */

#ifndef DRIVER_CAN_USER_H_
#define DRIVER_CAN_USER_H_

#include "can.h"
#include "interrupt.h"
#include "string.h"
#include "hw_can.h"

#define MSG_DATA_LENGTH 8
#define DEVICE_CLK_FREQ 120000000UL

typedef enum
{
    CTRL_RX_OBJ = 1,
    CTRL_RX_BROD_OBJ,
    ERR_RX_OBJ,
    SET_RX_OBJ,
    SET_RX_BROD_OBJ,
    QUERY_RX_OBJ,
    QUERY_RX_BROD_OBJ,

    ERR_TX_OBJ,
    CTRL_TX_OBJ,
    SET_TX_OBJ,
    QUERY_TX_OBJ,

    BOOT_RX_OBJ,

} MesObjType;

typedef enum
{
    READ_TYPE = 0x03,
    SET_TYPE = 0x00,
    CTRL_TYPE = 0x01,
} ID_Type;

typedef enum
{
    ERR_FLAG = 0,
    NON_ERR,
} SlaverErrFlag;


typedef enum
{
    BROADCAST_ID = 0,
    MASTER_ID = 0,
    SLAVER1_ID = 0x01,
    SLAVER2_ID = 0x02,
    SLAVER3_ID = 0x03,
    SLAVER4_ID = 0x04,
    SLAVER5_ID = 0x05,
    SLAVER6_ID = 0x06,
    SLAVER7_ID = 0x07,
    SLAVER8_ID = 0x08,
    SLAVER9_ID = 0x09,
    SLAVER10_ID = 0x0A,
    SLAVER11_ID = 0x0B,
    SLAVER12_ID = 0x0C,
} ID_manage;


typedef enum
{
    CAN_A = 0,
    CAN_B,
} CAN_MODULE;


typedef struct
{
    uint32_t uid;
    uint8_t  device_id1;
    uint8_t  device_id;
    uint8_t  A_ERR_TX_OK;
    uint8_t  A_CTRL_TX_OK;
    uint8_t  A_SET_TX_OK;
    uint8_t  A_GUERY_TX_OK;

    uint8_t B_ERR_TX_OK;
    uint8_t B_CTRL_TX_OK;
    uint8_t B_SET_TX_OK;
    uint8_t B_GUERY_TX_OK;
} CAN_USER;


extern CAN_USER can_opt;

void sCanAInit(void);
void sCanBInit(void);
void sCanidCheck(void);

void CAN_MessageObjectSet(CAN_MODULE mod, uint32_t objID, uint32_t msgID, CAN_MsgFrameType frame,
                          CAN_MsgObjType msgType, uint32_t msgIDMask, uint32_t flags, uint16_t msgLen);
#endif /* DRIVER_CAN_USER_H_ */
