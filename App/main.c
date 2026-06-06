/*******************************************************************************
 * File Name: main.c
 * Description: The main function for the system. The MCU initialization and Task
 *              creation will be finished in the function as well as the execution
 *              of the background loop.
 * Designer: Miller Xue
 * Version: 00
 * Copyright: Liyuan Haina Co.Ltd
 *******************************************************************************/
#include <J1939/J1939.h>
#include "PSFBController.h"
#include "F28x_Project.h"
#include "App.h"
#include "Gpio_User.h"
#include "Gpio_User.h"
#include "RTOS_APP.h"
#include "RTOS_Define.h"
#include "UartTask.h"
#include "UartFunction.h"
#include "ModbusRtu.h"
#include "SuperTask.h"
#include "MeasureTask.h"
#include "DataProcTask.h"
#include "ShareData.h"
#include "EEpromTask.h"
#include "ProtectTask.h"
#include "ParallelTask.h"
#include "ChargeProcTask.h"

void sDataInit(void);
void sSysTaskInit(void);
void sLedTask(void);
void sDigitalInputDet(void);
void sOnBoardLedCtrl(void);
void sExtOutputCtrl(void);

uint16_t u16_PwmTestOn = 0;
uint16_t u16_TestTaskCnt = 0;
uint16_t u16_TestTaskCnt1 = 0;
uint16_t u16_DigitalInput[6];

void main(void)
{
    sMcuInitial();
    sDataInit();

    //The RTOS initialization
    sRTOSInit();

    sRTOSTaskCreate(J1939_Poll,cPrioJ1939,c1ms,0);

    sRTOSTaskCreate(sSciTask,cPrioUart,c20ms,0);
    sRTOSTaskCreate(sParallelTask,cPrioPara,c20ms,0);
    sRTOSTaskCreate(sChargeProcTask,cPrioCharge,c10ms,0);
    sRTOSTaskCreate(sSuperTask,cPrioSuper,c20ms,1);
    sRTOSTaskCreate(sMeasureTask,cPrioMeasure,c20ms,3);
    sRTOSTaskCreate(sDataProcTask,cPrioDataProc,c20ms,13);
	sRTOSTaskCreate(sLedTask,cPrioLed,c250ms,0);
    sRTOSTaskCreate(sEEpromTask,cPrioEEprom,c20ms,10);
    sRTOSTaskCreate(sProtectTask,cPrioProtect,c20ms,17);
    sSysTaskInit();
    sRTOSTaskStart();

    for(;;)
    {
        //Task schedule
        (*Task_Struct_Table[sRTOSFindHighPrioRdyTask()].pTaskFuncAddr)();
    }

}

void sDataInit(void)
{
    sShareDataInit();
    SciQueueInit(SCIA,0);
    SciQueueInit(SCIB,0);
    sModbusInit();
    sPSFBControllerInit();
    J1939_Initialization();
}

void sSysTaskInit(void)
{
    sProtectTaskInit();
    sSuperTaskInit();
    sEEpromTaskInit();
    sMeasureTaskInit();
    sParallelTaskInit();
}
void sLedTask(void)
{
    uint16_t event;

//    event = suwRTOSGetEvent(cPrioLed);     //Get the current task event

//    if(event & ((uint16_t)1 << eLedTimerEvt))
//    {
//       if(u16_PwmTestOn == 1)
//       {
//           u16_PwmTestOn = 0;
//          sRTOSEventSend(cPrioSuper,eSuperTurnOnEvt);
//       }
//       else if(u16_PwmTestOn == 2)
//       {
//           u16_PwmTestOn = 0;
//           sRTOSEventSend(cPrioSuper,eSuperTurnOffEvt);
//       }
       //sOnBoardLedCtrl();
       mOnBoardLed1_Toggle();
//       sDigitalInputDet();
//       sExtOutputCtrl();
//    }
}

void sDigitalInputDet(void)
{

}

void sOnBoardLedCtrl(void)
{
    if((stSuper.u16_SysMode == cPowerOnMode) || (stSuper.u16_SysMode == cStandbyMode))
    {
       mOnBoardLed2_Off();
       if(++u16_TestTaskCnt >= 25)
       {
           u16_TestTaskCnt = 0;
           mOnBoardLed1_Toggle();
       }
    }
    else if(stSuper.u16_SysMode == cSoftStartMode)
    {
       mOnBoardLed2_Off();
       if(++u16_TestTaskCnt >= 12)
       {
           u16_TestTaskCnt = 0;
           mOnBoardLed1_Toggle();
       }
    }
    else if(stSuper.u16_SysMode == cRunMode)
    {
       mOnBoardLed2_Off();
       if(++u16_TestTaskCnt <= 50)
       {
           mOnBoardLed1_Off();
       }
       else if(u16_TestTaskCnt <= 100)
       {
           mOnBoardLed1_On();
       }
       else
       {
           u16_TestTaskCnt = 0;
       }
    }
    else if(stSuper.u16_SysMode == cFaultMode)
    {
        mOnBoardLed1_On();
        if(++u16_TestTaskCnt >= 12)
        {
            u16_TestTaskCnt = 0;
            mOnBoardLed2_Toggle();
        }
    }
}

void sExtOutputCtrl(void)
{

}
