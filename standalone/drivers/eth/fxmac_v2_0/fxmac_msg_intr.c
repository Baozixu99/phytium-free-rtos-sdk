/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: fxmac_msg_intr.c
 * Date: 2024-10-31 14:46:52
 * LastEditTime: 2024-10-31 14:46:58
 * Description:  This file contains functions related to interrupt handling.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huangjin   2024/10/31    first release
 */

#include "fxmac_msg.h"
#include "fxmac_msg_hw.h"
#include "fassert.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

u32 FXmacMsgGetIrqMask(u32 mask)
{
    u32 value = 0;

    value |= (mask & FXMAC_MSG_INT_TX_COMPLETE) ? FXMAC_MSG_BIT(TXCOMP) : 0;
    value |= (mask & FXMAC_MSG_INT_TX_ERR) ? FXMAC_MSG_BIT(DMA_ERR) : 0;
    value |= (mask & FXMAC_MSG_INT_RX_COMPLETE) ? FXMAC_MSG_BIT(RXCOMP) : 0;
    value |= (mask & FXMAC_MSG_INT_RX_OVERRUN) ? FXMAC_MSG_BIT(RXOVERRUN) : 0;
    value |= (mask & FXMAC_MSG_INT_RX_DESC_FULL) ? FXMAC_MSG_BIT(RUSED) : 0;
    value |= (mask & FXMAC_MSG_INT_LINK) ? FXMAC_MSG_BIT(LINK_CHANGE) : 0;
    value |= (mask & FXMAC_MSG_INT_TX_USED) ? FXMAC_MSG_BIT(TUSED) : 0;

    return value;
}

u32 FXmacMsgGetIrqStatus(u32 value)
{
    u32 status = 0;

    status |= (value & FXMAC_MSG_BIT(TXCOMP)) ? FXMAC_MSG_INT_TX_COMPLETE : 0;
    status |= (value & FXMAC_MSG_BIT(DMA_ERR)) ? FXMAC_MSG_INT_TX_ERR : 0;
    status |= (value & FXMAC_MSG_BIT(RXCOMP)) ? FXMAC_MSG_INT_RX_COMPLETE : 0;
    status |= (value & FXMAC_MSG_BIT(RXOVERRUN)) ? FXMAC_MSG_INT_RX_OVERRUN : 0;
    status |= (value & FXMAC_MSG_BIT(RUSED)) ? FXMAC_MSG_INT_RX_DESC_FULL : 0;
    status |= (value & FXMAC_MSG_BIT(LINK_CHANGE)) ? FXMAC_MSG_INT_LINK : 0;
    status |= (value & FXMAC_MSG_BIT(TUSED)) ? FXMAC_MSG_INT_TX_USED : 0;

    return status;
}

void FXmacMsgEnableIrq(FXmacMsgCtrl *pdata, int queue_index, u32 mask)
{
    u32 value;

    value = FXmacMsgGetIrqMask(mask);
    FXMAC_MSG_WRITE(pdata, FXMAC_MSG_INT_ER(queue_index), value);
}

void FXmacMsgDisableIrq(FXmacMsgCtrl *pdata, int queue_index, u32 mask)
{
    u32 value;

    value = FXmacMsgGetIrqMask(mask);
    FXMAC_MSG_WRITE(pdata, FXMAC_MSG_INT_DR(queue_index), value);
}

void FXmacMsgClearIrq(FXmacMsgCtrl *pdata, int queue_index, u32 mask)
{
    u32 value;

    value = FXmacMsgGetIrqMask(mask);
    FXMAC_MSG_WRITE(pdata, FXMAC_MSG_INT_SR(queue_index), value);
}

unsigned int FXmacMsgGetIrq(FXmacMsgCtrl *pdata, int queue_index)
{
    u32 status;
    u32 value;

    value = FXMAC_MSG_READ(pdata, FXMAC_MSG_INT_SR(queue_index));
    status = FXmacMsgGetIrqStatus(value);

    return status;
}

/**
 * @name: FXmacMsgSetHandler
 * @msg:  Install an asynchronous handler function for the given handler_type:
 *
 * @param instance_p is a pointer to the instance to be worked on.
 * @param handler_type indicates what interrupt handler type is.
 *        FXMAC_MSG_HANDLER_DMASEND, FXMAC_MSG_HANDLER_DMARECV and
 *        FXMAC_MSG_HANDLER_ERROR.
 * @param func_pointer is the pointer to the callback function
 * @param call_back_ref is the upper layer callback reference passed back when
 *        when the callback function is invoked.
 *
 * @return {FError} FT_SUCCESS set is ok
 */
FError FXmacMsgSetHandler(FXmacMsgCtrl *instance_p, u32 handler_type,
                          void *func_pointer, void *call_back_ref)
{
    FError status;
    FASSERT(instance_p != NULL);
    FASSERT(func_pointer != NULL);
    FASSERT(instance_p->is_ready == (u32)FT_COMPONENT_IS_READY);
    status = (FError)(FT_SUCCESS);

    switch (handler_type)
    {
        case FXMAC_MSG_HANDLER_DMASEND:
            instance_p->send_irq_handler = ((FXmacMsgIrqHandler)(void *)func_pointer);
            instance_p->send_args = call_back_ref;
            break;
        case FXMAC_MSG_HANDLER_DMARECV:
            instance_p->recv_irq_handler = ((FXmacMsgIrqHandler)(void *)func_pointer);
            instance_p->recv_args = call_back_ref;
            break;
        case FXMAC_MSG_HANDLER_ERROR:
            instance_p->error_irq_handler = ((FXmacMsgErrorIrqHandler)(void *)func_pointer);
            instance_p->error_args = call_back_ref;
            break;
        case FXMAC_MSG_HANDLER_LINKCHANGE:
            instance_p->link_change_handler = ((FXmacMsgIrqHandler)(void *)func_pointer);
            instance_p->link_change_args = call_back_ref;
            break;
        case FXMAC_MSG_HANDLER_RESTART:
            instance_p->restart_handler = ((FXmacMsgIrqHandler)(void *)func_pointer);
            instance_p->restart_args = call_back_ref;
            break;
        default:
            status = (FError)(FXMAC_MSG_ERR_INVALID_PARAM);
            break;
    }

    return status;
}

/**
 * @name: FXmacMsgIntrHandler
 * @msg:  中断处理函数
 * @param {s32} vector is interrrupt num
 * @param {void} *args is a arguments variables
 * @return {*}
 * @note 目前中断只支持单queue的情况
 */

void FXmacMsgIntrHandler(s32 vector, void *args)
{
    u32 reg_isr;
    u32 tx_queue_id; /* 0 ~ FXMAC_MSG_QUEUE_MAX_NUM ,Index queue number */
    u32 rx_queue_id; /* 0 ~ FXMAC_MSG_QUEUE_MAX_NUM ,Index queue number */
    FXmacMsgCtrl *instance_p = (FXmacMsgCtrl *)args;
    FASSERT(instance_p != NULL);
    FASSERT(instance_p->is_ready == (u32)FT_COMPONENT_IS_READY);

    tx_queue_id = instance_p->queues[0].index;
    rx_queue_id = instance_p->queues[0].index;
    FASSERT((rx_queue_id < FXMAC_MSG_QUEUE_MAX_NUM) && (tx_queue_id < FXMAC_MSG_QUEUE_MAX_NUM))

    /* This ISR will try to handle as many interrupts as it can in a single
	* call. However, in most of the places where the user's error handler
	* is called, this ISR exits because it is expected that the user will
	* reset the device in nearly all instances.
	*/
    reg_isr = FXMAC_MSG_READ(instance_p, FXMAC_MSG_INT_SR(instance_p->queues[0].index));

    /* 发送中断 */
    if ((u32)vector == instance_p->config.queue_irq_num[tx_queue_id])
    {
        if (tx_queue_id == 0)
        {
            /* 发送完成中断 */
            if ((reg_isr & BIT(FXMAC_MSG_TXCOMP_INDEX)) != 0x00000000U)
            {
                if (instance_p->send_irq_handler)
                {
                    instance_p->send_irq_handler(instance_p->send_args);
                }

                if (instance_p->caps & FXMAC_MSG_CAPS_ISR_CLEAR_ON_WRITE)
                {
                    FXMAC_MSG_WRITE(instance_p, FXMAC_MSG_INT_SR(instance_p->queues[0].index),
                                    BIT(FXMAC_MSG_TXCOMP_INDEX));
                }
            }

            /* 链接变化中断 */
            if ((reg_isr & BIT(FXMAC_MSG_LINK_CHANGE_INDEX)) != 0x00000000U)
            {
                if (instance_p->link_change_handler)
                {
                    instance_p->link_change_handler(instance_p->link_change_args);
                }

                if (instance_p->caps & FXMAC_MSG_CAPS_ISR_CLEAR_ON_WRITE)
                {
                    FXMAC_MSG_WRITE(instance_p, FXMAC_MSG_INT_SR(instance_p->queues[0].index),
                                    BIT(FXMAC_MSG_LINK_CHANGE_INDEX));
                }
            }
        }
    }

    /* 接收中断 */
    if ((u32)vector == instance_p->config.queue_irq_num[rx_queue_id])
    {
        if (rx_queue_id == 0)
        {
            /* 接收完成中断 */
            if ((reg_isr & BIT(FXMAC_MSG_RXCOMP_INDEX)) != 0x00000000U)
            {
                if (instance_p->recv_irq_handler)
                {
                    instance_p->recv_irq_handler(instance_p->recv_args);
                }

                if (instance_p->caps & FXMAC_MSG_CAPS_ISR_CLEAR_ON_WRITE)
                {
                    FXMAC_MSG_WRITE(instance_p, FXMAC_MSG_INT_SR(instance_p->queues[0].index),
                                    BIT(FXMAC_MSG_RXCOMP_INDEX));
                }
            }

            /* 接收错误中断​​ */
            if ((reg_isr & BIT(FXMAC_MSG_RUSED_INDEX)) != 0x00000000U)
            {
                if (instance_p->caps & FXMAC_MSG_CAPS_ISR_CLEAR_ON_WRITE)
                {
                    FXMAC_MSG_WRITE(instance_p, FXMAC_MSG_INT_SR(instance_p->queues[0].index),
                                    BIT(FXMAC_MSG_RUSED_INDEX));
                }
            }
        }
    }
}
