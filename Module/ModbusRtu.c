/*
 * ModbusRtu.c
 *
 *  Created on: 2025锟斤拷8锟斤拷6锟斤拷
 *      Author: Dell
 */

#include "ModbusRtu.h"
#include "UartFunction.h"
#include "string.h"


U_MOD_STA uModSta;
U_MOD_SET uModSet;

uint8_t UartTxbuff[SCI_NUM][300] = {0};

U_MOD_SET uModSetMinValue = {
    .REG.u16_PfcBusSet = 0,
     .REG.u16_RatedGridV= 0,
     .REG.u16_RatedGridFreq= 0,
     .REG.u16_RatedGrdiI= 0,
     .REG.u16_PfcFsw= 0,
     .REG.u16_PfcCtrlFreq= 0,
     .REG.U16_CmdWd.value= 0,
     .REG.u16_VoutSet= 0,
     .REG.u16_IoutSet= 0,
     .REG.u16_DcFw= 0,
     .REG.u16_DcCtrlFreq= 0,
     .REG.u16_DcOpenPs= 0,

};

U_MOD_SET uModSetMaxValue = {
    .REG.u16_PfcBusSet = 4300,
    .REG.u16_RatedGridV= 3000,
    .REG.u16_RatedGridFreq= 1,
    .REG.u16_RatedGrdiI= 600,
    .REG.u16_PfcFsw= 2000,
    .REG.u16_PfcCtrlFreq= 2000,
    .REG.U16_CmdWd.value= 0x7FFF,
    .REG.u16_VoutSet= 20000,
    .REG.u16_IoutSet= 2000,
    .REG.u16_DcFw= 2000,
    .REG.u16_DcCtrlFreq= 2000,
    .REG.u16_DcOpenPs= 100,

};

const unsigned int tblCRC[] = {
    0x0000, 0xC1C0, 0x81C1, 0x4001, 0x01C3, 0xC003, 0x8002, 0x41C2, 0x01C6, 0xC006, 0x8007, 0x41C7, 0x0005, 0xC1C5,
    0x81C4, 0x4004, 0x01CC, 0xC00C, 0x800D, 0x41CD, 0x000F, 0xC1CF, 0x81CE, 0x400E, 0x000A, 0xC1CA, 0x81CB, 0x400B,
    0x01C9, 0xC009, 0x8008, 0x41C8, 0x01D8, 0xC018, 0x8019, 0x41D9, 0x001B, 0xC1DB, 0x81DA, 0x401A, 0x001E, 0xC1DE,
    0x81DF, 0x401F, 0x01DD, 0xC01D, 0x801C, 0x41DC, 0x0014, 0xC1D4, 0x81D5, 0x4015, 0x01D7, 0xC017, 0x8016, 0x41D6,
    0x01D2, 0xC012, 0x8013, 0x41D3, 0x0011, 0xC1D1, 0x81D0, 0x4010, 0x01F0, 0xC030, 0x8031, 0x41F1, 0x0033, 0xC1F3,
    0x81F2, 0x4032, 0x0036, 0xC1F6, 0x81F7, 0x4037, 0x01F5, 0xC035, 0x8034, 0x41F4, 0x003C, 0xC1FC, 0x81FD, 0x403D,
    0x01FF, 0xC03F, 0x803E, 0x41FE, 0x01FA, 0xC03A, 0x803B, 0x41FB, 0x0039, 0xC1F9, 0x81F8, 0x4038, 0x0028, 0xC1E8,
    0x81E9, 0x4029, 0x01EB, 0xC02B, 0x802A, 0x41EA, 0x01EE, 0xC02E, 0x802F, 0x41EF, 0x002D, 0xC1ED, 0x81EC, 0x402C,
    0x01E4, 0xC024, 0x8025, 0x41E5, 0x0027, 0xC1E7, 0x81E6, 0x4026, 0x0022, 0xC1E2, 0x81E3, 0x4023, 0x01E1, 0xC021,
    0x8020, 0x41E0, 0x01A0, 0xC060, 0x8061, 0x41A1, 0x0063, 0xC1A3, 0x81A2, 0x4062, 0x0066, 0xC1A6, 0x81A7, 0x4067,
    0x01A5, 0xC065, 0x8064, 0x41A4, 0x006C, 0xC1AC, 0x81AD, 0x406D, 0x01AF, 0xC06F, 0x806E, 0x41AE, 0x01AA, 0xC06A,
    0x806B, 0x41AB, 0x0069, 0xC1A9, 0x81A8, 0x4068, 0x0078, 0xC1B8, 0x81B9, 0x4079, 0x01BB, 0xC07B, 0x807A, 0x41BA,
    0x01BE, 0xC07E, 0x807F, 0x41BF, 0x007D, 0xC1BD, 0x81BC, 0x407C, 0x01B4, 0xC074, 0x8075, 0x41B5, 0x0077, 0xC1B7,
    0x81B6, 0x4076, 0x0072, 0xC1B2, 0x81B3, 0x4073, 0x01B1, 0xC071, 0x8070, 0x41B0, 0x0050, 0xC190, 0x8191, 0x4051,
    0x0193, 0xC053, 0x8052, 0x4192, 0x0196, 0xC056, 0x8057, 0x4197, 0x0055, 0xC195, 0x8194, 0x4054, 0x019C, 0xC05C,
    0x805D, 0x419D, 0x005F, 0xC19F, 0x819E, 0x405E, 0x005A, 0xC19A, 0x819B, 0x405B, 0x0199, 0xC059, 0x8058, 0x4198,
    0x0188, 0xC048, 0x8049, 0x4189, 0x004B, 0xC18B, 0x818A, 0x404A, 0x004E, 0xC18E, 0x818F, 0x404F, 0x018D, 0xC04D,
    0x804C, 0x418C, 0x0044, 0xC184, 0x8185, 0x4045, 0x0187, 0xC047, 0x8046, 0x4186, 0x0182, 0xC042, 0x8043, 0x4183,
    0x0041, 0xC181, 0x8180, 0x4040,
};

static U_MOD_STA *pModSta = NULL;
static U_MOD_SET *pModSet = NULL;

static const U_MOD_SET *pModSetMaxValue = NULL;
static const U_MOD_SET *pModSetMinValue = NULL;

void sModbusInit(void)
{
    pModSta = &uModSta;
    pModSet = &uModSet;
    pModSetMaxValue = &uModSetMaxValue;
    pModSetMinValue = &uModSetMinValue;

    memset((uint8_t *)&uModSta, 0, sizeof(uModSta));
    memset((uint8_t *)&uModSet, 0, sizeof(uModSet));

    uModSet.REG.u16_PfcBusSet = 4000;
    uModSet.REG.u16_RatedGridV= 0,
    uModSet.REG.u16_RatedGridFreq= 0,
    uModSet.REG.u16_RatedGrdiI= 0,
    uModSet.REG.u16_PfcFsw= 0,
    uModSet.REG.u16_PfcCtrlFreq= 0,
    uModSet.REG.U16_CmdWd.value= 0,


    uModSet.REG.u32_CtrlK1.value = 3000;
    uModSet.REG.u32_CtrlK2.value = 100;
    uModSet.REG.u32_CtrlK3.value = 1000;//2000;
    uModSet.REG.u32_CtrlK4.value = 200;//100;
    uModSet.REG.u16_IoutSet = 5;
    uModSet.REG.u16_VoutSet = 712;

//    DINT;
//    stCtrlPiVout.f32_Kp = uModSet.REG.u32_CtrlK1.value * 0.00001f;
//    stCtrlPiVout.f32_Ki = uModSet.REG.u32_CtrlK2.value * 0.00001f;
//    stCtrlPiIdc.f32_Kp = uModSet.REG.u32_CtrlK3.value * 0.00001f;
//    stCtrlPiIdc.f32_Ki = uModSet.REG.u32_CtrlK4.value * 0.00001f;
//    EINT;

}

void Modbus_Master_Ask(ModbusOpt *pOpt)
{
    uint16_t crc = 0;
    uint8_t *ptxbuf = UartTxbuff[pOpt->mod];

    ptxbuf[0] = pOpt->station;
    ptxbuf[1] = pOpt->func_code;
    ptxbuf[2] = (pOpt->addr) >> 8;
    ptxbuf[3] = (pOpt->addr) & 0x00FF;

    switch (pOpt->func_code)
    {
    case READ_REG:
    {
        ptxbuf[4] = (pOpt->reg_num) >> 8;
        ptxbuf[5] = (pOpt->reg_num) & 0x00FF;
        crc = CRC16(ptxbuf, 6);
        ptxbuf[6] = crc & 0x00FF;
        ptxbuf[7] = crc >> 8;
        //sUartWrite(pOpt->mod, ptxbuf, 8);
        enMasterTXQueue(ptxbuf, 8);
    }
    break;

    case WRITE_SINGLE:
    {
        if (pOpt->pWriteData != NULL)
        {
            memcpy(ptxbuf + 4, pOpt->pWriteData, 2);
        }
        crc = CRC16(ptxbuf, 6);
        ptxbuf[6] = crc & 0x00ff;
        ptxbuf[7] = crc >> 8;
        //sUartWrite(pOpt->mod, ptxbuf, 8);
        enMasterTXQueue(ptxbuf, 8);
    }
    break;

    case WIRITE_MULTI:
    {
        ptxbuf[4] = (pOpt->reg_num) >> 8;
        ptxbuf[5] = (pOpt->reg_num) & 0x00ff;
        ptxbuf[6] = (pOpt->reg_num) * 2;
        if (pOpt->pWriteData != NULL)
        {
            memcpy(ptxbuf + 7, pOpt->pWriteData, ptxbuf[6]);
        }
        crc = CRC16(ptxbuf, ptxbuf[6] + 7);
        ptxbuf[7 + ptxbuf[6]] = crc & 0x00ff;
        ptxbuf[7 + ptxbuf[6] + 1] = crc >> 8;
        //sUartWrite(pOpt->mod, ptxbuf, 9 + ptxbuf[6]);
        enMasterTXQueue(ptxbuf, 9 + ptxbuf[6]);
    }
    break;

    case WRITE_AND_READ:
    {
        ptxbuf[4] = (pOpt->reg_num) >> 8;
        ptxbuf[5] = (pOpt->reg_num) & 0x00ff;
        ptxbuf[6] = (pOpt->addr1) >> 8;
        ptxbuf[7] = (pOpt->addr1) & 0x00FF;
        ptxbuf[8] = (pOpt->reg_num1) >> 8;
        ptxbuf[9] = (pOpt->reg_num1) & 0x00ff;
        ptxbuf[10] = (pOpt->reg_num1) * 2;

        if (pOpt->pWriteData != NULL)
        {
            memcpy(ptxbuf + 11, pOpt->pWriteData, ptxbuf[10]);
        }
        crc = CRC16(ptxbuf, 3 + (pOpt->reg_num) * 2);
        ptxbuf[11 + ptxbuf[10]] = crc & 0x00FF;
        ptxbuf[11 + ptxbuf[10] + 1] = crc >> 8;
        //sUartWrite(pOpt->mod, ptxbuf, 13 + ptxbuf[10]);
        enMasterTXQueue(ptxbuf, 13 + ptxbuf[10]);
    }
    break;

    default:
    {
    }
    break;
    }
}

void Modbus_Slaver_Respond(ModbusOpt *pOpt)
{
    uint16_t crc = 0;
    uint8_t *ptxbuf = UartTxbuff[pOpt->mod];

    ptxbuf[0] = pOpt->station;
    ptxbuf[1] = pOpt->func_code;

    if (pOpt->station == 0)
    {
        return;
    }

    switch (pOpt->func_code)
    {
    case READ_REG:
    {
        ptxbuf[2] = pOpt->reg_num * 2;
        if (pOpt->pReadData != NULL)
        {
            memcpy(ptxbuf + 3, pOpt->pReadData, pOpt->reg_num * 2);
        }
        crc = CRC16(ptxbuf, 3 + pOpt->reg_num * 2);
        ptxbuf[3 + pOpt->reg_num * 2] = crc & 0x00FF;
        ptxbuf[3 + pOpt->reg_num * 2 + 1] = crc >> 8;
        sUartWrite(pOpt->mod, ptxbuf, 3 + pOpt->reg_num * 2 + 2);
    }
    break;

    case WRITE_SINGLE:
    {
        ptxbuf[2] = (pOpt->addr) >> 8;
        ptxbuf[3] = (pOpt->addr) & 0x00FF;
        ptxbuf[4] = pOpt->pReadData[0];
        ptxbuf[5] = pOpt->pReadData[1];
        crc = CRC16(ptxbuf, 6);
        ptxbuf[6] = crc & 0x00ff;
        ptxbuf[7] = crc >> 8;
        sUartWrite(pOpt->mod, ptxbuf, 8);
    }
    break;

    case WIRITE_MULTI:
    {
        ptxbuf[2] = (pOpt->addr) >> 8;
        ptxbuf[3] = (pOpt->addr) & 0x00FF;
        ptxbuf[4] = (pOpt->reg_num) >> 8;
        ptxbuf[5] = (pOpt->reg_num) & 0x00ff;
        crc = CRC16(ptxbuf, 6);
        ptxbuf[6] = crc & 0x00ff;
        ptxbuf[7] = crc >> 8;
        sUartWrite(pOpt->mod, ptxbuf, 8);
    }
    break;

    case WRITE_AND_READ:
    {
        ptxbuf[2] = pOpt->reg_num * 2;
        if (pOpt->pReadData != NULL)
        {
            memcpy(ptxbuf + 3, pOpt->pReadData, pOpt->reg_num * 2);
        }
        crc = CRC16(ptxbuf, 3 + pOpt->reg_num * 2);
        ptxbuf[3 + pOpt->reg_num * 2] = crc & 0x00FF;
        ptxbuf[3 + pOpt->reg_num * 2 + 1] = crc >> 8;
        sUartWrite(pOpt->mod, ptxbuf, 3 + pOpt->reg_num * 2 + 2);
    }
    break;

    default:
    {
        ptxbuf[2] = pOpt->err_code;
        crc = CRC16(ptxbuf, 3);
        ptxbuf[3] = crc & 0x00ff;
        ptxbuf[4] = crc >> 8;
        sUartWrite(pOpt->mod, ptxbuf, 5);
    }
    break;
    }
}

void Value64SaveInBuff(const uint64_t *pvalue, uint8_t *pdata)
{
    uint8_t size = sizeof(*pvalue) * 2;
    uint8_t i = 0;
    if (pvalue == NULL || pdata == NULL)
    {
        return;
    }

    for (i = 0; i < size; i += 2)
    {
        pdata[i] = __byte((int16 *)pvalue, i + 1);
        pdata[i + 1] = __byte((int16 *)pvalue, i);
    }
}

void BuffSaveInValue64(uint8_t *pdata, uint64_t *pvalue)
{
    uint8_t size = sizeof(*pvalue) * 2;
    uint8_t i = 0;

    if (pvalue == NULL || pdata == NULL)
    {
        return;
    }

    for (i = 0; i < size; i += 2)
    {
        __byte((int16 *)pvalue, i) = pdata[i + 1];
        __byte((int16 *)pvalue, i + 1) = pdata[i];
    }
}

void Value32SaveInBuff(const uint32_t *pvalue, uint8_t *pdata)
{
    uint8_t size = sizeof(*pvalue) * 2;
    uint8_t i = 0;
    if (pvalue == NULL || pdata == NULL)
    {
        return;
    }

    for (i = 0; i < size; i += 2)
    {
        pdata[i] = __byte((int16 *)pvalue, i + 1);
        pdata[i + 1] = __byte((int16 *)pvalue, i);
    }
}

void BuffSaveInValue32(uint8_t *pdata, uint32_t *pvalue)
{
    uint8_t size = sizeof(*pvalue) * 2;
    uint8_t i = 0;

    if (pvalue == NULL || pdata == NULL)
    {
        return;
    }

    for (i = 0; i < size; i += 2)
    {
        __byte((int16 *)pvalue, i) = pdata[i + 1];
        __byte((int16 *)pvalue, i + 1) = pdata[i];
    }
}

void Value16SaveInBuff(const uint16_t *pvalue, uint8_t *pdata)
{
    uint8_t size = sizeof(*pvalue) * 2;
    uint8_t i = 0;
    if (pvalue == NULL || pdata == NULL)
    {
        return;
    }

    for (i = 0; i < size; i += 2)
    {
        pdata[i] = __byte((int16 *)pvalue, i + 1);
        pdata[i + 1] = __byte((int16 *)pvalue, i);
    }
}

void BuffSaveInValue16(uint8_t *pdata, uint16_t *pvalue)
{
    uint8_t size = sizeof(*pvalue) * 2;
    uint8_t i = 0;

    if (pvalue == NULL || pdata == NULL)
    {
        return;
    }

    for (i = 0; i < size; i += 2)
    {
        __byte((int16 *)pvalue, i) = pdata[i + 1];
        __byte((int16 *)pvalue, i + 1) = pdata[i];
    }
}


void read_REG(uint16_t addr, uint16_t reg_num, uint8_t *pdata)
{
    uint8_t  i = 0;
    uint16_t addr_base = addr & 0xFF80;
    uint16_t offset = addr - addr_base;

        if ((addr_base & 0x00FF) == 0x00)
        {
            for (i = 0; i < (reg_num * 2); i += 2)
            {
                *(pdata + i) = (pModSta->buff[offset + (i >> 1)]) >> 8;
                *(pdata + i + 1) = (pModSta->buff[offset + (i >> 1)]) & 0x00FF;
            }
        }
        else
        {
            for (i = 0; i < (reg_num * 2); i += 2)
            {
                *(pdata + i) = (pModSet->buff[offset + (i >> 1)]) >> 8;
                *(pdata + i + 1) = (pModSet->buff[offset + (i >> 1)]) & 0x00FF;
            }
        }
}

MDB_ErrCode write_REG(uint16_t addr, uint16_t reg_num, uint8_t *pdata)
{
    uint8_t     i = 0;
    int32_t    i32value, i32min, i32max = 0;
    MDB_ErrCode err = MBEC_SUCSSES;
    uint16_t    addr_base = addr & 0xFF80;
    uint16_t    offset = addr - addr_base;


        for (i = 0; i < reg_num * 2; i += 2)
        {
            if ((offset + (i >> 1)) < ((uint32_t)&pModSet->REG.u16_DcOpenPs - (uint32_t)pModSet->buff + 1))
            {
                i32value = (int16_t)(((*(pdata + i)) << 8) | (*(pdata + i + 1)));
                i32min = (int16_t)pModSetMinValue->buff[offset + (i >> 1)];
                i32max = pModSetMaxValue->buff[offset + (i >> 1)];
                if (i32min <= i32value && i32value <= i32max)
                {
                    pModSet->buff[offset + (i >> 1)] = i32value;
                }
                else
                {
                    err = MBEC_ERR_VALUE;
                }
            }
            else
            {
                pModSet->buff[offset + (i >> 1)] = ((*(pdata + i)) << 8) | (*(pdata + i + 1));
            }
        }

    return err;
}

unsigned int CRC16(unsigned int * puchMsg,unsigned int usDataLen)
{
    unsigned int uchCRCHi = 0xFF ; /* CRC 的高字节初始化*/
    unsigned int uchCRCLo = 0xFF ; /* CRC 的低字节初始化*/
    unsigned int uIndex ;          /* CRC 查询表索引 */
    unsigned int hi,low;

    while (usDataLen--)             /* 完成整个报文缓冲区*/
    {
        uIndex = uchCRCLo ^ *puchMsg++ ; /* 计算CRC */
        hi = tblCRC[uIndex] >> 8;
        low = tblCRC[uIndex] & 0xff;
        uchCRCLo = uchCRCHi ^ hi;
        uchCRCHi = low;
    }
    return (uchCRCHi << 8 | uchCRCLo) ;
}
