
#ifndef _SPIEEPROM_USER_H
#define _SPIEEPROM_USER_H

#include "f28003x_device.h"      // Headerfile Include "f28003x_spi.h" File
#include "App.h"
#include "UartDebug.h"


enum FUNCTION_STATUS
{
    Initial_State = 0,
    OnGoing_State = 1,
    Completed_State,
    Failed_State
};

typedef enum _POLL_MODE
{
    non_blocking_mode = 0,
    blocking_mode = 1
}POLL_MODE;

#define BUSY       1
#define FREETIME   0

//************************macro definition*********************************//
#define FM25V05_CS_SEL()      (GpioDataRegs.GPACLEAR.bit.GPIO27 = 1)
#define FM25V05_CS_UNSEL()    (GpioDataRegs.GPASET.bit.GPIO27 = 1)

//************************FM25V05 read/write *********************************//
//instruction table
#define WREN                    0x06  //Set Write Enable Latch
#define WRDI                    0x04  //Write Disable
#define RDSR                    0x05  //Read Status Register
#define WRSR                    0x01  //Write Status Register
#define READ                    0x03  //Read Memory Data
#define FSTRD                   0x0B  //Fast Read Memory Data
#define WRITE                   0x02  //Write Memory Data
#define SLEEP                   0xB9  //Enter Sleep Mode
#define RDID                    0x9F  //Read Device ID
#define SNR                     0xC3  //Read S/N

#define DUMMY_BYTE              0x00


//parameter start address
#define EEPROM_UNIT_START     0x00


#define EEPROM_CMD_READ     0
#define EEPROM_CMD_WRITE    1


typedef struct struct_eecmd {
    uint16_t  Cmd;
    uint16_t  EepromAddr;
    uint16_t* pram;
    uint16_t  length;
    uint16_t  reversed;
}stEepromCmd;

typedef union {
    uint16_t all[6];
    stEepromCmd bit;
}typeEepromCmd;


/********************************************************************************
**********************************************************************************/
typedef struct {
    uint16_t Magic;         //Determine if it has been written. 0xAA55 - Yes, 0xFFFF - No
    uint16_t Version;       //Parameter version

    uint32_t Cal_Parm_Flag;     //The flag indicating whether to use parameters or not
    uint32_t Cal_RefValue_I1;
    uint32_t Cal_ActValue_I1;
    uint32_t Cal_RefValue_I2;
    uint32_t Cal_ActValue_I2;
    uint32_t Cal_RefValue_I3;
    uint32_t Cal_ActValue_I3;
    uint32_t Cal_RefValue_I4;
    uint32_t Cal_ActValue_I4;
    uint32_t Cal_Parm_Flag1;
    uint32_t Cal_RefValue_V1;
    uint32_t Cal_ActValue_V1;
    uint32_t Cal_RefValue_V2;
    uint32_t Cal_ActValue_V2;
    uint32_t Cal_RefValue_V3;
    uint32_t Cal_ActValue_V3;
    uint32_t Cal_RefValue_V4;
    uint32_t Cal_ActValue_V4;
    uint16_t Cal_I_Num_Points;
    uint16_t Cal_V_Num_Points;
    uint16_t u16_Recv1;
    uint16_t u16_Recv2;
    uint16_t u16_Recv3;
    uint16_t u16_Recv4;

    uint32_t Ctrl_Ratio_Flag;   //The flag indicating whether to use parameters or not

    uint32_t u32_CurrLoopK1;
    uint32_t u32_CurrLoopK2;
    uint32_t u32_CurrLoopK3;
    uint32_t u32_CurrLoopK4;

    uint32_t u32_VoltLoopK1;
    uint32_t u32_VoltLoopK2;
    uint32_t u32_VoltLoopK3;
    uint32_t u32_VoltLoopK4;

    uint32_t Ctrl_Ratio_K1;
    uint32_t Ctrl_Ratio_K2;
    uint32_t Ctrl_Ratio_K3;
    uint32_t Ctrl_Ratio_K4;
    uint32_t Ctrl_Ratio_K5;
    uint32_t Ctrl_Ratio_K6;
    uint32_t Ctrl_Ratio_K7;
    uint32_t Ctrl_Ratio_K8;
    uint32_t Ctrl_Ratio_K9;
    uint32_t Ctrl_Ratio_K10;

    uint32_t EEPROM_CRC;
}typeUnitEEPROM;


extern QUEUE EepromCmdeQueue;
extern typeEepromCmd EepromCmdSend;
extern typeUnitEEPROM stEepromUnitData;


extern void eeprom_select_mode(POLL_MODE mode);
extern uint16_t SPI_ReadID(uint8_t  *pBuffer);
extern void test_ReadID(void);
extern uint16_t FM25_ReadSR(uint8_t *pStatus);
extern uint16_t FM25_WriteEnable(void);
extern uint16_t FM25_WriteDisable(void);
extern uint16_t FM25_WriteMultipleBytes(uint8_t  *pBuffer,uint16_t bufferSize, uint16_t addr);
extern uint16_t FM25_ReadMultipleBytes(uint8_t  *pBuffer,uint16_t bufferSize, uint16_t addr);
extern void test_FM25_ReadWriteMultipleBytes(void);
extern void SPIx_ReadWriteByte_Reset(void);
// Pointer to user called eeprom error function
typedef void (*eeprom_err_callback)(void);

// Set eeprom error callback function
void eeprom_set_err_callback(eeprom_err_callback __function);
extern void Eeprom_TimerControl();

extern uint16_t FRAMWrite(uint16_t FRAMAddr,uint16_t* pRAM,uint16_t wordLength);
extern uint16_t FRAMRead(uint16_t FRAMAddr,uint16_t* pRAM,uint16_t wordLength);
extern uint16_t UserReadWriteFRAM(uint16_t FRAMCmd,uint16_t FRAMAddr,uint16_t* pReg,uint16_t wordLength);
extern uint16_t ReadAllFRAM(uint16_t FRAMAddr,uint16_t* pReg,uint16_t wordLength);
extern void ReadAllData(void);
extern void WriteAllData(void);
extern unsigned int CRC16(unsigned int * puchMsg,unsigned int usDataLen);

#endif
