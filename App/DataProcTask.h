/*
 * DataProc.h
 *
 *  Created on: 2025Äê5ÔÂ23ÈÕ
 *      Author: Miller
 */

#ifndef APP_DATAPROCTASK_H_
#define APP_DATAPROCTASK_H_

#define    eDataProcTimerEvt             0
#define    cBootEvt                      1

#define    cModuleEnableMask             0xFFFF
#define    cTurnOnOffMask                0x0001
#define    cFaultClearMask               0x0004
#define    cIapCmdMask                   0x0008
#define    cPfcOpenLoopEnMask            0x0010
#define    cDcOpenLoopEnMask             0x0020
#define    cDataSaveCmdMask              0x0040

#define    EXT_CMD_WORD
#define    EXT_OP_MODE                   0
#define    EXT_IOUT_CMD                  0
#define    EXT_VOUT_CMD                  0
#define    EXT_POUT_CMD                  0
#define    EXT_CAL_EN                    0
#define    EXT_CAL_EXECMD                0
#define    EXT_IOUT_RAMPCMD                 0
#define    EXT_VOUT_RAMPCMD                 0
#define    EXT_POUT_RAMPCMD                 0
#define    EXT_FREQSYS_SET                  0
#define    EXT_PFC_OPENLOOP_DUTY            0
#define    EXT_DC_OPENLOOP_FREQ             0
#define    EXT_OVERLOAD_SET                 0
#define    EXT_MODULE_EN                    0xFFFF

#define    float_Para_Ratio                    0.00001f
#define    u32_Para_Ratio                      100000.0f

extern void sDataProcTask(void);
extern void sUpdateCtrlParaEeprom(void);
extern void sResetCtrlPara(void);



#endif /* APP_DATAPROCTASK_H_ */
