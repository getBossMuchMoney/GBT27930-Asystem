/*******************************************************************************
 * File Name: PWM_User.c
 * Description: The ePWM initialization functions.
 * Designer: Miller Xue
 * Version: 00
 * Copyright: Liyuan Haina Co.Ltd
 *******************************************************************************/
#ifndef _PWM_USER_H
#define _PWM_USER_H

#define cEPWM1          0
#define cEPWM2          1
#define cEPWM3          2
#define cEPWM4          3
#define cEPWM5          4
#define cEPWM6          5
#define cEPWM7          6
#define cEPWM8          7

#define Fs_PSFB         2000    //
#define cDeadtime       60      //0.5us
#define cDeadtime2      72
#define Prd_Half        (Fs_PSFB>>1)



#define mPSFB_PwmOn()      {EALLOW; \
                            EPwm1Regs.TZCLR.bit.OST = 1; \
                            EPwm2Regs.TZCLR.bit.OST = 1; \
                            stSuper.u16_SysStatus.BIT.PwmStatus = 1; \
                            EDIS;}

#define mPSFB_PwmOff()      {EALLOW; \
                            EPwm1Regs.TZFRC.bit.OST = 1; \
                            EPwm2Regs.TZFRC.bit.OST = 1; \
                            stSuper.u16_SysStatus.BIT.PwmStatus = 0; \
                            EDIS;}

#define mSetPhase(x)        {  EPwm1Regs.CMPA.bit.CMPA = 0; \
                               EPwm1Regs.CMPB.bit.CMPB = Prd_Half;\
                               EPwm2Regs.CMPA.bit.CMPA = x; \
                               EPwm2Regs.CMPB.bit.CMPB = (x + Prd_Half);\
                             }

#define mSetDeadtime(x,y)     {  EPwm1Regs.DBFED.bit.DBFED = x;    \
                                 EPwm1Regs.DBRED.bit.DBRED = y;    \
                                 EPwm2Regs.DBFED.bit.DBFED = x;    \
                                 EPwm2Regs.DBRED.bit.DBRED = y;    \
                                }

#define mFanSpdCtrl(x)       {EPwm4Regs.CMPA.bit.CMPA = x;}

extern void sPWMInit(void);
extern void sFanPwmInit(void);

#endif




