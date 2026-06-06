/*******************************************************************************
 * File Name: Cmpss_User.h
 * Description: The CMPSS initialization functions.
 * Designer: Miller Xue
 * Version: 00
 * Copyright: Liyuan Haina Co.Ltd
 *******************************************************************************/
#ifndef _CMPSS_USER_H
#define _CMPSS_USER_H

//definitions for selecting DACH reference
#define REFERENCE_VDDA     0
#define REFERENCE_VDAC     1
//definitions for COMPH input selection
#define NEGIN_DAC          0
#define NEGIN_PIN          1
//definitions for CTRIPH/CTRIPOUTH output selection
#define CTRIP_ASYNCH       0
#define CTRIP_SYNCH        1
#define CTRIP_FILTER       2
#define CTRIP_LATCH        3

#define CMPSS1_18V_LOW       2730              //2.2V
#define CMPSS2_V2_HIGH       3995//4095              //3.3v
#define CMPSS3_IPEAK_HIGH    2730
#define CMPSS4_5V_LOW        2730
#define AC_CURRENT_DAC_HIGH  3940              //1250A
#define AC_CURRENT_DAC_LOW   154               //-1250A


extern void sCmpssInit(void);

#endif




