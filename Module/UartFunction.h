/*
 * UartFunction.h
 *
 *      Author: dell
 */

#ifndef MODULE_UARTFUNCTION_H_
#define MODULE_UARTFUNCTION_H_

#include "App.h"
#include "F28x_Project.h"
#include "hw_types.h"
#include "UartDebug.h"

#define SCI_BUFF_SIZE       300

#define SCI_NUM             3
#define SCI_RCV_TIMEOUT     5
#define AFTER_TX_FINISH_CNT 2


#define cUartTaskTimerEvt     0
#define cUartTaskSciaEvt      1
#define cUartTaskScibEvt      2
#define cUartTaskScicEvt      3

#define QUEUE_DEPTH  10

typedef struct
{
    uint16_t TXwait_tima;
    uint16_t TXwait_timb;
    uint16_t TXwait_timc;

    uint16_t timer_a;
    uint16_t timer_b;
    uint16_t timer_c;

    uint8_t afterTXa;
    uint8_t afterTXb;
    uint8_t afterTXc;

    uint8_t rcv_a;
    uint8_t rcv_b;
    uint8_t rcv_c;
} SCI_TIM;

typedef enum
{
    SCIA = 0,
    SCIB,
    SCIC,
} SCI_MODE;

typedef struct
{
    uint16_t size;
    uint8_t  buff[50];
} DATA_BUFF;

typedef struct
{
    uint16_t head;
    uint16_t tail;
    uint16_t length;
    DATA_BUFF data[QUEUE_DEPTH];
}TX_QUEUE;


void set_sciTim(SCI_MODE mod);
void sUartRxISR(SCI_MODE mod);
void sUartTxISR(SCI_MODE mod);
void SciQueueInit(SCI_MODE mod,uint8_t bType);
void dataSplit(uint16_t wSendData);
uint8_t  sUartWrite(SCI_MODE mod,uint8_t *pstart,uint16_t wLength);
uint8_t sUartWriteBinary(SCI_MODE mod,uint16_t *pstart,uint16_t wLength);
void UartSendWaveData(void);
void sUartQ3Com(uint8_t *pdata);

void check_sciTurntoRCV(void);

extern SCI_TIM sci_tim;

void sci_tim_check(SCI_MODE mod);
void get_scidata(SCI_MODE mod, uint8_t *pdata, uint16_t *psize);
uint8_t isMasterTXQueueNotEmpty(void);
uint8_t enMasterTXQueue(uint8_t *pdata,uint16_t Size);
uint8_t deMasterTXQueue(uint8_t *pdata,uint16_t *pSize);
extern SciStruct   UartList[SCI_NUM];

#endif /* MODULE_UARTFUNCTION_H_ */
