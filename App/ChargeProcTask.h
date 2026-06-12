/*
 * ChargeProcTask.h
 *
 *  Created on: 2026年5月27日
 *      Author: Dell
 */

#ifndef APP_CHARGEPROCTASK_H_
#define APP_CHARGEPROCTASK_H_
#include <J1939/J1939.h>

//充电机时间同步CTS报文开关,默认关闭
#define cEnableCTS       0

#define CHARGE_CONNECT   1
#define CHARGE_RESET     0

#define     cChargTimerEvt           0

#define     READY           0

//状态机序号
#define cChargeStandBy      0
#define cChargeHandShake    1
#define cChargeConfig       2
#define cChargeRunning      3
#define cChargeEnding       4
#define cChargeTimeoutDeal  5
#define cChargeErr          6

#define cTim60s           60000/J1939_TP_FREQ
#define cTim10s           10000/J1939_TP_FREQ
#define cTim5s            5000/J1939_TP_FREQ
#define cTim1s            1000/J1939_TP_FREQ
#define cTim250ms         250/J1939_TP_FREQ
#define cTim500ms         500/J1939_TP_FREQ
#define cTim50ms          50/J1939_TP_FREQ


#define SECC 0x56       //充电机通信地址
#define EVCC 0xF4       //电动汽车通信地址

//规定通信超时重连次数
#define MAX_RECONNECT   2

#define RSHIFT(x)  (x>>8)

//参数组编号识别
#define BEM_PGN   RSHIFT(0x1E00)
#define CEM_PGN   RSHIFT(0x1F00)

#define CHM_PGN   RSHIFT(0x2600)
#define BHM_PGN   RSHIFT(0x2700)
#define CRM_PGN   RSHIFT(0x0100)
#define BRM_PGN   RSHIFT(0x0200)

#define BCP_PGN   RSHIFT(0x0600)
#define CTS_PGN   RSHIFT(0x0700)
#define CML_PGN   RSHIFT(0x0800)
#define BRO_PGN   RSHIFT(0x0900)
#define CRO_PGN   RSHIFT(0x0A00)

#define BCL_PGN   RSHIFT(0x1000)
#define BCS_PGN   RSHIFT(0x1100)
#define CCS_PGN   RSHIFT(0x1200)
#define BSM_PGN   RSHIFT(0x1300)
#define BMV_PGN   RSHIFT(0x1500)
#define BMT_PGN   RSHIFT(0x1600)
#define BSP_PGN   RSHIFT(0x1700)
#define BST_PGN   RSHIFT(0x1900)
#define CST_PGN   RSHIFT(0x1A00)

#define BSD_PGN   RSHIFT(0x1C00)
#define CSD_PGN   RSHIFT(0x1D00)

//报文错误代码
#define BRM_TIMEOUT 0x0001
#define BCP_TIMEOUT (0x0001<<2)
#define BRO_TIMEOUT (0x0001<<4)
#define BCS_TIMEOUT (0x0001<<6)
#define BCL_TIMEOUT (0x0001<<8)
#define BST_TIMEOUT (0x0001<<10)
#define BSD_TIMEOUT (0x0001<<12)
#define BSM_TIMEOUT (0x0001<<14)

#define BRM_UNR     0x0002
#define BCP_UNR     (0x0002<<2)
#define BRO_UNR     (0x0002<<4)
#define BCS_UNR     (0x0002<<6)
#define BCL_UNR     (0x0002<<8)
#define BST_UNR     (0x0002<<10)
#define BSD_UNR     (0x0002<<12)
#define BSM_UNR     (0x0002<<14)


#define OPEN_C1_C2  {}
#define CLOSE_C1_C2  {}


//帧优先级定义
typedef enum
{
    PRI0 = 0,
    PRI1,
    PRI2,
    PRI3,
    PRI4,
    PRI5,
    PRI6,
    PRI7,

}MsPriority;


//车辆CRM报文BMS及车辆识别代码
typedef enum
{
    NVI = 0,     //未识别
    VI = 0xAA,   //识别
}CRM_VI;

//车辆BRO报文充电就绪代码
typedef enum
{
    NVC = 0,     //车辆充电未准备
    VC = 0xAA,   //车辆充电就绪
    IVC = 0xFF,  //不确定状态
}BRO_VI;

//充电机CRO报文充电准备代码
typedef enum
{
    NCC = 0,     //充电机充电未准备
    CC = 0xAA,   //充电机充电就绪
    ICC = 0xFF,  //不确定状态
}CRO_VI;

//握手阶段序号
typedef enum
{
    SEND_CHM = 0,
    SEND_CRM,
    CHECK_BCP,
}HandShakeStep;

//充电配置阶段序号
typedef enum
{
    CHECK_BRO = 0,
    SEND_CRO,
    CHECK_BCL,
}ChargeConfigStep;

//充电运行阶段序号
typedef enum
{
    SEND_CCS = 0,
    SEND_CST,
}ChargeRunningStep;



typedef struct
{
   uint16_t u16_WaitTim50ms;
   uint16_t u16_WaitTim250ms;
   uint16_t u16_WaitTim500ms;
   uint16_t u16_WaitTim10000ms;
   uint16_t u16_TimeOutCnt;
   uint16_t u16_ReConnectCnt;
   float VdcC1C2;
   J1939_MESSAGE SendMesBuff;    //短消息发送缓存
   J1939_MESSAGE ReadMesBuff;    //短消息接收缓存
   TP_RX_MESSAGE LongMesBuff;    //长消息接收缓存

}ST_CHARGESTACK;


typedef union
{
    struct
    {
        union
        {
            struct
            {
                uint16_t u16_CondiFinish : 2;     //充电机设定条件中止 0：正常   1：达到条件中止  2：不可信状态
                uint16_t u16_ArtiAbort : 2;     //人工中止
                uint16_t u16_ErrAbort : 2;      //故障中止
                uint16_t u16_VehicleAbort : 2;  //车辆主动中止
                uint16_t Resv: 8;             //
            }type;
            uint16_t data;
        }ChargerAbortFlag;  //充电机中止充电原因

        union
        {
            struct
            {
                uint16_t u16_OverTemp : 2;           //充电机过温故障
                uint16_t u16_ConnectorFault : 2;     //充电连接器故障
                uint16_t u16_InsideOverTemp : 2;     //充电机内部过温故障
                uint16_t u16_BatteryNeedFault : 2;   //所需电量不能传送
                uint16_t Resv : 8;             //
            }type;
            uint16_t data;
        }ChargerFaultFlagLow; //充电机中止充电故障原因低8位

        union
        {
            struct
            {
                uint16_t u16_Jerk: 2;                 //急停故障
                uint16_t u16_OtherFault : 2;          //其他故障
                uint16_t u16_SelfCheckFault : 2;      //自检故障
                uint16_t u16_PrechargeFault : 2;      //预充故障
                uint16_t Resv : 8;                    //
            }type;
            uint16_t data;
        }ChargerFaultFlagHigh;  //充电机中止充电故障原因高8位

        union
        {
            struct
            {
                uint16_t u16_CurrentErr : 2;           //电流不匹配
                uint16_t u16_VoltErr : 2;              //电压异常
                uint16_t u16_ChargeConfigErr : 2;      //充电参数不匹配
                uint16_t Resv: 10;
            }type;
            uint16_t data;
        }ChargerErrFlag;   //充电机中止充电错误原因

    }type;

    uint8_t data[4];

}UCSTDATA;  //CST充电机中止充电报文


typedef struct
{
     uint16_t u16_oBatteryMaxVolt;  //单体蓄电池最高允许电压，0.01V/bit，0~24V
     int16_t i16_ChargeCurrMax;    //最高允许充电电流，0.1A/bit，-400A~***
     uint16_t u16_Eb;               //动力蓄电池标称总能量，0.1kWh/bit,0~1000KWH
     uint16_t u16_ChargeVoltMax;    //最高允许充电总电压，0.1V/bit,0~***
     int16_t i16_oBatteryMaxTemp;  //单体蓄电池最高允许温度，1°c/bit,-50°c~+200°c
     uint16_t u16_SOC;              //整车电荷状态，0.1%/bit,0~100%
     uint16_t u16_VehicleVolt;      //车辆接口当前电压测量值，0.1V/bit,0~***
}ST_VEHICLEMSG;                     //车辆BCP报文数据

typedef struct
{
    uint16_t u16_MaxChargeVolt;     //充电机最高充电电压，0.1V/bit,0~***
    uint16_t u16_MinChargeVolt;     //充电机最低充电电压，0.1V/bit,0~***
    uint16_t u16_MaxChargeCurr;     //充电机最大充电电流，0.1A/bit,-400A~*
    uint16_t u16_MinChargeCurr;     //充电机最小充电电流，0.1A/bit,-400A~*
}ST_CHARGERCONFIG;                  //充电机CML输出能力报文数据

typedef struct
{
    uint16_t u16_VehicleVolt;       //车辆接口充电电压需求，0.1V/bit,0~***
    int16_t  i16_VehicleCurrent;    //车辆接口充电电流需求，0.1A/bit,-400A~***
    uint16_t u16_ChargingType;      //充电模式 1:恒压充电,2:恒流充电
}ST_VEHICLEBCL;                     //车辆BCL充电需求报文数据


typedef struct
{
    uint16_t u16_VehicleVolt;       //车辆接口电压测量值，0.1V/bit,0~***
    int16_t  i16_VehicleCurrent;    //车辆接口电流测量值，0.1A/bit,-400A~***
    uint16_t u16_oBatteryMaxVolt;   //最高单体蓄电池电压，0.01V/bit,0~24v
    uint16_t u16_oBatteryGroup;     //最高单体蓄电池电压所在组号
    uint16_t u16_SOC;               //整车电荷状态，1%/bit,0~100%
    uint16_t u16_RemainTime;        //剩余充电时间
}ST_BATTERYBCS;                     //电池充电状态BCS报文数据


typedef struct
{
    uint16_t u16_ChargerVolt;       //充电机车辆接口充电电压测量值，0.1V/bit,0~***
    uint16_t u16_ChargerCurrent;    //充电机车辆接口充电电流测量值，0.1A/bit,-400A~***
    uint16_t u16_ChargingTime;      //累计充电时间，min/bit
    uint16_t u16_ChargingEnable;    //充电允许，0：赞同，1：允许
}ST_CHARGERCCS;                     //充电机充电状态CCS报文数据


typedef struct
{
    uint16_t u16_ChargingTime;      //充电机本次充电时间，1min/bit,0~600min
    uint16_t u16_OutputPower;       //充电机输出能量，0.1kWh/bit,0~1000kwh
    uint32_t u16_ChargerID;         //充电机编号
}ST_CHARGERCSD;                     //充电机统计数据CSD报文数据


typedef struct
{
    uint16_t u16_oBatteryIndex;        //最高单体蓄电池电压所在编号
    int16_t  i16_oBatteryMaxTemp;      //当前单体蓄电池最高温度
    uint16_t u16_BatteryMaxTempIndex;  //最高动力蓄电池检测点编号
    int16_t  i16_oBatteryMinTemp;      //当前单体蓄电池最低温度
    uint16_t u16_BatteryMinTempIndex;  //最低动力蓄电池检测点编号
    uint16_t u16_oBatteryVoltLevel;    //当体蓄电池电压水平，0：正常，1：过高，2：过低
    uint16_t u16_BatterySOCLevel;      //动力蓄电池电荷状态，0：正常，1：过高，2：过低
    uint16_t u16_BatteryOverCurrent;   //动力蓄电池充电过流状态  0：正常，1：过高，2：不可信
    uint16_t u16_BatteryOverTemp;      //动力蓄电池温度过高状态  0：正常，1：过高，2：不可信
    uint16_t u16_BatteryIsolSta;        //动力蓄电池绝缘状态
    uint16_t u16_BatteryConnectorSta;  //动力蓄电池输出连接器状态
    uint16_t u16_ChargingEnable;        //充电允许  0：禁止  1：允许
}ST_VEHICLEBSM;                         //车辆状态BSM报文数据


typedef struct
{
    struct
    {
        union
        {
            struct
            {
                uint16_t u16_ReachSOC : 2;              //达到所需SOC目标值 0：未达   1：达到  2：不可信状态
                uint16_t u16_ReachVolt : 2;             //达到总电压设定值  0：未达   1：达到  2：不可信状态
                uint16_t u16_ReachSingleVolt : 2;       //达到单体电压设定值  0：未达   1：达到  2：不可信状态
                uint16_t u16_ChargerStop : 2;           //充电机主动中止  0：正常   1：充电机中止  2：不可信状态
                uint16_t Resv: 8;                       //
            }type;
            uint16_t data;
        }VehicleAbortFlag;  //车辆中止充电原因

        union
        {
            struct
            {
                uint16_t u16_IsolFault : 2;          //绝缘故障
                uint16_t u16_SocketOverTemp : 2;     //车辆插座过温故障
                uint16_t u16_HarnessOverTemp : 2;    //车辆内部线束过温
                uint16_t u16_CouplerFault : 2;       //充电耦合器故障
                uint16_t Resv : 8;             //
            }type;
            uint16_t data;
        }VehicleFaultFlagLow; //车辆中止充电故障原因低8位

        union
        {
            struct
            {
                uint16_t u16_BatteryOverTemp: 2;      //电池组温度过高
                uint16_t u16_RelayFault : 2;          //高压继电器故障
                uint16_t u16_Check2VoltFault : 2;     //检测点2电压检测故障
                uint16_t u16_OtherFault : 2;          //其他故障
                uint16_t Resv : 8;                    //
            }type;
            uint16_t data;
        }VehicleFaultFlagHigh;  //车辆中止充电故障原因高8位

        union
        {
            struct
            {
                uint16_t u16_OverCurrent : 2;           //电流过大
                uint16_t u16_VoltErr : 2;               //电压异常
                uint16_t u16_ChargeConfigErr : 2;       //充电参数不匹配
                uint16_t Resv: 10;
            }type;
            uint16_t data;
        }VehicleErrFlag;   //车辆中止充电错误原因

    }type;

    uint8_t data[4];

}UBSTDATA;                        //车辆中止充电BST报文数据



void sChargeProcTask(void);


extern ST_CHARGERCONFIG stChargerCfg;
#endif /* APP_CHARGEPROCTASK_H_ */
