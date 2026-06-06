/*******************************************************************************
 * File Name: CanFunction.c
 * Description:
 * Designer: Dell
 * Version: 00
 * Copyright: Liyuan Haina Co.Ltd
 *******************************************************************************/
#include "F28x_Project.h"
#include "CanFunction.h"

uint32_t timercnt = 0;

CAN_MsgFrameType can_frame_type;
CanPara_OP ParaCan;
CanPara_OP *pParaCan = &ParaCan;

CAN_QUEUE can_RXqueue[1][1] = {
 {
  {
   .head = 0,
   .length = 0,
   .tail = 0,
  }
 },

};

CAN_QUEUE can_ForwdQueue = {
   .head = 0,
   .length = 0,
   .tail = 0,
};


static CAN_QUEUE (*pCanRXQueue)[1][1] = &can_RXqueue;

uint32_t get_timer(void)
{
    return timercnt;
}

uint8_t enCanRXQueue(void)
{
    uint16_t *ptail = &((*pCanRXQueue)[pParaCan->RXmod][pParaCan->rxMsgType].tail);
    uint16_t *plength = &((*pCanRXQueue)[pParaCan->RXmod][pParaCan->rxMsgType].length);
    CAN_BUFF *plist = (*pCanRXQueue)[pParaCan->RXmod][pParaCan->rxMsgType].data;
    uint8_t *pbuff = plist[*ptail].buff;

    if (*plength == CAN_QUEUE_DEPTH)
    {
        return 0;
    }
    plist[*ptail].msgID = pParaCan->rxMsgID;
    plist[*ptail].frameType = pParaCan->frameType;
    plist[*ptail].size = pParaCan->rxSize;

    memcpy(pbuff,pParaCan->rxmsgData,8);

    (*plength)++;

    if ((*ptail) < (CAN_QUEUE_DEPTH - 1))
    {
        (*ptail)++;
    }
    else
    {
        *ptail = 0;
    }

    return 1;
}


uint8_t deCanRXQueue(CAN_MODULE mod, PriorityType cmdType, CAN_BUFF *pmbuff)
{
    uint16_t *phead = &((*pCanRXQueue)[mod][cmdType].head);
    uint16_t *plength = &((*pCanRXQueue)[mod][cmdType].length);
    CAN_BUFF *plist = (*pCanRXQueue)[mod][cmdType].data;
    uint8_t *pbuff = plist[*phead].buff;

    if (*plength == 0)
    {
        return 0;
    }
    pmbuff->msgID = plist[*phead].msgID;
    pmbuff->frameType = plist[*phead].frameType;
    pmbuff->size = plist[*phead].size;
    memcpy(pmbuff->buff, pbuff, pmbuff->size);

    (*plength)--;

    if (*phead < CAN_QUEUE_DEPTH - 1)
    {
        (*phead)++;
    }
    else
    {
        *phead = 0;
    }

    return 1;
}


CAN_ERRCODE CAN_SendData(CanPara_OP *pPara)
{
    CAN_ERRCODE err = CAN_OK;
    uint32_t    t0, t1 = 0;

    if (pPara->TXmod == CAN_A)
    {
        pPara->txLen = MSG_DATA_LENGTH;
        CAN_sendMes(pPara);
        t0 = get_timer();
        while (1)
        {
            t1 = get_timer();
            if (*((uint8_t *)(&can_opt.A_ERR_TX_OK + pPara->TXobjID - ERR_TX_OBJ)) == 1)
            {
                *((uint8_t *)(&can_opt.A_ERR_TX_OK + pPara->TXobjID - ERR_TX_OBJ)) = 0;
                break;
            }
            if ((t1 - t0) > TIME_LEVEL1)
            {
                err = CAN_SEND_TIMEOUT;
                break;
            }
        }
        return err;
    }
    else
    {
        pPara->txLen = MSG_DATA_LENGTH;
        CAN_sendMes(pPara);
        t0 = get_timer();
        while (1)
        {
            t1 = get_timer();
            if (*((uint8_t *)(&can_opt.B_ERR_TX_OK + pPara->TXobjID - ERR_TX_OBJ)) == 1)
            {
                *((uint8_t *)(&can_opt.B_ERR_TX_OK + pPara->TXobjID - ERR_TX_OBJ)) = 0;
                break;
            }
            if ((t1 - t0) > TIME_LEVEL1)
            {
                err = CAN_SEND_TIMEOUT;
                break;
            }
        }
        return err;
    }
}


bool CAN_readMesID(CanPara_OP *pPara)
{
    bool     status;
    uint16_t msgCtrl = 0U;


    if (pPara->RXmod == CAN_A)
    {
        CanaRegs.CAN_IF2CMD.all =
            ((uint32_t)CAN_IF2CMD_DATA_A | (uint32_t)CAN_IF2CMD_DATA_B | (uint32_t)CAN_IF2CMD_CONTROL |
             (pPara->RXobjID & CAN_IF2CMD_MSG_NUM_M) | (uint32_t)CAN_IF2CMD_ARB);

        // Wait for busy bit to clear
        while (1 == CanaRegs.CAN_IF2CMD.bit.Busy);



        msgCtrl = CanaRegs.CAN_IF2MCTL.all;

        if ((msgCtrl & CAN_IF2MCTL_NEWDAT) == CAN_IF2MCTL_NEWDAT)
        {
            pPara->rxSize = (uint32_t)msgCtrl & CAN_IF2MCTL_DLC_M;

            pPara->rxmsgData[0] = (CanaRegs.CAN_IF2DATA.all)&0xFF;
            pPara->rxmsgData[1] = (CanaRegs.CAN_IF2DATA.all >> 8)&0xFF;
            pPara->rxmsgData[2] = (CanaRegs.CAN_IF2DATA.all >> 16)&0xFF;
            pPara->rxmsgData[3] = (CanaRegs.CAN_IF2DATA.all >> 24)&0xFF;
            pPara->rxmsgData[4] = (CanaRegs.CAN_IF2DATB.all)&0xFF;
            pPara->rxmsgData[5] = (CanaRegs.CAN_IF2DATB.all>>8)&0xFF;
            pPara->rxmsgData[6] = (CanaRegs.CAN_IF2DATB.all>>16)&0xFF;
            pPara->rxmsgData[7] = (CanaRegs.CAN_IF2DATB.all>>24)&0xFF;

            status = true;
            CanaRegs.CAN_IF2CMD.all = ((uint32_t)CAN_IF2CMD_TXRQST | (pPara->RXobjID & CAN_IF2CMD_MSG_NUM_M));
            // Wait for busy bit to clear
            //while (1 == CanaRegs.CAN_IF2CMD.bit.Busy);
        }
        else
        {
            status = false;
        }


        if (status)
        {
            if (((CanaRegs.CAN_IF2ARB.all) & CAN_IF2ARB_XTD) != 0U)
            {
                pPara->frameType = CAN_MSG_FRAME_EXT;
                pPara->rxMsgID = CanaRegs.CAN_IF2ARB.all & CAN_IF2ARB_ID_M;
            }
            else
            {
                pPara->frameType = CAN_MSG_FRAME_STD;
                pPara->rxMsgID = (CanaRegs.CAN_IF2ARB.all & CAN_IF2ARB_STD_ID_M) >> CAN_IF2ARB_STD_ID_S;
            }
        }
    }
    else
    {
//        CanbRegs.CAN_IF2CMD.all =
//            ((uint32_t)CAN_IF2CMD_DATA_A | (uint32_t)CAN_IF2CMD_DATA_B | (uint32_t)CAN_IF2CMD_CONTROL |
//             (pPara->RXobjID & CAN_IF2CMD_MSG_NUM_M) | (uint32_t)CAN_IF2CMD_ARB);
//
//        // Wait for busy bit to clear
//        while (1 == CanbRegs.CAN_IF2CMD.bit.Busy);
//
//        msgCtrl = CanbRegs.CAN_IF2MCTL.all;
//
//        if ((msgCtrl & CAN_IF2MCTL_NEWDAT) == CAN_IF2MCTL_NEWDAT)
//        {
//
//            pPara->rxSize = (uint32_t)msgCtrl & CAN_IF2MCTL_DLC_M;
//
//            pPara->rxmsgData[0] = (CanbRegs.CAN_IF2DATA.all)&0xFF;
//            pPara->rxmsgData[1] = (CanbRegs.CAN_IF2DATA.all >> 8)&0xFF;
//            pPara->rxmsgData[2] = (CanbRegs.CAN_IF2DATA.all >> 16)&0xFF;
//            pPara->rxmsgData[3] = (CanbRegs.CAN_IF2DATA.all >> 24)&0xFF;
//            pPara->rxmsgData[4] = (CanbRegs.CAN_IF2DATB.all)&0xFF;
//            pPara->rxmsgData[5] = (CanbRegs.CAN_IF2DATB.all>>8)&0xFF;
//            pPara->rxmsgData[6] = (CanbRegs.CAN_IF2DATB.all>>16)&0xFF;
//            pPara->rxmsgData[7] = (CanbRegs.CAN_IF2DATB.all>>24)&0xFF;
//
//            status = true;
//            CanbRegs.CAN_IF2CMD.all = ((uint32_t)CAN_IF2CMD_TXRQST | (pPara->RXobjID & CAN_IF2CMD_MSG_NUM_M));
//            // Wait for busy bit to clear
////            while (1 == CanbRegs.CAN_IF2CMD.bit.Busy);
//        }
//        else
//        {
//            status = false;
//        }
//
//        if (status)
//        {
//            if (((CanbRegs.CAN_IF2ARB.all) & CAN_IF2ARB_XTD) != 0U)
//            {
//                pPara->frameType = CAN_MSG_FRAME_EXT;
//                pPara->rxMsgID = CanbRegs.CAN_IF2ARB.all & CAN_IF2ARB_ID_M;
//            }
//            else
//            {
//                pPara->frameType = CAN_MSG_FRAME_STD;
//                pPara->rxMsgID = (CanbRegs.CAN_IF2ARB.all & CAN_IF2ARB_STD_ID_M) >> CAN_IF2ARB_STD_ID_S;
//            }
//        }
    }

    return status;
}

void CAN_sendMes(CanPara_OP *pPara)
{
    uint32_t msgCtrl, idx = 0U;
    uint32_t dataReg = 0;

    if (pPara->TXmod == CAN_A)
    {
        dataReg = (uint32_t)(&CanaRegs.CAN_IF1DATA);
        while (1 == CanaRegs.CAN_IF1CMD.bit.Busy);
        CanaRegs.CAN_IF1ARB.bit.ID = (pPara->txMsgID & CAN_IF1ARB_ID_M);
        CanaRegs.CAN_IF1CMD.all = ((uint32_t)CAN_IF1CMD_CONTROL | CAN_IF1CMD_DIR | CAN_IF1CMD_ARB | (pPara->TXobjID & CAN_IF1CMD_MSG_NUM_M));
        // Wait for busy bit to clear
        while (1 == CanaRegs.CAN_IF1CMD.bit.Busy);
        // Read IF message control
        msgCtrl = CanaRegs.CAN_IF1MCTL.all;

        // Write the data out to the CAN Data registers.

        for (idx = 0U; idx < (msgCtrl & CAN_IF1MCTL_DLC_M); idx++)
        {
            // Write out the data 8 bits at a time.
            __byte((int16 *)dataReg, 0) = pPara->ptxdata[idx];
            dataReg++;
        }

        //  Set Data to be transferred from IF
        if (pPara->txLen > 0U)
        {
            msgCtrl = CAN_IF1CMD_DATA_B | CAN_IF1CMD_DATA_A;
        }
        else
        {
            msgCtrl = 0U;
        }

        // Set Direction to write
        // Set Tx Request Bit
        // Transfer the message object to the message object specified by
        // objID.
        CanaRegs.CAN_IF1CMD.all =
            (msgCtrl | (uint32_t)CAN_IF1CMD_DIR | (uint32_t)CAN_IF1CMD_TXRQST | (pPara->TXobjID & CAN_IF1CMD_MSG_NUM_M));
    }
    else
    {
//        dataReg = (uint32_t)(&CanbRegs.CAN_IF1DATA);
//        while(1 == CanbRegs.CAN_IF1CMD.bit.Busy);
//        CanbRegs.CAN_IF1ARB.bit.ID = (pPara->txMsgID & CAN_IF1ARB_ID_M);
//        CanbRegs.CAN_IF1CMD.all = ((uint32_t)CAN_IF1CMD_CONTROL | CAN_IF1CMD_DIR | CAN_IF1CMD_ARB | (pPara->TXobjID & CAN_IF1CMD_MSG_NUM_M));
//        // Wait for busy bit to clear
//        while (1 == CanbRegs.CAN_IF1CMD.bit.Busy);
//        // Read IF message control
//        msgCtrl = CanbRegs.CAN_IF1MCTL.all;
//
//        // Write the data out to the CAN Data registers.
//
//        for (idx = 0U; idx < (msgCtrl & CAN_IF1MCTL_DLC_M); idx++)
//        {
//            // Write out the data 8 bits at a time.
//            __byte((int16 *)dataReg, 0) = pPara->ptxdata[idx];
//            dataReg++;
//        }
//
//        //  Set Data to be transferred from IF
//        if (pPara->txLen > 0U)
//        {
//            msgCtrl = CAN_IF1CMD_DATA_B | CAN_IF1CMD_DATA_A;
//        }
//        else
//        {
//            msgCtrl = 0U;
//        }
//
//        // Set Direction to write
//        // Set Tx Request Bit
//        // Transfer the message object to the message object specified by
//        // objID.
//        CanbRegs.CAN_IF1CMD.all =
//            (msgCtrl | (uint32_t)CAN_IF1CMD_DIR | (uint32_t)CAN_IF1CMD_TXRQST | (pPara->TXobjID & CAN_IF1CMD_MSG_NUM_M));
    }
}
