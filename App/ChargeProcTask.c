/*
 * ChargeProcTask.c
 *
 *  Created on: 2026年5月27日
 *      Author: Dell
 */

#include "App.h"
#include "F28x_Project.h"
#include <J1939/J1939.h>
#include "ChargeProcTask.h"
#include "RTOS_APP.h"
#include "RTOS_Define.h"
#include "string.h"

uint8_t LongDataBuff[J1939_TP_MAX_MESSAGE_LENGTH] = {0};  //长消息接收缓存，默认240个字节
uint8_t u8_ChargeState = cChargeStandBy;
uint16_t CEM_CODE = 0;   //CEM错误代码
static HandShakeStep eHandShakeStep = SEND_CHM;
static ChargeConfigStep eChargeConfigStep = CHECK_BRO;
static ChargeRunningStep eChargeRunningStep = SEND_CCS;
static uint8_t VehicleBEMflag = 0;   //收到BEM报文标志位

uint8_t CTS_Data[7] = {0x13,0x20,0x15,0x11,0x10,0x20,0x19}; //CTS报文格式：2019年10月11日15时20分13秒，如开启CTS报文，需更新此处数据

ST_CHARGESTACK stChargeData = {
                               .u16_WaitTim50ms = 0,
                               .u16_WaitTim250ms = 0,
                               .u16_WaitTim500ms = 0,
                               .u16_WaitTim10000ms = 0,
                               .u16_TimeOutCnt = 0,
                               .u16_ReConnectCnt = 0,
                               .SendMesBuff = {
                                               .Mxe.SourceAddress = SECC,
                                               .Mxe.PDUSpecific = EVCC,
                                               .Mxe.DataPage = 0,
                                               .Mxe.Res = 0,

                               },

                               .LongMesBuff = {
                                               .data = LongDataBuff,
                                               .data_num = J1939_TP_MAX_MESSAGE_LENGTH, //必须初始化长帧缓冲区大小

                               },

};

ST_VEHICLEMSG stVehicleChargeMsg = {0,0,0,0,0,0,0};        //车辆BCP报文转化数据

ST_CHARGERCONFIG stChargerCfg = {0,0,0,0};                 //充电机CML报文发送转化缓存

ST_VEHICLEBCL stVehicleChargeCfg = {0,0,0};                //车辆BCL报文转化数据

ST_BATTERYBCS stBatteryChargeSta = {0,0,0,0,0,0};         //车辆电池BCS报文转化数据

ST_VEHICLEBSM stVehicleSta = {0,0,0,0,0,0,0,0,0,0,0,0};  //车辆BSM报文转化数据

ST_CHARGERCCS stChargerSta = { 0,0,0,0};     //充电机CCS发送待转化数据
ST_CHARGERCSD stChargerEndding = {0,0,1};   //充电机充电结束CSD发送待转化数据

UCSTDATA  uCSTData = {.data = {0,0,0,0},};   //充电机CST报文数据缓存
UBSTDATA  uBSTData = {.data = {0,0,0,0},};   //车辆BST报文转化数据

void Gbt27930_Asystem(void);
void sStartCharging(void);
void sStopCharging(void);
uint8_t sCheckVehicleConfig(void);

void SendCHM(void);
void SendCRM(CRM_VI resault);
void SendCTS(void);
void SendCML(void);
void SendCRO(CRO_VI sta);

void SendCCS(void);
void SendCSD(void);

void SendCST(UCSTDATA *pUdata);
void SendCEM(uint16_t ErrFlag);

void sGetVehicleChargeMsg(ST_VEHICLEMSG* pStData,uint8_t* pdata);
void sGetVehicleChargeCfg(ST_VEHICLEBCL* pStData,uint8_t* pdata);
void sGetBatteryChargeSta(ST_BATTERYBCS* pStData,uint8_t* pdata);
void sGetVehicleSta(ST_VEHICLEBSM* pStData,uint8_t* pdata);
void sGetVehicleBST(UBSTDATA* pStData,uint8_t* pdata);

void sChargeStandBy(void);
void sChargeHandShake(void);
void sChargeConfig(void);
void sChargeRunning(void);
void sChargeEnding(void);
void sChargeTimeoutDeal(void);
void sChargeErr(void);


void sChargeProcTask(void)
{
    uint16_t event;

    event = suwRTOSGetEvent(cPrioCharge);

    if(event & ((uint16_t)1 << cChargTimerEvt))
    {
        //J1939_Poll();         //执行SAE-J1939协议栈
        Gbt27930_Asystem();   //执行GBT27930-2023 A类系统协议栈
    }

}


void sStartCharging(void)
{
    //充电机执行开始充电操作

}

void sStopCharging(void)
{
    //充电机执行中止充电操作
}


uint8_t sCheckVehicleConfig(void)
{
    //判断车辆充电参数是否匹配，符合返回1，否则返回0
    return 1;
}




uint8_t sChargerSelfCheck(void)
{
   //充电机自检步骤检查，正常返回1，否则返回0

    return 1;
}





void SendCEM(uint16_t ErrFlag)
{
    J1939_MESSAGE *pTxBuff = &stChargeData.SendMesBuff;
    pTxBuff->Mxe.PDUFormat = CEM_PGN;
    pTxBuff->Mxe.Priority = PRI2;
    pTxBuff->Mxe.DataLength = 4;
    pTxBuff->Mxe.Data[0] = (ErrFlag & 0xFF);
    pTxBuff->Mxe.Data[1] = (ErrFlag>>8);
    J1939_Send_Message(pTxBuff,Select_CAN_NODE_1);

}

void SendCHM(void)
{
    J1939_MESSAGE *pTxBuff = &stChargeData.SendMesBuff;
    pTxBuff->Mxe.PDUFormat = CHM_PGN;
    pTxBuff->Mxe.Priority = PRI6;
    pTxBuff->Mxe.DataLength = 3;
    pTxBuff->Mxe.Data[0] = 1;
    pTxBuff->Mxe.Data[1] = 1;
    pTxBuff->Mxe.Data[2] = 0;
    J1939_Send_Message(pTxBuff,Select_CAN_NODE_1);
}


void SendCRM(CRM_VI resault)
{
    J1939_MESSAGE *pTxBuff = &stChargeData.SendMesBuff;
    pTxBuff->Mxe.PDUFormat = CRM_PGN;
    pTxBuff->Mxe.Priority = PRI6;
    pTxBuff->Mxe.DataLength = 8;
    pTxBuff->Mxe.Data[0] = resault;
    pTxBuff->Mxe.Data[1] = stChargerEndding.u16_ChargerID & 0xFF;
    pTxBuff->Mxe.Data[2] = (stChargerEndding.u16_ChargerID >> 8)& 0xFF;
    pTxBuff->Mxe.Data[3] = (stChargerEndding.u16_ChargerID >> 16)& 0xFF;
    pTxBuff->Mxe.Data[4] = (stChargerEndding.u16_ChargerID >> 24)& 0xFF;
    pTxBuff->Mxe.Data[5] = 0;
    pTxBuff->Mxe.Data[6] = 0;
    pTxBuff->Mxe.Data[7] = 0;
    J1939_Send_Message(pTxBuff,Select_CAN_NODE_1);
}

void SendCTS(void)
{
    J1939_MESSAGE *pTxBuff = &stChargeData.SendMesBuff;
    pTxBuff->Mxe.PDUFormat = CTS_PGN;
    pTxBuff->Mxe.Priority = PRI6;
    pTxBuff->Mxe.DataLength = 7;
    memcpy(pTxBuff->Mxe.Data,CTS_Data,pTxBuff->Mxe.DataLength);
    J1939_Send_Message(pTxBuff,Select_CAN_NODE_1);

}

void SendCML(void)
{
    J1939_MESSAGE *pTxBuff = &stChargeData.SendMesBuff;
    pTxBuff->Mxe.PDUFormat = CML_PGN;
    pTxBuff->Mxe.Priority = PRI6;
    pTxBuff->Mxe.DataLength = 8;
    pTxBuff->Mxe.Data[0] = stChargerCfg.u16_MaxChargeVolt & 0xFF;
    pTxBuff->Mxe.Data[1] = stChargerCfg.u16_MaxChargeVolt>>8;
    pTxBuff->Mxe.Data[2] = stChargerCfg.u16_MinChargeVolt & 0xFF;
    pTxBuff->Mxe.Data[3] = stChargerCfg.u16_MinChargeVolt>>8;
    pTxBuff->Mxe.Data[4] = stChargerCfg.u16_MaxChargeCurr & 0xFF;
    pTxBuff->Mxe.Data[5] = stChargerCfg.u16_MaxChargeCurr>>8;
    pTxBuff->Mxe.Data[6] = stChargerCfg.u16_MinChargeCurr & 0xFF;
    pTxBuff->Mxe.Data[7] = stChargerCfg.u16_MinChargeCurr>>8;
    J1939_Send_Message(pTxBuff,Select_CAN_NODE_1);
}


void SendCRO(CRO_VI sta)
{
    J1939_MESSAGE *pTxBuff = &stChargeData.SendMesBuff;
    pTxBuff->Mxe.PDUFormat = CRO_PGN;
    pTxBuff->Mxe.Priority = PRI4;
    pTxBuff->Mxe.DataLength = 1;
    pTxBuff->Mxe.Data[0] = sta;
    J1939_Send_Message(pTxBuff,Select_CAN_NODE_1);
}


void SendCCS(void)
{
    J1939_MESSAGE *pTxBuff = &stChargeData.SendMesBuff;
    pTxBuff->Mxe.PDUFormat = CCS_PGN;
    pTxBuff->Mxe.Priority = PRI6;
    pTxBuff->Mxe.DataLength = 7;
    pTxBuff->Mxe.Data[0] = stChargerSta.u16_ChargerVolt & 0xFF;
    pTxBuff->Mxe.Data[1] = stChargerSta.u16_ChargerVolt>>8;
    pTxBuff->Mxe.Data[2] = stChargerSta.u16_ChargerCurrent & 0xFF;
    pTxBuff->Mxe.Data[3] = stChargerSta.u16_ChargerCurrent>>8;
    pTxBuff->Mxe.Data[4] = stChargerSta.u16_ChargingTime & 0xFF;
    pTxBuff->Mxe.Data[5] = stChargerSta.u16_ChargingTime>>8;
    pTxBuff->Mxe.Data[6] = stChargerSta.u16_ChargingEnable;
    J1939_Send_Message(pTxBuff,Select_CAN_NODE_1);
}


void SendCSD(void)
{
    J1939_MESSAGE *pTxBuff = &stChargeData.SendMesBuff;
    pTxBuff->Mxe.PDUFormat = CSD_PGN;
    pTxBuff->Mxe.Priority = PRI6;
    pTxBuff->Mxe.DataLength = 7;
    pTxBuff->Mxe.Data[0] = stChargerEndding.u16_ChargingTime & 0xFF;
    pTxBuff->Mxe.Data[1] = stChargerEndding.u16_ChargingTime>>8;
    pTxBuff->Mxe.Data[2] = stChargerEndding.u16_OutputPower & 0xFF;
    pTxBuff->Mxe.Data[3] = stChargerEndding.u16_OutputPower>>8;
    pTxBuff->Mxe.Data[4] = stChargerEndding.u16_ChargerID & 0xFF;
    pTxBuff->Mxe.Data[5] = (stChargerEndding.u16_ChargerID>>8) & 0xFF;
    pTxBuff->Mxe.Data[6] = (stChargerEndding.u16_ChargerID>>16) & 0xFF;
    pTxBuff->Mxe.Data[7] = (stChargerEndding.u16_ChargerID>>24) & 0xFF;
    J1939_Send_Message(pTxBuff,Select_CAN_NODE_1);

}


//充电机中止充电报文
void SendCST(UCSTDATA *pUdata)
{
    J1939_MESSAGE *pTxBuff = &stChargeData.SendMesBuff;
    pTxBuff->Mxe.PDUFormat = CST_PGN;
    pTxBuff->Mxe.Priority = PRI4;
    pTxBuff->Mxe.DataLength = 4;
    memcpy(pTxBuff->Mxe.Data,pUdata->data,pTxBuff->Mxe.DataLength);
    J1939_Send_Message(pTxBuff,Select_CAN_NODE_1);
}

//解析BCP报文数据
void sGetVehicleChargeMsg(ST_VEHICLEMSG* pStData,uint8_t* pdata)
{
    pStData->u16_oBatteryMaxVolt = pdata[0] | (((uint16_t)pdata[1])<<8);
    pStData->i16_ChargeCurrMax = (pdata[2] | (((uint16_t)pdata[3])<<8)) -400 * 10;
    pStData->u16_Eb = pdata[4] | (((uint16_t)pdata[5])<<8);
    pStData->u16_ChargeVoltMax = pdata[6] | (((uint16_t)pdata[7])<<8);
    pStData->i16_oBatteryMaxTemp = pdata[8] - 50;
    pStData->u16_SOC = pdata[9] | (((uint16_t)pdata[10])<<8);
    pStData->u16_VehicleVolt = pdata[11] | (((uint16_t)pdata[12])<<8);
}

//解析BCL充电报文需求数据
void sGetVehicleChargeCfg(ST_VEHICLEBCL* pStData,uint8_t* pdata)
{
    pStData->u16_VehicleVolt = pdata[0] | (((uint16_t)pdata[1])<<8);
    pStData->i16_VehicleCurrent = (pdata[2] | (((uint16_t)pdata[3])<<8)) -400 * 10;
    pStData->u16_ChargingType = pdata[4];
}

//解析BCS报文数据
void sGetBatteryChargeSta(ST_BATTERYBCS* pStData,uint8_t* pdata)
{
    pStData->u16_VehicleVolt = pdata[0] | (((uint16_t)pdata[1])<<8);
    pStData->i16_VehicleCurrent = (pdata[2] | (((uint16_t)pdata[3])<<8)) - 400*10;
    pStData->u16_oBatteryMaxVolt = (pdata[4] | (((uint16_t)pdata[5])<<8))&0x0FFF;
    pStData->u16_oBatteryGroup = (pdata[4] | (((uint16_t)pdata[5])<<8))>>12;
    pStData->u16_SOC = pdata[6];
    pStData->u16_RemainTime = pdata[7] | (((uint16_t)pdata[8])<<8);
}


//解析BMS报文数据
void sGetVehicleSta(ST_VEHICLEBSM* pStData,uint8_t* pdata)
{
    pStData->u16_oBatteryIndex = pdata[0];
    pStData->i16_oBatteryMaxTemp = pdata[1] - 50;
    pStData->u16_BatteryMaxTempIndex = pdata[2];
    pStData->i16_oBatteryMinTemp = pdata[3] - 50;
    pStData->u16_BatteryMinTempIndex = pdata[4];
    pStData->u16_oBatteryVoltLevel = pdata[5] & 0x03;
    pStData->u16_BatterySOCLevel = (pdata[5]>>2) & 0x03;
    pStData->u16_BatteryOverCurrent = (pdata[5]>>4) & 0x03;
    pStData->u16_BatteryOverTemp = (pdata[5]>>6) & 0x03;
    pStData->u16_BatteryIsolSta = pdata[6] & 0x03;
    pStData->u16_BatteryConnectorSta = (pdata[6]>>2) & 0x03;
    pStData->u16_ChargingEnable = (pdata[6]>>4) & 0x03;

}


void sGetVehicleBST(UBSTDATA* pStData,uint8_t* pdata)
{
    memcpy(pStData->data,pdata,sizeof(pStData->data));

}

void sChargeStandBy(void)
{
    if(CHARGE_CONNECT)
    {
        //检测充电物理连接成功跳到充电握手阶段
        u8_ChargeState = cChargeHandShake;
    }
}

void sChargeHandShake(void)
{
    J1939_MESSAGE *pRxBuff = &stChargeData.ReadMesBuff;
    TP_RX_MESSAGE *pLongRxbuff = &stChargeData.LongMesBuff;

    if(stChargeData.u16_WaitTim250ms != READY)
    {
       stChargeData.u16_WaitTim250ms--;
    }

    switch(eHandShakeStep)
    {
        case SEND_CHM:
        {

            if(stChargeData.u16_WaitTim250ms == READY)
            {
                stChargeData.u16_WaitTim250ms = cTim250ms;
                SendCHM();
            }

            if(RC_SUCCESS == J1939_Read_Message(pRxBuff,Select_CAN_NODE_1))
            {
                if(pRxBuff->Mxe.PDUFormat == BHM_PGN)
                {
                    if(sChargerSelfCheck())
                    {
                        stChargeData.u16_TimeOutCnt = 0;
                        eHandShakeStep = SEND_CRM;
                        stChargeData.u16_WaitTim250ms = READY;
                        return;
                    }
                    else
                    {
                        stChargeData.u16_TimeOutCnt = 0;
                        stChargeData.u16_WaitTim250ms = READY;
                        eHandShakeStep = SEND_CHM;
                        memset(uCSTData.data,0,4);
                        uCSTData.type.ChargerFaultFlagHigh.type.u16_SelfCheckFault = 1;
                        u8_ChargeState = cChargeErr;
                        //执行其他操作
                        sStopCharging();
                        return;

                    }
                }
            }

            stChargeData.u16_TimeOutCnt++;
            if(stChargeData.u16_TimeOutCnt > cTim10s)
            {
                stChargeData.u16_TimeOutCnt = 0;
                stChargeData.u16_WaitTim250ms = READY;
                eHandShakeStep = SEND_CHM;
                u8_ChargeState = cChargeStandBy;

                //执行其他操作
                sStopCharging();
                return;
            }

        }break;

        case SEND_CRM:
        {

            if(stChargeData.u16_WaitTim250ms == READY)
            {
                stChargeData.u16_WaitTim250ms = cTim250ms;
                SendCRM(NVI);
            }

            if(RC_SUCCESS == J1939_Read_Message(pRxBuff,Select_CAN_NODE_1))
            {
                //超时处理收到车辆接收超时报文BEM
                if(pRxBuff->Mxe.PDUFormat == BEM_PGN)
                {
                    stChargeData.u16_TimeOutCnt = 0;
                    stChargeData.u16_WaitTim250ms = READY;
                    u8_ChargeState = cChargeTimeoutDeal;
                    //执行其他操作
                    VehicleBEMflag = 1;
                    sStopCharging();
                    return;
                }
            }

            if(RC_SUCCESS == J1939_TP_RX_Message(pLongRxbuff,Select_CAN_NODE_1))
            {
                if(RSHIFT(pLongRxbuff->PGN) == BRM_PGN)
                {
                    //第一次收到BRM报文
                    eHandShakeStep = CHECK_BCP;
                }

            }

            stChargeData.u16_TimeOutCnt++;
            if(stChargeData.u16_TimeOutCnt > cTim5s)
            {
                stChargeData.u16_TimeOutCnt = 0;
                stChargeData.u16_WaitTim250ms = READY;
                u8_ChargeState = cChargeTimeoutDeal;
                CEM_CODE = BRM_TIMEOUT;
                //执行其他操作
                sStopCharging();
                return;
            }
        }break;

        case CHECK_BCP:
        {
            if(RC_SUCCESS == J1939_TP_RX_Message(pLongRxbuff,Select_CAN_NODE_1))
            {
                if(RSHIFT(pLongRxbuff->PGN) == BCP_PGN)
                {
                    //第一次收到BCP报文
                    sGetVehicleChargeMsg(&stVehicleChargeMsg,pLongRxbuff->data);
                    //这里获取bcp报文后可判断充电参数是否匹配，不匹配退出充电
                    if(!sCheckVehicleConfig())
                    {
                          SendCML();
                          memset(uCSTData.data,0,4);
                          uCSTData.type.ChargerErrFlag.type.u16_ChargeConfigErr = 1;
                          stChargeData.u16_TimeOutCnt = 0;
                          stChargeData.u16_WaitTim250ms = READY;
                          eHandShakeStep = SEND_CHM;
                          u8_ChargeState = cChargeErr;
                          //执行其他操作
                          sStopCharging();
                          return;
                    }

                    stChargeData.u16_WaitTim250ms = READY;
                    stChargeData.u16_TimeOutCnt = 0;
                    eHandShakeStep = SEND_CHM;
                    //跳转到充电配置阶段
                    u8_ChargeState = cChargeConfig;
                    return;
                }

            }


            if(stChargeData.u16_WaitTim250ms == READY)
            {
                stChargeData.u16_WaitTim250ms = cTim250ms;
                SendCRM(VI);
            }

            if(RC_SUCCESS == J1939_Read_Message(pRxBuff,Select_CAN_NODE_1))
            {
                //超时处理收到车辆接收超时报文BEM
                if(pRxBuff->Mxe.PDUFormat == BEM_PGN)
                {
                    stChargeData.u16_TimeOutCnt = 0;
                    stChargeData.u16_WaitTim250ms = READY;
                    u8_ChargeState = cChargeTimeoutDeal;
                    VehicleBEMflag = 1;
                    //执行其他操作
                    sStopCharging();
                    return;
                }
            }


            stChargeData.u16_TimeOutCnt++;
            if(stChargeData.u16_TimeOutCnt > cTim5s)
            {
                stChargeData.u16_TimeOutCnt = 0;
                stChargeData.u16_WaitTim250ms = READY;
                u8_ChargeState = cChargeTimeoutDeal;
                CEM_CODE = BCP_TIMEOUT;
                //执行其他操作
                sStopCharging();
                return;
            }

        }break;


        default:break;
    }
}


void sChargeConfig(void)
{
    J1939_MESSAGE *pRxBuff = &stChargeData.ReadMesBuff;
    TP_RX_MESSAGE *pLongRxbuff = &stChargeData.LongMesBuff;
    static uint16_t u16_TimeoutCntChange = cTim5s;

    if(stChargeData.u16_WaitTim250ms != READY)
    {
       stChargeData.u16_WaitTim250ms--;
    }

    if(stChargeData.u16_WaitTim500ms != READY)
    {
       stChargeData.u16_WaitTim500ms--;
    }

    switch(eChargeConfigStep)
    {

    case CHECK_BRO:
    {
#if cEnableCTS
        //根据实际选择是否开启充电机发送时间同步报文
        if(stChargeData.u16_WaitTim500ms == READY)
        {
            SendCTS();
            stChargeData.u16_WaitTim500ms = cTim500ms;
        }
#endif

        if(stChargeData.u16_WaitTim250ms == READY)
        {
            stChargeData.u16_WaitTim250ms = cTim250ms;
            SendCML();
        }

        if(RC_SUCCESS == J1939_Read_Message(pRxBuff,Select_CAN_NODE_1))
        {
            if(pRxBuff->Mxe.PDUFormat == BST_PGN)
            {
                //收到车辆中止充电报文，退出充电
                stChargeData.u16_TimeOutCnt = 0;
                eChargeConfigStep = CHECK_BRO;
                stChargeData.u16_WaitTim250ms = READY;
                stChargeData.u16_WaitTim500ms = READY;
                memset(uCSTData.data,0,4);
                uCSTData.type.ChargerAbortFlag.type.u16_VehicleAbort = 1;
                u16_TimeoutCntChange = cTim5s;
                u8_ChargeState = cChargeErr;
                sGetVehicleBST(&uBSTData,pRxBuff->Mxe.Data);
                sStopCharging();
                return;
            }
            else if(pRxBuff->Mxe.PDUFormat == BRO_PGN)
            {
                u16_TimeoutCntChange = cTim60s;
                if(!sCheckVehicleConfig())
                {
                    eChargeConfigStep = CHECK_BRO;
                    stChargeData.u16_WaitTim500ms = READY;
                    stChargeData.u16_WaitTim250ms = READY;
                    u16_TimeoutCntChange = cTim5s;
                    u8_ChargeState = cChargeErr;
                    memset(uCSTData.data,0,4);
                    uCSTData.type.ChargerErrFlag.type.u16_ChargeConfigErr = 1;
                    sStopCharging();
                    return;
                }

                if(pRxBuff->Mxe.Data[0] == VC)
                {
                    stChargeData.u16_TimeOutCnt = 0;
                    u16_TimeoutCntChange = cTim5s;
                    stChargeData.u16_WaitTim250ms = READY;
                    stChargeData.u16_WaitTim500ms = READY;
                    eChargeConfigStep = SEND_CRO;
                    return;
                }
            }
            else if(pRxBuff->Mxe.PDUFormat == BEM_PGN)
            {
                //超时处理收到车辆接收超时报文BEM
                stChargeData.u16_TimeOutCnt = 0;
                stChargeData.u16_WaitTim250ms = READY;
                stChargeData.u16_WaitTim500ms = READY;
                VehicleBEMflag = 1;
                eChargeConfigStep = CHECK_BRO;
                u16_TimeoutCntChange = cTim5s;
                u8_ChargeState = cChargeTimeoutDeal;
                //执行其他操作
                sStopCharging();
                return;
            }

        }


        if(RC_SUCCESS == J1939_TP_RX_Message(pLongRxbuff,Select_CAN_NODE_1))
        {
            if(RSHIFT(pLongRxbuff->PGN) == BCP_PGN)
            {
                //更新BCP报文参数
                sGetVehicleChargeMsg(&stVehicleChargeMsg,pLongRxbuff->data);
            }

        }


        stChargeData.u16_TimeOutCnt++;
        if(stChargeData.u16_TimeOutCnt > u16_TimeoutCntChange)
        {
            stChargeData.u16_TimeOutCnt = 0;
            stChargeData.u16_WaitTim250ms = READY;
            stChargeData.u16_WaitTim500ms = READY;
            u16_TimeoutCntChange = cTim5s;
            eChargeConfigStep = CHECK_BRO;
            CEM_CODE = BCP_TIMEOUT;
            u8_ChargeState = cChargeTimeoutDeal;
            //执行其他操作
            sStopCharging();
            return;
        }

    }break;

    case SEND_CRO:
    {
        if(stChargeData.u16_WaitTim250ms == READY)
        {
            stChargeData.u16_WaitTim250ms = cTim250ms;
            SendCRO(NCC);
        }

        if(RC_SUCCESS == J1939_Read_Message(pRxBuff,Select_CAN_NODE_1))
        {
            if(pRxBuff->Mxe.PDUFormat == BST_PGN)
            {
                //收到车辆中止充电报文，退出充电
                stChargeData.u16_TimeOutCnt = 0;
                eChargeConfigStep = CHECK_BRO;
                stChargeData.u16_WaitTim250ms = READY;
                stChargeData.u16_WaitTim500ms = READY;
                u8_ChargeState = cChargeErr;
                memset(uCSTData.data,0,4);
                uCSTData.type.ChargerAbortFlag.type.u16_VehicleAbort = 1;
                sGetVehicleBST(&uBSTData,pRxBuff->Mxe.Data);
                sStopCharging();
            }
            else if(pRxBuff->Mxe.PDUFormat == BRO_PGN)
            {
                //车辆准备好充电
                if(pRxBuff->Mxe.Data[0] == VC)
                {
                    //充电机准备就绪
                    stChargeData.u16_TimeOutCnt = 0;
                    eChargeConfigStep = CHECK_BCL;
                    return;
                }
                else
                {
                    //预充电错误，退出充电
                    stChargeData.u16_TimeOutCnt = 0;
                    eChargeConfigStep = CHECK_BRO;
                    stChargeData.u16_WaitTim250ms = READY;
                    stChargeData.u16_WaitTim500ms = READY;
                    u8_ChargeState = cChargeErr;
                    memset(uCSTData.data,0,4);
                    uCSTData.type.ChargerFaultFlagHigh.type.u16_PrechargeFault = 1;
                    sStopCharging();
                    return;
                }
            }
            else if(pRxBuff->Mxe.PDUFormat == BEM_PGN)
            {
                //超时处理收到车辆接收超时报文BEM
                stChargeData.u16_TimeOutCnt = 0;
                stChargeData.u16_WaitTim250ms = READY;
                stChargeData.u16_WaitTim500ms = READY;
                VehicleBEMflag = 1;
                eChargeConfigStep = CHECK_BRO;
                u8_ChargeState = cChargeTimeoutDeal;
                //执行其他操作
                sStopCharging();
                return;
            }
        }

        stChargeData.u16_TimeOutCnt++;
        if(stChargeData.u16_TimeOutCnt > cTim5s)
        {
            stChargeData.u16_TimeOutCnt = 0;
            stChargeData.u16_WaitTim250ms = READY;
            stChargeData.u16_WaitTim500ms = READY;
            eChargeConfigStep = CHECK_BRO;
            CEM_CODE = BRO_TIMEOUT;
            u8_ChargeState = cChargeTimeoutDeal;
            //执行其他操作
            sStopCharging();
            return;
        }

    }break;

    case CHECK_BCL:
    {
        if(stChargeData.u16_WaitTim250ms == READY)
        {
            stChargeData.u16_WaitTim250ms = cTim250ms;
            SendCRO(CC);
        }

        if(RC_SUCCESS == J1939_Read_Message(pRxBuff,Select_CAN_NODE_1))
        {
            if(pRxBuff->Mxe.PDUFormat == BCL_PGN)
            {
                //跳转到充电阶段

                //解析车辆充电需求
                sGetVehicleChargeCfg(&stVehicleChargeCfg,pRxBuff->Mxe.Data);
                stChargeData.u16_TimeOutCnt = 0;
                stChargeData.u16_WaitTim250ms = READY;
                stChargeData.u16_WaitTim500ms = READY;
                eChargeConfigStep = CHECK_BRO;
                u8_ChargeState = cChargeRunning;
                return;
            }
            else if(pRxBuff->Mxe.PDUFormat == BEM_PGN)
            {
                //超时处理收到车辆接收超时报文BEM
                stChargeData.u16_TimeOutCnt = 0;
                stChargeData.u16_WaitTim250ms = READY;
                stChargeData.u16_WaitTim500ms = READY;
                eChargeConfigStep = CHECK_BRO;
                VehicleBEMflag = 1;
                u8_ChargeState = cChargeTimeoutDeal;
                //执行其他操作
                sStopCharging();
                return;
            }
        }

        stChargeData.u16_TimeOutCnt++;
        if(stChargeData.u16_TimeOutCnt > cTim1s)
        {
            stChargeData.u16_TimeOutCnt = 0;
            stChargeData.u16_WaitTim250ms = READY;
            stChargeData.u16_WaitTim500ms = READY;
            eChargeConfigStep = CHECK_BRO;
            CEM_CODE = BCL_TIMEOUT;
            u8_ChargeState = cChargeTimeoutDeal;
            sStopCharging();
            return;
        }

    }break;

    default:break;


    }

}


void sChargeRunning(void)
{
    J1939_MESSAGE *pRxBuff = &stChargeData.ReadMesBuff;
    TP_RX_MESSAGE *pLongRxbuff = &stChargeData.LongMesBuff;
    static uint16_t u16_TimeoutCntChange = cTim1s;
    static uint8_t startCharing = 0;

    if(stChargeData.u16_WaitTim50ms != READY)
    {
       stChargeData.u16_WaitTim50ms--;
    }

    switch(eChargeRunningStep)
    {
    case SEND_CCS:
    {
        if(stChargeData.u16_WaitTim50ms == READY)
        {
            stChargeData.u16_WaitTim50ms = cTim50ms;
            SendCCS();
            //开启充电
            if(startCharing == 0 || stVehicleSta.u16_ChargingEnable == 1)
            {
                startCharing = 1;
                sStartCharging();
            }
        }

        //充电机达到充电结束条件
        if(uCSTData.type.ChargerAbortFlag.type.u16_CondiFinish == 1)
        {
            sGetVehicleBST(&uBSTData,pRxBuff->Mxe.Data);
            eChargeRunningStep = SEND_CST;
            stChargeData.u16_WaitTim50ms = READY;
            stChargeData.u16_TimeOutCnt = 0;
            u16_TimeoutCntChange = cTim5s;
            sStopCharging();
            return;
        }

        if(RC_SUCCESS == J1939_Read_Message(pRxBuff,Select_CAN_NODE_1))
        {
            if(pRxBuff->Mxe.PDUFormat == BCL_PGN)
            {
                //解析车辆充电需求
                sGetVehicleChargeCfg(&stVehicleChargeCfg,pRxBuff->Mxe.Data);
                stChargeData.u16_TimeOutCnt = 0;
                u16_TimeoutCntChange = cTim1s;
            }
            else if(pRxBuff->Mxe.PDUFormat == BSM_PGN)
            {
                sGetVehicleSta(&stVehicleSta,pRxBuff->Mxe.Data);
                stChargeData.u16_TimeOutCnt = 0;
                u16_TimeoutCntChange = cTim5s;
                if(stVehicleSta.u16_ChargingEnable == 0)
                {
                    //暂停充电
                    sStopCharging();
                }
            }
            else if(pRxBuff->Mxe.PDUFormat == BEM_PGN)
            {
                //超时处理收到车辆接收超时报文BEM
                stChargeData.u16_TimeOutCnt = 0;
                stChargeData.u16_WaitTim50ms = READY;
                stChargeData.u16_WaitTim250ms = READY;
                stChargeData.u16_WaitTim500ms = READY;
                eChargeRunningStep = SEND_CCS;
                VehicleBEMflag = 1;
                startCharing = 0;
                u16_TimeoutCntChange = cTim1s;
                u8_ChargeState = cChargeTimeoutDeal;
                //执行其他操作
                sStopCharging();
                return;
            }
            else if(pRxBuff->Mxe.PDUFormat == BST_PGN)
            {
                sGetVehicleBST(&uBSTData,pRxBuff->Mxe.Data);
                eChargeRunningStep = SEND_CST;
                stChargeData.u16_WaitTim50ms = READY;
                stChargeData.u16_TimeOutCnt = 0;
                u16_TimeoutCntChange = cTim5s;
                return;
            }
        }

        if(RC_SUCCESS == J1939_TP_RX_Message(pLongRxbuff,Select_CAN_NODE_1))
        {
            if(RSHIFT(pLongRxbuff->PGN) == BCS_PGN)
            {
                //更新BCS报文参数
                stChargeData.u16_TimeOutCnt = 0;
                sGetBatteryChargeSta(&stBatteryChargeSta,pLongRxbuff->data);
                u16_TimeoutCntChange = cTim5s;
            }

        }

        stChargeData.u16_TimeOutCnt++;
        if(stChargeData.u16_TimeOutCnt > u16_TimeoutCntChange)
        {
            stChargeData.u16_TimeOutCnt = 0;
            stChargeData.u16_WaitTim50ms = READY;
            stChargeData.u16_WaitTim500ms = READY;
            u16_TimeoutCntChange = cTim1s;
            eChargeRunningStep = SEND_CCS;
            startCharing = 0;
            CEM_CODE = BCL_TIMEOUT;
            u8_ChargeState = cChargeTimeoutDeal;
            //执行其他操作
            sStopCharging();
        }

    }break;

    case SEND_CST:
    {
        SendCST(&uCSTData);

        //如果充电机主动中止
        if((uCSTData.data[0] & 0x3F) != 0 || uCSTData.data[1] != 0 || uCSTData.data[2] != 0
                || uCSTData.data[3] != 0)
        {
            if(RC_SUCCESS == J1939_Read_Message(pRxBuff,Select_CAN_NODE_1))
            {
                if(pRxBuff->Mxe.PDUFormat == BST_PGN)
                {
                     sGetVehicleBST(&uBSTData,pRxBuff->Mxe.Data);
                     eChargeRunningStep = SEND_CST;
                     stChargeData.u16_WaitTim50ms = READY;
                     u16_TimeoutCntChange = cTim10s;
                }
                else if(pRxBuff->Mxe.PDUFormat == BSD_PGN)
                {
                    //结束充电
                    stChargeData.u16_TimeOutCnt = 0;
                    stChargeData.u16_WaitTim50ms = READY;
                    stChargeData.u16_WaitTim500ms = READY;
                    u16_TimeoutCntChange = cTim1s;
                    eChargeRunningStep = SEND_CCS;
                    startCharing = 0;
                    u8_ChargeState = cChargeEnding;
                    //执行其他操作
                    sStopCharging();
                }

            }

        }
        else
        {
            u16_TimeoutCntChange = cTim10s;
            if(RC_SUCCESS == J1939_Read_Message(pRxBuff,Select_CAN_NODE_1))
            {
                if(pRxBuff->Mxe.PDUFormat == BSD_PGN)
                {
                    ////结束充电,跳到充电结束阶段
                    stChargeData.u16_TimeOutCnt = 0;
                    stChargeData.u16_WaitTim50ms = READY;
                    stChargeData.u16_WaitTim500ms = READY;
                    u16_TimeoutCntChange = cTim1s;
                    eChargeRunningStep = SEND_CCS;
                    startCharing = 0;
                    u8_ChargeState = cChargeEnding;
                    //执行其他操作
                    sStopCharging();
                }

            }
        }


        stChargeData.u16_TimeOutCnt++;
        if(stChargeData.u16_TimeOutCnt > u16_TimeoutCntChange)
        {
            stChargeData.u16_TimeOutCnt = 0;
            stChargeData.u16_WaitTim50ms = READY;
            stChargeData.u16_WaitTim500ms = READY;
            u16_TimeoutCntChange = cTim1s;
            eChargeRunningStep = SEND_CCS;
            startCharing = 0;
            CEM_CODE = BCL_TIMEOUT;
            u8_ChargeState = cChargeTimeoutDeal;
            //执行其他操作
            sStopCharging();
        }


    }break;

    default:break;

    }

}


void sChargeEnding(void)
{
    if(stChargeData.u16_WaitTim250ms != READY)
    {
       stChargeData.u16_WaitTim250ms--;
    }

    if(stChargeData.u16_WaitTim250ms == READY)
    {
        stChargeData.u16_WaitTim250ms = cTim250ms;
        SendCSD();
    }

    if(!CHARGE_CONNECT)
    {
        //检测断开返回待机状态
        u8_ChargeState = cChargeStandBy;
    }


}


void sChargeTimeoutDeal(void)
{

    if(stChargeData.u16_WaitTim250ms != READY)
    {
       stChargeData.u16_WaitTim250ms--;
    }


    if(stChargeData.u16_WaitTim250ms == READY)
    {
        stChargeData.u16_WaitTim250ms = cTim250ms;
        if(!VehicleBEMflag)
        {
            SendCEM(CEM_CODE);
        }
    }

    if(stChargeData.u16_ReConnectCnt == MAX_RECONNECT)
    {
        stChargeData.u16_ReConnectCnt = 0;
        stChargeData.u16_TimeOutCnt = 0;
        VehicleBEMflag = 0;
        stChargeData.u16_WaitTim50ms = READY;
        stChargeData.u16_WaitTim250ms = READY;
        stChargeData.u16_WaitTim500ms = READY;
        stChargeData.u16_WaitTim10000ms = READY;
        eHandShakeStep = SEND_CHM;
        eChargeConfigStep = CHECK_BRO;
        eChargeRunningStep = SEND_CCS;
        u8_ChargeState = cChargeErr;
        memset(uCSTData.data,0,4);
        uCSTData.type.ChargerAbortFlag.type.u16_ErrAbort = 1;
        //执行其他操作
        sStopCharging();
        return;

    }


    if((stChargeData.VdcC1C2 > -60) && (stChargeData.VdcC1C2 < 60))
    {
        OPEN_C1_C2;
        stChargeData.u16_ReConnectCnt++;
        stChargeData.u16_WaitTim250ms = READY;
        VehicleBEMflag = 0;
        stChargeData.u16_TimeOutCnt = 0;
        u8_ChargeState = cChargeHandShake;
        eHandShakeStep = SEND_CRM;
        return;
    }


    stChargeData.u16_TimeOutCnt++;
    if(stChargeData.u16_TimeOutCnt > cTim10s)
    {
        stChargeData.u16_TimeOutCnt = 0;
        VehicleBEMflag = 0;
        stChargeData.u16_WaitTim50ms = READY;
        stChargeData.u16_WaitTim250ms = READY;
        stChargeData.u16_WaitTim500ms = READY;
        stChargeData.u16_WaitTim10000ms = READY;
        eHandShakeStep = SEND_CHM;
        eChargeConfigStep = CHECK_BRO;
        eChargeRunningStep = SEND_CCS;
        memset(uCSTData.data,0,4);
        uCSTData.type.ChargerAbortFlag.type.u16_ErrAbort = 1;
        u8_ChargeState = cChargeErr;
        //执行其他操作
        sStopCharging();
        return;
    }


}


void sChargeErr(void)
{
    TP_RX_MESSAGE *pLongRxbuff = &stChargeData.LongMesBuff;
    //如果手动复位或者断开回到待机状态
    if(CHARGE_RESET)
    {
        u8_ChargeState = cChargeStandBy;
    }
    //退出充电定期发送CST报文
    SendCST(&uCSTData);

}


void Gbt27930_Asystem(void)
{
    //J1939_MESSAGE *pTxBuff = &stChargeData.SendMesBuff;
    switch(u8_ChargeState)
    {
    case cChargeStandBy:
    {
        sChargeStandBy();
    }break;

    case cChargeHandShake:
    {
        sChargeHandShake();

    }break;

    case cChargeConfig:
    {
        sChargeConfig();

    }break;

    case cChargeRunning:
    {
        sChargeRunning();
    }break;

    case cChargeEnding:
    {
        sChargeEnding();

    }break;

    case cChargeTimeoutDeal:
    {
        sChargeTimeoutDeal();

    }break;

    case cChargeErr:
    {
        sChargeErr();

    }break;

    default:break;


    }


}

