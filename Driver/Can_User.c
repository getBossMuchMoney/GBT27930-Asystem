/*******************************************************************************
 * File Name: CAN_User.c
 * Description: The CMPSS initialization functions.
 * Designer: Miller Xue
 * Version: 00
 * Copyright: Liyuan Haina Co.Ltd
 *******************************************************************************/

#include "F28x_Project.h"
#include "CAN_User.h"
#include "string.h"
#include "App.h"
#include "Gpio_User.h"


CAN_USER can_opt = {
    .uid = 0,
    .device_id1 = 0,
    .device_id = 0,
    .A_ERR_TX_OK = 0,
    .A_CTRL_TX_OK = 0,
    .A_SET_TX_OK = 0,
    .A_GUERY_TX_OK = 0,
    .B_ERR_TX_OK = 0,
    .B_CTRL_TX_OK = 0,
    .B_SET_TX_OK = 0,
    .B_GUERY_TX_OK = 0
};


void sCanidCheck(void)
{
    uint16_t SwValue = mPhyAddrData();
    if (SwValue & 0x10)
    {
        if((SwValue & 0x0F) < SLAVER1_ID)
        {
            can_opt.device_id = SLAVER1_ID;
        }
        else if((SwValue & 0x0F) > SLAVER12_ID)
        {
            can_opt.device_id = SLAVER12_ID;
        }
        else
        {
            can_opt.device_id = SwValue & 0x0F;
        }

    }
    else
    {
        can_opt.device_id = SLAVER1_ID;
    }
}


void sCanAInit(void)
{
    uint32_t msgID, mask = 0;
    EALLOW;
    GpioCtrlRegs.GPAGMUX2.bit.GPIO30 = 0;   // CANA_RX
    GpioCtrlRegs.GPAMUX2.bit.GPIO30 = 1;

    GpioCtrlRegs.GPAGMUX2.bit.GPIO31 = 0;   // CANA_TX
    GpioCtrlRegs.GPAMUX2.bit.GPIO31 = 1;
    EDIS;


    CAN_initModule(CANA_BASE);
    CAN_setBitRate(CANA_BASE, DEVICE_CLK_FREQ, 250000, 8);
    CAN_enableInterrupt(CANA_BASE, CAN_INT_IE0 | CAN_INT_ERROR | CAN_INT_STATUS);
    CAN_enableGlobalInterrupt(CANA_BASE, CAN_GLOBAL_INT_CANINT0);

    mask = 0x00FF;
    msgID = 0xF4;    //电动汽车充电通信地址

    CAN_MessageObjectSet(CAN_A,
                         CTRL_RX_OBJ,
                         msgID,
                         CAN_MSG_FRAME_EXT,
                         CAN_MSG_OBJ_TYPE_RX,
                         mask,
                         CAN_MSG_OBJ_RX_INT_ENABLE | CAN_MSG_OBJ_USE_EXT_FILTER | CAN_MSG_OBJ_USE_ID_FILTER,
                         MSG_DATA_LENGTH);

    mask = (((uint32_t)0x00FF) << 16) |((uint32_t)0x00FF);
    msgID = (((uint32_t)0X0009) << 16) |(uint32_t)0x3F;

    CAN_MessageObjectSet(CAN_A,
                         BOOT_RX_OBJ,
                         msgID,
                         CAN_MSG_FRAME_EXT,
                         CAN_MSG_OBJ_TYPE_RX,
                         mask,
                         CAN_MSG_OBJ_RX_INT_ENABLE | CAN_MSG_OBJ_USE_EXT_FILTER | CAN_MSG_OBJ_USE_ID_FILTER,
                         MSG_DATA_LENGTH);


    CAN_MessageObjectSet(CAN_A, CTRL_TX_OBJ, 0x1FFFFFFF, CAN_MSG_FRAME_EXT, CAN_MSG_OBJ_TYPE_TX, 0, CAN_MSG_OBJ_TX_INT_ENABLE, MSG_DATA_LENGTH);

    CAN_startModule(CANA_BASE);
    CAN_enableAutoBusOn(CANA_BASE);
}



void CAN_MessageObjectSet(CAN_MODULE mod, uint32_t objID, uint32_t msgID, CAN_MsgFrameType frame,
                          CAN_MsgObjType msgType, uint32_t msgIDMask, uint32_t flags, uint16_t msgLen)
{
    uint32_t cmdMaskReg = 0U;
    uint32_t maskReg = 0U;
    uint32_t arbReg = 0U;
    uint32_t msgCtrl = 0U;

    if (mod == CAN_A)
    {
        // Wait for busy bit to clear
        while (1 == CanaRegs.CAN_IF1CMD.bit.Busy);
    }
    else
    {
        // Wait for busy bit to clear
//        while (1 == CanbRegs.CAN_IF1CMD.bit.Busy);
    }

    switch (msgType)
    {
    // Transmit message object.
    case CAN_MSG_OBJ_TYPE_TX:
    {
        // Set message direction to transmit.
        arbReg = CAN_IF1ARB_DIR;
        break;
    }

    // Remote frame receive remote, with auto-transmit message object.
    case CAN_MSG_OBJ_TYPE_RXTX_REMOTE:
    {
        // Set message direction to Tx for remote receivers.
        arbReg = CAN_IF1ARB_DIR;

        // Set this object to auto answer if a matching identifier is seen.
        msgCtrl = (uint32_t)((uint32_t)CAN_IF1MCTL_RMTEN | (uint32_t)CAN_IF1MCTL_UMASK);
        break;
    }

    // Transmit remote request message object (CAN_MSG_OBJ_TYPE_TX_REMOTE)
    // or Receive message object (CAN_MSG_OBJ_TYPE_RX).
    default:
    {
        //
        // Set message direction to read.
        //
        arbReg = 0U;

        break;
    }
    }

    // Set values based on Extended Frame or Standard Frame
    if (frame == CAN_MSG_FRAME_EXT)
    {
        // Configure the Mask Registers for 29 bit Identifier mask.
        if ((flags & CAN_MSG_OBJ_USE_ID_FILTER) == CAN_MSG_OBJ_USE_ID_FILTER)
        {
            maskReg = msgIDMask & CAN_IF1MSK_MSK_M;
        }

        // Set the 29 bit version of the Identifier for this message
        // object. Mark the message as valid and set the extended ID bit.
        arbReg |= (msgID & CAN_IF1ARB_ID_M) | CAN_IF1ARB_MSGVAL | CAN_IF1ARB_XTD;
    }
    else
    {
        // Configure the Mask Registers for 11 bit Identifier mask.
        if ((flags & CAN_MSG_OBJ_USE_ID_FILTER) == CAN_MSG_OBJ_USE_ID_FILTER)
        {
            maskReg = ((msgIDMask << CAN_IF1ARB_STD_ID_S) & CAN_IF1ARB_STD_ID_M);
        }

        // Set the 11 bit version of the Identifier for this message
        // object. The lower 18 bits are set to zero. Mark the message as
        // valid.
        arbReg |= ((msgID << CAN_IF1ARB_STD_ID_S) & CAN_IF1ARB_STD_ID_M) | CAN_IF1ARB_MSGVAL;
    }

    // If the caller wants to filter on the extended ID bit then set it.
    maskReg |= (flags & CAN_MSG_OBJ_USE_EXT_FILTER);

    // The caller wants to filter on the message direction field.
    maskReg |= (flags & CAN_MSG_OBJ_USE_DIR_FILTER);

    // If any filtering is requested, set the UMASK bit to use mask register
    if (((flags & CAN_MSG_OBJ_USE_ID_FILTER) | (flags & CAN_MSG_OBJ_USE_DIR_FILTER) |
         (flags & CAN_MSG_OBJ_USE_EXT_FILTER)) != 0U)
    {
        msgCtrl |= CAN_IF1MCTL_UMASK;
    }

    // Set the data length for the transfers. This is applicable only for
    // Tx mailboxes. For Rx mailboxes, dlc is updated on receving a frame.
    if ((msgType == CAN_MSG_OBJ_TYPE_TX) || (msgType == CAN_MSG_OBJ_TYPE_RXTX_REMOTE))
    {
        msgCtrl |= ((uint32_t)msgLen & CAN_IF1MCTL_DLC_M);
    }


    // If this is a single transfer or the last mailbox of a FIFO, set EOB bit.
    // If this is not the last entry in a FIFO, leave the EOB bit as 0.
    if ((flags & CAN_MSG_OBJ_FIFO) == 0U)
    {
        msgCtrl |= CAN_IF1MCTL_EOB;
    }


    // Enable transmit interrupts if they should be enabled.
    msgCtrl |= (flags & CAN_MSG_OBJ_TX_INT_ENABLE);

    // Enable receive interrupts if they should be enabled.
    msgCtrl |= (flags & CAN_MSG_OBJ_RX_INT_ENABLE);


    // Set the Control, Arb, and Mask bit so that they get transferred to the
    // Message object.
    cmdMaskReg |= CAN_IF1CMD_ARB;
    cmdMaskReg |= CAN_IF1CMD_CONTROL;
    cmdMaskReg |= CAN_IF1CMD_MASK;
    cmdMaskReg |= CAN_IF1CMD_DIR;


    // Write out the registers to program the message object.

    if (mod == CAN_A)
    {
        CanaRegs.CAN_IF1MSK.all = maskReg;
        CanaRegs.CAN_IF1ARB.all = arbReg;
        CanaRegs.CAN_IF1MCTL.all = msgCtrl;
        // Transfer data to message object RAM
        CanaRegs.CAN_IF1CMD.all = cmdMaskReg | (objID & CAN_IF1CMD_MSG_NUM_M);
    }
    else
    {
//        CanbRegs.CAN_IF1MSK.all = maskReg;
//        CanbRegs.CAN_IF1ARB.all = arbReg;
//        CanbRegs.CAN_IF1MCTL.all = msgCtrl;
//        // Transfer data to message object RAM
//        CanbRegs.CAN_IF1CMD.all = cmdMaskReg | (objID & CAN_IF1CMD_MSG_NUM_M);
    }
}
