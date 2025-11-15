/*
 * Copyright (C) 2024, Phytium Technology Co., Ltd.   All Rights Reserved.
 *
 * Licensed under the BSD 3-Clause License (the "License"); you may not use
 * this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 * 
 * FilePath: fuart_msg_intr.c
 * Created Date: 2024-10-10 19:18:01
 * Last Modified: 2025-06-09 16:52:57
 * Description:  This file is for
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 */

/***************************** Include Files *********************************/
#include "fuart_msg.h"
#include "fuart_msg_hw.h"
#include "fdebug.h"
/************************** Constant Definitions *****************************/

#define FUART_DEBUG_TAG "FUART-MSG-INTR"
#define FUART_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FUART_DEBUG_TAG, format, ##__VA_ARGS__)
#define FUART_WARN(format, ...) FT_DEBUG_PRINT_W(FUART_DEBUG_TAG, format, ##__VA_ARGS__)
#define FUART_INFO(format, ...) FT_DEBUG_PRINT_I(FUART_DEBUG_TAG, format, ##__VA_ARGS__)
#define FUART_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FUART_DEBUG_TAG, format, ##__VA_ARGS__)

/***************** Macros (Inline Functions) Definitions *********************/

/*************************TEMP************************************/
/*****************************************************************/
static inline unsigned int FUartMsgRead(uintptr addr, u32 reg)
{
    return FUART_MSG_READREG32(addr, reg);
}

static inline void FUartMsgWrite(uintptr addr, u32 reg, u32 value)
{
    FUART_MSG_WRITEREG32(addr, reg, value);
    return;
}

/************************** Function Definitions *****************************/

extern u32 FUartMsgSendBuffer(FUartMsg *uart_p);
extern u32 FUartMsgReceiveBuffer(FUartMsg *uart_p);

static void FUartMsgReceiveDataHandler(FUartMsg *uart_p);
static void FUartMsgSendDataHandler(FUartMsg *uart_p, u32 InterruptStatus);

/************************** Function Prototypes ******************************/
/*****************************************************************************/

/**
 * @name: FUartMsgSetHandler
 * @msg:  This function sets the handler that will be called when an event (interrupt)
 * occurs that needs application's attention.
 * @param uart_p is a pointer to the uart instance
 * @param fun_p is the pointer to the callback function.
 * @param args is the upper layer callback reference passed back
 *      when the callback function is invoked.
 * @return {*}
 */
void FUartMsgSetHandler(FUartMsg *uart_p, FUartMsgEventHandler fun_p, void *args)
{
    FASSERT(uart_p != NULL);
    FASSERT(fun_p != NULL);
    FASSERT(uart_p->is_ready == FT_COMPONENT_IS_READY);

    uart_p->handler = fun_p;
    uart_p->args = args;
}


/**
 * @name: FUartMsgInterruptHandler
 * @msg:  This function is the interrupt handler for the driver.
 *          It must be connected to an interrupt system by the application such that it
 *          can be called when an interrupt occurs.
 * @param vector Irq num ,Don't need attention .
 * @param param  contains a pointer to the driver instance
 */
void FUartMsgInterruptHandler(s32 vector, void *param)
{
    FUartMsg *uart_p = (FUartMsg *)param;
    u32 reg_file_value = 0;
    /* u32 modem_status = 0; */

    FASSERT(uart_p != NULL);
    FASSERT(uart_p->is_ready == FT_COMPONENT_IS_READY);

    reg_file_value = FUartMsgRead(uart_p->config.msg.regfile, FMSG_S2M_INT_STATE);
    /* Clear the interrupt status. */
    FUartMsgInterruptClearAll(uart_p);
    if ((reg_file_value & ((u32)RX_TAIL_INT)) != (u32)0)
    {
        /* Received data interrupt */
        FUartMsgReceiveDataHandler(uart_p);
    }

    if ((reg_file_value & ((u32)M2S_MSG_DATA_COMPLETED)) != (u32)0)
    {
        /* Transmit data interrupt */
        FUartMsgSendDataHandler(uart_p, reg_file_value);
    }
    /*
    if (((reg_file_value) & ((u32)MODEM_INT)) != (u32)0)
    {
        modem_status = FUartMsgModemStatus(&(uart_p->config.msg));
        if (uart_p->handler)
        {
            uart_p->handler(uart_p->args, FUART_EVENT_MODEM, modem_status);
        }
    }

    if ((reg_file_value & ((u32)TX_HEAD_INT)) != (u32)0)
    {
        
    }
    */

    return;
}

/**
 * @name: FUartMsgReceiveDataHandler
 * @msg:
 * @param {*}
 * @return {*}
 */
static void FUartMsgReceiveDataHandler(FUartMsg *uart_p)
{
    u32 received_count = 0;
    if ((u32)0 != uart_p->receive_buffer.remaining_bytes)
    {
        received_count = FUartMsgReceiveBuffer(uart_p);
    }
    if ((u32)0 == uart_p->receive_buffer.remaining_bytes || received_count > 0)
    {
        if (uart_p->handler)
        {
            uart_p->handler(uart_p->args, FUART_EVENT_RECV_DATA,
                            uart_p->receive_buffer.requested_bytes -
                                uart_p->receive_buffer.remaining_bytes);
        }
    }
}


static void FUartMsgSendDataHandler(FUartMsg *uart_p, u32 InterruptStatus)
{
    FUartMsgSendBuffer(uart_p);
    if (uart_p->send_buffer.remaining_bytes == (u32)0)
    {
        if (uart_p->handler)
        {
            uart_p->handler(uart_p->args, FUART_EVENT_SENT_DATA, uart_p->send_buffer.requested_bytes);
        }
    }
}


void FUartMsgEnableInterrups(FUartMsg *uart_p)
{
    FASSERT(uart_p != NULL);

    u32 temp_mask = 0;
    u32 i;
    u32 status;
    u16 rx_head, rx_tail;

    u32 buffer_size = RX_BUFFER_SIZE;

    /* disable RX_TAIL_INT interrupt*/
    status = FUartMsgRead(uart_p->config.msg.regfile, FMSG_S2M_INT_STATE);
    FUartMsgWrite(uart_p->config.msg.regfile, FMSG_S2M_INT_MASK, status & ~RX_TAIL_INT);

    for (i = 0; i < buffer_size * 2; i++)
    {
        if (FUartMsgRxRingBufferIsEmpty(uart_p->config.msg.regfile))
        {
            break;
        }
        rx_head = FUartMsgRead(uart_p->config.msg.regfile, FMSG_RX_HEAD) & BUFFER_POINTER_MASK;
        rx_head = (rx_head + 1) & BUFFER_POINTER_MASK;
        FUartMsgWrite(uart_p->config.msg.regfile, FMSG_RX_HEAD, rx_head);
    }

    /* enable RX_TAIL interrupt*/
    rx_tail = FUartMsgRead(uart_p->config.msg.regfile, FMSG_RX_TAIL);
    FUartMsgWrite(uart_p->config.msg.regfile, FMSG_RX_TAIL, rx_tail | RX_TAIL_INT_ENABLE);
    /* unmask RX_TAIL interrupt*/
    temp_mask = FUartMsgRead(uart_p->config.msg.regfile, FMSG_S2M_INT_MASK);
    FUartMsgWrite(uart_p->config.msg.regfile, FMSG_S2M_INT_MASK,
                  temp_mask & ~(RX_TAIL_INT | TX_HEAD_INT | M2S_MSG_DATA_COMPLETED | MODEM_INT));

    return;
}

/**
 * @name: FUartMsgInterruptDisable
 * @msg:  This function disables the specified interrupt.
 * @param uart_p is a pointer to the uart instance
 * @param interrupt is the interrupt to be disabled.
 */
void FUartMsgInterruptDisable(FUartMsg *uart_p)
{

    FASSERT(uart_p != NULL);
    FUartMsgInfo msgInfo;
    /* reset uart */
    FUartMsgFill(&msgInfo, UART_MODULE_ID, MSG_SET, MSG_DEFAULT_SUBID, 0);
    FM2sMsgInsert(&(uart_p->config.msg), &msgInfo);

    /* clear all s2m interrupts status */
    FUartMsgWrite(uart_p->config.msg.regfile, FMSG_S2M_INT_STATE, 0x0);
    FUartMsgWrite(uart_p->config.msg.regfile, FMSG_S2M_INT_CLEAN, 0xffff);
    return;
}

/**
 * @name: FUartMsgGetInterruptMask
 * @msg:  This function gets the interrupt mask.
 * @param uart_p is a pointer to the uart instance
 * @return {*}
 */
u32 FUartMsgGetInterruptMask(FUartMsg *uart_p)
{
    FASSERT(uart_p != NULL);
    return FUartMsgRead(uart_p->config.msg.regfile, FMSG_S2M_INT_MASK);
}


/**
 * @name: FUartMsgSetInterruptMask
 * @msg:  This function sets the interrupt mask.
 * @param uart_p is a pointer to the uart instance
 * @param mask contains the interrupts to be enabled or disabled.
 *      A '1' enables an interrupt, and a '0' disables.
 */
void FUartMsgSetInterruptMask(FUartMsg *uart_p, u32 mask)
{
    u32 temp_mask = mask;
    FASSERT(uart_p != NULL);

    temp_mask &= INTR_MASK_ALL;

    FUartMsgWrite(uart_p->config.msg.regfile, FMSG_S2M_INT_MASK, temp_mask);
}

/**
 * @name: FUartMsgInterruptClear
 * @msg:  This function clears all interrupt state.
 * @param uart_p is a pointer to the uart instance
 */
void FUartMsgInterruptClearAll(FUartMsg *uart_p)
{
    FASSERT(uart_p != NULL);

    /* clear all s2m interrupts status */
    FUartMsgWrite(uart_p->config.msg.regfile, FMSG_S2M_INT_STATE, 0x0);
    FUartMsgWrite(uart_p->config.msg.regfile, FMSG_S2M_INT_CLEAN, 0xffff);
}