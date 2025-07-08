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
 * FilePath: fuart_msg_os.h
 * Date: 2025-04-24 13:42:19
 * LastEditTime: 2025-05-13 16:59:51
 * Description:  This file is for required function defination of serial_v2 driver used in FreeRTOS.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   liyilun   2025/05/13   first commit
 */

#ifndef FUART_MSG_OS_H
#define FUART_MSG_OS_H

#include "FreeRTOS.h"
#include "event_groups.h"
#include "semphr.h"
#include "fuart_msg.h"
#include "fuart_msg_hw.h"
#include "ftypes.h"
#include "ferror_code.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define FREERTOS_UART_SEM_ERROR FT_CODE_ERR(ErrModPort, 0, 0x1)
#define FREERTOS_UART_EVENT_ERROR FT_CODE_ERR(ErrModPort, 0, 0x2)
#define FREERTOS_UART_FIFO_ERROR FT_CODE_ERR(ErrModPort, 0, 0x3)
#define FREERTOS_UART_RECV_ERROR FT_CODE_ERR(ErrModPort, 0, 0x4)
#define FREERTOS_UART_INVAILD_PARAM FT_CODE_ERR(ErrModPort, 0, 0x5)

#define UART0_ID FUART0_ID
#define UART1_ID FUART1_ID
#define UART2_ID FUART2_ID
#define UART3_ID FUART3_ID

#define RTOS_RX_INTR_EVENT_MASK             0x1
#define RTOS_TX_INTR_EVENT_MASK             0x2
/*! @brief Event flag - transfer complete. */
#define RTOS_UART_RECEIVE_COMPLETE          0x1
#define RTOS_UART_SEND_COMPLETE             0x2


typedef struct
{
    u32 uart_msg_instance; /* select uart global object */
    u32 isr_priority;  /* irq Priority */
    u32 isr_event_mask; /* followed by RTOS_UART_ISR_XX */
    u32 uart_baudrate;
} FtFreertosUartMsgConfig;

typedef struct
{
    /* Uart Object */
    FUartMsg bsp_uart_msg;
    FtFreertosUartMsgConfig config;
    SemaphoreHandle_t rx_semaphore; /*!< RX semaphore for resource sharing */
    SemaphoreHandle_t tx_semaphore; /*!< TX semaphore for resource sharing */
    EventGroupHandle_t rx_event;    /*!< RX completion event */
    EventGroupHandle_t tx_event;    /*!< TX completion event */
} FtFreertosUartMsg;

void FtFreertosUartMsgInit(FtFreertosUartMsg *uart_msg_p, FtFreertosUartMsgConfig *config_p);
// FError FtFreertosUartMsgBlcokingSend(FtFreertosUartMsg *uart_msg_p, u8 *buffer, u32 length);
FError FtFreertosUartMsgReceiveBuffer(FtFreertosUartMsg *uart_msg_p, u8 *buffer, u32 length, u32 *received_length);

#ifdef __cplusplus
}
#endif

#endif // !