
#include "F28x_Project.h"
#include "SpiEeprom_User.h"
#include "f28003x_spi.h"
#include "App.h"
#include "UartDebug.h"
#include "ShareData.h"
#include "DataProcTask.h"


QUEUE EepromCmdeQueue;
typeEepromCmd EepromCmdSend;

typeUnitEEPROM stEepromUnitData = {0};

extern const unsigned int tblCRC[];


/*
 * FM25V05: 512Kbit = 64K * 8bit,the user addresses 64K locations of 8 data bits each.
 * operation principle:
 * 1)时钟运行频率最高40MHZ, write operations at bus speed. No write delays are incurred.
 *   Data is written to the memory array immediately after it has been transferred to the device.
 *   The next bus cycle may commence without the need for data polling.
 * 2)支持SPI Mode0(CPOL=0,CPHA=0)和 Mode3(CPOL=1,CPHA=1)
 * 3)Manufacturer and Product ID
 *   Manufacturer ID有6个字节, 前面5个字节(高位字节)为0x7F,第6个字节为0xC2;
 *   Deivce ID有2个字节，第1个Byte为0x13, 第2个Bye为0x00;
 *
 * 4)Status Register
 *    Bit      7    6    5   4    3     2    1     0
 *    Name   WPEN   1    0   0   BP1   BP0   WEL   0
 *
 *         BP1      BP0         Protected Address Range
 *          0        0            None
 *          0        1            C000h to FFFFh
 *          1        0            8000h to FFFFh
 *          1        1            0000h to FFFFh(all)
 * 5)写(0x02):
 *   写操作时，无论是写数据进内存还是写状态寄存器，都需要开启写使能（0x06）
 *   写数据时, 先发送写地址区域的首地址，为2个字节的地址值，接下来发送的就是要写的数据，可以一直发送数据，地址
 *   值会自动增加。直到地址增加到FFFFh,随后会自动从0开始，
 *    ---transfer most significant bit (MSB) first => 对于16位寄存器，8位字节的操作赋值给高8位；
 * 6)读(0x03):
 *   当主机发送一个读内存的命令(0x06)之后，接下来发送两个Byte的读起始地址(A15~A0),芯片就会根据读的起始地址，然后只要产生时钟信号,
 *   就会一直往下读，地址会自动增加。理论上如果产生足够多的时钟信号，一次可以把整个芯片所有的内容读完
 *    ---Data is read MSB first. =>对于16位寄存器，8位字节的读取，从寄存器的低8位读取
 * 7)Fast Read(0x0B):
 *   为了兼容Serial Flash而定义的指令，在发送完地址之后，还需要发送一个字节的dummy byte
 */

enum BYTE_WRITE_READ_STATUS
{
    Initial_Status = 0,
    Waiting_SendBuffer_Empty_Status = 1,
    SendData_Status
};

/*
 *   一、函数功能说明: 因为SPI是全双工，在发送一个字节的同时，也会接收一个字节；
 *                因为SPI使用查询方式, 所以使用了FIFO，查询的也是FIFO的Buffer
 *   二、输入参数说明: TxData是要发送的字节, *pRxData是接收字节的存放地址
 *   三、输出(被调用)说明: 有两个返回内容，一个是读取的字节，通过指针返回；
 *                    另一个是状态机的状态索引，是函数的返回值；
 *                    注意：如果超时，需要额外的操作来复位状态索引
 */
static uint16_t ReadWriteByte_Status = Waiting_SendBuffer_Empty_Status;
uint16_t SPIx_ReadWriteByte(uint8_t TxData, uint8_t *pRxData)
{
    uint16_t bRet = Initial_State;

    switch(ReadWriteByte_Status)
    {
    case Waiting_SendBuffer_Empty_Status:
        /*SPISTS : SPISTS contains interrupt and status bits
         * Bit15~8   Reserved
         * Bit7      OVERRUN_FLAG  : SPI Receiver Overrun Flag
         *                           ----------------------------------------------------------------------------------------------------------------
         *                           This bit is a read/clear-only flag. The SPI hardware sets this bit when a receive or transmit operation completes
         *                           before the previous character has been read from the buffer. The bit is cleared in one of three ways:
         *                           - Writing a 1 to this bit
         *                           - Writing a 0 to SPI SW RESET (SPICCR.7)
         *                           - Resetting the system
         *                           if the OVERRUN INT ENA bit (SPICTL.4) is set, the SPI requests only one interrupt upon the first occurrence of
         *                           setting the RECEIVER OVERRUN Flag bit. Subsequent overruns will not request additional interrupts
         *                           if this flag bit is already set. This means that in order to allow new overrun interrupt requests the user
         *                           must clear this flag bit by writing a 1 to SPISTS.7 each time an overrun condition occurs.
         *                           In other words, if the RECEIVER OVERRUN Flag bit is left set (not cleared) by the interrupt service routine,
         *                           another overrun interrupt will not be immediately re-entered when the interrupt service routine is exited.
         *                           ------------------------------------------------------------------------------------------------------------------
         *                0h (R/W) = A receive overrun condition has not occurred. Software forcing is disabled and has no effect
         *                1h (R/W) = The last received character has been overwritten and therefore lost (when the SPIRXBUF was overwritten by the
         *                           SPI module before the previous character was read by the user application).
         *                           Writing a '1' will clear this bit. The RECEIVER OVERRUN Flag bit should be cleared
         *                           during the interrupt service routine because the RECEIVER OVERRUN Flag bit and SPI INT FLAG bit (SPISTS.6)
         *                           share the same interrupt vector. This will alleviate any possible doubt as to the source of the interrupt
         *                           when the next byte is received.
         * Bit6           INT_FLAG : SPI Interrupt Flag
         *                           ----------------------------------------------------------------------------------------------------------------
         *                           SPI INT FLAG is a read-only flag. Hardware sets this bit to indicate that the SPI has completed sending or receiving
         *                           the last bit and is ready to be serviced. This flag causes an interrupt to be requested
         *                           if the SPI INT ENA bit (SPICTL.0) is set. The received character is placed in the receiver buffer
         *                           at the same time this bit is set. This bit is cleared in one of three ways:
         *                           - Reading SPIRXBUF
         *                           - Writing a 0 to SPI SW RESET (SPICCR.7)
         *                           - Resetting the system
         *                              Note: This bit should not be used if FIFO mode is enabled. The internal process of copying the received word
         *                           from SPIRXBUF to the Receive FIFO will clear this bit.
         *                           Use the FIFO status, or FIFO interrupt bits for similar functionality
         *                           ------------------------------------------------------------------------------------------------------------------
         *                0h (R/W) = No full words have been received or transmitted.
         *                1h (R/W) = Indicates that the SPI has completed sending or receiving the last bit and is ready to be serviced.
         * Bit5    BUFFULL_FLAG    : SPI Transmit Buffer Full Flag
         *  *                        ----------------------------------------------------------------------------------------------------------------
         *                           This read-only bit gets set to 1 when a character is written to the SPI Transmit buffer SPITXBUF.
         *                           It is cleared when the character is automatically loaded into SPIDAT when the shifting out of a previous
         *                           character is complete
         *                           ------------------------------------------------------------------------------------------------------------------
         *                0h (R/W) = Transmit buffer is not full.
         *                1h (R/W) = Transmit buffer is full
         * Bit4~0  Reserved
         */
        if(SPI_REGS_EEPROM.SPISTS.bit.BUFFULL_FLAG == 0)   //waiting until send buffer is empty
        {
            ReadWriteByte_Status = SendData_Status;
            SPI_REGS_EEPROM.SPITXBUF = ((uint16_t)TxData & 0xFF) << 8;
            bRet = OnGoing_State;
        }
        else
        {
            bRet = OnGoing_State;
        }
        break;
    case SendData_Status:
        /*
         * SPIFFRX：SPIFFRX contains both control and status bits related to the input FIFO buffer
         *     Bit15       RXFFOVF： Receive FIFO Overflow Flag
         *                   0h (R/W) = Receive FIFO has not overflowed. This is a read-only bit.
         *                   1h (R/W) = Receive FIFO has overflowed, read-only bit.
         *                            More than 16 words have been received in to the FIFO, and the first received word is lost.
         *     Bit14       RXFFOVFCLR： Receive FIFO Overflow Clear
         *                              Write 1 to clear SPIFFRX[RXFFOVF].
         *     Bit13       RXFIFORESET: Receive FIFO Reset
         *                   0h (R/W) = Write 0 to reset the FIFO pointer to zero, and hold in reset.
         *                   1h (R/W) = Re-enable receive FIFO operation.
         *     Bit12~8     RXFFST : Receive FIFO Status
         *                   0h (R/W) = Receive FIFO is empty.
         *                   1h (R/W) = Receive FIFO has 1 word.
         *                   2h (R/W) = Receive FIFO has 2 words.
         *                   10h (R/W) = Receive FIFO has 16 words, which is the maximum.
         *                   1Fh (R/W) = Reserved
         *     Bit7        RXFFINT : Receive FIFO Interrupt Flag
         *                   0h (R/W) = RXFIFO interrupt has not occurred. This is a read-only bit.
         *                   1h (R/W) = RXFIFO interrupt has occurred. This is a read-only bit.
         *     Bit6        RXFFINTCLR :  Receive FIFO Interrupt Clear
         *                   0h (R/W) = Write 0 has no effect on RXFIFINT flag bit, Bit reads back a zero.
         *                   1h (R/W) = Write 1 to clear SPIFFRX[RXFFINT] flag
         *     Bit5        RXFFIENA :  RX FIFO Interrupt Enable
         *                   0h (R/W) = RX FIFO interrupt based on RXFFIL match (greater than or equal to) will be disabled
         *                   1h (R/W) = RX FIFO interrupt based on RXFFIL match (greater than or equal to) will be enabled.
         *     Bit4~0      RXFFIL :  Receive FIFO Interrupt Level Bits
         *                           Receive FIFO generates an interrupt when the FIFO status bits (RXFFST4-0) are
         *                           greater than or equal to the FIFO level bits (RXFFIL4-0). The default value of these bits after reset is 11111.
         *                           This avoids frequent interrupts after reset, as the receive FIFO will be empty most of the time.
         *                    0h (R/W) = A RX FIFO interrupt request is generated when there is 0 or more words in the RX buffer.
         *                    1h (R/W) = A RX FIFO interrupt request is generated when there are 1 or more words in the RX buffer.
         *                    2h (R/W) = A RX FIFO interrupt request is generated when there are 2 or more words in the RX buffer.
         *                    10h (R/W) = A RX FIFO interrupt request is generated when there are 16 words in the RX buffer.
         *                    1Fh (R/W) = Reserved
         */
        if(SPI_REGS_EEPROM.SPIFFRX.bit.RXFFST >= 1)
        {
            ReadWriteByte_Status = Waiting_SendBuffer_Empty_Status;
            *pRxData = SPI_REGS_EEPROM.SPIRXBUF & 0xFF;
            bRet = Completed_State;
        }
        else
        {
            bRet = Failed_State;
        }
    break;
    default:
        ReadWriteByte_Status = Waiting_SendBuffer_Empty_Status;
        bRet = Initial_State;
        break;
    }

    return bRet;
}


void SPIx_ReadWriteByte_Reset()
{
    ReadWriteByte_Status = Waiting_SendBuffer_Empty_Status;
    SPI_REGS_EEPROM.SPICCR.bit.SPISWRESET = 0;
    SPI_REGS_EEPROM.SPICCR.bit.SPISWRESET = 1;

}
/*****************************************Function definition***************************************/
enum FLASH_OPERATION_STEP
{
    Step0 = 0,
    Step1 = 1,
    Step2 = 2,
    Step3,
    Step4,
    Step5,
    Step6,
    Step7,
    Step8,
    Step9,
    Step10,
    Step11,
    Error_State
};


/* time out flag for flash operation */
uint16_t FlashOperation_TimeOutCounter = 0;
/*
 * 在每次调用EEPROM操作函数之前，先置位FlashOpertion_TimerBegin_TrigFlag，
 * 然后在定时中断中，如果FlashOpertion_TimerBegin_TrigFlag为1，则计时FlashOperation_TimeOutCounter；
 */
static uint16_t FlashOperation_TimerEnable = 0;
uint16_t FlashOpertion_TimerBegin_TrigFlag = 0;


//static   uint16_t Flash_TopLayerState = Step0;

eeprom_err_callback eeprom_err_callback_f = 0;

uint16_t SPI_Flash_Busy_Inquiry(void); //查询状态寄存器

// Set eeprom error callback function
void eeprom_set_err_callback(eeprom_err_callback __function)
{
    eeprom_err_callback_f = __function;
}

/*
 * 一、函数功能说明: 使用EEPROM的场合，有两处：
 *    1)在main_Loop之前，此时希望是阻塞模式---只有运行完当前函数功能，才能执行下一个函数;
 *           因为在MainLoop之前，不希望使用定时器（定时器也是查询方式），所以屏蔽状态机的超时功能
 *    2)在main_Loop之间，此时希望是非阻塞模式 ---每次只运行一次，不存在CPU等待的问题；
 *           因为使用定时器来检查状态机是否超时，所以需要使能定时器函数中相应的变量计时功能
 * 二、输入参数说明: TxData是要发送的字节, *pRxData是接收字节的存放地址
 * 三、输出(被调用)说明: 有两个返回内容，一个是读取的字节，通过指针返回；
 *                    另一个是状态机的状态索引，是函数的返回值；
 *                    注意：如果超时，需要额外的操作来复位状态索引
 */
void eeprom_select_mode(POLL_MODE mode)
{
    if(mode == non_blocking_mode)
    {
        FlashOperation_TimerEnable = 1;
    }
    else
    {
        FlashOperation_TimerEnable = 0;
    }
}

/*
 * 一、函数功能说明: 读取IC的ID，主要用于调试，无实际作用
 * 二、输入参数说明:
 * 三、输出(被调用)说明:
 */
uint16_t ReadID_State = Step0;
uint16_t SPI_ReadID(uint8_t  *pBuffer)
{
    static uint16_t u16Index;
    uint16_t u16Temp = 0;
    uint8_t u8RecData;
    uint16_t bRet = Initial_State;

    switch(ReadID_State)
    {
    case Step0:
        FM25V05_CS_SEL();                              //使能器件
        if(FlashOperation_TimerEnable == 1)
        {
            FlashOpertion_TimerBegin_TrigFlag = TRUE;
        }
        else
        {
            FlashOpertion_TimerBegin_TrigFlag = FALSE;
        }
        FlashOperation_TimeOutCounter = 0;
        ReadID_State = Step1;
        u16Index = 0;

        bRet = OnGoing_State;
        break;

    case Step1:
        u16Temp = SPIx_ReadWriteByte(RDID,&u8RecData);//发送读取ID命令
        if(u16Temp == Completed_State)
        {
            FlashOperation_TimeOutCounter = 0;
            ReadID_State = Step2;
        }
        else
        {
            if (FlashOperation_TimeOutCounter > 100)  //超时100ms
            {
                ReadID_State = Error_State;
            }
        }
        bRet = OnGoing_State;
        break;

    case Step2:
        u16Temp = SPIx_ReadWriteByte(DUMMY_BYTE,&u8RecData);//读取ID数据
        if(u16Temp == Completed_State)
        {
            pBuffer[u16Index] = u8RecData;
            FlashOperation_TimeOutCounter = 0;
            ReadID_State = Step3;
        }
        else
        {
            if (FlashOperation_TimeOutCounter > 100)  //超时100ms
            {
                ReadID_State = Error_State;
            }
        }
        bRet = OnGoing_State;
        break;

    case Step3:
        u16Index ++;
        if(u16Index < 9) //ID共有8个Byte
        {
            ReadID_State = Step2;
            bRet = OnGoing_State;
        }
        else
        {
            FlashOperation_TimeOutCounter = 0;
            FlashOpertion_TimerBegin_TrigFlag = FALSE;
            bRet = Completed_State;
            ReadID_State = Step0;
            FM25V05_CS_UNSEL();                            //取消片选
        }
        break;
    case Error_State:
    default: //EEPROM 操作故障
        FlashOperation_TimeOutCounter = 0;
        FlashOpertion_TimerBegin_TrigFlag = FALSE;
        eeprom_err_callback_f();
        bRet = Failed_State;
        ReadID_State = Step0;
        FM25V05_CS_UNSEL();
        break;
    }

    return bRet;
}

/*
 * 一、函数功能说明: 主要用于调试
 * 二、输入参数说明:
 * 三、输出(被调用)说明:
 */
uint8_t  u8EepromID[8];
void test_ReadID(void)
{
    while(SPI_ReadID(u8EepromID) != Completed_State)
    {

    }
}

/*
 * 一、函数功能说明: 在写数据之前，有的设计会先读取EEPROM的状态寄存器，看看是否存在“写操作锁死---不让写”
 * 二、输入参数说明: 无
 * 三、输出(被调用)说明:
 *      1)指针指向的地址存放返回的状态寄存器值
 *      2)状态机的状态索引
 */
static uint16_t ReadSR_State = Step0;
uint16_t FM25_ReadSR(uint8_t *pStatus)
{
    uint8_t u8RecData;
    uint16_t u16Temp;
    uint16_t bRet = Initial_State;

    switch(ReadSR_State)
    {
    case Step0:
        FM25V05_CS_SEL();                              //使能器件
        ReadSR_State = Step1;
        bRet = OnGoing_State;
        break;

    case Step1:
        u16Temp = SPIx_ReadWriteByte(RDSR,&u8RecData);//发送读取ID命令
        if(u16Temp == Completed_State)
        {
            ReadSR_State = Step2;
        }
        bRet = OnGoing_State;
        break;

    case Step2:
        u16Temp = SPIx_ReadWriteByte(DUMMY_BYTE,&u8RecData);//读取ID数据
        if(u16Temp == Completed_State)
        {
            *pStatus = u8RecData;
            bRet = Completed_State;
            ReadSR_State = Step0;
            FM25V05_CS_UNSEL();                            //取消片选
        }
        else
        {
            bRet = OnGoing_State;
        }

        break;

    case Error_State:
    default: //EEPROM 操作故障
        bRet = Failed_State;
        ReadSR_State = Step0;
        FM25V05_CS_UNSEL();
        break;
    }

    return bRet;
}

/*
 * 一、函数功能说明: 在读取状态寄存器的状态机中，如果发生异常，复位状态机索引及SPI外设
 * 二、输入参数说明: 无
 * 三、输出(被调用)说明: 无
 */
void FM25_ReadSR_Reset()
{
    ReadSR_State = Step0;
    SPI_REGS_EEPROM.SPICCR.bit.SPISWRESET = 0;
    SPI_REGS_EEPROM.SPICCR.bit.SPISWRESET = 1;
}

/*
 * 一、函数功能说明: 在写操作之前，必须先往EEPROM中写WREN
 * 二、输入参数说明: 无
 * 三、输出(被调用)说明:
 *      1)状态机的状态索引
 */
static uint16_t WREN_State = Step0;
uint16_t FM25_WriteEnable(void)
{
    uint16_t u16Temp;
    uint8_t  u8RecData;
    uint16_t bRet = Initial_State;

    switch(WREN_State)
    {
    case Step0:
        FM25V05_CS_SEL();                              //使能器件
        WREN_State = Step1;
        bRet = OnGoing_State;
        break;

    case Step1:
        u16Temp = SPIx_ReadWriteByte(WREN,&u8RecData);//Send "Write Enable" Command
        if(u16Temp == Completed_State)
        {
            bRet = Completed_State;
            WREN_State = Step0;
            FM25V05_CS_UNSEL();                            //取消片选
        }
        else
        {
            bRet = OnGoing_State;
        }
        break;

    case Error_State:
    default: //EEPROM 操作故障
        bRet = Failed_State;
        WREN_State = Step0;
        FM25V05_CS_UNSEL();
        break;
    }

    return bRet;
}

/*
 * 一、函数功能说明:如果写WREN的状态机发生异常，用于复位状态机索引及SPI外设
 * 二、输入参数说明: 无
 * 三、输出(被调用)说明:无
 */
void FM25_WriteEnable_Reset()
{
    WREN_State = Step0;
    SPI_REGS_EEPROM.SPICCR.bit.SPISWRESET = 0;
    SPI_REGS_EEPROM.SPICCR.bit.SPISWRESET = 1;
}

/*
 * 一、函数功能说明: 在写完操作之后，必须先往EEPROM中写WRDI
 * 二、输入参数说明: 无
 * 三、输出(被调用)说明:
 *      1)状态机的状态索引
 */
static uint16_t WRDISEN_State = Step0;
uint16_t FM25_WriteDisable(void)
{
    uint16_t u16Temp;
    uint8_t  u8RecData;
    uint16_t bRet = Initial_State;

    switch(WRDISEN_State)
    {
    case Step0:
        FM25V05_CS_SEL();                              //使能器件
        WRDISEN_State = Step1;
        bRet = OnGoing_State;
        break;

    case Step1:
        u16Temp = SPIx_ReadWriteByte(WRDI,&u8RecData);//发送读取ID命令
        if(u16Temp == Completed_State)
        {
            bRet = Completed_State;
            WRDISEN_State = Step0;
            FM25V05_CS_UNSEL();                            //取消片选
        }
        else
        {
            bRet = OnGoing_State;
        }
        break;

    case Error_State:
    default: //EEPROM 操作故障
        bRet = Failed_State;
        WRDISEN_State = Step0;
        FM25V05_CS_UNSEL();
        break;
    }

    return bRet;
}

/*
 * 一、函数功能说明:如果写WRDI的状态机发生异常，用于复位状态机索引及SPI外设
 * 二、输入参数说明: 无
 * 三、输出(被调用)说明:无
 */
void FM25_WriteDisable_Reset()
{
    WRDISEN_State = Step0;
    SPI_REGS_EEPROM.SPICCR.bit.SPISWRESET = 0;
    SPI_REGS_EEPROM.SPICCR.bit.SPISWRESET = 1;
}


/*
 * 一、函数功能说明: 写多个字节的数据
 * 二、输入参数说明: 写入数据的数组指针，数组长度，数据写入的首地址
 * 三、输出(被调用)说明:
 *      1)状态机的状态索引
 */
uint16_t WriteMultByte_State = Step0;
uint16_t FM25_WriteMultipleBytes(uint8_t  *pBuffer,uint16_t bufferSize, uint16_t addr)
{
    static uint16_t u16Index;
    uint16_t u16Temp = 0;
    uint8_t u8RecData;
    uint16_t bRet = Initial_State;

    switch(WriteMultByte_State)
    {
    case Step0:
        if(FlashOperation_TimerEnable == 1)
        {
            FlashOpertion_TimerBegin_TrigFlag = TRUE;
        }
        else
        {
            FlashOpertion_TimerBegin_TrigFlag = FALSE;
        }
        FlashOperation_TimeOutCounter = 0;
        u16Index = 0;

        WriteMultByte_State = Step1;
        bRet = OnGoing_State;
        break;

    case Step1:
        u16Temp = FM25_WriteEnable();
        if(u16Temp == Completed_State)
        {
            FlashOperation_TimeOutCounter = 0;
            FM25V05_CS_SEL();                              //使能器件
            WriteMultByte_State = Step2;
        }
        else
        {
            if (FlashOperation_TimeOutCounter > 100)  //超时100ms
            {
                FM25_WriteEnable_Reset();
                WriteMultByte_State = Error_State;
            }
        }
        bRet = OnGoing_State;
        break;

    case Step2:
        u16Temp = SPIx_ReadWriteByte(WRITE, &u8RecData);
        if(u16Temp == Completed_State)
        {
            FlashOperation_TimeOutCounter = 0;
            WriteMultByte_State = Step3;
        }
        else
        {
            if (FlashOperation_TimeOutCounter > 100)  //超时100ms
            {
                SPIx_ReadWriteByte_Reset();
                WriteMultByte_State = Error_State;
            }
        }

        bRet = OnGoing_State;
        break;

    case Step3:
        u16Temp = SPIx_ReadWriteByte((uint8_t)(addr>>8),&u8RecData);//发送高8位地址
        if(u16Temp == Completed_State)
        {
            FlashOperation_TimeOutCounter = 0;
            WriteMultByte_State = Step4;
        }
        else
        {
            if (FlashOperation_TimeOutCounter > 100)  //超时100ms
            {
                SPIx_ReadWriteByte_Reset();
                WriteMultByte_State = Error_State;
            }
        }
        bRet = OnGoing_State;
        break;

    case Step4:
        u16Temp = SPIx_ReadWriteByte((uint8_t)addr,&u8RecData);//发送低8位地址
        if(u16Temp == Completed_State)
        {
            FlashOperation_TimeOutCounter = 0;
            WriteMultByte_State = Step5;
        }
        else
        {
            if (FlashOperation_TimeOutCounter >100)  //超时100ms
            {
                SPIx_ReadWriteByte_Reset();
                WriteMultByte_State = Error_State;
            }
        }
        bRet = OnGoing_State;
        break;

    case Step5:
        u16Temp = SPIx_ReadWriteByte(*(pBuffer+u16Index),&u8RecData);//写数据
        if(u16Temp == Completed_State)
        {
            FlashOperation_TimeOutCounter = 0;
            WriteMultByte_State = Step6;
        }
        else
        {
            if (FlashOperation_TimeOutCounter > 100)  //超时100ms
            {
                SPIx_ReadWriteByte_Reset();
                WriteMultByte_State = Error_State;
            }
        }
        bRet = OnGoing_State;
        break;

    case Step6:
        u16Index ++;
        if(u16Index < bufferSize) //ID共有8个Byte
        {
            WriteMultByte_State = Step5;
        }
        else
        {
            FM25V05_CS_UNSEL();                            //取消片选
            WriteMultByte_State = Step7;
        }
        bRet = OnGoing_State;
        break;

    case Step7:
        u16Temp = FM25_WriteDisable();
        if(u16Temp == Completed_State)
        {
            FlashOperation_TimeOutCounter = 0;
            FlashOpertion_TimerBegin_TrigFlag = FALSE;
            bRet = Completed_State;
            WriteMultByte_State = Step0;
        }
        else
        {
            if (FlashOperation_TimeOutCounter > 100)  //超时100ms
            {
                SPIx_ReadWriteByte_Reset();
                WriteMultByte_State = Error_State;
            }
        }
        break;

    case Error_State:
    default: //EEPROM 操作故障
        FlashOperation_TimeOutCounter = 0;
        FlashOpertion_TimerBegin_TrigFlag = FALSE;
        eeprom_err_callback_f();
        bRet = Failed_State;
        WriteMultByte_State = Step0;
        FM25V05_CS_UNSEL();
        break;
    }

    return bRet;
}


/*
 * 一、函数功能说明: 读多个字节的数据
 * 二、输入参数说明: 读取数据后存放的数组名，数据长度，（要读取的）数据所在的首地址
 * 三、输出(被调用)说明:
 *      1)状态机的状态索引
 */
uint16_t ReadMultByte_State = Step0;
uint16_t FM25_ReadMultipleBytes(uint8_t  *pBuffer,uint16_t bufferSize, uint16_t addr)
{
    static uint16_t u16Index;
    uint16_t u16Temp = 0;
    uint8_t u8RecData;
    uint16_t bRet = Initial_State;

    switch(ReadMultByte_State)
    {
    case Step0:
        if(FlashOperation_TimerEnable == 1)
        {
            FlashOpertion_TimerBegin_TrigFlag = TRUE;
        }
        else
        {
            FlashOpertion_TimerBegin_TrigFlag = FALSE;
        }
        FlashOperation_TimeOutCounter = 0;
        u16Index = 0;
        FM25V05_CS_SEL();                              //使能器件
        ReadMultByte_State = Step1;
        bRet = OnGoing_State;
        break;

    case Step1:
        u16Temp = SPIx_ReadWriteByte(READ, &u8RecData);
        if(u16Temp == Completed_State)
        {
            FlashOperation_TimeOutCounter = 0;
            ReadMultByte_State = Step2;
        }
        else
        {
            if (FlashOperation_TimeOutCounter > 100)  //超时100ms
            {
                SPIx_ReadWriteByte_Reset();
                ReadMultByte_State = Error_State;
            }
        }

        bRet = OnGoing_State;
        break;

    case Step2:
        u16Temp = SPIx_ReadWriteByte((uint8_t)(addr>>8),&u8RecData);//发送高8位地址
        if(u16Temp == Completed_State)
        {
            FlashOperation_TimeOutCounter = 0;
            ReadMultByte_State = Step3;
        }
        else
        {
            if (FlashOperation_TimeOutCounter > 100)  //超时100ms
            {
                SPIx_ReadWriteByte_Reset();
                ReadMultByte_State = Error_State;
            }
        }
        bRet = OnGoing_State;
        break;

    case Step3:
        u16Temp = SPIx_ReadWriteByte((uint8_t)addr,&u8RecData);//发送低8位地址
        if(u16Temp == Completed_State)
        {
            FlashOperation_TimeOutCounter = 0;
            ReadMultByte_State = Step4;
        }
        else
        {
            if (FlashOperation_TimeOutCounter >100)  //超时100ms
            {
                SPIx_ReadWriteByte_Reset();
                ReadMultByte_State = Error_State;
            }
        }
        bRet = OnGoing_State;
        break;

    case Step4:
        u16Temp = SPIx_ReadWriteByte(DUMMY_BYTE,&u8RecData);//写数据
        if(u16Temp == Completed_State)
        {
            pBuffer[u16Index] = u8RecData;
            FlashOperation_TimeOutCounter = 0;
            ReadMultByte_State = Step5;
        }
        else
        {
            if (FlashOperation_TimeOutCounter > 100)  //超时100ms
            {
                SPIx_ReadWriteByte_Reset();
                ReadMultByte_State = Error_State;
            }
        }
        bRet = OnGoing_State;
        break;

    case Step5:
        u16Index ++;
        if(u16Index < bufferSize) //ID共有8个Byte
        {
            ReadMultByte_State = Step4;
            bRet = OnGoing_State;
        }
        else
        {
            FlashOperation_TimeOutCounter = 0;
            FlashOpertion_TimerBegin_TrigFlag = FALSE;
            bRet = Completed_State;
            ReadMultByte_State = Step0;
            FM25V05_CS_UNSEL();                            //取消片选
        }
        break;

    case Error_State:
    default: //EEPROM 操作故障
        FlashOperation_TimeOutCounter = 0;
        FlashOpertion_TimerBegin_TrigFlag = FALSE;
        eeprom_err_callback_f();
        bRet = Failed_State;
        ReadMultByte_State = Step0;
        FM25V05_CS_UNSEL();
        break;
    }

    return bRet;
}

/*
 * 定时运行，查询Eeprom操作是否超时
 */
void Eeprom_TimerControl()
{
    if(FlashOpertion_TimerBegin_TrigFlag == TRUE)
    {
        FlashOperation_TimeOutCounter++;
    }
    else
    {
        FlashOperation_TimeOutCounter = 0;
    }
}


uint8_t readbuffer[10] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
uint8_t writebuffer[10] = {0x11,0x33,0x55,0x77,0x99,0xBB,0xDD,0xFF,0x22,0x44};
void test_FM25_ReadWriteMultipleBytes(void)
{
    //uint16_t u16Temp;


    uint16_t bufferSize = 10;

    uint16_t addr = 0x100;

    while(FM25_WriteMultipleBytes(writebuffer,bufferSize,addr) != Completed_State)
    {

    }

    DELAY_US(100);
    while(FM25_ReadMultipleBytes(readbuffer,bufferSize,addr) != Completed_State)
    {

    }

}


/**********************************************************************
函数名:FRAMWrite
功  能: 写入数据到FRAM
输  入: FRAMAddr:写FRAM的地址
     pRAM:数据指针
     wordLength：字长度(1word = 2byte = 16bit)
返  回:  0:失败，1:成功
**********************************************************************/
uint16_t FRAMWrite(uint16_t FRAMAddr,uint16_t* pRAM,uint16_t wordLength)
{
    uint16_t ui8Temp[32] = {0};
    uint16_t i = 0, Count = wordLength;
    uint16_t RunWirteCnt = 0;

//    if((wordLength<<1) > 32){Count = 32>>1;}
    if((wordLength<<1) > 32)
        return 0;

    for(i=0;i<Count;i++)
    {
        ui8Temp[i*2] = (*(pRAM+i) & 0xFF);
        ui8Temp[i*2+1] = (*(pRAM+i) >> 8) & 0xFF;
    }

    while(FM25_WriteMultipleBytes(&ui8Temp[0],Count<<1,FRAMAddr) != Completed_State)
    {
        if(RunWirteCnt++ >= 3000)
        {
            return 0;
        }
    }

    return 1;
}

/**********************************************************************
函数名:FRAMRead
功  能: 从FRAM中读取数据
输  入: FRAMAddr:写FRAM的地址
     pRAM:数据指针
     wordLength：字长度(1word = 2byte = 16bit)
返  回:  0:失败，1:成功
**********************************************************************/
uint16_t FRAMRead(uint16_t FRAMAddr,uint16_t* pRAM,uint16_t wordLength)
{
    uint16_t ui8Temp[32] = {0};
    uint16_t i = 0, Count = wordLength;
    uint16_t RunReadCnt = 0;

//    if((wordLength<<1) > 32){Count = 32>>1;}
    if((wordLength<<1) > 32)
        return 0;

    while(FM25_ReadMultipleBytes(&ui8Temp[0],Count<<1,FRAMAddr) != Completed_State)
    {
        if(RunReadCnt++ >= 3000)
        {
            return 0;
        }
    }

    for(i=0;i<Count;i++)
    {
        *(pRAM+i) = (ui8Temp[i*2+1] << 8) + ui8Temp[i*2];
    }

    return 1;
}

/**********************************************************************
函数名:UserReadWriteFRAM
功  能: 将FRAM的信息放入任务队列中，一次任务队列最多读取或写入32byte
输  入:FRAMCmd:操作指令，读取或写入
     FRAMAddr:FRAM的地址
     pReg:数据指针
     wordLength：字长度(1word = 2byte = 16bit)
返  回:  0:失败，1:成功
**********************************************************************/
uint16_t UserReadWriteFRAM(uint16_t FRAMCmd,uint16_t FRAMAddr,uint16_t* pReg,uint16_t wordLength)
{
    if((FRAMAddr%2) != 0)
        return 0;

    uint16_t index = 0;
    uint16_t byteCount = wordLength<<1;
    uint16_t endAddr = FRAMAddr + (byteCount>>5)*32;
    uint16_t endLen = (FRAMAddr==endAddr)?byteCount:(byteCount%32);
    uint16_t addr = FRAMAddr,len = 32;

    do
    {
        if(addr == endAddr)
        {
            len = endLen;
        }
        else
        {
            len = 32;
        }

        EepromCmdSend.bit.Cmd = FRAMCmd;
        EepromCmdSend.bit.EepromAddr = addr;
        EepromCmdSend.bit.length = len>>1;
        EepromCmdSend.bit.pram = &pReg[index];
        if(cQBufFull == sQDataIn(&EepromCmdeQueue,&EepromCmdSend.all[0],cQCoverLast))
            return 0;

        index += len>>1;
        addr += len;
    }
    while(addr < FRAMAddr+byteCount);

    return 1;
}

/**********************************************************************
**********************************************************************/

uint16_t ReadAllFRAM(uint16_t FRAMAddr,uint16_t* pReg,uint16_t wordLength)
{
    if((FRAMAddr%2) != 0)
        return 0;

    uint16_t index = 0;
    uint16_t byteCount = wordLength<<1;
    uint16_t endAddr = FRAMAddr + (byteCount>>5)*32;
    uint16_t endLen = (FRAMAddr==endAddr)?byteCount:(byteCount%32);
    uint16_t addr = FRAMAddr,len = 32;

    do
    {
        if(addr == endAddr)
        {
            len = endLen;
        }
        else
        {
            len = 32;
        }

        FRAMRead(addr, &pReg[index], len>>1);

        index += len>>1;
        addr += len;
    }
    while(addr < FRAMAddr+byteCount);

    return 1;
}

void ReadAllData(void) // 上电初始化调用
{
    uint16_t u16_Read_CRC = 0;

    //从EEPROM中读取所有数据
    ReadAllFRAM(EEPROM_UNIT_START, (uint16_t*)&stEepromUnitData, sizeof(stEepromUnitData));

    //CRC校验比对，正确即写入，错误即赋值为默认值
    u16_Read_CRC = CRC16((uint16_t*)&stEepromUnitData, sizeof(stEepromUnitData)-2);
    if(u16_Read_CRC == stEepromUnitData.EEPROM_CRC)
    {
        if(stEepromUnitData.Magic == 0xAA55) // 判断是否写入过
        {
            if(stEepromUnitData.Cal_Parm_Flag == 1)
            {
                //将结构体数据赋值到RAM
                sCalCoeffCalculate(&stIoutCal,stEepromUnitData.Cal_I_Num_Points,&stEepromUnitData.Cal_RefValue_I1);
            }
            else
            {
                //赋值为默认值
                stIoutCal.CalibrateFlag = 0;
            }

            if(stEepromUnitData.Cal_Parm_Flag1 == 1)
            {
                //将结构体数据赋值到RAM
                sCalCoeffCalculate(&stVoutCal,stEepromUnitData.Cal_V_Num_Points,&stEepromUnitData.Cal_RefValue_V1);
            }
            else
            {
                //赋值为默认值
                stVoutCal.CalibrateFlag = 0;
            }

            if(stEepromUnitData.Ctrl_Ratio_Flag == 1)
            {
                //将结构体数据赋值到RAM
                sUpdateCtrlParaEeprom();
            }
            else
            {
                sResetCtrlPara();//赋值为默认值
            }
        }
        else
        {
            //赋值为默认值
        }
    }
    else
    {
        //赋值为默认值
    }
}

void WriteAllData(void)
{
    uint16_t u16_Write_CRC = 0;
    //将所有数据更新赋值到结构体中
    /****************Test*****************/
    stEepromUnitData.Magic = 0xAA55;
    stEepromUnitData.Version = 2001;

    /****************Test*****************/
//    stEepromUnitData.Magic = 0xAA55;
//    stEepromUnitData.Version = 2001;
//    stEepromUnitData.Cal_Parm_Flag = 0;
//    stEepromUnitData.Cal_RefValue_I1 = 34504;
//    stEepromUnitData.Cal_ActValue_I1 = 35505;
//    stEepromUnitData.Cal_RefValue_I2 = 36506;
//    stEepromUnitData.Cal_ActValue_I2 = 37507;
//    stEepromUnitData.Cal_RefValue_I3 = 38508;
//    stEepromUnitData.Cal_ActValue_I3 = 39509;
//    stEepromUnitData.Cal_RefValue_I4 = 40510;
//    stEepromUnitData.Cal_ActValue_I4 = 41511;
//    stEepromUnitData.Cal_RefValue_I5 = 42512;
//    stEepromUnitData.Cal_ActValue_I5 = 43513;
//    stEepromUnitData.Cal_RefValue_I6 = 44514;
//    stEepromUnitData.Cal_ActValue_I6 = 45515;
//    stEepromUnitData.Cal_RefValue_I7 = 46516;
//    stEepromUnitData.Cal_ActValue_I7 = 47517;
//    stEepromUnitData.Cal_RefValue_I8 = 48518;
//    stEepromUnitData.Cal_ActValue_I8 = 49519;
//    stEepromUnitData.Cal_Parm_Flag1 = 0;
//    stEepromUnitData.Cal_RefValue_V1 = 50520;
//    stEepromUnitData.Cal_ActValue_V1 = 51521;
//    stEepromUnitData.Cal_RefValue_V2 = 52522;
//    stEepromUnitData.Cal_ActValue_V2 = 53523;
//    stEepromUnitData.Cal_RefValue_V3 = 54524;
//    stEepromUnitData.Cal_ActValue_V3 = 55525;
//    stEepromUnitData.Cal_RefValue_V4 = 56526;
//    stEepromUnitData.Cal_ActValue_V4 = 57527;
//    stEepromUnitData.Cal_RefValue_V5 = 58528;
//    stEepromUnitData.Cal_ActValue_V5 = 59529;
//    stEepromUnitData.Cal_RefValue_V6 = 60530;
//    stEepromUnitData.Cal_ActValue_V6 = 61531;
//    stEepromUnitData.Cal_RefValue_V7 = 62532;
//    stEepromUnitData.Cal_ActValue_V7 = 63533;
//    stEepromUnitData.Cal_RefValue_V8 = 64534;
//    stEepromUnitData.Cal_ActValue_V8 = 65535;
//    stEepromUnitData.Cal_I_Num_Points = 65535;
//    stEepromUnitData.Cal_V_Num_Points = 65535;
//    stEepromUnitData.u16_Recv1 = 7788;
//    stEepromUnitData.u16_Recv2 = 5566;
//    stEepromUnitData.u16_Recv3 = 3344;
//    stEepromUnitData.u16_Recv4 = 2211;
//    stEepromUnitData.Ctrl_Ratio_Flag = 0;
//    stEepromUnitData.Ctrl_Ratio_K1 = 65536;
//    stEepromUnitData.Ctrl_Ratio_K2 = 265537;
//    stEepromUnitData.Ctrl_Ratio_K3 = 465538;
//    stEepromUnitData.Ctrl_Ratio_K4 = 665539;
//    stEepromUnitData.Ctrl_Ratio_K5 = 865540;
//    stEepromUnitData.Ctrl_Ratio_K6 = 1065541;
//    stEepromUnitData.Ctrl_Ratio_K7 = 2065542;
//    stEepromUnitData.Ctrl_Ratio_K8 = 4065543;
//    stEepromUnitData.Ctrl_Ratio_K9 = 6065544;
//    stEepromUnitData.Ctrl_Ratio_K10 = 8065545;
//    stEepromUnitData.Ctrl_Ratio_K11 = 0;//10065546;
//    stEepromUnitData.Ctrl_Ratio_K12 = 0;//20065547;
//    stEepromUnitData.Ctrl_Ratio_K13 = 0;//40065548;
//    stEepromUnitData.Ctrl_Ratio_K14 = 0;//60065549;
//    stEepromUnitData.Ctrl_Ratio_K15 = 0;//80065550;
//    stEepromUnitData.Ctrl_Ratio_K16 = 0;//100065551;
//    stEepromUnitData.Ctrl_Ratio_K17 = 0;//200065552;
//    stEepromUnitData.Ctrl_Ratio_K18 = 0;//400065553;
//    stEepromUnitData.Ctrl_Ratio_K19 = 0;//600065554;
//    stEepromUnitData.Ctrl_Ratio_K20 = 0;//800065555;
//    stEepromUnitData.u32_Recv1 = 0;//12345678;
//    stEepromUnitData.u32_Recv2 = 0;//21345678;
//    stEepromUnitData.u32_Recv3 = 0;//32145678;
//    stEepromUnitData.u32_Recv4 = 0;//43215678;


    //计算CRC，并赋值到写入数据中
    u16_Write_CRC = CRC16((uint16_t*)&stEepromUnitData, sizeof(stEepromUnitData)-2);
    stEepromUnitData.EEPROM_CRC = u16_Write_CRC;

    //将所有数据按任务写入EEPROM
    UserReadWriteFRAM(EEPROM_CMD_WRITE, EEPROM_UNIT_START, (uint16_t*)&stEepromUnitData, sizeof(stEepromUnitData));
}


