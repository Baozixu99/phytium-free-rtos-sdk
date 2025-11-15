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
 * FilePath: fuart_msg.c
 * Created Date: 2024-10-10 19:19:11
 * Last Modified: 2025-06-09 16:52:03
 * Description:  This file is for
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 *  1.0   LiuSM    2024-10-10  First Release
 */

/***************************** Include Files ********************************/
#include "fio.h"
#include "fuart_msg_hw.h"
#include "fuart_msg.h"
#include "ferror_code.h"
#include "fdrivers_port.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

#define SEND_WAIT_TIMES 100

#define FUART_DEBUG_TAG "FUART-MSG"
#define FUART_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FUART_DEBUG_TAG, format, ##__VA_ARGS__)
#define FUART_WARN(format, ...) FT_DEBUG_PRINT_W(FUART_DEBUG_TAG, format, ##__VA_ARGS__)
#define FUART_INFO(format, ...) FT_DEBUG_PRINT_I(FUART_DEBUG_TAG, format, ##__VA_ARGS__)
#define FUART_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FUART_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/

u32 FUartMsgSendBuffer(FUartMsg *uart_p);
u32 FUartMsgReceiveBuffer(FUartMsg *uart_p);

static void FUartMsgDefaultHandler(void *Args, u32 Event, u32 ByteCount);
/*****************************************************************************/
static void FUartMsgDefaultHandler(void *args, u32 event, u32 byte_count)
{
    (void)args;
    (void)event;
    (void)byte_count;

    FASSERT(0);
}

FError FUartMsgCfgInitialize(FUartMsg *uart_p, FUartMsgConfig *config)
{
    FError ret = FT_SUCCESS;
    FASSERT(uart_p != NULL);
    FASSERT(config != NULL);

    uart_p->config.instance_id = config->instance_id;
    uart_p->config.msg.regfile = config->msg.regfile;
    uart_p->config.msg.shmem = config->msg.shmem;
    uart_p->config.irq_num = config->irq_num;
    uart_p->config.baudrate = config->baudrate;
    uart_p->config.ref_clk_freq = config->ref_clk_freq;

    uart_p->handler = FUartMsgDefaultHandler;

    uart_p->send_buffer.byte_p = NULL;
    uart_p->send_buffer.requested_bytes = 0;
    uart_p->send_buffer.remaining_bytes = 0;

    uart_p->receive_buffer.byte_p = NULL;
    uart_p->receive_buffer.requested_bytes = 0;
    uart_p->receive_buffer.remaining_bytes = 0;
    uart_p->rxbs_error = 0;

    ret = FUartMsgSetBaudRate(&(uart_p->config.msg), uart_p->config.baudrate,
                              uart_p->config.ref_clk_freq);
    if (ret != FT_SUCCESS)
    {
        return ret;
    }
    uart_p->is_ready = FT_COMPONENT_IS_READY;

    return ret;
}

/**
 * @name: FUartMsgSend
 * @msg: This functions sends the specified buffer using the device in either
 * polled or interrupt driven mode.
 * @return  The number of bytes actually sent.
 * @param uart_p is a pointer to the FUartMsg instance.
 * @param byte_p  is pointer to a buffer of data to be sent.
 * @param  length  ontains the number of bytes to be sent. Any data that was already put into the
 *      transmit FIFO will be sent.
 */
u32 FUartMsgSend(FUartMsg *uart_p, u8 *byte_p, u32 length)
{
    u32 sent_count = 0;
    FASSERT(uart_p != NULL);
    FASSERT(byte_p != NULL);
    FASSERT(uart_p->is_ready == FT_COMPONENT_IS_READY);

    uart_p->send_buffer.byte_p = byte_p;
    uart_p->send_buffer.requested_bytes = length;
    uart_p->send_buffer.remaining_bytes = length;
    sent_count = FUartMsgSendBuffer(uart_p);
    return sent_count;
}

/**
 * @name: FUartMsgSendBuffer
 * @msg: send data buffer through uart,if you close FIFO and open interrupt,recommend use FUartMsgBlockSend
 * @return {*}
 * @param  uart_p
 */
u32 FUartMsgSendBuffer(FUartMsg *uart_p)
{
    FASSERT(uart_p != NULL);
    u32 sent_count;
    u32 sent_tmp;

    /*
     * If the TX FIFO is full, send nothing.
     * Otherwise put bytes into the TX FIFO unil it is full, or all of the
     * data has been put into the FIFO.
     */
    sent_count = uart_p->send_buffer.requested_bytes - uart_p->send_buffer.remaining_bytes;
    sent_tmp = FUartMsgTxChars(&(uart_p->config.msg), &(uart_p->send_buffer.byte_p[sent_count]),
                               uart_p->send_buffer.remaining_bytes);
    uart_p->send_buffer.remaining_bytes -= sent_tmp;
    return sent_tmp;
}

/**
 * @name: FUartMsgReceive
 * @msg:  This function attempts to receive a specified number of bytes of data
 * from the device and store it into the specified buffer.
 * @param  uart_p is a pointer to the FUartMsg instance
 * @param  byte_p is pointer to buffer for data to be received into
 * @param  length is the number of bytes to be received.
 * @return The number of bytes received.
 */
u32 FUartMsgReceive(FUartMsg *uart_p, u8 *byte_p, u32 length)
{
    u32 received;

    FASSERT(uart_p != NULL);
    FASSERT(byte_p != NULL);
    FASSERT(uart_p->is_ready == FT_COMPONENT_IS_READY);

    uart_p->receive_buffer.byte_p = byte_p;
    uart_p->receive_buffer.requested_bytes = length;
    uart_p->receive_buffer.remaining_bytes = length;

    received = FUartMsgReceiveBuffer(uart_p);

    return received;
}


u32 FUartMsgReceiveBuffer(FUartMsg *uart_p)
{
    u32 received_count = 0U;

    while ((received_count < uart_p->receive_buffer.remaining_bytes) &&
           !FUartMsgRxRingBufferIsEmpty(uart_p->config.msg.regfile))
    {
        received_count = FUartMsgRxChars(&(uart_p->config.msg), uart_p->receive_buffer.byte_p,
                                         uart_p->receive_buffer.remaining_bytes);
        if (received_count == 0U)
        {
            uart_p->rxbs_error = 1U;
        }
        uart_p->receive_buffer.byte_p += received_count;
    }
    uart_p->receive_buffer.remaining_bytes -= received_count;

    return received_count;
}

/**
 * @name: FUartMsgBlockSend
 * @msg: initiate uart block send
 * @return {*}
 * @param  uart_p
 * @param  byte_p
 * @param  length
 */
void FUartMsgBlockSend(FUartMsg *uart_p, u8 *byte_p, u32 length)
{
    u32 index;
    u32 wait_times = SEND_WAIT_TIMES;

    FASSERT(uart_p != NULL);
    FASSERT(byte_p != NULL);
    FASSERT(uart_p->is_ready == FT_COMPONENT_IS_READY);

    for (index = 0; index < length; index++)
    {
        while (-1 == FUartMsgTxChar(&(uart_p->config.msg), byte_p[index]))
        {
            FDriverUdelay(10);
            wait_times--;
            if (wait_times == 0)
            {
                FUART_ERROR("FUartMsgBlockSend timeout, send failed.");
                break;
            }
        }
    }
}

/**
 * @name: FUartMsgBlockReceive
 * @msg: initiate uart block receive
 * @return {*}
 * @param  uart_p
 */
u8 FUartMsgBlockReceive(FUartMsg *uart_p)
{
    FASSERT(uart_p != NULL);
    FASSERT(uart_p->is_ready == FT_COMPONENT_IS_READY);
    u8 data;
    FUartMsgRxChars(&(uart_p->config.msg), &data, 1U);
    return data;
}
