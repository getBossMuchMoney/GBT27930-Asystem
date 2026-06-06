/*
 * ModbusRtu.h
 *
 *  Created on: 2025Äê8ÔÂ5ÈÕ
 *      Author: Dell
 */

#ifndef MODULE_MODBUSRTU_H_
#define MODULE_MODBUSRTU_H_

#include "F28x_Project.h"
#include "stdlib.h"
#include "UartFunction.h"

#define MOD_STA_SIZE sizeof(MOD_STA)
#define MOD_SET_SIZE sizeof(MOD_SET)

typedef enum
{
    MBEC_SUCSSES = 0,
    MBEC_ERR_CMD = 1,
    MBEC_ERR_ADDR = 2,
    MBEC_ERR_VALUE = 3,
    MBEC_ERR_FIALD = 4,
    MBEC_ERR_PWD = 5,
    MBEC_ERR_FRAME = 6,
    MBEC_READ_ONLY = 7,
    MBEC_RUN_EDIT = 8,
    MBEC_PWD_PROTECT = 9,
    MBEC_ERR_REG_NUM = 0xA,
    MBEC_NO_AUTHORITY = 0xB,
} MDB_ErrCode;


typedef struct
{
    uint8_t *pReadData;
    uint8_t *pWriteData;
    SCI_MODE mod;
    uint16_t station;
    uint16_t addr;
    uint16_t addr1;
    uint16_t func_code;
    uint16_t err_code;
    uint16_t reg_num;
    uint16_t reg_num1;
    uint16_t rcv_flag;
    uint16_t SendWaitCnt;


}ModbusOpt;


typedef enum
{
    READ_REG = 0x03,
    WRITE_SINGLE = 0x06,
    WIRITE_MULTI = 0x10,
    WRITE_AND_READ = 0x17,
} MODB_FUNCC;


typedef union
{
    uint16_t buf[2];
    uint32_t value;
} U_u32;

typedef union
{
    uint16_t buf[2];
    int32_t  value;
} U_i32;

typedef union
{
    uint16_t value;
    struct {
        uint16_t u16_TurnOnOff:1;
        uint16_t u16_PfcOnOff:1;
        uint16_t u16_ClearErr:1;
        uint16_t u16_Resv2:1;
        uint16_t u16_PfcOpenLoopEn:1;
        uint16_t u16_DcOpenLoopEn:1;
        uint16_t u16_Resv3:10;

    }BIT;
}U_SysCMD;


typedef struct
{
    uint16_t u16_PfcStatus;
    uint16_t u16_PfcFaultCode;
    uint16_t u16_Grid_V;
    uint16_t u16_Grid_I;
    int16_t  u16_Vbus;
    uint16_t u16_Vout;
    uint16_t u16_Iout;
    int16_t  i16_Temp1;
    int16_t  i16_Temp2;
    uint16_t u16_DcWorkMode;
    uint16_t u16_DcStatus;
    uint16_t u16_DcFaultCode;
    uint16_t u16_DcFwVersion;
    uint16_t u16_PfcFwVersion;
    uint16_t u16_Vscap;

} MOD_STA;


typedef struct
{
    uint16_t u16_PfcBusSet;
    uint16_t u16_RatedGridV;
    uint16_t u16_RatedGridFreq;
    uint16_t u16_RatedGrdiI;
    uint16_t u16_PfcFsw;
    uint16_t u16_PfcCtrlFreq;
    U_SysCMD U16_CmdWd;
    uint16_t u16_VoutSet;
    uint16_t u16_IoutSet;
    uint16_t u16_DcFw;
    uint16_t u16_DcCtrlFreq;
    uint16_t u16_DcOpenPs;
    U_u32 u32_CtrlK1;
    U_u32 u32_CtrlK2;
    U_u32 u32_CtrlK3;
    U_u32 u32_CtrlK4;
    U_u32 u32_CtrlK5;
    U_u32 u32_CtrlK6;
    U_u32 u32_CtrlK7;
    U_u32 u32_CtrlK8;
    U_u32 u32_CtrlK9;
    U_u32 u32_CtrlK10;
    U_u32 u32_CtrlK11;
    U_u32 u32_CtrlK12;
    U_u32 u32_CtrlK13;
    U_u32 u32_CtrlK14;
}MOD_SET;

typedef union
{
    MOD_STA  REG;
    uint16_t buff[MOD_STA_SIZE];
} U_MOD_STA;


typedef union
{
    MOD_SET  REG;
    uint16_t buff[MOD_SET_SIZE];
} U_MOD_SET;

extern U_MOD_STA uModSta;
extern U_MOD_SET uModSet;
extern uint8_t UartTxbuff[SCI_NUM][300];

void Modbus_Slaver_Respond(ModbusOpt *pOpt);
void Modbus_Master_Ask(ModbusOpt *pOpt);

void Value64SaveInBuff(const uint64_t *pvalue, uint8_t *pdata);
void BuffSaveInValue64(uint8_t *pdata,uint64_t *pvalue);
void Value32SaveInBuff(const uint32_t *pvalue, uint8_t *pdata);
void BuffSaveInValue32(uint8_t *pdata,uint32_t *pvalue);
void Value16SaveInBuff(const uint16_t *pvalue, uint8_t *pdata);
void BuffSaveInValue16(uint8_t *pdata,uint16_t *pvalue);
void read_REG(uint16_t addr, uint16_t reg_num, uint8_t *pdata);
void sModbusInit(void);

MDB_ErrCode write_REG(uint16_t addr, uint16_t reg_num, uint8_t *pdata);

unsigned int CRC16(unsigned int * puchMsg,unsigned int usDataLen);



#endif /* MODULE_MODBUSRTU_H_ */
