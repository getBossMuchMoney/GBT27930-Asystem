/*******************************************************************************
 * File Name: EEpromTask.c
 * Description: The system status machine.
 * Designer: Miller Xue
 * Version: 00
 * Copyright: Liyuan Haina Co.Ltd
 *******************************************************************************/
#ifndef _EEpromTask_H_
#define _EEpromTask_H_
#include <stdint.h>

//EEprom task definition
#define    eEEpromTimerEvt                  0


#define EepromCmdSize       6
#define EepromCmdMaxNum     18
#define EepromCmdBufferSize (EepromCmdMaxNum*EepromCmdSize)


extern uint16_t EepromCmdBuffer[EepromCmdBufferSize]; //6*18


extern void sEEpromTaskInit(void);
extern void sEEpromTask(void);


#endif

