/*
 * UartFunction.c
 *
 *  Created on: 2024��12��17��
 *      Author: dell
 */
#include "App.h"
#include "ModbusRtu.h"
#include "F28x_Project.h"
#include "Gpio_User.h"
#include "RTOS_APP.h"
#include "RTOS_Define.h"
#include "Sci_User.h"
#include "string.h"
#include "UartFunction.h"
#include "UartDebug.h"


uint8_t Uart_RX_buff[SCI_NUM][SCI_BUFF_SIZE] = {0};
SciStruct   UartList[SCI_NUM];
QUEUE       UartRX_QList[SCI_NUM];
uint8_t *pRxBuffer[SCI_NUM];
uint8_t u8SendHighHalfByte,u8Word_H,u8Word_L= 0;

SCI_TIM sci_tim = {
    .timer_a = 0,
    .timer_b = 0,
    .timer_c = 0,
    .rcv_a = 0,
    .rcv_b = 0,
    .rcv_b = 0,
};

TX_QUEUE MasterTxQueue = {.head = 0,
                           .tail = 0,
                           .length = 0,
};

void SciQueueInit(SCI_MODE mod,uint8_t bType)
{
    SciStruct   *pSci;
    QUEUE       *pQ;

    pSci =&UartList[mod];

    pSci->pqRx =&UartRX_QList[mod];
    pQ = pSci->pqRx;
    sQInit(pQ,Uart_RX_buff[mod],SCI_BUFF_SIZE,sizeof(Uart_RX_buff[mod][0]));

    pSci->bTxStatus = cSciTxRdy;
    pSci->wTxLength = 0;

    pSci->bSciType = bType;

    pRxBuffer[mod] = Uart_RX_buff[mod];

}

void sUartRxISR(SCI_MODE mod)
{
    SciStruct       *psci;
    QUEUE           *pq;
    uint8_t data = 0;

    psci =&UartList[mod];
    pq = psci->pqRx;

    if(sbSciGetRxRdy(mod) == cSciRxRdy)  // confirm recerve finished
    {
        //sSciResetRx(mod);
        data = sbSciGetRxData(mod);
        CheckErrSingal(data);
        set_sciTim(mod);
        if(sQDataIn(pq,&data,cQCoverLast) == cQBufFull)
        {
            sRTOSEventSendIsr(cPrioUart, cUartTaskSciaEvt + mod);
        }
    }
}

void sUartTxISR(SCI_MODE mod)
{
        SciStruct       *psci;

        psci =&UartList[mod];

        if(sbSciGetTxRdy(mod) == cSciTxRdy)
        {
            //sSciResetTx(mod);
            if(psci->wTxLength == 0)
            {
                psci->u8BinaryMode = 0;
                psci->bTxStatus = cSciTxRdy;

                *((uint16_t *)(&sci_tim.afterTXa + mod)) = 1;
                *((uint16_t *)(&sci_tim.TXwait_tima + mod)) = AFTER_TX_FINISH_CNT;

                switch (mod)
                {
                  case SCIA:
                  {
                      SCIA_TXINT_DIS();
                  }
                   break;

                  case SCIB:
                  {
                      SCIB_TXINT_DIS();
                  }
                  break;

                  case SCIC:
                  {
                      SCIC_TXINT_DIS();
                  }
                  break;

                  default:
                  {}break;
                }

            }
            else
            {
                if(psci->u8BinaryMode == 0)
                {
                    sSciTxData(mod,*(psci->pbTx));
                    (psci->pbTx)++;
                    (psci->wTxLength)--;
                }
                else    // sent 16-bit data in binary mode
                {
                    if(u8SendHighHalfByte == 1)  // low byte to be sent
                    {
                        sSciTxData(mod,u8Word_L);
                        u8SendHighHalfByte = 0;
                        (psci->pwTx)++;
                        (psci->wTxLength)--;
                    }
                    else    // high byte to be sent
                    {
                        dataSplit(*(psci->pwTx));
                        u8SendHighHalfByte = 1;
                        sSciTxData(mod,u8Word_H);
                    }
                }
            }
        }
}

#if !cDebugUartEn
#if cDataBufEn
void UartSendWaveData(void)
{
    uwTranmitDataBuff[0] = 0xAAAA;
    uwTranmitDataBuff[1] = 0x0001;
    uwTranmitDataBuff[2] = Gui_wSampleLength*4;

    sUartWriteBinary(SCIB,uwTranmitDataBuff,(Gui_wSampleLength*4 + 3));
}

void sUartQ3Com(uint8_t *pdata)
{
    uint16_t uwSnatchDataCntTemp;
    uint16_t uwIntervalTemp;
    uint16_t uwDataKindTemp1;
    uint16_t uwDataKindTemp2;
    uint16_t uwDataKindTemp3;
    uint16_t uwDataKindTemp4;

    uwSnatchDataCntTemp = (pdata[2] - 48) * 1000 + (pdata[3] - 48) * 100 + (pdata[4] - 48)*10 + (pdata[5] - 48);
    //bbb
    uwIntervalTemp = (pdata[7] - 48) * 10 + pdata[8] - 48;
    //cc
    uwDataKindTemp1 = (pdata[10] - 48) * 10 + pdata[11] - 48;
    //dd
    uwDataKindTemp2 = (pdata[13] - 48) * 10 + pdata[14] - 48;
    //ee
    uwDataKindTemp3 = (pdata[16] - 48) * 10 + pdata[17] - 48;
    //ff
    uwDataKindTemp4 = (pdata[19] - 48) * 10 + pdata[20] - 48;

    DINT;
    Gui_wSampleLength = uwSnatchDataCntTemp;
    Gui_wSampleRate = uwIntervalTemp;
    Gui_CH1_ID = uwDataKindTemp1;
    Gui_CH2_ID = uwDataKindTemp2;
    Gui_CH3_ID = uwDataKindTemp3;
    Gui_CH4_ID = uwDataKindTemp4;
    Gui_wLogEnable = 1;
    Gui_wLogStatus = 1;
    EINT;

}

#endif
#endif

void dataSplit(uint16_t wSendData)
{
    u8Word_H = (uint8_t)(wSendData >> 8);
    u8Word_L = (uint8_t)(wSendData & 0x00FF);
}


uint8_t sUartWrite(SCI_MODE mod,uint8_t *pstart,uint16_t wLength)
{
    SciStruct       *psci;

    psci =&UartList[mod];

    if (*((uint8_t *)(&sci_tim.rcv_a + mod)) == 1)
    {
        return cSciRxBusy; // sci recieve busy
    }

    if(psci->bTxStatus == cSciTxBusy)
    {
        return(cSciTxBusy);
    }

    DINT;
    psci->pbTx = pstart;
    psci->wTxLength = wLength;
    psci->bTxStatus = cSciTxBusy;

    sSciTxData(mod,*(psci->pbTx));
    (psci->pbTx)++;
    (psci->wTxLength)--;
    EINT;

    return(cSciTxRdy);
}


uint8_t sUartWriteBinary(SCI_MODE mod,uint16_t *pstart,uint16_t wLength)
{
    SciStruct       *psci;

    psci =&UartList[mod];

    if (*((uint8_t *)(&sci_tim.rcv_a + mod)) == 1)
    {
        return cSciRxBusy; // sci recieve busy
    }

    if(psci->bTxStatus == cSciTxBusy)
    {
        return(cSciTxBusy);
    }

    DINT;
    psci->u8BinaryMode = 1;
    psci->pwTx = pstart;
    psci->wTxLength = wLength;
    psci->bTxStatus = cSciTxBusy;
    //split
    dataSplit(*(psci->pwTx));
    u8SendHighHalfByte = 1;
    sSciTxData(mod,u8Word_H);
    EINT;
    return(cSciTxRdy);
}



void sci_tim_check(SCI_MODE mod)
{
    if (*((uint16_t *)(&sci_tim.timer_a + mod)) > 0)
    {
        (*((uint16_t *)(&sci_tim.timer_a + mod)))--;
    }
    else
    {
        if (1 == *((uint16_t *)(&sci_tim.rcv_a + mod)))
        {
            *((uint16_t *)(&sci_tim.rcv_a + mod)) = 0;
            sRTOSEventSendIsr(cPrioUart, cUartTaskSciaEvt + mod);
        }
    }
}


void get_scidata(SCI_MODE mod, uint8_t *pdata, uint16_t *psize)
{

    uint16_t i = 0;

    QUEUE           *pq;
    SciStruct       *psci;

    psci =&UartList[mod];
    pq=psci->pqRx;
    *psize = pq->length;
    for(i = 0; i < (*psize);i++)
    {
       sQDataOut(pq,pdata + i);
    }

}

void set_sciTim(SCI_MODE mod)
{
    *((uint16_t *)((&sci_tim.timer_a) + mod)) = SCI_RCV_TIMEOUT;
    *((uint16_t *)((&sci_tim.rcv_a) + mod)) = 1;
}


void check_sciTurntoRCV(void)
{
    if (sci_tim.afterTXa == 1)
    {
        if (sci_tim.TXwait_tima != 0)
        {
            sci_tim.TXwait_tima--;
        }
        else
        {
            sci_tim.afterTXa = 0;
            m485_RX1_EN();
        }
    }

    if (sci_tim.afterTXb == 1)
    {
        if (sci_tim.TXwait_timb != 0)
        {
            sci_tim.TXwait_timb--;
        }
        else
        {
            sci_tim.afterTXb = 0;
            m485_RX2_EN();
        }
    }

    if (sci_tim.afterTXc == 1)
    {
        if (sci_tim.TXwait_timc != 0)
        {
            sci_tim.TXwait_timc--;
        }
        else
        {
            sci_tim.afterTXc = 0;
            m485_RX3_EN();
        }
    }
}


uint8_t isMasterTXQueueNotEmpty(void)
{
    uint16_t *plength = &MasterTxQueue.length;

    if(*plength > 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

uint8_t enMasterTXQueue(uint8_t *pdata,uint16_t Size)
{
    uint16_t *ptail = &MasterTxQueue.tail;
    uint16_t *plength = &MasterTxQueue.length;
    DATA_BUFF *plist = MasterTxQueue.data;
    uint8_t *pbuff = plist[*ptail].buff;

    if (*plength == QUEUE_DEPTH)
    {
        return 0;
    }

    plist[*ptail].size = Size;

    memcpy(pbuff,pdata,Size);

    (*plength)++;

    if ((*ptail) < (QUEUE_DEPTH - 1))
    {
        (*ptail)++;
    }
    else
    {
        *ptail = 0;
    }

    return 1;
}


uint8_t deMasterTXQueue(uint8_t *pdata,uint16_t *pSize)
{
    uint16_t *phead = &MasterTxQueue.head;
    uint16_t *plength = &MasterTxQueue.length;
    DATA_BUFF *plist = MasterTxQueue.data;
    uint8_t *pbuff = plist[*phead].buff;

    if (*plength == 0)
    {
        return 0;
    }

    *pSize = plist[*phead].size;
    memcpy(pdata, pbuff, *pSize);

    (*plength)--;

    if (*phead < QUEUE_DEPTH - 1)
    {
        (*phead)++;
    }
    else
    {
        *phead = 0;
    }

    return 1;
}
