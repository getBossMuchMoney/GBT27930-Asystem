/*******************************************************************************
 * File Name: Interrupt.c
 * Description: The interrupt ISR functions.
 * Designer: Miller Xue
 * Version: 00
 * Copyright: Liyuan Haina Co.Ltd
 *******************************************************************************/
#include "PSFBController.h"
#include "App.h"
#include "F28x_Project.h"
#include "Adc_User.h"
#include "Gpio_User.h"
#include "RTOS_APP.h"
#include "RTOS_Define.h"
#include "UartFunction.h"
#include "UartDebug.h"
#include "Can_User.h"
#include "CanFunction.h"
#include "DataProcTask.h"
#include "ParallelTask.h"
#include "ModbusRtu.h"
#include "SuperTask.h"

interrupt void sADCA1_isr(void);
interrupt void sRtosTimerInterruptIsr(void);
interrupt void sSciaRxIsr(void);
interrupt void sSciaTxIsr(void);
interrupt void sScibRxIsr(void);
interrupt void sScibTxIsr(void);
interrupt void CANA_ISR(void);

/*******************************************************************************
 * Function Name: sADCA1_isr
 * Description: The function is for PWM rectifier of PFC controller.
 *******************************************************************************/
#ifdef _FLASH
#pragma CODE_SECTION(sADCA1_isr, ".TI.ramfunc");
#endif
interrupt void sADCA1_isr(void)
{
    sAdcCpuSample();
    sSampelCalculation();
    sPSFBController();

#if cDataBufEn == 1
    sDataLog();
#endif

#if cDebugUartEn
    if(u16_Tx2RxFlag[0] != 0)
    {
        u16_Tx2RxFlag[0]++;
        if(u16_Tx2RxFlag[0] >= 8)  //1ms
        {
            u16_Tx2RxFlag[0] = 0;
            m485_RX1_EN();
        }
    }
#else
    //check_sciTurntoRCV();       //Delay for 485_RX_EN
#endif

    //Check PFC Fault
    if((stSuper.u16_SysMode != cPowerOnMode) && (uModSta.REG.u16_PfcFaultCode != 0))
    {
        sAllPWMOff();
        sRTOSEventSendIsr(cPrioSuper,eSuperFaultEvt);
    }

    //OpRelay On
    if((stSuper.u16_SysMode == cSoftStartMode) && (stSuper.u16_SysStatus.BIT.OpPreCharge == 1))
    {
        if((stSampleCal.f32_Output_V >= (stSampleCal.f32_SC_V - 1.0f)) &&
           (stSampleCal.f32_Output_V <= (stSampleCal.f32_SC_V + 1.0f)))
        {
             if(stSuper.u16_SysStatus.BIT.OpRly == 0)
             {
                 mOpRelay_On();
             }
        }
    }

    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //clear INT1 flag
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}


#pragma CODE_SECTION(sRtosTimerInterruptIsr, ".TI.ramfunc");
interrupt void sRtosTimerInterruptIsr(void)
{
    timercnt++;
	check_sciTurntoRCV();
	sci_tim_check(SCIA);
	sci_tim_check(SCIB);
	sRTOSTimerTicker();

    //Reset the interrupt
//    CpuTimer0Regs.TCR.bit.TIF = 1;
    PieCtrlRegs.PIEACK.bit.ACK1 = 1;
}

interrupt void sSciaRxIsr(void)
{
    if(SciaRegs.SCIRXST.bit.RXERROR)    //soft reset sci module when errors exist
    {
        SciaRegs.SCICTL1.bit.SWRESET = 0;
        SciaRegs.SCICTL1.bit.SWRESET = 1;
    }
    else
    {
      sUartRxISR(SCIA);
      // Enable future SCIA RX (PIE Group 9) interrupts
      SciaRegs.SCIFFRX.bit.RXFFOVRCLR = 1;
    }
    //SciaRegs.SCIFFRX.bit.RXFFINTCLR = 1;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP9;
}

interrupt void sSciaTxIsr(void)
{
    sUartTxISR(SCIA);
    // Enable future SCIA TX (PIE Group 9) interrupts
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP9;
}

interrupt void sScibRxIsr(void)
{
    if(ScibRegs.SCIRXST.bit.RXERROR)    //soft reset sci module when errors exist
    {
        ScibRegs.SCICTL1.bit.SWRESET = 0;
        ScibRegs.SCICTL1.bit.SWRESET = 1;
    }
    else
    {
      sUartRxISR(SCIB);
      // Enable future SCIA RX (PIE Group 9) interrupts
      ScibRegs.SCIFFRX.bit.RXFFOVRCLR = 1;
    }
    //ScibRegs.SCIFFRX.bit.RXFFINTCLR = 1;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP9;
}

interrupt void sScibTxIsr(void)
{
    sUartTxISR(SCIB);
    // Enable future SCIA TX (PIE Group 9) interrupts
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP9;
}

CANID_UNION textid;
interrupt void sCanAIsr(void)
{
    uint32_t status;
    CanPara_OP *pPara = &ParaCan;

    // Read the CAN interrupt status to find the cause of the interrupt
    status = CanaRegs.CAN_INT.all;
    // If the cause is a controller status interrupt, then get the status
    if (status == CAN_INT_INT0ID_STATUS)
    {
        status = CanaRegs.CAN_ES.all;
        // Check to see if an error occurred.
        if (((status & ~(CAN_STATUS_TXOK | CAN_STATUS_RXOK)) != 7) &&
            ((status & ~(CAN_STATUS_TXOK | CAN_STATUS_RXOK)) != 0))
        {
            // Set a flag to indicate some errors may have occurred.
            CanaRegs.CAN_CTL.bit.Init = 1;
            CanaRegs.CAN_CTL.bit.SWR = 1;
        }
    }
    else
    {
        switch(status)
        {
        case ERR_TX_OBJ:
        {
            can_opt.A_ERR_TX_OK = 1;
            CanaRegs.CAN_IF1CMD.all = ((uint32_t)CAN_IF1CMD_CLRINTPND | (ERR_TX_OBJ & CAN_IF1CMD_MSG_NUM_M));

        }break;

        case CTRL_TX_OBJ:
        {
            can_opt.A_CTRL_TX_OK = 1;
            CanaRegs.CAN_IF1CMD.all = ((uint32_t)CAN_IF1CMD_CLRINTPND | (CTRL_TX_OBJ & CAN_IF1CMD_MSG_NUM_M));

        }break;

        case CTRL_RX_OBJ:
        {
            pPara->RXmod = CAN_A;
            pPara->RXobjID = CTRL_RX_OBJ;
            pPara->rxMsgType = CTRL;
            CAN_readMesID(pPara);
            enCanRXQueue();
            CanaRegs.CAN_IF1CMD.all = ((uint32_t)CAN_IF1CMD_CLRINTPND | (CTRL_RX_OBJ & CAN_IF1CMD_MSG_NUM_M));

        }break;

        case BOOT_RX_OBJ:
        {
            pPara->RXmod = CAN_A;
            pPara->RXobjID = BOOT_RX_OBJ;
            CAN_readMesID(pPara);
            sRTOSEventSendIsr(cPrioDataProc, cBootEvt);
            CanaRegs.CAN_IF1CMD.all = ((uint32_t)CAN_IF1CMD_CLRINTPND | (BOOT_RX_OBJ & CAN_IF1CMD_MSG_NUM_M));
        }break;


        default:{}break;
        }
    }

    // Clear the global interrupt flag for the CAN interrupt line
    CanaRegs.CAN_GLB_INT_CLR.all |= 1;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP9;
}


