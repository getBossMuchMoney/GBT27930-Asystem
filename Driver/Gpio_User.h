/*******************************************************************************
 * File Name: Gpio_User.h
 * Description: The GPIO initialization functions.
 * Designer: Miller Xue
 * Version: 00
 * Copyright: Liyuan Haina Co.Ltd
 *******************************************************************************/
#ifndef _GPIO_USER_H
#define _GPIO_USER_H

#define mOnOffEn()               (GpioDataRegs.GPADAT.bit.GPIO8)


//#define  mOnBoardLed1_On()       {GpioDataRegs.GPACLEAR.bit.GPIO23 = 1;}
//#define  mOnBoardLed1_Off()      {GpioDataRegs.GPASET.bit.GPIO23 = 1;}
//#define  mOnBoardLed1_Toggle()      {GpioDataRegs.GPATOGGLE.bit.GPIO23 = 1;}

#define  mOnBoardLed1_On()       {GpioDataRegs.GPBCLEAR.bit.GPIO54 = 1;}
#define  mOnBoardLed1_Off()      {GpioDataRegs.GPBSET.bit.GPIO54 = 1;}
#define  mOnBoardLed1_Toggle()      {GpioDataRegs.GPBTOGGLE.bit.GPIO54 = 1;}

#define  mOnBoardLed2_On()       {GpioDataRegs.GPACLEAR.bit.GPIO13 = 1;}
#define  mOnBoardLed2_Off()      {GpioDataRegs.GPASET.bit.GPIO13 = 1;}
#define  mOnBoardLed2_Toggle()      {GpioDataRegs.GPATOGGLE.bit.GPIO13 = 1;}

#define  mPhyAddrData()         GpioDataRegs.GPBDAT.bit.GPIO47

#define  mACRelay_On()      {GpioDataRegs.GPBCLEAR.bit.GPIO32 = 1;stSuper.u16_SysStatus.BIT.ACRly = 1;}
#define  mACRelay_Off()     {GpioDataRegs.GPBSET.bit.GPIO32 = 1;stSuper.u16_SysStatus.BIT.ACRly = 0;}

#define  mOpRelay_On()      {GpioDataRegs.GPBCLEAR.bit.GPIO34 = 1;stSuper.u16_SysStatus.BIT.OpRly = 1;}
#define  mOpRelay_Off()     {GpioDataRegs.GPBSET.bit.GPIO34 = 1;stSuper.u16_SysStatus.BIT.OpRly = 0;}

#define  mSoftRelay_On()      {GpioDataRegs.GPBCLEAR.bit.GPIO33 = 1; stSuper.u16_SysStatus.BIT.SoftRly = 1;}
#define  mSoftRelay_Off()     {GpioDataRegs.GPBSET.bit.GPIO33 = 1; stSuper.u16_SysStatus.BIT.SoftRly = 0;}

#define  mExtRunLed_On()      {GpioDataRegs.GPACLEAR.bit.GPIO6 = 1; stSuper.u16_SysStatus.BIT.RunLedOn = 1;stSuper.u16_SysStatus.BIT.RunStatus = 1;}
#define  mExtRunLed_Off()     {GpioDataRegs.GPASET.bit.GPIO6 = 1; stSuper.u16_SysStatus.BIT.RunLedOn = 0;stSuper.u16_SysStatus.BIT.RunStatus = 0;}

#define  mExtBuzzer_On()      {GpioDataRegs.GPACLEAR.bit.GPIO7 = 1; stSuper.u16_SysStatus.BIT.BuzzerOn = 1;}
#define  mExtBuzzer_Off()     {GpioDataRegs.GPASET.bit.GPIO7 = 1; stSuper.u16_SysStatus.BIT.BuzzerOn = 0;}

//#define  mStart_Sts()           (GpioDataRegs.GPBDAT.bit.GPIO34)
#define  mStop_Sts()            (GpioDataRegs.GPBDAT.bit.GPIO59)
#define  mEmgcyStop_Sts()       (GpioDataRegs.GPADAT.bit.GPIO8)
#define  mProcessStop_Sts()      (GpioDataRegs.GPBDAT.bit.GPIO58)
#define  mIGBTFault1_Sts()       (GpioDataRegs.GPADAT.bit.GPIO22)
#define  mIGBTFault2_Sts()       (GpioDataRegs.GPADAT.bit.GPIO14)
#define  mPowerFault1_Sts()      (GpioDataRegs.GPADAT.bit.GPIO131)
#define  mPowerFault2_Sts()      (GpioDataRegs.GPADAT.bit.GPIO19)
#define  mPowerFault3_Sts()      (GpioDataRegs.GPADAT.bit.GPIO129)

#define  m485_RX1_EN()               //{GpioDataRegs.GPACLEAR.bit.GPIO28 = 1; }
#define  m485_TX1_EN()               //{GpioDataRegs.GPASET.bit.GPIO28 = 1; }

#define  m485_RX2_EN()               {GpioDataRegs.GPBCLEAR.bit.GPIO47 = 1; }
#define  m485_TX2_EN()               {GpioDataRegs.GPBSET.bit.GPIO47 = 1; }

#define  m485_RX3_EN()               {GpioDataRegs.GPBCLEAR.bit.GPIO58 = 1; }
#define  m485_TX3_EN()               {GpioDataRegs.GPBSET.bit.GPIO58 = 1; }

#define  mTempSelect(x)         {GpioDataRegs.GPCDAT.all |= ((uint32_t)(x&0x7)<<22);}

extern void sGpioInit(void);

#endif




