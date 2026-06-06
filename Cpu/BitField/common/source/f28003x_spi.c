//###########################################################################
//
// FILE:   f28003x_spi.c
//
// TITLE:  f28003x SPI Initialization & Support Functions.
//
//#############################################################################
//
//
// $Copyright:
// Copyright (C) 2024 Texas Instruments Incorporated - http://www.ti.com/
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions 
// are met:
// 
//   Redistributions of source code must retain the above copyright 
//   notice, this list of conditions and the following disclaimer.
// 
//   Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the 
//   documentation and/or other materials provided with the   
//   distribution.
// 
//   Neither the name of Texas Instruments Incorporated nor the names of
//   its contributors may be used to endorse or promote products derived
//   from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// $
//#############################################################################

//
// Included Files
//
#include "f28003x_device.h"      // Headerfile Include File
#include "f28003x_examples.h"    // Examples Include File
#include "f28003x_spi.h"

//
// Calculate BRR: 7-bit baud rate register value
// SPI CLK freq = 500 kHz
// LSPCLK freq  = CPU freq / 4  (by default)
// BRR          = (LSPCLK freq / SPI CLK freq) - 1
//
#if CPU_FRQ_120MHZ
#define SPI_BRR        ((120E6 / 4) / 500E3) - 1
#endif

#define DELAY_CYCLE_WORD 10u


// spi config & control registers
void InitSPI(void)
{
    /*SPICCR : controls the setup of the SPI for operation.
      *    Bit15~8    Reserved
      *    Bit7       SPISWRESET : SPI Software Reset
      *                 0(R/W): Initializes the SPI operating flags to the reset condition
      *                 1(R/W): SPI is ready to transmit or receive the next character.
      *    Bit6       CLKPOLARITY: This bit controls the polarity of the SPICLK signal.
      *                            CLOCK POLARITY and POLARITY CLOCK PHASE (SPICTL.3) control four
      *                            clocking schemes on the SPICLK pin.
      *                 0h (R/W) = Data is output on rising edge and input on falling edge.
      *                            When no SPI data is sent, SPICLK is at low level. The data input
      *                            and output edges depend on the value of the CLOCK PHASE bit (SPICTL.3) as follows:
      *                            - CLOCK PHASE = 0: Data is output on the rising edge of the SPICLK signal.
      *                                               Input data is latched on the falling edge of the SPICLK signal.
      *                            - CLOCK PHASE = 1: Data is output one half-cycle before the first
      *                                               rising edge of the SPICLK signal and on subsequent falling edges
      *                                               of the SPICLK signal. Input data is latched on the rising edge of
      *                                               the SPICLK signal.
      *                 1(R/W): 1h (R/W) = Data is output on falling edge and input on rising edge. When no SPI data is sent,
      *                                     SPICLK is at high level. The data input and output edges depend on the value of
      *                                     the CLOCK PHASE bit (SPICTL.3) as follows:
      *                             - CLOCK PHASE = 0: Data is output on the falling edge of the SPICLK signal.
      *                                                Input data is latched on the rising edge of the SPICLK signal.
      *                             - CLOCK PHASE = 1: Data is output one half-cycle before the first
      *                                                falling edge of the SPICLK signal and on subsequent rising edges
      *                                                of the SPICLK signal. Input data is latched on the falling edge of
      *                                                the SPICLK signal.
      *    Bit5       Reserved
      *    Bit4       SPILBK : SPI Loopback Mode Select
      *                        Loopback mode allows module validation during device testing.
      *                        This mode is valid only in master mode of the SPI.
      *                   0(R/W) : SPI loopback mode disabled
      *                   1(R/W) = SPI loopback mode enabled
      *    Bit3~0     SPICHAR : Character Length Control Bits
      *                         These four bits determine the number of bits to be shifted in or SPI
      *                         CHAR0 out as a single character during one shift sequence.
      *                         7h (R/W) = 8-bit word
     */
    SPI_REGS_EEPROM.SPICCR.bit.SPISWRESET = 0; //清除接收端溢出标志(SPISTS.7), SPI INT标志位（SPISTS.6)
    //SPI_REGS_EEPROM.SPICCR.bit.SPISWRESET = 1;  //  clear the SPI software reset bit to force the SPI to the reset state
    SPI_REGS_EEPROM.SPICCR.bit.SPICHAR = 7;      //8 data bit
    SPI_REGS_EEPROM.SPICCR.bit.CLKPOLARITY = 0; //0;  //数据在上升沿输出，在下降沿输入
                                                 //When no SPI data is sent, SPICLK is at high level
    /*
     * SPICTL : SPICTL controls data transmission, the SPI's ability to generate interrupts, the SPICLK phase, and the operational mode (slave or master)
     *     Bit15~5     Reserved
     *     Bit4        OVERRUNINTENA: Overrun Interrupt Enable
     *                   0h (R/W) = Disable RECEIVER OVERRUN interrupts.
     *                   1h (R/W) = Enable RECEIVER_OVERRUN interrupts.
     *     Bit3        CLK_PHASE: SPI Clock Phase Select
     *                   0h (R/W) = Normal SPI clocking scheme, depending on the CLOCK POLARITY bit (SPICCR.6).
     *                   1h (R/W) = SPICLK signal delayed by one half-cycle. Polarity determined by the CLOCK POLARITY bit
     *     Bit2        MASTER_SLAVE: SPI Network Mode Control
     *                   0h (R/W) = SPI is configured as a slave
     *                   1h (R/W) = SPI is configured as a master
     *     Bit1        TALK: Transmit Enable
     *                   0h (R/W) = Disables transmission:
     *                   1h (R/W) = Enables transmission For the 4-pin option, ensure to enable the receiver's SPISTEn input pin.
     *     Bit0        SPIINTENA: SPI Interrupt Enable
     *                   0h (R/W) = Disables the interrupt.
     *                   1h (R/W) = Enables the interrupt.
     */
    SPI_REGS_EEPROM.SPICTL.bit.SPIINTENA = 0;
    SPI_REGS_EEPROM.SPICTL.bit.CLK_PHASE = 1;        // 正常的SPI时钟方案，取决于时钟极性位(SPICCR.6)
    SPI_REGS_EEPROM.SPICTL.bit.MASTER_SLAVE = 1;     // 设置为主机
    SPI_REGS_EEPROM.SPICTL.bit.TALK = 1;     //开启四线模式, Enables transmission
    /*
     * SPIBRR : SPIBRR contains the bits used for baud-rate selection
     *     Bit15~7     Reserved
     *     Bit6~0      SPI_BIT_RATE: SPI Baud Rate Control
     *                    In master mode, the SPI clock is generated by the SPI and is output on the SPICLK pin.
     *                    The SPI baud rates are determined by the following formula:
     *                    For SPIBRR = 3 to 127: SPI Baud Rate = LSPCLK / (SPIBRR + 1)
     *                    For SPIBRR = 0, 1, or 2: SPI Baud Rate = LSPCLK / 4
     *                  in this project, LOSPCP.LSPCLK = SYSCLKOUT/(2*ADC_MODCLK) = 90/(127+1) = 703.125 KHz
     *                  SPI Baud Rate
     */
    SPI_REGS_EEPROM.SPIBRR.all = SPI_BRR;            //

    /*
     *SPIPRI : SPIPRI controls auxiliary functions for the SPI including emulation control, SPISTE inversion, and 3-wire control
     *  Bit15~6   Reserved
     *  Bit5      SOFT      : Emulation Soft Run
     *            0(h)      = Transmission stops midway in the bit stream while TSUSPEND is asserted.
     *                        Once TSUSPEND is deasserted without a system reset, the remainder of the bits pending in the DATBUF are shifted.
     *            1(h)      = If the emulation suspend occurs before the start of a transmission, (that is, before the first SPICLK pulse) then the
     *                        transmission will not occur. If the emulation suspend occurs after the start of a transmission,
     *                        then the data will be shifted out to completion. When the start of transmission occurs is dependent on the baud rate used.
     *  Bit4      FREE      : Emulation Free Run
     *            0h (R/W)  = Emulation mode is selected by the SOFT bit
     *            1h (R/W)  = Free run, continue SPI operation regardless of suspend or when the suspend occurred.
     *  Bit3~2    Reserved
     *  Bit1      STEINV    : SPISTEn Inversion Bit
     *            0h (R/W)  = SPISTEn is active low (normal)
     *            1h (R/W)  = SPISTE is active high (inverted)
     *  Bit0      TRIWIRE   : SPI 3-wire Mode Enable
     *             0h (R/W) = Normal 4-wire SPI mode
     *             1h (R/W) = 3-wire SPI mode enabled. The unused pin becomes a GPIO pin
     */
    SPI_REGS_EEPROM.SPIPRI.bit.FREE = 1;        // Set so breakpoints don't disturb xmission
    SPI_REGS_EEPROM.SPIPRI.bit.STEINV = 0;      //
    SPI_REGS_EEPROM.SPIPRI.bit.TRIWIRE = 0;
    DELAY_US(10);

     /*FM25V05在读/写数据时，地址会自动加1，所以可以使用FIFIO及DMA来操作

     * SPIFFTX: contains both control and status bits related to the output FIFO buffer
     *     Bit15      SPIRST: SPI Reset
     *     Bit14      SPIFFENA:  SPI FIFO Enhancements Enable
     *                    0h (R/W) = SPI FIFO enhancements are disabled
     *                    1h (R/W) = SPI FIFO enhancements are enabled
     *     Bit13      TXFIFO  :  TX FIFO Reset
     *                    0h (R/W) = Write 0 to reset the FIFO pointer to zero, and hold in reset.
     *                    1h (R/W) = Release transmit FIFO from reset
     *     Bit12~8    TXFFST  :  Transmit FIFO Status
     *                    0h (R/W) = Transmit FIFO is empty.
     *                    1h (R/W) = Transmit FIFO has 1 word.
     *                    2h (R/W) = Transmit FIFO has 2 words.
     *                    10h (R/W) = Transmit FIFO has 16 words, which is the maximum.
     *                    1Fh (R/W) = Reserved.
     *     Bit7       TXFFINT :   TX FIFO Interrupt Flag
     *                    0h (R/W) = TXFIFO interrupt has not occurred, This is a read-only bit.
     *                    1h (R/W) = TXFIFO interrupt has occurred, This is a read-only bit
     *     Bit6       TXFFINTCLR :  TXFIFO Interrupt Clear
     *                    0h (R/W) = Write 0 has no effect on TXFIFINT flag bit, Bit reads back a zero.
     *                    1h (R/W) = Write 1 to clear SPIFFTX[TXFFINT] flag
     *     Bit5       TXFFIENA : TX FIFO Interrupt Enable
     *                    0h (R/W) = TX FIFO interrupt based on TXFFIL match (less than or equal to) will be disabled.
     *                    1h (R/W) = TX FIFO interrupt based on TXFFIL match (less than or equal to) will be enabled
     *     Bit4~0     TXFFIL : Transmit FIFO Interrupt Level Bits.
     *                         Transmit FIFO will generate interrupt when the FIFO status bits (TXFFST4-0) and FIFO level bits (TXFFIL4-0 )
     *                         match (less than or equal to)
     *                    0h (R/W) = A TX FIFO interrupt request is generated when there are no words remaining in the TX buffer.
     *                    1h (R/W) = A TX FIFO interrupt request is generated when there is 1 word or no words remaining in the TX buffer.
     *                    2h (R/W) = A TX FIFO interrupt request is generated when there is 2 words or fewer remaining in the TX buffer.
     *                    10h (R/W) = A TX FIFO interrupt request is generated when there are 16 words or fewer remaining in the TX buffer.
     *                    1Fh (R/W) = Reserved
     */
    SPI_REGS_EEPROM.SPIFFTX.bit.TXFIFO = 0;
    SPI_REGS_EEPROM.SPIFFTX.bit.SPIRST = 0;
    SPI_REGS_EEPROM.SPIFFTX.bit.SPIRST = 1;
    SPI_REGS_EEPROM.SPIFFTX.bit.TXFIFO = 1;

    SPI_REGS_EEPROM.SPIFFTX.bit.SPIFFENA = 1;
    SPI_REGS_EEPROM.SPIFFTX.bit.TXFFINTCLR = 1;
    SPI_REGS_EEPROM.SPIFFTX.bit.TXFFIENA = 0;
    SPI_REGS_EEPROM.SPIFFTX.bit.TXFFIL = 0;
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
    SPI_REGS_EEPROM.SPIFFRX.bit.RXFIFORESET = 0;
    SPI_REGS_EEPROM.SPIFFRX.bit.RXFIFORESET = 1;
    SPI_REGS_EEPROM.SPIFFRX.bit.RXFFOVFCLR = 1;
    SPI_REGS_EEPROM.SPIFFRX.bit.RXFFINTCLR = 1;
    SPI_REGS_EEPROM.SPIFFRX.bit.RXFFIENA = 0;
    SPI_REGS_EEPROM.SPIFFRX.bit.RXFFIL = 0;

    /*
     * SPIFFCT : SPIFFCT controls the FIFO transmit delay bits
     *           FIFO Transmit Delay Bits These bits define the delay between every transfer from FIFO
     *           transmit buffer to transmit shift register. The delay is defined in number SPI serial clock cycles.
     *           The 8-bit register could define a minimum delay of 0 serial clock cycles and a maximum of 255 serial
     *           clock cycles. In FIFO mode, the buffer (TXBUF) between the shift register and the FIFO should be filled only
     *           after the shift register has completed shifting of the last bit. This is required to pass on the
     *           delay between transfers to the data stream. In the FIFO mode TXBUF should not be treated as one additional level of buffer.
     */
    SPI_REGS_EEPROM.SPIFFCT.all = DELAY_CYCLE_WORD; //上一个字传输完成后,TX FIFO Buffer中的下一个字在等待DELAY_CYCLE_WORD个SPI Cycle之后被传输到SPITXBUF

    SPI_REGS_EEPROM.SPICCR.bit.SPISWRESET = 1;
}


//
// InitSpibGpio - Initialize SPIB GPIOs
//
void InitSpibGpio()
{
    EALLOW;

    //
    // Enable internal pull-up for the selected pins
    //
    // Pull-ups can be enabled or disabled by the user.
    // This will enable the pullups for the specified pins.
    // Comment out other unwanted lines.
    //
    GpioCtrlRegs.GPAPUD.bit.GPIO24 = 0; // Enable pull-up on GPIO24 (SPISIMOA)
    GpioCtrlRegs.GPAPUD.bit.GPIO25 = 0; // Enable pull-up on GPIO25 (SPISOMIA)
    GpioCtrlRegs.GPAPUD.bit.GPIO26 = 0; // Enable pull-up on GPIO26 (SPICLKA)

    //
    // Set qualification for selected pins to asynch only
    //
    // This will select asynch (no qualification) for the selected pins.
    // Comment out other unwanted lines.
    //
    GpioCtrlRegs.GPAQSEL2.bit.GPIO24 = 3; // Asynch input GPIO24 (SPISIMOA)
    GpioCtrlRegs.GPAQSEL2.bit.GPIO25 = 3; // Asynch input GPIO25 (SPISOMIA)
    GpioCtrlRegs.GPAQSEL2.bit.GPIO26 = 3; // Asynch input GPIO26 (SPICLKA)

    //
    //Configure SPI-B pins using GPIO regs
    //
    // This specifies which of the possible GPIO pins will be SPI functional
    // pins.
    // Comment out other unwanted lines.
    //
    GpioCtrlRegs.GPAMUX2.bit.GPIO24 = 2; // Configure GPIO24 as SPISIMOA
    GpioCtrlRegs.GPAMUX2.bit.GPIO25 = 2; // Configure GPIO25 as SPISOMIA
    GpioCtrlRegs.GPAMUX2.bit.GPIO26 = 2; // Configure GPIO26 as SPICLKA
    GpioCtrlRegs.GPAMUX2.bit.GPIO27 = 0; // Configure GPIO27 as SPISTEA
    GpioCtrlRegs.GPADIR.bit.GPIO27 = 1;  //OUTPUT
    GpioDataRegs.GPASET.bit.GPIO27 = 1; //SET HIGH

    GpioCtrlRegs.GPAGMUX2.bit.GPIO24 = 1; // Configure GPIO24 as SPISIMOA
    GpioCtrlRegs.GPAGMUX2.bit.GPIO25 = 1; // Configure GPIO25 as SPISOMIA
    GpioCtrlRegs.GPAGMUX2.bit.GPIO26 = 1; // Configure GPIO26 as SPICLKA

    EDIS;
}

//
//
//
void sSpiInit(void)
{
    InitSpibGpio();
    InitSPI();
}


//
// End of file
//
