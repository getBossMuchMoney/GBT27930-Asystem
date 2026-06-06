/******************************************************************************
File name: UartDebug.c
Author: Miller
Version: V00
Date: 2020.4.9
Description: 	for IPMOS debug
Notes:
Copyright:
******************************************************************************/
#include "PSFBController.h"
#include "app.h"
#include "F28x_Project.h"
#include "RTOS_APP.h"
#include "RTOS_Define.h"
#include "UartDebug.h"
#include "Gpio_User.h"
#include "Adc_User.h"
#include "SuperTask.h"

#ifdef cDebugUartEn
int16_t Gui_wSampleLength;
uint16_t    uwTranmitDataBuff[4020];
#endif

#if cDataBufEn
#define DLOG_SIZE 1000
int16_t Gui_wSampleRate;
int16_t Gui_CH1_ID,Gui_CH2_ID,Gui_CH3_ID,Gui_CH4_ID;
int16_t Gui_wLogEnable,wDataLogCnt,wDataLogPointer;
int16_t i16_TriggerLog,Gui_wLogStatus;
//int16_t DBUFF[4][DLOG_SIZE];


//float value
float *f32_GraphIdTab[50] = {
	0,
	&stSampleCal.f32_Output_I,   &stSampleCal.f32_Output_V,  &stSampleCal.f32_SC_V, &stSampleCal.f32_SC_V, //4
	&stCtrlPiVout.f32_Ref,    &stCtrlPiVout.f32_Fbk,   &stCtrlPiVout.f32_Uo,  &stCtrlPiIdc.f32_Ref, //8
	&stCtrlPiIdc.f32_Fbk,     &stCtrlPiIdc.f32_Uo,     &stPSFBCmpr.f32_Winner,     &stPSFBCmpr.f32_Td, //12
//	&stInvCtrl.InvCurrErrorGraph,    &stInvCtrl.Voltlimit,    &stInvCtrl.InvVoltError, &stInvCtrl.RepetCtrlValue, //16
//	&stInvCtrl.VBeforeSaturation, &stInvCtrl.K1_1, &stInvCtrl.Kpwm, &stInvCtrl.Voltlimit, &stInvCtrl.Vref,   //21
//	&stOutputCtrl.f32_IoutRefCmd, &stOutputCtrl.f32_VoutRefCmd, &stOutputCtrl.f32_IoutRef,         &stOutputCtrl.f32_VoutRef,    //24
//	&stClarkGridCurr1.f32_As,  &stClarkGridCurr1.f32_Bs,   &stClarkGridCurr1.f32_Zs,    &stClarkGridCurr2.f32_As,    //28
//	&stClarkGridCurr2.f32_Bs,  &stClarkGridCurr1.f32_Zs,   &stDcPwmCmpr.f32_DcFreq2,     &stDcPwmCmpr.f32_DcFreq1,    //32
//	&stDcPwmCmpr.f32_PhaseShift1,     &stDcPwmCmpr.f32_PhaseShift3,      &stCtrlPiQr2.f32_Ref,      &stCtrlPiQr2.f32_Fbk,     //36
//	&stCtrlPiDr1.f32_Ref,     &stCtrlPiDr1.f32_Fbk,       &stCtrlPiVout.f32_Ref,      &stCtrlPiVout.f32_Fbk,     //40
//    &stCtrlPiVout.f32_Uo,     &stCtrlPiDr2.f32_Ref,       &stDqPll.f32_SinGraph,      &stCtrlPiDr2.f32_Fbk,     //44
//    &stCtrlPiQr1.f32_Ref,     &stCtrlPiPll.f32_Uo,       &f32_PfcBusRefCmd,          &stCtrlPiQr1.f32_Fbk,      //48
};

//Fix point value
int16_t *i16_GraphIdTab[50] = {
   0,
    &stAdcSample.i16_Output_I, &stAdcSample.i16_Output_V, &stAdcSample.i16_SC_V, &stAdcSample.u16_Temperature_3,   //4
     &stPSFBCmpr.i16_Td,       &stAdcSample.u16_Temperature_1, &stAdcSample.u16_Temperature_2, &stAdcSample.u16_L_I_Mid,    //8
 //    &stAdcSample.u16_Output_I_Mid, &stAdcSample.u16_Output_V_Mid, &stAdcSample.u16_MidCalFlag,  &stInvCtrl.i16_LoopOutput1,//12
//   &stAdcSample.i16_Bus_V,    &stAdcSample.i16_NBus_V,    &stSampleCla.i16_Output_I_Small,    &stSampleCla.i16_Output_V2,//16
//   &stSampleCla.i16_Output_I, &stSampleCla.i16_Output_V,  (int16_t *)&stPwmCmpr1.u16_PwmCmpr1,    (int16_t *)&stPwmCmpr1.u16_PwmCmpr2,  //20
//   (int16_t *)&stPwmCmpr1.u16_PwmCmpr3, (int16_t *)&stDcPwmCmpr.u16_PeriodCnt1, (int16_t *)&stDcPwmCmpr.u16_PeriodCnt2, //23
//   (int16_t *)&stDcPwmCmpr.u16_WinnerFlag1, (int16_t *)&stDcPwmCmpr.u16_WinnerFlag2,
};
#endif


#if cDebugUartEn == 1
//Viriable
uint8_t bUserDataBuf[MAX_SCI_NO][cUserBufSize];
uint8_t bCommandBuffer[MAX_SCI_NO][MAX_COMMAND_LENGTH];
uint8_t bConoffCnt=0;
uint8_t *pbCmdBuffer[MAX_SCI_NO];
uint16_t	uwCommandLength[MAX_SCI_NO];
uint16_t 	uwInterval = 0;			// the interval between two data
uint16_t 	uwInterval1 = 0;
uint16_t 	uwTrigger = 0;
uint16_t 	uwTriggerSource = 0;
uint16_t 	uwTriggerWaiting = 0;
uint16_t 	uwSnatchDataCnt = 0;	// the number of data to be snatched

extern uint16_t wTurnTestCmd;
void sSendMatlabGraphdata(void);
void sSendGuiGraphData(void);

void 	sRs232Chk(uint8_t bSciID);
void 	sParsing(uint8_t bSciID);
void 	sQ1Command(uint8_t bSciID);
void    sSciQ3Com(uint8_t bSciID);
void    sSciQDCom(uint8_t bSciID);
void    sSciTONCom(uint8_t bSciID);
void    sSciTOFFCom(uint8_t bSciID);
void    sSciRONCom(uint8_t bSciID);
void    sSciIACom(uint8_t bSciID);
void    sSciIDCom(uint8_t bSciID);
void    sSciKICom(uint8_t bSciID);
void    sSciKPCom(uint8_t bSciID);
void    sSciOPCom(uint8_t bSciID);
void    sSciSCCom(uint8_t bSciID);
void    sSciQHCom(uint8_t bSciID);
void    sSciSPCom(uint8_t bSciID);


void	sInitialSci(uint8_t bSciID,uint8_t bType);
void	sSplit(uint16_t wSendData);
uint8_t	sSciGetTxStatus(uint8_t bSciID);
uint8_t	sBaudChangeStatue[MAX_SCI_NO];
uint8_t	sSciGetTxRdy0(void);
uint8_t	sSciGetRxRdy0(void);
void	sSciTx0(uint8_t bData);
void	sSciStopTx0(void);
void	sSciStopRx0(void);
void	sSciStartRx0(void);
void	sSciResetRx0(void);
void	sSciResetTx0(void);
void	sSciChangeBaudRate0(uint8_t bBandrate);
uint8_t	sbGetSciRxRdy0(void);
uint8_t	sbGetSciTxRdy0(void);

uint8_t	sSciGetTxRdy1(void);
uint8_t	sSciGetRxRdy1(void);
void	sSciTx1(uint8_t bData);
void	sSciStopTx1(void);
void	sSciStopRx1(void);
void	sSciStartRx1(void);
void	sSciResetRx1(void);
void	sSciResetTx1(void);
void	sSciChangeBaudRate1(uint8_t bBandrate);
uint8_t	sbGetSciRxRdy1(void);
uint8_t	sbGetSciTxRdy1(void);

uint8_t	sSciGetTxRdy2(void);
uint8_t	sSciGetRxRdy2(void);
void	sSciTx2(uint8_t bData);
void	sSciStopTx2(void);
void	sSciStopRx2(void);
void	sSciStartRx2(void);
void	sSciResetRx2(void);
void	sSciResetTx2(void);
void	sSciChangeBaudRate2(uint8_t bBandrate);
uint8_t	sbGetSciRxRdy2(void);
uint8_t	sbGetSciTxRdy2(void);
#endif

void    sSciTxData0(uint8_t bData);
void    sSciTxData1(uint8_t bData);
void    sSciTxData2(uint8_t bData);

uint8_t sbGetSciRxData0(void);
uint8_t sbGetSciRxData1(void);
uint8_t sbGetSciRxData2(void);
void	sQInit(QUEUE *pq,void *start,uint16_t size,uint16_t AddrStep);
uint8_t	sQDataIn(QUEUE	*pq,void *pdata,uint8_t option);
uint8_t	sQDataOut(QUEUE *pq,void *pdata);



#if (cDataBufEn == 1)

void sDataLog(void);
void sDataLogInit(void);

#ifdef _FLASH
#pragma CODE_SECTION(sDataLog, ".TI.ramfunc");
#endif
void sDataLog(void)
{
    uint16_t wIndex;
    uint16_t u16_IdTemp;
    if(Gui_wLogEnable == 1)
    {
        wDataLogCnt++;
        if(wDataLogCnt >= Gui_wSampleRate)
        {
            wDataLogCnt = 0;
            wIndex = 3 + wDataLogPointer;

            u16_IdTemp = Gui_CH1_ID;
            if(u16_IdTemp < 50)
            {
                uwTranmitDataBuff[wIndex] = (uint16_t)(*(i16_GraphIdTab[u16_IdTemp]));
            }
            else
            {
                u16_IdTemp = u16_IdTemp - 50;
                uwTranmitDataBuff[wIndex] = (int16_t)((*f32_GraphIdTab[u16_IdTemp]) * 10.0f);
            }
            wIndex += Gui_wSampleLength;

            u16_IdTemp = Gui_CH2_ID;
            if(u16_IdTemp < 50)
            {
                uwTranmitDataBuff[wIndex] = (uint16_t)(*(i16_GraphIdTab[u16_IdTemp]));
            }
            else
            {
                u16_IdTemp = u16_IdTemp - 50;
                uwTranmitDataBuff[wIndex] = (int16_t)((*f32_GraphIdTab[u16_IdTemp]) * 10.0f);
            }
            wIndex += Gui_wSampleLength;

            u16_IdTemp = Gui_CH3_ID;
            if(u16_IdTemp < 50)
            {
                uwTranmitDataBuff[wIndex] = (uint16_t)(*(i16_GraphIdTab[u16_IdTemp]));
            }
            else
            {
                u16_IdTemp = u16_IdTemp - 50;
                uwTranmitDataBuff[wIndex] = (int16_t)((*f32_GraphIdTab[u16_IdTemp]) * 10.0f);
            }
            wIndex += Gui_wSampleLength;

            u16_IdTemp = Gui_CH4_ID;
            if(u16_IdTemp < 50)
            {
                uwTranmitDataBuff[wIndex] = (uint16_t)(*(i16_GraphIdTab[u16_IdTemp]));
            }
            else
            {
                u16_IdTemp = u16_IdTemp - 50;
                uwTranmitDataBuff[wIndex] = (int16_t)((*f32_GraphIdTab[u16_IdTemp]) * 10.0f);
            }

            /*uwTranmitDataBuff[wIndex] = (uint16_t)(*(i16_GraphIdTab[Gui_CH1_ID]));
            wIndex += Gui_wSampleLength;
            uwTranmitDataBuff[wIndex] = (uint16_t)(*(i16_GraphIdTab[Gui_CH2_ID]));
            wIndex += Gui_wSampleLength;
            uwTranmitDataBuff[wIndex] = (uint16_t)(*(i16_GraphIdTab[Gui_CH3_ID]));
            wIndex += Gui_wSampleLength;
            uwTranmitDataBuff[wIndex] = (uint16_t)(*(i16_GraphIdTab[Gui_CH4_ID]));*/
            wDataLogPointer++;
            if(wDataLogPointer >= Gui_wSampleLength)
            {
                wDataLogPointer = 0;
                Gui_wLogEnable = 0;
                Gui_wLogStatus = 2;
            }
        }
    }
    else
    {
        wDataLogPointer = 0;
        wDataLogCnt = 0;
    }
}

void sDataLogInit(void)
{
    Gui_CH1_ID = 1;
    Gui_CH2_ID = 2;
    Gui_CH3_ID = 3;
    Gui_CH4_ID = 4;
    Gui_wSampleRate = 1;
    Gui_wSampleLength = DLOG_SIZE;
    Gui_wLogEnable = 0;
    Gui_wLogStatus = 0;
    wDataLogCnt = 0;
    wDataLogPointer = 0;
    i16_TriggerLog = 0;
}

#endif

//////debug define
#if cDebugUartEn == 1
uint8_t sbNumToXXXx(uint16_t wNumber, uint8_t *pbBuffer);
uint8_t sbNumToXXxx(uint16_t wNumber, uint8_t *pbBuffer);
uint8_t sbNumToXXXxx(uint16_t wNumber, uint8_t *pbBuffer);
uint8_t sbNumAsciiPick(uint16_t wNumber, uint8_t bPosition);
uint8_t sbNumToXXXX(uint16_t wNumber, uint8_t *pbBuffer);
uint8_t sbNumToXxxxx(uint16_t wNumber, uint8_t *pbBuffer);
uint8_t sbNumToXXxxx(uint16_t wNumber, uint8_t *pbBuffer);


uint8_t sbNumToXXXxUnit(uint16_t wNumber, uint8_t *pbBuffer, uint8_t bUnit);
uint8_t sbNumToXXxxUnit(uint16_t wNumber, uint8_t *pbBuffer, uint8_t bUnit);
uint8_t sbNumToXXXXX(uint16_t wNumber, uint8_t *pbBuffer);


// List of Sci structure and queue
uint8_t   wBinaryMode = 0;
uint8_t   bHighByte = 0;
uint8_t   bLowByte = 0;
uint8_t   bSendHighHalfByte = 0;
uint8_t	bSciDataTemp = 0;
uint16_t  u16_MatlabGraphDataStart = 0;
uint16_t  u16_Tx2RxFlag[MAX_SCI_NO];

SciStruct   SciList[MAX_SCI_NO];
QUEUE       QList[MAX_SCI_NO];
uint8_t		bQueueBuffer[MAX_SCI_NO][cQueueBufSize];

uint16_t wscitasktest;

unsigned char GetCheckSum(unsigned char *buf, unsigned char bLen);
void sSetCode_CH0(uint8_t bSciID);


/*******************************************************************************
Function name:	sRs232Chk(uint8_t bSciID)
Description:  	check the state of command be received, then put data into 
				command buffer by calling sSciRead()				
Calls:        	sParsingCent()
Called By: 		sInterfaceTask()
Parameters: 	bSciID: the ID of SCI channel
Return: 		void
*******************************************************************************/
void 	sRs232Chk(uint8_t bSciID)
{
	uint16_t	uwSciTemp;
	
	while(1)
	{
#if cDataBufEn
		if(Gui_wLogStatus == 2)
		{
			sSendGuiGraphData();
			Gui_wLogStatus = 0;
		}
#endif
		uwSciTemp = sSciRead(bSciID,pbCmdBuffer[bSciID]);
		
		if(uwSciTemp == cSciRxBufEmpty)
		{			
			break;
		}
		
		if(uwCommandLength[bSciID] >= MAX_COMMAND_LENGTH)
		{
			pbCmdBuffer[bSciID] = bCommandBuffer[bSciID];
			uwCommandLength[bSciID] = 0;
		}
		else if((bCommandBuffer[bSciID][0] == 'S') &&
		        (bCommandBuffer[bSciID][1] == 'P'))
		{
		    if((uwCommandLength[bSciID] == bCommandBuffer[bSciID][2]))
		    {
		        sSciSPCom(bSciID);
                uwCommandLength[bSciID] = 0;
                pbCmdBuffer[bSciID] = bCommandBuffer[bSciID];
		    }
		    else
		    {
		        uwCommandLength[bSciID] ++;
		        pbCmdBuffer[bSciID] ++;
		    }

		}
		else if(((*pbCmdBuffer[bSciID]) == 0x0D))
		{	// the end of current command
			*pbCmdBuffer[bSciID] = cEOI;	
			sParsing(bSciID);
			wscitasktest++;
			pbCmdBuffer[bSciID] = bCommandBuffer[bSciID];
			uwCommandLength[bSciID] = 0;
		}
		else
		{
			uwCommandLength[bSciID] ++;
			pbCmdBuffer[bSciID] ++;
		}
	}
}

/*******************************************************************************
Function name:	sParsing
Description:  	Parse command & do something according to the command.	
				Search command index by dichotomy. We use strncmp() function 
				from string standard library
Calls:        	strncmp()
Called By: 		sRs232Chk()
Parameters: 	bSciID: the ID of SCI channel
Return: 		void
*******************************************************************************/
void 	sParsing(uint8_t bSciID)
{	
	if ((bCommandBuffer[bSciID][0] == 'Q') && (bCommandBuffer[bSciID][1] == '1'))			//PSnGnLnHnEnDn
	{
		sQ1Command(bSciID);
	}
	else if ((bCommandBuffer[bSciID][0] == 'Q') && (bCommandBuffer[bSciID][1] == '3'))			//PSnGnLnHnEnDn
	{
#if cDataBufEn
		sSciQ3Com(bSciID);
#endif
	}
	else if ((bCommandBuffer[bSciID][0] == 'Q') && (bCommandBuffer[bSciID][1] == 'D'))			//PSnGnLnHnEnDn
	{
#if cDataBufEn
		sSciQDCom(bSciID);
#endif
	}
	else if ((bCommandBuffer[bSciID][0] == 'T') && (bCommandBuffer[bSciID][1] == 'N'))          //PSnGnLnHnEnDn
    {
	    sSciTONCom(bSciID);
    }
	else if ((bCommandBuffer[bSciID][0] == 'T') && (bCommandBuffer[bSciID][1] == 'F'))          //PSnGnLnHnEnDn
    {
        sSciTOFFCom(bSciID);
    }

}


void sSetCode_CH0(uint8_t bSciID)
{
	
}

void    sSciSPCom(uint8_t bSciID)
{


}

void sSendGuiGraphData(void)
{
//  uint16_t uwTemp;
//  int i;
    //long wCheckSum;

    uwTranmitDataBuff[0] = 0xAAAA;
    uwTranmitDataBuff[1] = 0x0001;
    uwTranmitDataBuff[2] = Gui_wSampleLength*4;

    //wCheckSum = 0x01 + Gui_wSampleLength;
    /*for(i = 0;i < Gui_wSampleLength;i ++)
    {
        uwTranmitDataBuff[3 + i] = DBUFF[0][i];
        uwTranmitDataBuff[3 + i + Gui_wSampleLength] = DBUFF[1][i];
        uwTranmitDataBuff[3 + i + Gui_wSampleLength*2] = DBUFF[2][i];
        uwTranmitDataBuff[3 + i + Gui_wSampleLength*3] = DBUFF[1][i];
        //wCheckSum += DBUFF[uwTemp][i];
    }*/

    //uwTranmitDataBuff[2 + Gui_wSampleLength] = wCheckSum & 0x0000FFFF;

    sSciWriteBinary(0,uwTranmitDataBuff,(Gui_wSampleLength*4 + 3));
}


unsigned char GetCheckSum(unsigned char *buf, unsigned char bLen)
{
	unsigned char Checksum, i;
	Checksum = buf[0];
	for(i = 1; i < bLen; i++)
	{
	       Checksum = Checksum^buf[i];
	}
	return Checksum;
}


void 	sQ1Command(uint8_t bSciID)
{
    /*uint8_t	bStrLen = 0;
    uint16_t i;
    uint16_t *pDataBuf;
    int16_t i16_Temp;
    pDataBuf = &stMeasure.u16_Grid_V_RS_Rms;

	if((bCommandBuffer[bSciID][2] == 0x0d))
	{	
	    bUserDataBuf[bSciID][bStrLen++] = 'Q';
	    bUserDataBuf[bSciID][bStrLen++] = '1';
	    bUserDataBuf[bSciID][bStrLen++] = '0';
	    bUserDataBuf[bSciID][bStrLen++] = '1';
	    bUserDataBuf[bSciID][bStrLen++] = 0;
	    bUserDataBuf[bSciID][bStrLen++] = 0;

	    for(i=0; i<cMeasureLength; i++)
	    {
	        bUserDataBuf[bSciID][bStrLen++] = ((*pDataBuf)>>8) & 0xff;
	        bUserDataBuf[bSciID][bStrLen++] = (*pDataBuf) & 0xff;
	        pDataBuf++;
	    }

	    //physical addr
	    bUserDataBuf[bSciID][bStrLen++] = (stSuper.u16_PhysicalAddr>>8) & 0xff;
	    bUserDataBuf[bSciID][bStrLen++] = (stSuper.u16_PhysicalAddr) & 0xff;

	    //system mode
	    bUserDataBuf[bSciID][bStrLen++] = (stSuper.u16_SysMode>>8) & 0xff;
	    bUserDataBuf[bSciID][bStrLen++] = (stSuper.u16_SysMode) & 0xff;


	    stSuper.u16_SysStatus.BIT.DcOpenLoopEn = u16_DcOpenLoop;
	    stSuper.u16_SysStatus.BIT.PfcOpenLoopEn = u16_PfcOpenLoop;

	    //system status
	    bUserDataBuf[bSciID][bStrLen++] = (stSuper.u16_SysStatus.all>>8) & 0xff;
	    bUserDataBuf[bSciID][bStrLen++] = (stSuper.u16_SysStatus.all) & 0xff;

	    //fault code
	    bUserDataBuf[bSciID][bStrLen++] = (stSuper.u32_FaultCode.all>>24) & 0xff;
	    bUserDataBuf[bSciID][bStrLen++] = (stSuper.u32_FaultCode.all>>16) & 0xff;
	    bUserDataBuf[bSciID][bStrLen++] = (stSuper.u32_FaultCode.all>>8) & 0xff;
	    bUserDataBuf[bSciID][bStrLen++] = (stSuper.u32_FaultCode.all) & 0xff;

	    //software version
	    bUserDataBuf[bSciID][bStrLen++] = (stSuper.u32_SwVersion>>24) & 0xff;
        bUserDataBuf[bSciID][bStrLen++] = (stSuper.u32_SwVersion>>16) & 0xff;
        bUserDataBuf[bSciID][bStrLen++] = (stSuper.u32_SwVersion>>8) & 0xff;
        bUserDataBuf[bSciID][bStrLen++] = (stSuper.u32_SwVersion) & 0xff;

        //Output mode
        bUserDataBuf[bSciID][bStrLen++] = stOutputCtrl.u16_OutputMode & 0xff;

        //Iout cmd
        i16_Temp = (int16_t)(stOutputCtrl.f32_IoutRefCmd * 10.0f);
        bUserDataBuf[bSciID][bStrLen++] = (i16_Temp>>8) & 0xff;
        bUserDataBuf[bSciID][bStrLen++] = (i16_Temp) & 0xff;
        //Iout step
        i16_Temp = (int16_t)(stOutputCtrl.f32_IoutStep * 1000.0f);
        bUserDataBuf[bSciID][bStrLen++] = (i16_Temp>>8) & 0xff;
        bUserDataBuf[bSciID][bStrLen++] = (i16_Temp) & 0xff;

        //Vout cmd
        i16_Temp = (int16_t)(stOutputCtrl.f32_VoutRefCmd * 10.0f);
        bUserDataBuf[bSciID][bStrLen++] = (i16_Temp>>8) & 0xff;
        bUserDataBuf[bSciID][bStrLen++] = (i16_Temp) & 0xff;
        //Vout step
        i16_Temp = (int16_t)(stOutputCtrl.f32_VoutStep * 1000.0f);
        bUserDataBuf[bSciID][bStrLen++] = (i16_Temp>>8) & 0xff;
        bUserDataBuf[bSciID][bStrLen++] = (i16_Temp) & 0xff;

        //Pout cmd
        i16_Temp = (int16_t)(stOutputCtrl.f32_PoutRefCmd * 0.01f);
        bUserDataBuf[bSciID][bStrLen++] = (i16_Temp>>8) & 0xff;
        bUserDataBuf[bSciID][bStrLen++] = (i16_Temp) & 0xff;
        //Pout step
        i16_Temp = (int16_t)(stOutputCtrl.f32_PoutStep * 0.01f);
        bUserDataBuf[bSciID][bStrLen++] = (i16_Temp>>8) & 0xff;
        bUserDataBuf[bSciID][bStrLen++] = (i16_Temp) & 0xff;

        bUserDataBuf[bSciID][4] = (bStrLen>>8) & 0xff;
        bUserDataBuf[bSciID][5] = (bStrLen) & 0xff;

		sSciWrite(bSciID,bUserDataBuf[bSciID],bStrLen);
	}*/
}

#if cDataBufEn
void sSendMatlabGraphdata(void)
{
	/*uint8_t	bStrLen = 0;
	int16_t   i16_Temp = 0;
	uint8_t   bDataTemp = 0;
	uint8_t   bCheckSum = 0;

	bUserDataBuf[0][bStrLen++] = 0xAA;
	bCheckSum += 0xAA;          //1
	bUserDataBuf[0][bStrLen++] = 0xAA;
	bCheckSum += 0xAA;          //2    

	//i16_Temp = stOisAlgData.iq_Gyro_X>>12;
	bDataTemp = (uint8_t)((i16_Temp>>8) & 0xff);
	bUserDataBuf[0][bStrLen++] = bDataTemp;
	bCheckSum += bDataTemp;					 //3     gyro_x
	
	bDataTemp = (uint8_t)((i16_Temp) & 0xff);
	bUserDataBuf[0][bStrLen++] = bDataTemp;
	bCheckSum += bDataTemp;					    //4
	
	//i16_Temp = stOisAlgData.iq_Gyro_Y>>12;
	bDataTemp = (uint8_t)((i16_Temp>>8) & 0xff);
	bUserDataBuf[0][bStrLen++] = bDataTemp;
	bCheckSum += bDataTemp;						 //5   gyro_y
	
	bDataTemp = (uint8_t)((i16_Temp) & 0xff);
	bUserDataBuf[0][bStrLen++] = bDataTemp;
	bCheckSum += bDataTemp; 					 //6
	
	//i16_Temp = stOisAlgData.iq_Gyro_Z>>12;
	bDataTemp = (uint8_t)((i16_Temp>>8) & 0xff);
	bUserDataBuf[0][bStrLen++] = bDataTemp;
	bCheckSum += bDataTemp;					    //7   gyroz
	
	bDataTemp = (uint8_t)((i16_Temp) & 0xff);
	bUserDataBuf[0][bStrLen++] = bDataTemp;
	bCheckSum += bDataTemp;  				    //8   
	
	i16_Temp = stOisAlgData.iq_Pitch_Graph>>12;
	bDataTemp = (uint8_t)((i16_Temp>>8) & 0xff);
	bUserDataBuf[0][bStrLen++] = bDataTemp;
	bCheckSum += bDataTemp;				      //9   pitch
	
	bDataTemp = (uint8_t)((i16_Temp) & 0xff);
	bUserDataBuf[0][bStrLen++] = bDataTemp;
	bCheckSum += bDataTemp;					 //10
	
	i16_Temp = stOisAlgData.iq_Yaw_Graph>>12;
	bDataTemp = (uint8_t)((i16_Temp>>8) & 0xff);
	bUserDataBuf[0][bStrLen++] = bDataTemp;
	bCheckSum += bDataTemp;					 //11   yaw
	
	bDataTemp = (uint8_t)((i16_Temp) & 0xff);
	bUserDataBuf[0][bStrLen++] = bDataTemp;
	bCheckSum += bDataTemp;					 //12
	
	i16_Temp = stOisAlgData.iq_Roll_Graph>>12;
	bDataTemp = (uint8_t)((i16_Temp>>8) & 0xff);
	bUserDataBuf[0][bStrLen++] = bDataTemp;
	bCheckSum += bDataTemp;						 //13   roll
	
	bDataTemp = (uint8_t)((i16_Temp) & 0xff);
	bUserDataBuf[0][bStrLen++] = bDataTemp;
	bCheckSum += bDataTemp;					 //14
	
	bDataTemp = 0;
	bUserDataBuf[0][bStrLen++] = bDataTemp;
	bCheckSum += bDataTemp;						 //15   temp
	
	bDataTemp = 0;
	bUserDataBuf[0][bStrLen++] = bDataTemp;
	bCheckSum += bDataTemp;					 //16
	
	bCheckSum &= 0xff;
	bUserDataBuf[0][bStrLen++] = bCheckSum;
	//bCheckSum += bDataTemp;;                //17
	
	bUserDataBuf[0][bStrLen++] = 0x55;       //18
	bUserDataBuf[0][bStrLen++] = 0x55;       //19

	sSciWrite(0,bUserDataBuf[0],bStrLen); */
}


/*******************************************************************************
Function name:	sQDCommand(unsigned char bSciID)
Description:  	transmit Data has been snatched
Calls:        	
Called By: 		
Parameters: 	bSciID: the ID of SCI channel
Return: 		void
*******************************************************************************/

void    sSciQDCom(uint8_t bSciID)
{
	uint16_t uwTemp;
	int i;
	long wCheckSum;
	
	uwTemp = bCommandBuffer[bSciID][2] - 48;
	
	if((uwTemp >= 4) || (Gui_wLogStatus != 2))
	{
		return;
	}

	uwTranmitDataBuff[0] = 0x01;		
	uwTranmitDataBuff[1] = Gui_wSampleLength;	
	
	wCheckSum = 0x01 + Gui_wSampleLength;
	for(i = 0;i < Gui_wSampleLength;i ++)
	{
		//uwTranmitDataBuff[2 + i] = DBUFF[uwTemp][i];
		//wCheckSum += DBUFF[uwTemp][i];
	}	
	
	uwTranmitDataBuff[2 + Gui_wSampleLength] = wCheckSum & 0x0000FFFF;

	sSciWriteBinary(bSciID,uwTranmitDataBuff,Gui_wSampleLength + 3);
}

/*******************************************************************************
Function name:  sQ3Command(unsigned char bSciID)
Description:    Set parameters for SnatchData
Calls:
Called By:
Parameters:     bSciID: the ID of SCI channel
Return:         void
*******************************************************************************/
void sSciQ3Com(uint8_t bSciID)
{
    uint16_t uwSnatchDataCntTemp;
    uint16_t uwIntervalTemp;
    uint16_t uwDataKindTemp1;
    uint16_t uwDataKindTemp2;
    uint16_t uwDataKindTemp3;
    uint16_t uwDataKindTemp4;
    //uint16_t uwTriggerTemp;
    //uint16_t i;
    //uint16_t uwLengthTemp;

    /*uwLengthTemp = uwCommandLength[bSciID];

    for(i = 0;i < uwLengthTemp;i ++)
    {
        bUserDataBuf[bSciID][i] = bCommandBuffer[bSciID][i];
    }*/

    //sSciWrite(bSciID,bUserDataBuf[bSciID],uwLengthTemp);

    //aaa
    uwSnatchDataCntTemp = (bCommandBuffer[bSciID][2] - 48) * 1000 +
                 (bCommandBuffer[bSciID][3] - 48) * 100 + (bCommandBuffer[bSciID][4] - 48)*10 +
                 (bCommandBuffer[bSciID][5] - 48);
    //bbb
    uwIntervalTemp = (bCommandBuffer[bSciID][7] - 48) * 10 + bCommandBuffer[bSciID][8] - 48;
    //cc
    uwDataKindTemp1 = (bCommandBuffer[bSciID][10] - 48) * 10 +
                      bCommandBuffer[bSciID][11] - 48;
    //dd
    uwDataKindTemp2 = (bCommandBuffer[bSciID][13] - 48) * 10 +
                      bCommandBuffer[bSciID][14] - 48;
    //ee
    uwDataKindTemp3 = (bCommandBuffer[bSciID][16] - 48) * 10 +
                      bCommandBuffer[bSciID][17] - 48;
    //ff
    uwDataKindTemp4 = (bCommandBuffer[bSciID][19] - 48) * 10 +
                      bCommandBuffer[bSciID][20] - 48;
    //gg
    //uwTriggerTemp = bCommandBuffer[bSciID][22] - 48;


    /*if(uwSnatchDataCntTemp > DLOG_SIZE || uwIntervalTemp > DLOG_SIZE || uwTriggerTemp > 4)
    {
        return;
    }*/

    DINT;
    //uwSaveDataCnt = 0;
    Gui_wSampleLength = uwSnatchDataCntTemp;
    Gui_wSampleRate = uwIntervalTemp;
    //uwInterval1 = uwIntervalTemp;
    Gui_CH1_ID = uwDataKindTemp1;
    Gui_CH2_ID = uwDataKindTemp2;
    Gui_CH3_ID = uwDataKindTemp3;
    Gui_CH4_ID = uwDataKindTemp4;
    Gui_wLogEnable = 1;
    Gui_wLogStatus = 1;
    //i16_TriggerLog = 1;
    EINT;
    //sRTOSEventSend(cPrioSuper,eSuperTurnOnEvt);
}

#endif

void    sSciTONCom(uint8_t bSciID)
{
	/*uint8_t	bStrLen = 0;


	bUserDataBuf[bSciID][bStrLen++] = 'O';
	bUserDataBuf[bSciID][bStrLen++] = 'K';
	bUserDataBuf[bSciID][bStrLen++] = 0xd;
	sSciWrite(bSciID,bUserDataBuf[bSciID],bStrLen);*/
    //sRTOSEventSend(cPrioSuper,eSuperTurnOnEvt);
}
void    sSciTOFFCom(uint8_t bSciID)
{
	/*uint8_t	bStrLen = 0;

	bUserDataBuf[bSciID][bStrLen++] = 'O';
	bUserDataBuf[bSciID][bStrLen++] = 'K';
	bUserDataBuf[bSciID][bStrLen++] = 0xd;
	sSciWrite(bSciID,bUserDataBuf[bSciID],bStrLen);*/
    //sRTOSEventSend(cPrioSuper,eSuperTurnOffEvt);
}
/**** global function ****/
/*******************************************************************************
Function name:	sInitialSci(void)
Description:  	initialize sci struct & receive buffer
Calls:     sQInit()
Called By: SCI task
Parameters:	bSciID: the number of sci port
			bType: the type of sci
Return: 	null
*******************************************************************************/
void    sInitialSci(uint8_t bSciID,uint8_t bType)
{
        SciStruct   *pSci;
        QUEUE       *pQ;

        pSci =& SciList[bSciID];

        pSci->pqRx =& QList[bSciID];
        pQ = pSci->pqRx;
        sQInit(pQ,bQueueBuffer[bSciID],cQueueBufSize,sizeof(bQueueBuffer[bSciID][0]));

        pSci->bTxStatus = cSciTxRdy;
        pSci->wTxLength = 0;

        pSci->bSciType = bType;

		pbCmdBuffer[bSciID] = bCommandBuffer[bSciID];
		u16_Tx2RxFlag[bSciID] = 0;
}

/*******************************************************************************
Function name:	sSciRxISR()
Description:  	deal with SCI receive interrupt & put received data into queue
Calls:
Called By: ISR of SCI receive
Parameters:	bSciID: the number of sci port
Return: 	null
*******************************************************************************/
void    sSciRxISR(uint8_t bSciID)
{
    SciStruct       *psci;
    QUEUE           *pq;

    psci =& SciList[bSciID];
    pq = psci->pqRx;

    if(sbSciGetRxRdy(bSciID) == cSciRxRdy)	// confirm recerve finished
    {
        //sSciResetRx(bSciID);
        bSciDataTemp = sbSciGetRxData(bSciID);
        (void)sQDataIn(pq,&bSciDataTemp,cQCoverLast);
    }
}

/*******************************************************************************
Function name:	sSciRead()
Description:  	Transfer data read from queue to the array for applications
Calls:
Called By: SCI timer event
Parameters:	bSciID: the number of sci port
			pBuf:	the pointer to destination array
Return: 	cSciRxBufEmpty: no more data
			cSciRxRdy: more data to be transfered
*******************************************************************************/
uint8_t   sSciRead(uint8_t  bSciID,uint8_t *pBuf)
{
        QUEUE           *pq;
        uint8_t   temp;
        SciStruct       *psci;

        psci =& SciList[bSciID];
        pq=psci->pqRx;


        DINT;
        temp = sQDataOut(pq,pBuf);
        EINT;

        if(temp == cQBufEmpty)
        {
            return(cSciRxBufEmpty);
        }
        else
        {
            return(cSciRxRdy);
        }
}

/*******************************************************************************
Function name:	sSciTxISR()
Description:  	deal with SCI transmission interrupt & decides next action which
				is to stop from transmitting or to continue
Calls:
Called By: ISR of SCI transmission
Parameters:	bSciID: the number of sci port
Return: 	null
*******************************************************************************/
void    sSciTxISR(uint8_t bSciID)
{
        SciStruct       *psci;

        psci =& SciList[bSciID];

        if(sbSciGetTxRdy(bSciID) == cSciTxRdy)
        {
            sSciResetTx(bSciID);
            if(psci->wTxLength == 0)
            {
                wBinaryMode = 0;
                psci->bTxStatus = cSciTxRdy;
                sSciStopTx(bSciID);
            }
            else
            {
                if(wBinaryMode == 0)
                {
                    sSciTxData(bSciID,*(psci->pbTx));
                    (psci->pbTx)++;
                    (psci->wTxLength)--;
                }
                else	// sent 16-bit data in binary mode
                {
                    if(bSendHighHalfByte == 1)	// low byte to be sent
                    {
                        sSciTxData(bSciID,bLowByte);
                        bSendHighHalfByte = 0;
                        (psci->pwTx)++;
                        (psci->wTxLength)--;
                    }
                    else	// high byte to be sent
                    {
                        sSplit(*(psci->pwTx));
                        bSendHighHalfByte = 1;
                        sSciTxData(bSciID,bHighByte);
                    }
                }
            }
        }
}

/*******************************************************************************
Function name:	sSciWrite()
Description:  	Initiate transmission of SCI in ASCII model
Calls:
Called By: User applications
Parameters:	bSciID: the number of sci port
			pstart: pointer to the pointer of start address
			wLength:  the length of data to be transmitted
Return: 	cSciTxBusy: failed to transmit for transmision is proceeding
			cSciTxRdy: begin transmission
*******************************************************************************/
uint8_t   sSciWrite(uint8_t bSciID,uint8_t *pstart,uint16_t wLength)
{
    SciStruct       *psci;

    psci =& SciList[bSciID];

    if(psci->bTxStatus == cSciTxBusy)
    {
        return(cSciTxBusy);
    }

    DINT;
    psci->pbTx = pstart;
    psci->wTxLength = wLength;
    psci->bTxStatus = cSciTxBusy;

    sSciTxData(bSciID,*(psci->pbTx));
    (psci->pbTx)++;
    (psci->wTxLength)--;
    EINT;


    return(cSciTxRdy);
}

/*******************************************************************************
Function name:	sSplit()
Description:  	split a 16-bit word into two bytes,
					high 8-bit byte & low 8-bit byte
Calls:
Called By: sSciWriteBinary()
Parameters:	wSendData: the value to be splitted

Return: 	cSciTxBusy: failed to transmit for transmision is proceeding
			cSciTxRdy: begin transmission
*******************************************************************************/
void sSplit(uint16_t wSendData)
{
    bHighByte = (uint8_t)(wSendData >> 8);
    bLowByte = (uint8_t)(wSendData & 0x00FF);
}

/*******************************************************************************
Function name:	sSciWriteBinary()
Description:  	Initiate transmission of SCI in binary model
Calls:
Called By: User applications
Parameters:	bSciID: the number of sci port
			pstart: pointer to the pointer of start address
			wLength:  the length of data to be transmitted
Return: 	cSciTxBusy: failed to transmit for transmision is proceeding
			cSciTxRdy: begin transmission
*******************************************************************************/
uint8_t   sSciWriteBinary(uint8_t bSciID,uint16_t *pstart,uint16_t wLength)
{
    SciStruct       *psci;

    psci =& SciList[bSciID];

    if(psci->bTxStatus == cSciTxBusy)
    {
        return(cSciTxBusy);
    }

    DINT;
    wBinaryMode = 1;
    psci->pwTx = pstart;
    psci->wTxLength = wLength;
    psci->bTxStatus = cSciTxBusy;
    //split
    sSplit(*(psci->pwTx));
    bSendHighHalfByte = 1;
    sSciTxData(bSciID,bHighByte);
    EINT;
    return(cSciTxRdy);
}

/*******************************************************************************
Function name:	sSciGetTxStatus()
Description:  	Get the status of SCI transmission
Calls:
Called By: SCI timer event
Parameters:	bSciID: the number of sci port
Return: 	cSciTxBusy: transmision is proceeding
			cSciTxRdy: complete transmission
*******************************************************************************/
uint8_t   sSciGetTxStatus(uint8_t bSciID)
{
    SciStruct       *psci;

    psci =& SciList[bSciID];

    return(psci->bTxStatus);
}



/*******************************************************************************
Function name:	sSciStopTx()
Description:  	Stop TXD
Calls:
Called By:
Parameters:	bSciID: the number of sci port
			bData: the data-byte to be transimitted
Return: 	null
*******************************************************************************/
void	sSciStopTx(uint8_t bSciID)
{
	switch(bSciID)
	{
		case 0:
			sSciStopTx0();
			u16_Tx2RxFlag[bSciID] = 1;
			break;
		case 1:
			sSciStopTx1();
			break;
		case 2:
			sSciStopTx2();
			break;
		default:
			break;
	}
}

/*******************************************************************************
Function name:	sSciStopRx()
Description:  	Stop RXD
Calls:
Called By:
Parameters:	bSciID: the number of sci port
Return: 	null
*******************************************************************************/
void	sSciStopRx(uint8_t bSciID)
{
	switch(bSciID)
	{
		case 0:
			sSciStopRx0();
			break;
		case 1:
			sSciStopRx1();
			break;
		case 2:
			sSciStopRx2();
			break;
		default:
			break;
	}
}

/*******************************************************************************
Function name:	sSciStartRx()
Description:  	Start TXD
Calls:
Called By:
Parameters:	bSciID: the number of sci port
Return: 	null
*******************************************************************************/
void	sSciStartRx(uint8_t bSciID)
{
	switch(bSciID)
	{
		case 0:
			sSciStartRx0();
			break;
		case 1:
			sSciStartRx1();
			break;
		case 2:
			sSciStartRx2();
			break;
		default:
			break;
	}
}

/*******************************************************************************
Function name:	sSciResetRx()
Description:  	Reset RXD
Calls:
Called By:
Parameters:	bSciID: the number of sci port
Return: 	null
*******************************************************************************/
void	sSciResetRx(uint8_t bSciID)
{
	switch(bSciID)
	{
		case 0:
			sSciResetRx0();
			break;
		case 1:
			sSciResetRx1();
			break;
		case 2:
			sSciResetRx2();
			break;
		default:
			break;
	}
}

/*******************************************************************************
Function name:	sSciResetTx()
Description:  	Reset TXD
Calls:
Called By:
Parameters:	bSciID: the number of sci port
Return: 	null
*******************************************************************************/
void	sSciResetTx(uint8_t bSciID)
{
	switch(bSciID)
	{
		case 0:
			sSciResetTx0();
			break;
		case 1:
			sSciResetTx1();
			break;
		case 2:
			sSciResetTx2();
			break;
		default:
			break;
	}
}



/******************************************************************************/
/*disable tx interrupt														  */
/******************************************************************************/
void	sSciStopTx0(void)
{
	mSciADisableTxInt();
	//m485_RX1_EN();
}

void	sSciStopTx1(void)
{
	mSciBDisableTxInt();
}

void	sSciStopTx2(void)
{
	mSciCDisableTxInt();
}
/******************************************************************************/
/*Disable rx interrupt														  */
/******************************************************************************/
void	sSciStopRx0(void)
{
	mSciADisableRxInt();
}

void	sSciStopRx1(void)
{
	mSciBDisableRxInt();
}

void	sSciStopRx2(void)
{
	mSciCDisableRxInt();
}
/******************************************************************************/
/*Enable rx interrupt														  */
/******************************************************************************/
void	sSciStartRx0(void)
{
	mSciAEnableRxInt();
}

void	sSciStartRx1(void)
{
	mSciBEnableRxInt();
}

void	sSciStartRx2(void)
{
	mSciCEnableRxInt();
}

/******************************************************************************/
/*Reset Rx interrupt														  */
/******************************************************************************/
void	sSciResetRx0(void)
{
	return;
}

void	sSciResetRx1(void)
{
	return;
}

void	sSciResetRx2(void)
{
	return;
}
/******************************************************************************/
/*Reset Tx interrupt														  */
/******************************************************************************/
void	sSciResetTx0(void)
{
	return;
}

void	sSciResetTx1(void)
{
	return;
}

void	sSciResetTx2(void)
{
	return;
}
#endif
////////debug

#if cDebugUartEn == 1
/******************************************************************************
Function Name:
	sbNumToXXXx: Convert input wNumber into 3 digital numbers including
		     1 decimal represented byASCII code.
Parameters:
	wNumber: Number to be converted into ASCII code
	pbBuffer: pointer of conversion register
Internal Variable:
	wNumFormated: Number after formatting
	bConvResult: conversion result
Return Value:
	1:1 ( The Number to be converted is normal )
	0:	0 ( The Number to be converted is over 999.9, and limited to 999.9 )
*******************************************************************************/
uint8_t sbNumToXXXx(uint16_t wNumber, uint8_t *pbBuffer)
{
	uint16_t	wNumFormated;
	uint8_t	bConvResult= 1;
	
    	if(wNumber > 9999)
    	{
    		wNumFormated= 9999;
    		bConvResult= 0;
    	}
    	else
    	{
    		wNumFormated= wNumber;
    	}
    	*(pbBuffer)=sbNumAsciiPick(wNumFormated,3);
    	*(pbBuffer+1)=sbNumAsciiPick(wNumFormated,2);
    	*(pbBuffer+2)=sbNumAsciiPick(wNumFormated,1);
    	*(pbBuffer+3)='.';
    	*(pbBuffer+4)=sbNumAsciiPick(wNumFormated,0);
    	*(pbBuffer+5)= ' ';
    	return(bConvResult);
}

uint8_t sbNumToXXXxUnit(uint16_t wNumber, uint8_t *pbBuffer, uint8_t bUnit)
{
	uint16_t	wNumFormated;
	uint8_t	bConvResult= 1;
	
	if(wNumber > 9999)
	{
		wNumFormated= 9999;
		bConvResult= 0;
	}
	else
	{
		wNumFormated= wNumber;
	}
	*(pbBuffer)=sbNumAsciiPick(wNumFormated,3);
	*(pbBuffer+1)=sbNumAsciiPick(wNumFormated,2);
	*(pbBuffer+2)=sbNumAsciiPick(wNumFormated,1);
	*(pbBuffer+3)='.';
	*(pbBuffer+4)=sbNumAsciiPick(wNumFormated,0);
	*(pbBuffer+5)= bUnit;
	*(pbBuffer+6)= ' ';
	return(bConvResult);
}


/******************************************************************************
Function Name:
	sbNumToXXxx: Convert input wNumber into 2 digital numbers including
		     2 decimal represented byASCII code.
Parameters:
	wNumber: Number to be converted into ASCII code
	pbBuffer: pointer of conversion register
Internal Variable:
	wNumFormated: Number after formatting
	bConvResult: conversion result
Return Value:
	1:1 ( The Number to be converted is normal )
	0:	0 ( The Number to be converted is over 99.99, and limited to 99.99 )
*******************************************************************************/
uint8_t sbNumToXXxx(uint16_t wNumber, uint8_t *pbBuffer)
{
	uint16_t	wNumFormated;
	uint8_t	bConvResult= 1;
	
    	if(wNumber > 9999)
    	{
    		wNumFormated= 9999;
    		bConvResult=0;
    	}
    	else
    	{
    		wNumFormated= wNumber;
    	}
    	*(pbBuffer)=sbNumAsciiPick(wNumFormated,3);
    	*(pbBuffer+1)=sbNumAsciiPick(wNumFormated,2);
    	*(pbBuffer+2)='.';
    	*(pbBuffer+3)=sbNumAsciiPick(wNumFormated,1);
    	*(pbBuffer+4)=sbNumAsciiPick(wNumFormated,0);
    	*(pbBuffer+5)= ' ';
    	return(bConvResult);
}

uint8_t sbNumToXXxxUnit(uint16_t wNumber, uint8_t *pbBuffer, uint8_t bUnit)
{
	uint16_t	wNumFormated;
	uint8_t	bConvResult= 1;
	
	if(wNumber > 9999)
	{
		wNumFormated= 9999;
		bConvResult=0;
	}
	else
	{
		wNumFormated= wNumber;
	}
	*(pbBuffer)=sbNumAsciiPick(wNumFormated,3);
	*(pbBuffer+1)=sbNumAsciiPick(wNumFormated,2);
	*(pbBuffer+2)='.';
	*(pbBuffer+3)=sbNumAsciiPick(wNumFormated,1);
	*(pbBuffer+4)=sbNumAsciiPick(wNumFormated,0);
	*(pbBuffer+5)= bUnit;
	*(pbBuffer+6)= ' ';
	return(bConvResult);
}


/******************************************************************************
Function Name:
	sbNumToXXXxx: Convert input wNumber into 3 digital numbers including
		      2 decimal represented byASCII code.
Parameters:
	wNumber: Number to be converted into ASCII code
	pbBuffer: pointer of conversion register
Internal Variable:
	bConvResult: conversion result
Return Value:
	1:1 ( The Number to be converted is normal )
*******************************************************************************/
uint8_t sbNumToXXXxx(uint16_t wNumber, uint8_t *pbBuffer)
{
	uint8_t	bConvResult= 1;
	
    	*(pbBuffer)=sbNumAsciiPick(wNumber,4);
    	*(pbBuffer+1)=sbNumAsciiPick(wNumber,3);
    	*(pbBuffer+2)=sbNumAsciiPick(wNumber,2);
    	*(pbBuffer+3)='.';
    	*(pbBuffer+4)=sbNumAsciiPick(wNumber,1);
    	*(pbBuffer+5)=sbNumAsciiPick(wNumber,0);
    	*(pbBuffer+6)= ' ';
    	return(bConvResult);
}

uint8_t sbNumToXXxxx(uint16_t wNumber, uint8_t *pbBuffer)
{
	uint8_t	bConvResult= 1;
	
    	*(pbBuffer)=sbNumAsciiPick(wNumber,4);
    	*(pbBuffer+1)=sbNumAsciiPick(wNumber,3);
    	*(pbBuffer+2)='.';
		*(pbBuffer+3)=sbNumAsciiPick(wNumber,2);
    	*(pbBuffer+4)=sbNumAsciiPick(wNumber,1);
    	*(pbBuffer+5)=sbNumAsciiPick(wNumber,0);
    	*(pbBuffer+6)= ' ';
    	return(bConvResult);
}



uint8_t sbNumToXxxxx(uint16_t wNumber, uint8_t *pbBuffer)
{
	uint8_t	bConvResult= 1;
	
    	*(pbBuffer)=sbNumAsciiPick(wNumber,4);
    	*(pbBuffer+1)='.';
    	*(pbBuffer+2)=sbNumAsciiPick(wNumber,3);
    	*(pbBuffer+3)=sbNumAsciiPick(wNumber,2);
    	*(pbBuffer+4)=sbNumAsciiPick(wNumber,1);
    	*(pbBuffer+5)=sbNumAsciiPick(wNumber,0);
    	*(pbBuffer+6)= ' ';
    	return(bConvResult);
}


/******************************************************************************
Function Name:
	sbNumAsciiPick: Convert the secleced digital to Ascii code
Parameters:
	wNumber: Number to be converted into ASCII code
	bPosition:
Return Value:
	bCode
*******************************************************************************/
uint8_t	sbNumAsciiPick(uint16_t wNumber, uint8_t bPosition)
{
	uint8_t	bCode;
	switch( bPosition)
	{
		case 0: default:
			bCode= wNumber%10 + '0';
			break;
		case 1:
			bCode= (wNumber/10)%10 + '0';
			break;
		case 2:
			bCode= (wNumber/100)%10 + '0';
			break;
		case 3:
			bCode= (wNumber/1000)%10 + '0';
			break;
		case 4:
			bCode= (wNumber/10000)%10 + '0';
			break;
		case 5:
			bCode= (wNumber/100000)%10 + '0';
			break;
	}
	return(bCode);
}

/******************************************************************************
Function Name:
	sbNumToXXXX: Convert input wNumber into 4 digital numbers represented by
		    ASCII code.
Parameters:
	wNumber: Number to be converted into ASCII code
	pbBuffer: pointer of conversion register
Internal Variable:
	wNumFormated: Number after formatting
	bConvResult: conversion result
Return Value:
	1:1 ( The Number to be converted is normal )
	0:	0 ( The Number to be converted is over 999, and limited to 999 )
*******************************************************************************/
uint8_t sbNumToXXXX(uint16_t wNumber, uint8_t *pbBuffer)
{
	uint16_t	wNumFormated;
	uint8_t	bConvResult= 1;
	
    	if(wNumber > 9999)
    	{
    		wNumFormated= 9999;
    		bConvResult= 0;
    	}
    	else
    	{
    		wNumFormated= wNumber;
    	}
    	*(pbBuffer)=sbNumAsciiPick(wNumFormated,3);
    	*(pbBuffer+1)=sbNumAsciiPick(wNumFormated,2);
    	*(pbBuffer+2)=sbNumAsciiPick(wNumFormated,1);
		*(pbBuffer+3)=sbNumAsciiPick(wNumFormated,0);
    	*(pbBuffer+4)= ' ';
    	return(bConvResult);
}

uint8_t sbNumToXXXXX(uint16_t wNumber, uint8_t *pbBuffer)
{
    	uint8_t	bConvResult= 1;
    	*(pbBuffer)=sbNumAsciiPick(wNumber,4);
    	*(pbBuffer+1)=sbNumAsciiPick(wNumber,3);
    	*(pbBuffer+2)=sbNumAsciiPick(wNumber,2);
		*(pbBuffer+3)=sbNumAsciiPick(wNumber,1);
		*(pbBuffer+4)=sbNumAsciiPick(wNumber,0);
    	*(pbBuffer+5)= ' ';
    	return(bConvResult);
}
#endif


/************************************************************************************
*Function name: sQInit                                                              *
*Parameters:    pq: pointer to queue structure to be initialized                    *
*               start:start address of ring buffer                                  *
*               size:the size of the ring buffer                                    *
*               QueueType:Type of the address point in queue                        *
*Description:   initialize a queue structure                                        *
*************************************************************************************/
void    sQInit(QUEUE *pq,void *start,uint16_t size,uint16_t AddrStep)
{
    pq->pIn = start;
    pq->pOut = start;
    pq->pStart = start;
    pq->length = 0;
    pq->size = size;
    pq->step = AddrStep;
}

/************************************************************************************
*Function name: sQDataIn                                                            *
*Parameters:    pq: pointer to queue structure to be initialized                    *
*               pdata:the data point to be inserted into the queue                  *
*               option:how to deal with the data when the buffer is full            *
*               cQCoverFirst:cover the first data                                   *
*               cQCoverLast:cover the latest data                                   *
*Returns:       cQBufNormal:data has been inserted into the queue                   *
*               cQBufFull:the buffer is full                                        *
*Description:   insert a data into the queue                                        *
*************************************************************************************/
uint8_t sQDataIn(QUEUE  *pq,void *pdata,uint8_t option)
{

    uint16_t uwAddrStep,uwCnt;
    uint8_t *pubTemp;

    uwAddrStep = pq->step;
    if(pq->length == pq->size)
    {
        if(option == cQCoverFirst)
        {
            //*(pq->pIn) = data;
            for(uwCnt = 0;uwCnt < uwAddrStep;uwCnt++)
            {
                *((uint8_t *)pq->pIn + uwCnt) = *((uint8_t *)pdata + uwCnt);
            }
            if((uint8_t *)pq->pIn == ((uint8_t *)pq->pStart + (pq->size-1) * uwAddrStep))
            {
                pq->pIn = pq->pStart;
            }
            else
            {
                pq->pIn = (uint8_t *)pq->pIn + uwAddrStep;
            }
            pq->pOut = pq->pIn;
        }
        else if(option == cQCoverLast)
        {
            if(pq->pIn == pq->pStart)
            {
                //*(pq->pStart + (pq->size - 1)) = data;
                pubTemp = (uint8_t *)pq->pStart + (pq->size - 1) * uwAddrStep;
                for(uwCnt = 0;uwCnt < uwAddrStep;uwCnt++)
                {
                    *(pubTemp+ uwCnt) = *((uint8_t *)pdata + uwCnt);
                }
            }
            else
            {
                //*(pq->pIn - 1) = data;
                pubTemp = (uint8_t *)pq->pIn - uwAddrStep;
                for(uwCnt = 0;uwCnt < uwAddrStep;uwCnt++)
                {
                    *(pubTemp+ uwCnt) = *((uint8_t *)pdata + uwCnt);
                }
            }
        }
        return(cQBufFull);
    }
    else
    {
        //*(pq->pIn) = data;
        for(uwCnt = 0;uwCnt < uwAddrStep;uwCnt++)
        {
            *((uint8_t *)pq->pIn + uwCnt) = *((uint8_t *)pdata + uwCnt);
        }
        pq->length++;
        if((uint8_t *)pq->pIn == (uint8_t *)pq->pStart + (pq->size - 1) * uwAddrStep)
        {
            pq->pIn = pq->pStart;
        }
        else
        {
            pq->pIn = (uint8_t *)pq->pIn + uwAddrStep;
        }
        return(cQBufNormal);
    }
}
/************************************************************************************
*Function name: sQDataOut                                                           *
*Parameters:    pq: pointer to queue structure to be initialized                    *
*               pdata:the address to save the data                                  *
*Returns:       cQBufNormal:data has been inserted into the queue                   *
*               cQBufEmpty:the buffer is empty                                      *
*Description:   Get a data from the queue                                           *
*************************************************************************************/
uint8_t sQDataOut(QUEUE *pq,void *pdata)
{
    uint16_t uwAddrStep,uwCnt;

    uwAddrStep = pq->step;
    if(pq->length == 0)
    {
        return(cQBufEmpty);
    }
    //*pdata = *pq->pOut;
    for(uwCnt = 0;uwCnt < uwAddrStep;uwCnt++)
    {
        *(((uint8_t *)pdata + uwCnt)) = *(((uint8_t *)(pq->pOut) + uwCnt));
    }

    pq->length--;

    if((uint8_t *)pq->pOut == ((uint8_t *)pq->pStart + (pq->size- 1) * uwAddrStep))
    {
        pq->pOut = pq->pStart;
    }
    else
    {
        pq->pOut = (uint8_t *)pq->pOut + uwAddrStep;
    }
    return(cQBufNormal);
}


/******************************************************************************/
/*tx a byte and enable tx interrupt                                           */
/******************************************************************************/
void    sSciTxData0(uint8_t bData)
{
    //LL_USART_ClearFlag_TC(USART2);
    m485_TX1_EN();
    mSciAEnableTxInt();
    mSciATxData(bData);
}

void    sSciTxData1(uint8_t bData)
{
    m485_TX2_EN();
    mSciBEnableTxInt();
    mSciBTxData(bData);
}

void    sSciTxData2(uint8_t bData)
{
    m485_TX3_EN();
    mSciCEnableTxInt();
    mSciCTxData(bData);
}


/******************************************************************************/
/*Get Sci Tx status                                                           */
/******************************************************************************/
uint8_t sbGetSciTxRdy0(void)
{
    if(SCIA_TX_RDY_STATUS == 1)
    {
        return(cSciTxRdy);
    }
    else
    {
        return(cSciTxBusy);
    }
}

uint8_t sbGetSciTxRdy1(void)
{
    if(SCIB_TX_RDY_STATUS == 1)
    {
        return(cSciTxRdy);
    }
    else
    {
        return(cSciTxBusy);
    }
}

uint8_t sbGetSciTxRdy2(void)
{
    if(SCIC_TX_RDY_STATUS == 1)
    {
        return(cSciTxRdy);
    }
    else
    {
        return(cSciTxBusy);
    }
}


/******************************************************************************/
/*Get Sci Rx ready status                                                     */
/******************************************************************************/
uint8_t sbGetSciRxRdy0(void)
{
    if(SCIA_RX_RDY_STATUS == 1)
    {
        return(cSciRxRdy);
    }
    else
    {
        return(cSciRxBusy);
    }
}

uint8_t sbGetSciRxRdy1(void)
{
    if(SCIB_RX_RDY_STATUS == 1)
    {
        return(cSciRxRdy);
    }
    else
    {
        return(cSciRxBusy);
    }
}

uint8_t sbGetSciRxRdy2(void)
{
    if(SCIC_RX_RDY_STATUS == 1)
    {
        return(cSciRxRdy);
    }
    else
    {
        return(cSciRxBusy);
    }
}

/******************************************************************************/
/*Get Sci Rx Data                                                             */
/******************************************************************************/
uint8_t sbGetSciRxData0(void)
{
    return(SCIA_RX_DATA);
}

uint8_t sbGetSciRxData1(void)
{
    return(SCIB_RX_DATA);
}

uint8_t sbGetSciRxData2(void)
{
    return(SCIC_RX_DATA);
}


/*******************************************************************************
Function name:  sbSciGetRxData()
Description:    Get data from SCI register
Calls:
Called By:
Parameters: bSciID: the number of sci port
Return:     return data be received
*******************************************************************************/
uint8_t sbSciGetRxData(uint8_t bSciID)
{
    switch(bSciID)
    {
        case 0:
            return(sbGetSciRxData0());
            //break;
        case 1:
            return(sbGetSciRxData1());
            //break;
        case 2:
            return(sbGetSciRxData2());
            //break;
        default:
            break;
    }
    return(0);
}

/*******************************************************************************
Function name:  sbSciGetRxRdy()
Description:    Get state of RXD from SCI register
Calls:
Called By:
Parameters: bSciID: the number of sci port
Return:     return state value of RXD
*******************************************************************************/
uint8_t sbSciGetRxRdy(uint8_t bSciID)
{
    switch(bSciID)
    {
        case 0:
            return(sbGetSciRxRdy0());
            //break;
        case 1:
            return(sbGetSciRxRdy1());
            //break;
        case 2:
            return(sbGetSciRxRdy2());
            //break;
        default:
            break;
    }
    return(0);
}

/*******************************************************************************
Function name:  sbSciGetTxRdy()
Description:    Get state of TXD from SCI register
Calls:
Called By:
Parameters: bSciID: the number of sci port
Return:     return state value of TXD
*******************************************************************************/
uint8_t sbSciGetTxRdy(uint8_t bSciID)
{
    switch(bSciID)
    {
        case 0:
            return(sbGetSciTxRdy0());
            //break;
        case 1:
            return(sbGetSciTxRdy1());
            //break;
        case 2:
            return(sbGetSciTxRdy2());
            //break;
        default:
            break;
    }
    return(0);
}

/*******************************************************************************
Function name:  sSciTxData()
Description:    write data to SCI register
Calls:
Called By:
Parameters: bSciID: the number of sci port
            bData: the data-byte to be transimitted
Return:     null
*******************************************************************************/
void    sSciTxData(uint8_t bSciID,uint8_t bData)
{
    switch(bSciID)
    {
        case 0:
            sSciTxData0(bData);
            break;
        case 1:
            sSciTxData1(bData);
            break;
        case 2:
            sSciTxData2(bData);
            break;
        default:
            break;
    }
}

