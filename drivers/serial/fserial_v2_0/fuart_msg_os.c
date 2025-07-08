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
 * FilePath: fuart_msg_os.c
 * Date: 2025-04-24 13:42:19
 * LastEditTime: 2025-05-13 16:59:51
 * Description:  This file is for required function implementations of serial_v2 driver used in FreeRTOS.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   liyilun   2025/05/13   first commit
 */

#include <stdio.h>
#include "fuart_msg.h"
#include "fuart_msg_hw.h"
#include "finterrupt.h"
#include "ftypes.h"
#include "fassert.h"
#include "fdebug.h"
#include "sdkconfig.h"
#include "fcpu_info.h"
#include "fuart_msg_os.h"
#include "fio_mux.h"

#define FUART_MSG_DEBUG_TAG "FFreeRTOSUARTMSG"
#define FUART_MSG_ERROR(format, ...) FT_DEBUG_PRINT_E(FUART_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
#define FUART_MSG_WARN(format, ...) FT_DEBUG_PRINT_W(FUART_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
#define FUART_MSG_INFO(format, ...) FT_DEBUG_PRINT_I(FUART_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
#define FUART_MSG_DEBUG(format, ...) FT_DEBUG_PRINT_D(FUART_MSG_DEBUG_TAG, format, ##__VA_ARGS__)

static void FtFreeRtosUartMsgCallback(void *args, u32 event, u32 event_data)
{
    FtFreertosUartMsg *uart_msg_p = (FtFreertosUartMsg *)args;
    BaseType_t xhigher_priority_task_woken = pdFALSE;
    BaseType_t x_result = pdFALSE;
    if (event == FUART_EVENT_RECV_DATA)
    {
        x_result = xEventGroupSetBitsFromISR(uart_msg_p->rx_event, RTOS_UART_RECEIVE_COMPLETE, &xhigher_priority_task_woken);
    }
    else if (event == FUART_EVENT_SENT_DATA)
    {
        x_result = xEventGroupSetBitsFromISR(uart_msg_p->tx_event, RTOS_UART_SEND_COMPLETE, &xhigher_priority_task_woken);
    }
    if (x_result != pdFAIL)
    {
        portYIELD_FROM_ISR(xhigher_priority_task_woken);
    }
}

void FtFreertosUartMsgInit(FtFreertosUartMsg *uart_msg_p, FtFreertosUartMsgConfig *config_p)
{
    FUartMsg *bsp_uart_msg_p = NULL;
    FError ret;
    u32 intr_mask;
    u32 cpu_id = 0;
    FUartMsgConfig driver_config;
    FASSERT(uart_msg_p != NULL);
    FASSERT(config_p != NULL);
    bsp_uart_msg_p = &uart_msg_p->bsp_uart_msg;
    uart_msg_p->config = *config_p;
    driver_config = *FUartMsgLookupConfig(config_p->uart_msg_instance);

    FIOMuxInit();
    FIOPadSetUartMux(config_p->uart_msg_instance);

    driver_config.baudrate = config_p->uart_baudrate;
    ret = FUartMsgCfgInitialize(bsp_uart_msg_p, &driver_config);
    FASSERT(FT_SUCCESS == ret);

    FUartMsgSetStartUp(bsp_uart_msg_p);

    FUartMsgSetHandler(bsp_uart_msg_p, FtFreeRtosUartMsgCallback, uart_msg_p);
    FASSERT((uart_msg_p->rx_semaphore = xSemaphoreCreateMutex()) != NULL);
    FASSERT((uart_msg_p->tx_semaphore = xSemaphoreCreateMutex()) != NULL);
    FASSERT((uart_msg_p->tx_event = xEventGroupCreate()) != NULL);
    FASSERT((uart_msg_p->rx_event = xEventGroupCreate()) != NULL);

    GetCpuId(&cpu_id);
    InterruptSetTargetCpus(bsp_uart_msg_p->config.irq_num, cpu_id);

    InterruptSetPriority(bsp_uart_msg_p->config.irq_num, config_p->isr_priority);
    InterruptInstall(bsp_uart_msg_p->config.irq_num, (IrqHandler)FUartMsgInterruptHandler, bsp_uart_msg_p, "uart");

    InterruptUmask(bsp_uart_msg_p->config.irq_num);
    FUartMsgEnableInterrups(bsp_uart_msg_p);
}
#ifdef CONFIG_LETTER_SHELL_UART_MSG_INTR_MODE
FError FtFreertosUartMsgReceiveBuffer(FtFreertosUartMsg *uart_msg_p, u8 *buffer, u32 length, u32 *received_length)
{
    u32 get_length;
    FError ret = FT_SUCCESS;
    FUartMsg *bsp_uart_msg_p = NULL;
    EventBits_t ev;
    FASSERT(NULL != uart_msg_p);
    FASSERT(NULL != buffer);
    bsp_uart_msg_p = &uart_msg_p->bsp_uart_msg;

    if (length == 0)
    {
        *received_length = 0;
        return FT_SUCCESS;
    }
    /* New transfer can be performed only after current one is finished */
    if (pdFALSE == xSemaphoreTake(uart_msg_p->rx_semaphore, portMAX_DELAY))
    {
        /* We could not take the semaphore, exit with 0 data received */
        ret = FREERTOS_UART_SEM_ERROR;
    }
    if (uart_msg_p->config.isr_event_mask & RTOS_RX_INTR_EVENT_MASK)
    {
        get_length = FUartMsgReceive(bsp_uart_msg_p, buffer, length);
        if (get_length > 0)
        {
            *received_length = get_length;
            ret = FT_SUCCESS;
        }
        else
        {
            ev = xEventGroupWaitBits(uart_msg_p->rx_event, RTOS_UART_RECEIVE_COMPLETE | RTOS_UART_SEND_COMPLETE,
                                     pdTRUE, pdFALSE, portMAX_DELAY);

            if (ev & (RTOS_UART_RECEIVE_COMPLETE | RTOS_UART_SEND_COMPLETE))
            {
                ret = FT_SUCCESS;
                *received_length = bsp_uart_msg_p->receive_buffer.requested_bytes - bsp_uart_msg_p->receive_buffer.remaining_bytes;
            }
            else
            {
                ret = FREERTOS_UART_RECV_ERROR;
                *received_length = 0;
            }
        }
    }
    /* Enable next transfer. Current one is finished */
    if (pdFALSE == xSemaphoreGive(uart_msg_p->rx_semaphore))
    {
        /* We could not post the semaphore, exit with error */
        FUART_MSG_ERROR("FST_FAILURE xSemaphoreGive.");
        ret = FREERTOS_UART_SEM_ERROR;
    }
    return ret;
}
#endif

#ifdef CONFIG_LETTER_SHELL_UART_MSG_POLLED_MODE

FError FtFreertosUartMsgReceiveBuffer(FtFreertosUartMsg *uart_msg_p, u8 *buffer, u32 length, u32 *received_length)
{
    u32 get_length;
    FError ret = FT_SUCCESS;
    FUartMsg *bsp_uart_msg_p = NULL;
    FASSERT(NULL != uart_msg_p);
    FASSERT(NULL != buffer);
    bsp_uart_msg_p = &uart_msg_p->bsp_uart_msg;

    if (length == 0)
    {
        *received_length = 0;
        return FT_SUCCESS;
    }
    get_length = FUartMsgReceive(bsp_uart_msg_p, buffer, length);
    if (get_length > 0)
    {
        *received_length = get_length;
        ret = FT_SUCCESS;
    }
    else
    {
        *received_length = 0;
        ret = FREERTOS_UART_RECV_ERROR;
    }

    return ret;
}

#endif