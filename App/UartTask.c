/*******************************************************************************
 * File Name: UartTask.c
 * Description: The system status machine.
 * Designer: DELL
 * Version: 00
 * Copyright: Liyuan Haina Co.Ltd
 *******************************************************************************/
#include "UartTask.h"
#include "App.h"
#include "DataProcTask.h"
#include "F28x_Project.h"
#include "Gpio_User.h"
#include "ModbusRtu.h"
#include "RTOS_APP.h"
#include "RTOS_Define.h"
#include "Sci_User.h"
#include "UartDebug.h"
#include "MeasureTask.h"
#include "SuperTask.h"
#include "ShareData.h"
#include "DataProcTask.h"
#include "Can_User.h"
#include "UartFunction.h"
#include "string.h"

uint8_t WriteREGbuff[100] = {0};
uint8_t WriteREGbuff1[100] = {0};
uint8_t ReadDatabuff[300] = {0};
uint8_t ReadDatabuff1[300] = {0};
uint8_t sci_buff[SCI_BUFF_SIZE] = {0};
PFC_STA stPfcSta;

ModbusOpt Opt = {
    .pWriteData = NULL,
    .pReadData = ReadDatabuff,
    .mod = SCIB,
    .station = 0,
    .addr = 0,
    .addr1 = 0,
    .func_code = 0,
    .err_code = 0,
    .reg_num = 0,
    .reg_num = 0
};

ModbusOpt MasterOpt = {.pWriteData = WriteREGbuff1,
                       .pReadData = ReadDatabuff1,
                       .mod = SCIA,
                       .station = 0,
                       .addr = 0,
                       .addr1 = 0,
                       .func_code = 0,
                       .err_code = 0,
                       .reg_num = 0,
                       .reg_num1 = 0,
                       .rcv_flag = 0,
                       .SendWaitCnt = 0
};


static uint8_t check_REG_Read(ModbusOpt *pOpt);
static uint8_t check_REG_Write(ModbusOpt *pOpt);
void processWriteREG(uint16_t addr, uint16_t reg_num);

static uint8_t check_REG_Read(ModbusOpt *pOpt)
{
    uint16_t addr_base = pOpt->addr & 0x80;

    if (addr_base == 0x00)
    {
        if ((pOpt->addr & 0xFF) > (addr_base + MOD_STA_SIZE - 1))
        {
            pOpt->func_code += 0x80;
            pOpt->err_code = MBEC_ERR_ADDR;
            if (pOpt->station != 0)
            {
                Modbus_Slaver_Respond(pOpt);
            }
            return 0;
        }
        else if (((pOpt->addr & 0xFF) + pOpt->reg_num) >
                 (addr_base + MOD_STA_SIZE))
        {
            pOpt->func_code += 0x80;
            pOpt->err_code = MBEC_ERR_REG_NUM;
            if (pOpt->station != 0)
            {
                Modbus_Slaver_Respond(pOpt);
            }
            return 0;
        }
        // sRTOSEventSendIsr(cPrioUpdateData, cUpdateDataTaskUpdateEvt);
    }
    else
    {
        if ((pOpt->addr & 0xFF) > (addr_base + MOD_SET_SIZE - 1))
        {
            pOpt->func_code += 0x80;
            pOpt->err_code = MBEC_ERR_ADDR;
            if (pOpt->station != 0)
            {
                Modbus_Slaver_Respond(pOpt);
            }
            return 0;
        }
        else if (((pOpt->addr & 0xFF) + pOpt->reg_num) >
                 (addr_base + MOD_SET_SIZE))
        {
            pOpt->func_code += 0x80;
            pOpt->err_code = MBEC_ERR_REG_NUM;
            if (pOpt->station != 0)
            {
                Modbus_Slaver_Respond(pOpt);
            }
            return 0;
        }
    }

    read_REG(pOpt->addr, pOpt->reg_num, pOpt->pReadData);
    pOpt->err_code = MBEC_SUCSSES;
    if (pOpt->station != 0)
    {
        Modbus_Slaver_Respond(pOpt);
    }
    return 1;
}

static uint8_t check_REG_Write(ModbusOpt *pOpt)
{
    uint16_t addr = 0, addr_base = 0, reg_num = 0;
    if (pOpt->func_code == WRITE_AND_READ)
    {
        addr = pOpt->addr1;
        reg_num = pOpt->reg_num1;
    }
    else
    {
        addr = pOpt->addr;
        reg_num = pOpt->reg_num;
    }

    addr_base = addr & 0x80;

    if (addr_base == 0x80)
    {
        if ((addr & 0X80) > (addr_base + MOD_SET_SIZE - 1))
        {
            pOpt->func_code += 0x80;
            pOpt->err_code = MBEC_ERR_ADDR;
            if (pOpt->station != 0)
            {
                Modbus_Slaver_Respond(pOpt);
            }
            return 0;
        }
        else if (((addr & 0X80) + reg_num) > (addr_base + MOD_SET_SIZE))
        {
            pOpt->func_code += 0x80;
            pOpt->err_code = MBEC_ERR_REG_NUM;
            if (pOpt->station != 0)
            {
                Modbus_Slaver_Respond(pOpt);
            }
            return 0;
        }
        else
        {
            if (write_REG(addr, reg_num, pOpt->pWriteData) == MBEC_ERR_VALUE)
            {
                pOpt->func_code += 0x80;
                pOpt->err_code = MBEC_ERR_VALUE;
                if (pOpt->station != 0)
                {
                    Modbus_Slaver_Respond(pOpt);
                }
                return 0;
            }
            read_REG(addr, reg_num, pOpt->pReadData);

            if (pOpt->func_code != WRITE_AND_READ)
            {
                pOpt->err_code = MBEC_SUCSSES;
                if (pOpt->station != 0)
                {
                    Modbus_Slaver_Respond(pOpt);
                }
            }

            return 1;
        }
    }
    else
    {
        pOpt->func_code += 0x80;
        pOpt->err_code = MBEC_ERR_ADDR;
        if (pOpt->station != 0)
        {
            Modbus_Slaver_Respond(pOpt);
        }
        return 0;
    }
}

uint16_t u16_PfcLossComCnt = 0;
void sSciTask(void)
{
    uint16_t addr = 0, event = 0, crc16 = 0, sci_size = 0;
    uint16_t i = 0;
    uint16_t tx_size = 0;
    ModbusOpt *pOpt = &Opt;
    ModbusOpt *pMasterOpt = &MasterOpt;

    event = suwRTOSGetEvent(cPrioUart); // Get the current task event

    if (event & ((uint16_t)1 << cUartTaskScibEvt))
    {
        pOpt->mod = SCIB;
        get_scidata(pOpt->mod, sci_buff, &sci_size);

        if (sci_size < 4)
        {
            memset(sci_buff, 0, sci_size);
            return;
        }

        crc16 = CRC16(sci_buff, sci_size - 2);
        addr = sci_buff[2] << 8 | sci_buff[3];

        if (((crc16 & 0x00FF) == sci_buff[sci_size - 2]) &&
            ((crc16 >> 8) == sci_buff[sci_size - 1]))
        {
            pOpt->station = sci_buff[0];

            if (pOpt->station > 1)
            {
                return;
            }

            if (pOpt->station == 0)
            {
                pOpt->addr = addr & 0xFF;
            }
            else
            {
                pOpt->addr = addr;
            }

            pOpt->func_code = sci_buff[1];

            switch (pOpt->func_code)
            {
            case READ_REG:
            {
                pOpt->reg_num = sci_buff[4] >> 8 | sci_buff[5];
                check_REG_Read(pOpt);
            }
            break;

            case WRITE_SINGLE:
            {
                pOpt->reg_num = 1;
                pOpt->pWriteData = sci_buff + 4;

                if(check_REG_Write(pOpt))
                {
                    //if((pOpt->addr & 0xFF) <= PFC_REG_END)    //PFC
                    {
                        uint16_t u16_cmd = 0;
                        BuffSaveInValue16(sci_buff + 4,&u16_cmd);
                        pMasterOpt->station = pOpt->station;
                        pMasterOpt->func_code = WRITE_SINGLE;
                        pMasterOpt->addr = pOpt->addr;
                        pMasterOpt->reg_num = pOpt->reg_num;
                        pMasterOpt->pWriteData[0] = u16_cmd>>8;
                        pMasterOpt->pWriteData[1] = u16_cmd & 0xFF;
                        Modbus_Master_Ask(pMasterOpt);
                    }
                }
            }
            break;

            case WIRITE_MULTI:
            {
                pOpt->reg_num = sci_buff[4] >> 8 | sci_buff[5];
                pOpt->pWriteData = sci_buff + 7;
                if(check_REG_Write(pOpt))
                {
                    pMasterOpt->station = pOpt->station;
                    pMasterOpt->func_code = WIRITE_MULTI;
                    pMasterOpt->addr = pOpt->addr;
                    pMasterOpt->reg_num = pOpt->reg_num;
                    pMasterOpt->pWriteData = pOpt->pWriteData;
                    Modbus_Master_Ask(pMasterOpt);
                }
            }
            break;

            case WRITE_AND_READ:
            {
                pOpt->reg_num = sci_buff[4] >> 8 | sci_buff[5];
                pOpt->addr1 = sci_buff[6] << 8 | sci_buff[7];
                pOpt->reg_num1 = sci_buff[8] << 8 | sci_buff[9];
                pOpt->pWriteData = sci_buff + 11;

                if (check_REG_Write(pOpt))
                {
                    check_REG_Read(pOpt);
                }
            }
            break;

            default:
            {
                //
                pOpt->func_code += 0x80;
                pOpt->err_code = MBEC_ERR_CMD;
                if (pOpt->station != 0)
                {
                    Modbus_Slaver_Respond(pOpt);
                }
            }
            break;
            }
        }
        else
        {
            if ((sci_buff[0] == 'Q') && (sci_buff[1] == '3') &&
                (sci_buff[sci_size - 1] == 0x0D))
            {
#ifdef cDebugUartEn
#if !cDebugUartEn
#if cDataBufEn
                sUartQ3Com(sci_buff);
#endif
#endif
#endif
            }
            else if ((sci_buff[0] == 's') && (sci_buff[1] == 't') &&
                     (sci_buff[2] == 'o') && (sci_buff[3] == 'p'))
            {
                sRTOSEventSend(cPrioDataProc, cBootEvt);
            }
            else
            {
            }
        }
        memset(sci_buff, 0, sci_size);
    }

    u16_PfcLossComCnt++;
    if(u16_PfcLossComCnt > 250) //5s
    {
        u16_PfcLossComCnt = 0;
        sAllPWMOff();
        stSuper.u32_FaultCode.BIT.PfcLossCom = 1;
        sRTOSEventSend(cPrioSuper,eSuperFaultEvt);
    }
    if (event & ((uint16_t)1 << cUartTaskSciaEvt))
    {
        pOpt->mod = SCIA;
        get_scidata(pOpt->mod, sci_buff, &sci_size);
        crc16 = CRC16(sci_buff, sci_size - 2);
        pOpt->station = sci_buff[0];
        pOpt->func_code = sci_buff[1];

        pMasterOpt->rcv_flag = 1;
        pMasterOpt->SendWaitCnt = 0;
        if (((crc16 & 0x00FF) == sci_buff[sci_size - 2]) &&
            ((crc16 >> 8) == sci_buff[sci_size - 1]))
        {
             if(pOpt->station == 1)
             {
                 if(pOpt->func_code == READ_REG)
                 {
                    u16_PfcLossComCnt = 0;

                    for(i = 0;i< 9;i++)
                    {
                        BuffSaveInValue16(sci_buff + 3 + i*2,(uint16_t*)(&stPfcSta.u16_Grid_V + i));
                    }

                    uModSta.REG.u16_Grid_V = stPfcSta.u16_Grid_V;
                    uModSta.REG.u16_Grid_I = stPfcSta.u16_Grid_I;
                    uModSta.REG.i16_Temp1 = stPfcSta.i16_Temp1;
                    uModSta.REG.u16_PfcFaultCode = stPfcSta.u16_PfcFaultCode;
                    uModSta.REG.u16_PfcFwVersion = stPfcSta.u16_PfcFwVersion;
                    uModSta.REG.u16_PfcStatus = stPfcSta.u16_PfcStatus;
                    uModSta.REG.u16_Vbus = stPfcSta.u16_Vbus;
                 }

             }
        }
    }

    if (event & ((uint16_t)1 << cUartTaskTimerEvt))
    {
#if !cDebugUartEn
#if cDataBufEn
        if (Gui_wLogStatus == 2)
        {
            UartSendWaveData();
            Gui_wLogStatus = 0;
        }
#endif
#endif

        if ((*((uint8_t *)(&sci_tim.rcv_a + pMasterOpt->mod)) != 1)
                          && (UartList[pMasterOpt->mod].bTxStatus != cSciTxBusy))
        {
            if(isMasterTXQueueNotEmpty())
            {
                if(pMasterOpt->rcv_flag == 1)
                {
                    pMasterOpt->rcv_flag = 0;
                    pMasterOpt->SendWaitCnt = 0;
                    if(deMasterTXQueue(UartTxbuff[pMasterOpt->mod],&tx_size))
                    {
                        sUartWrite(pMasterOpt->mod, UartTxbuff[pMasterOpt->mod], tx_size);
                    }
                }
                else
                {
                    pMasterOpt->SendWaitCnt++;
                    if(pMasterOpt->SendWaitCnt > 5)
                    {
                        pMasterOpt->SendWaitCnt = 0;
                        if(deMasterTXQueue(UartTxbuff[pMasterOpt->mod],&tx_size))
                        {
                            sUartWrite(pMasterOpt->mod, UartTxbuff[pMasterOpt->mod], tx_size);
                        }
                    }

                }
            }
        }
    }
}



//dc-dc串口a中断接收函数使用
void CheckErrSingal(uint8_t data)
{
    static uint8_t Tro = 0;

    switch(Tro)
    {
      case 0:
      {
          if(data == 11)
          {
              Tro = data;
          }

      }break;

      case 11:
      {
          if(data != 22)
          {
              Tro = 0;
          }
          else
          {
              Tro = data;
          }

      }break;

      case 22:
      {
          if(data != 33)
          {
              Tro = 0;
          }
          else
          {
              Tro = data;
          }

      }break;

      case 33:
      {
          if(data != 44)
          {
              Tro = 0;
          }
          else
          {
              Tro = data;
          }

      }break;

      case 44:
      {
          if(data != 55)
          {
              Tro = 0;
          }
          else
          {
              //pfc故障关机
              sAllPWMOff();
              sRTOSEventSend(cPrioSuper,eSuperFaultEvt);
              Tro = 0;
          }

      }break;


      default:
      {
          Tro = 0;
          break;
      }

    }
}
