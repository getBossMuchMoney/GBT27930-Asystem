/*******************************************************************************
 * File Name: EEpromTask.c
 * Description: The system status machine.
 * Designer: Miller Xue
 * Version: 00
 * Copyright: Liyuan Haina Co.Ltd
 *******************************************************************************/
#include "EEpromTask.h"
#include "RTOS_APP.h"
#include "RTOS_Define.h"
#include "App.h"
#include "SpiEeprom_User.h"

typeEepromCmd EepromCmd;
uint16_t EepromCmdBuffer[EepromCmdBufferSize];



void sEEpromTaskInit(void);
void sEEpromTask(void);


void sEEpromTaskInit(void)
{
    sQInit(&EepromCmdeQueue,EepromCmdBuffer,EepromCmdMaxNum,EepromCmdSize);

    /****************Test*****************/
//    WriteAllData();

    ReadAllData();


    /****************Test*****************/
//    EepromUnitCal.Magic = 0xAA55;
//    EepromUnitCal.Version = 2001;
//    EepromUnitCal.Cal_Parm_Flag = 0;
//    EepromUnitCal.Cal_RefValue_I1 = 34504;
//    EepromUnitCal.Cal_ActValue_I1 = 35505;
//    EepromUnitCal.Cal_RefValue_I2 = 36506;
//    EepromUnitCal.Cal_ActValue_I2 = 37507;
//    EepromUnitCal.Cal_RefValue_I3 = 38508;
//    EepromUnitCal.Cal_ActValue_I3 = 39509;
//    EepromUnitCal.Cal_RefValue_I4 = 40510;
//    EepromUnitCal.Cal_ActValue_I4 = 41511;
//    EepromUnitCal.Cal_RefValue_I5 = 42512;
//    EepromUnitCal.Cal_ActValue_I5 = 43513;
//    EepromUnitCal.Cal_RefValue_I6 = 44514;
//    EepromUnitCal.Cal_ActValue_I6 = 45515;
//    EepromUnitCal.Cal_RefValue_I7 = 46516;
//    EepromUnitCal.Cal_ActValue_I7 = 47517;
//    EepromUnitCal.Cal_RefValue_I8 = 48518;
//    EepromUnitCal.Cal_ActValue_I8 = 49519;
//    EepromUnitCal.Cal_Parm_Flag1 = 0;
//    EepromUnitCal.Cal_RefValue_V1 = 50520;
//    EepromUnitCal.Cal_ActValue_V1 = 51521;
//    EepromUnitCal.Cal_RefValue_V2 = 52522;
//    EepromUnitCal.Cal_ActValue_V2 = 53523;
//    EepromUnitCal.Cal_RefValue_V3 = 54524;
//    EepromUnitCal.Cal_ActValue_V3 = 55525;
//    EepromUnitCal.Cal_RefValue_V4 = 56526;
//    EepromUnitCal.Cal_ActValue_V4 = 57527;
//    EepromUnitCal.Cal_RefValue_V5 = 58528;
//    EepromUnitCal.Cal_ActValue_V5 = 59529;
//    EepromUnitCal.Cal_RefValue_V6 = 60530;
//    EepromUnitCal.Cal_ActValue_V6 = 61531;
//    EepromUnitCal.Cal_RefValue_V7 = 62532;
//    EepromUnitCal.Cal_ActValue_V7 = 63533;
//    EepromUnitCal.Cal_RefValue_V8 = 64534;
//    EepromUnitCal.Cal_ActValue_V8 = 65535;
//    EepromUnitCal.Cal_I_Num_Points = 65535;
//    EepromUnitCal.Cal_V_Num_Points = 65535;
//    EepromUnitCal.u16_Recv1 = 7788;
//    EepromUnitCal.u16_Recv2 = 5566;
//    EepromUnitCal.u16_Recv3 = 3344;
//    EepromUnitCal.u16_Recv4 = 2211;
//    EepromUnitCal.Ctrl_Ratio_Flag = 0;
//    EepromUnitCal.Ctrl_Ratio_K1 = 65536;
//    EepromUnitCal.Ctrl_Ratio_K2 = 265537;
//    EepromUnitCal.Ctrl_Ratio_K3 = 465538;
//    EepromUnitCal.Ctrl_Ratio_K4 = 665539;
//    EepromUnitCal.Ctrl_Ratio_K5 = 865540;
//    EepromUnitCal.Ctrl_Ratio_K6 = 1065541;
//    EepromUnitCal.Ctrl_Ratio_K7 = 2065542;
//    EepromUnitCal.Ctrl_Ratio_K8 = 4065543;
//    EepromUnitCal.Ctrl_Ratio_K9 = 6065544;
//    EepromUnitCal.Ctrl_Ratio_K10 = 8065545;
//    EepromUnitCal.Ctrl_Ratio_K11 = 10065546;
//    EepromUnitCal.Ctrl_Ratio_K12 = 20065547;
//    EepromUnitCal.Ctrl_Ratio_K13 = 40065548;
//    EepromUnitCal.Ctrl_Ratio_K14 = 60065549;
//    EepromUnitCal.Ctrl_Ratio_K15 = 80065550;
//    EepromUnitCal.Ctrl_Ratio_K16 = 100065551;
//    EepromUnitCal.Ctrl_Ratio_K17 = 200065552;
//    EepromUnitCal.Ctrl_Ratio_K18 = 400065553;
//    EepromUnitCal.Ctrl_Ratio_K19 = 600065554;
//    EepromUnitCal.Ctrl_Ratio_K20 = 800065555;
//    EepromUnitCal.u32_Recv1 = 12345678;
//    EepromUnitCal.u32_Recv2 = 21345678;
//    EepromUnitCal.u32_Recv3 = 32145678;
//    EepromUnitCal.u32_Recv4 = 43215678;
//    EepromUnitCal.EEPROM_CRC = 800065555;
//
//    UserReadWriteFRAM(EEPROM_CMD_WRITE, 0x00, (uint16_t*)&EepromUnitCal, sizeof(EepromUnitCal));


//    EepromUnitCal.Magic = 0;
//    EepromUnitCal.Version = 0;
//    EepromUnitCal.Cal_Parm_Flag = 0;
//    EepromUnitCal.Cal_RefValue_I1 = 0;
//    EepromUnitCal.Cal_ActValue_I1 = 0;
//    EepromUnitCal.Cal_RefValue_I2 = 0;
//    EepromUnitCal.Cal_ActValue_I2 = 0;
//    EepromUnitCal.Cal_RefValue_I3 = 0;
//    EepromUnitCal.Cal_ActValue_I3 = 0;
//    EepromUnitCal.Cal_RefValue_I4 = 0;
//    EepromUnitCal.Cal_ActValue_I4 = 0;
//    EepromUnitCal.Cal_RefValue_I5 = 0;
//    EepromUnitCal.Cal_ActValue_I5 = 0;
//    EepromUnitCal.Cal_RefValue_I6 = 0;
//    EepromUnitCal.Cal_ActValue_I6 = 0;
//    EepromUnitCal.Cal_RefValue_I7 = 0;
//    EepromUnitCal.Cal_ActValue_I7 = 0;
//    EepromUnitCal.Cal_RefValue_I8 = 0;
//    EepromUnitCal.Cal_ActValue_I8 = 0;
//    EepromUnitCal.Cal_Parm_Flag1 = 0;
//    EepromUnitCal.Cal_RefValue_V1 = 0;
//    EepromUnitCal.Cal_ActValue_V1 = 0;
//    EepromUnitCal.Cal_RefValue_V2 = 0;
//    EepromUnitCal.Cal_ActValue_V2 = 0;
//    EepromUnitCal.Cal_RefValue_V3 = 0;
//    EepromUnitCal.Cal_ActValue_V3 = 0;
//    EepromUnitCal.Cal_RefValue_V4 = 0;
//    EepromUnitCal.Cal_ActValue_V4 = 0;
//    EepromUnitCal.Cal_RefValue_V5 = 0;
//    EepromUnitCal.Cal_ActValue_V5 = 0;
//    EepromUnitCal.Cal_RefValue_V6 = 0;
//    EepromUnitCal.Cal_ActValue_V6 = 0;
//    EepromUnitCal.Cal_RefValue_V7 = 0;
//    EepromUnitCal.Cal_ActValue_V7 = 0;
//    EepromUnitCal.Cal_RefValue_V8 = 0;
//    EepromUnitCal.Cal_ActValue_V8 = 0;
//    EepromUnitCal.Cal_I_Num_Points = 0;
//    EepromUnitCal.Cal_V_Num_Points = 0;
//    EepromUnitCal.u16_Recv1 = 0;
//    EepromUnitCal.u16_Recv2 = 0;
//    EepromUnitCal.u16_Recv3 = 0;
//    EepromUnitCal.u16_Recv4 = 0;
//    EepromUnitCal.Ctrl_Ratio_Flag = 0;
//    EepromUnitCal.Ctrl_Ratio_K1 = 0;
//    EepromUnitCal.Ctrl_Ratio_K2 = 0;
//    EepromUnitCal.Ctrl_Ratio_K3 = 0;
//    EepromUnitCal.Ctrl_Ratio_K4 = 0;
//    EepromUnitCal.Ctrl_Ratio_K5 = 0;
//    EepromUnitCal.Ctrl_Ratio_K6 = 0;
//    EepromUnitCal.Ctrl_Ratio_K7 = 0;
//    EepromUnitCal.Ctrl_Ratio_K8 = 0;
//    EepromUnitCal.Ctrl_Ratio_K9 = 0;
//    EepromUnitCal.Ctrl_Ratio_K10 = 0;
//    EepromUnitCal.Ctrl_Ratio_K11 = 0;
//    EepromUnitCal.Ctrl_Ratio_K12 = 0;
//    EepromUnitCal.Ctrl_Ratio_K13 = 0;
//    EepromUnitCal.Ctrl_Ratio_K14 = 0;
//    EepromUnitCal.Ctrl_Ratio_K15 = 0;
//    EepromUnitCal.Ctrl_Ratio_K16 = 0;
//    EepromUnitCal.Ctrl_Ratio_K17 = 0;
//    EepromUnitCal.Ctrl_Ratio_K18 = 0;
//    EepromUnitCal.Ctrl_Ratio_K19 = 0;
//    EepromUnitCal.Ctrl_Ratio_K20 = 0;
//    EepromUnitCal.u32_Recv1 = 0;
//    EepromUnitCal.u32_Recv2 = 0;
//    EepromUnitCal.u32_Recv3 = 0;
//    EepromUnitCal.u32_Recv4 = 0;
//    EepromUnitCal.EEPROM_CRC = 0;
//
////    UserReadWriteFRAM(EEPROM_CMD_READ, 0x00, (uint16_t*)&EepromUnitCal, sizeof(EepromUnitCal));
//    ReadAllFRAM(0x00, (uint16_t*)&EepromUnitCal, sizeof(EepromUnitCal));
}

void sEEpromTask(void)
{
    uint32_t event;

    event = suwRTOSGetEvent(cPrioEEprom);     //Get the current task event

    if(event & ((uint16_t)1 << eEEpromTimerEvt))
    {
        if(cQBufNormal == sQDataOut(&EepromCmdeQueue,&EepromCmd.all[0]))
        {
            if(EepromCmd.bit.Cmd == EEPROM_CMD_READ)
            {
                FRAMRead(EepromCmd.bit.EepromAddr,EepromCmd.bit.pram,EepromCmd.bit.length);
            }
            else if(EepromCmd.bit.Cmd == EEPROM_CMD_WRITE)
            {
                FRAMWrite(EepromCmd.bit.EepromAddr,EepromCmd.bit.pram,EepromCmd.bit.length);
            }
        }
    }
}





