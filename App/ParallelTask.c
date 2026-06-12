/*******************************************************************************
 * File Name: ParallelTask.c
 * Description: The Parallel task functions.
 * Designer: Miller Xue
 * Version: 00
 * Copyright: Liyuan Haina Co.Ltd
 *******************************************************************************/
#include "App.h"
#include "F28x_Project.h"
#include "Adc_User.h"
#include "PWM_User.h"
#include "Gpio_User.h"
#include "RTOS_APP.h"
#include "RTOS_Define.h"
#include "MeasureTask.h"
#include "ShareData.h"
#include "math.h"
#include "stdio.h"
#include "SuperTask.h"
#include "ParallelTask.h"
#include "ModbusRtu.h"
#include "Can_User.h"
#include "CanFunction.h"
#include "UartTask.h"

PARA_STRUCT     stParaStruct;


void     sParallelTaskInit(void);
void     sParallelTask(void);
void     sParaSignInState(void);
void     sParaOnlineState(void);
void     sParaFaultState(void);
uint16_t sParaCanWrite(uint16_t u16_CanBase, uint16_t u16_MboxId, CAN_FRAME *stCanFrameTx);
uint16_t sParaCanRead(uint16_t u16_CanBase, uint16_t u16_MboxId, CAN_FRAME *stCanFrameRx);


void sSlaveWaitForMasterCmd(void);
void sSlaveResponseCmd(void);
void sSlaveExecuteReadCmd(uint16_t u16_RegAddr, uint16_t u16_Length, uint16_t *pdata);
uint8_t sSlaveExecuteWriteCmd(uint16_t u16_RegAddr, uint16_t u16_Length, uint16_t *pData);


void sParallelTaskInit(void)
{
    uint16_t i;
    stParaStruct.u16_ParaState = cParaSignInState;
//    stParaStruct.u16_PhyId = can_opt.device_id;
//    stParaStruct.u32_McuId = UidRegs.UID_UNIQUE;
    for (i = 0; i < 8; i++)
    {
        stParaStruct.stCanFrameTx.u8_CanData[i] = 0;
    }

//    if (stParaStruct.u16_PhyId != MASTER_ID)
//    {
//        stParaStruct.u16_Role = cParaSlave;
//    }
//    else
//    {
//        stParaStruct.u16_Role = cParaMaster;
//        stParaStruct.u16_InqCmdIndex = 0;
//    }
    stParaStruct.u16_SlaveExistFlag = 0;
    stParaStruct.u16_MasterExistFlag = 0;
    stParaStruct.u16_SlaveResponseFlag = 0;
    stParaStruct.u16_SlaveSetResponsFlag = 0;
    stParaStruct.u16_SlaveReadResponseFlag = 0;
    stParaStruct.u16_SlaveCtrlResponseFlag = 0;
    stParaStruct.u16_SlaveNum = 0;
    stParaStruct.u16_SyncMode = cParaSynNone;
    stParaStruct.u16_ModeCnt = 0;

    // Can ID frame
    stParaStruct.stCanFrameTx.CanId.IdFrame = 0;
    stParaStruct.stCanFrameTx.CanId.BIT.AckSts = cParaAckNormal;
    stParaStruct.stCanFrameTx.CanId.BIT.DesId = cParaBroad;
    stParaStruct.stCanFrameTx.CanId.BIT.FuncCode = 0;
    stParaStruct.stCanFrameTx.CanId.BIT.SlaveForceAck = cParaNormalAck;
    stParaStruct.stCanFrameTx.CanId.BIT.SrcId = 1;

}

void sParallelTask(void)
{
    if(CanaRegs.CAN_CTL.bit.Init == 1)
    {
        sCanAInit();
    }
    //sParaCanWrite(cParaCanBase, cParaCtrlMboxId, &stParaStruct.stCanFrameTx);
    //sParaOnlineState();
}

void sParaSignInState(void)
{
    uint16_t event;

    event = suwRTOSGetEvent(cPrioPara);   // Get the current task event

    if (event & ((uint16_t)1 << cParaTimerEvt))
    {
        if (sParaCanRead(cParaCanBase, cParaInqRxMboxId, &stParaStruct.stCanFrameRx) == cRxSuccess)
        {
            if (stParaStruct.stCanFrameRx.CanId.BIT.SrcId == MASTER_ID)   // Master Exist
            {
                sSlaveResponseCmd();
                stParaStruct.u16_ParaState = cParaOnlineState;
                return;
            }
        }
    }

}

void sParaOnlineState(void)
{
    uint16_t event;
    static uint8_t Refl = 0;
    static uint16_t ForwdTimeCnt = 0;
    ModbusOpt *pMasterOpt = &MasterOpt;
    uint8_t i = 0;
    event = suwRTOSGetEvent(cPrioPara);   // Get the current task event
    if (event & ((uint16_t)1 << cParaTimerEvt))
    {
        sSlaveWaitForMasterCmd();
        ForwdTimeCnt++;
        if(ForwdTimeCnt == cPara_200ms)
        {
            ForwdTimeCnt = 0;

            if(Refl)
            {
                pMasterOpt->func_code = WIRITE_MULTI;
                pMasterOpt->addr = 0x180;
                pMasterOpt->reg_num = 7;
                for(i = 0;i<pMasterOpt->reg_num;i++)
                {
                    Value16SaveInBuff(uModSet.buff + i,pMasterOpt->pWriteData + i*2);
                }
                if(u16_ClearErrFlag == 1)
                {
                    u16_ClearErrFlag = 0;
                    pMasterOpt->pWriteData[13] |= 0x04;
                }

                Refl = 0;
            }
            else
            {
                pMasterOpt->station = 1;
                pMasterOpt->func_code = READ_REG;
                pMasterOpt->addr = 0x100;
                pMasterOpt->reg_num = 9;
                Refl = 1;
            }
            Modbus_Master_Ask(pMasterOpt);

        }
    }
}

void sParaFaultState(void)
{
    uint16_t event;
    event = suwRTOSGetEvent(cPrioPara);   // Get the current task event
    if (event & ((uint16_t)1 << cParaTimerEvt))
    {
        if (sParaCanRead(cParaCanBase, cParaInqRxMboxId, &stParaStruct.stCanFrameRx) == cRxSuccess)
        {
            if (stParaStruct.stCanFrameRx.CanId.BIT.SrcId == MASTER_ID)   // Master Exist
            {
                sSlaveResponseCmd();
                stParaStruct.u16_ParaState = cParaOnlineState;
                return;
            }
        }
    }
}


void sSlaveResponseCmd(void)
{
    uint8_t offset = 0,reg_num = 0,FuncCode = 0;

    stParaStruct.u16_MasterExistFlag = 1;
    FuncCode = stParaStruct.stCanFrameRx.CanId.BIT.FuncCode;


    memset(stParaStruct.stCanFrameTx.u8_CanData,0,8);
    stParaStruct.stCanFrameTx.CanId.BIT.AckSts = 0;

    if(FuncCode <= 0x0A)
    {
        if(stParaStruct.stCanFrameRx.CanId.BIT.CtrlFlag == 1)
        {
            if(sSlaveExecuteWriteCmd(FuncCode*4 + 384,4,stParaStruct.stCanFrameRx.u8_CanData) == 1)
            {
                stParaStruct.stCanFrameTx.CanId.BIT.AckSts = 0;
            }
            else
            {
                stParaStruct.stCanFrameTx.CanId.BIT.AckSts = 3;
            }

        }
        sSlaveExecuteReadCmd(FuncCode*4 + 256,4,stParaStruct.stCanFrameTx.u8_CanData);
    }
    else
    {
        if(FuncCode == 0x20)
        {
            offset = stParaStruct.stCanFrameRx.u8_CanData[0];
            reg_num = stParaStruct.stCanFrameRx.u8_CanData[1];

            if((offset <= cSetRegEnd) && ((offset + reg_num) <= (cSetRegEnd + 1)))
            {
                if(sSlaveExecuteWriteCmd(offset,reg_num,stParaStruct.stCanFrameRx.u8_CanData + 2) == 1)
                {
                    stParaStruct.stCanFrameTx.CanId.BIT.AckSts = 0;
                }
                else
                {
                    stParaStruct.stCanFrameTx.CanId.BIT.AckSts = 3;
                }

            }
            else
            {
                stParaStruct.stCanFrameTx.CanId.BIT.AckSts = 2;
            }

            memcpy(stParaStruct.stCanFrameTx.u8_CanData,stParaStruct.stCanFrameRx.u8_CanData,8);

        }
        else if(FuncCode == 0x21)
        {
            offset = stParaStruct.stCanFrameRx.u8_CanData[0];
            reg_num = stParaStruct.stCanFrameRx.u8_CanData[1];

            stParaStruct.stCanFrameTx.u8_CanData[0] = stParaStruct.stCanFrameRx.u8_CanData[0];
            stParaStruct.stCanFrameTx.u8_CanData[1] = stParaStruct.stCanFrameRx.u8_CanData[1];

            if(offset <  cSetRegStart)
            {
                if((offset <= cStaRegEnd) && ((offset + reg_num) <= (cStaRegEnd + 1)))
                {
                    stParaStruct.stCanFrameTx.CanId.BIT.AckSts = 0;
                    sSlaveExecuteReadCmd(offset,reg_num,stParaStruct.stCanFrameTx.u8_CanData + 2);
                }
                else
                {
                    stParaStruct.stCanFrameTx.CanId.BIT.AckSts = 2;
                }

            }
            else
            {
                if((offset <= cSetRegEnd) && ((offset + reg_num) <= (cSetRegEnd + 1)))
                {
                    stParaStruct.stCanFrameTx.CanId.BIT.AckSts = 0;
                    sSlaveExecuteReadCmd(offset,reg_num,stParaStruct.stCanFrameTx.u8_CanData + 2);
                }
                else
                {
                    stParaStruct.stCanFrameTx.CanId.BIT.AckSts = 2;
                }

            }

        }
        else
        {
            //非法功能码
            stParaStruct.stCanFrameTx.CanId.BIT.AckSts = 1;
            memcpy(stParaStruct.stCanFrameTx.u8_CanData,stParaStruct.stCanFrameRx.u8_CanData,8);
        }

    }

    stParaStruct.stCanFrameTx.CanId.BIT.FuncCode = FuncCode;
    stParaStruct.stCanFrameTx.CanId.BIT.CtrlFlag = stParaStruct.stCanFrameRx.CanId.BIT.CtrlFlag;
    stParaStruct.stCanFrameTx.CanId.BIT.DesId = stParaStruct.stCanFrameRx.CanId.BIT.SrcId;
    stParaStruct.stCanFrameTx.CanId.BIT.SlaveForceAck =1;
    sParaCanWrite(cParaCanBase, cParaCtrlMboxId, &stParaStruct.stCanFrameTx);
}

void sSlaveWaitForMasterCmd(void)
{
    if (sParaCanRead(cParaCanBase, cParaCtrlRxMboxId, &stParaStruct.stCanFrameRx) == cRxSuccess)
    {
        if (stParaStruct.stCanFrameRx.CanId.BIT.SrcId == MASTER_ID)
        {
            sSlaveResponseCmd();
        }
    }

}


uint16_t sParaCanWrite(uint16_t u16_CanBase, uint16_t u16_MboxId, CAN_FRAME *stCanFraeTx)
{
    CanPara_OP *pPara = &ParaCan;
    pPara->TXmod = (CAN_MODULE)u16_CanBase;
    pPara->TXobjID = u16_MboxId;
    pPara->ptxdata = stCanFraeTx->u8_CanData;
    pPara->txMsgID = stCanFraeTx->CanId.IdFrame;
    if (CAN_SendData(pPara) != CAN_OK)
    {
        return cTxFail;
    }
    return cTxSuccess;
}

uint16_t sParaCanRead(uint16_t u16_CanBase, uint16_t u16_MboxId, CAN_FRAME *stCanFrameRx)
{
    CAN_BUFF data;
    if (u16_CanBase == CAN_A)
    {
        if (deCanRXQueue(CAN_A, CTRL, &data))
        {
            stCanFrameRx->CanId.IdFrame = data.msgID;
            memcpy(stCanFrameRx->u8_CanData, data.buff, data.size);
        }
        else
        {
            return cRxEmpty;
        }

    }
    else
    {
        if (deCanRXQueue(CAN_B, CTRL, &data))
        {
            stCanFrameRx->CanId.IdFrame = data.msgID;
            memcpy(stCanFrameRx->u8_CanData, data.buff, data.size);
        }
        else
        {
            return cRxEmpty;
        }
    }
    return cRxSuccess;
}


uint8_t sSlaveExecuteWriteCmd(uint16_t u16_RegAddr, uint16_t u16_Length, uint16_t *pData)
{
    if(write_REG(u16_RegAddr, u16_Length, pData) == MBEC_SUCSSES)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void sSlaveExecuteReadCmd(uint16_t u16_RegAddr, uint16_t u16_Length, uint16_t *pdata)
{
    read_REG(u16_RegAddr, u16_Length, pdata);
}






