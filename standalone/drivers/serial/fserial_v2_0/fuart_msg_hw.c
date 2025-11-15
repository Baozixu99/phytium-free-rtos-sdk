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
 * FilePath: fuart_msg_hw.c
 * Created Date: 2024-10-10 14:24:23
 * Last Modified: 2025-06-11 15:42:39
 * Description:  This file is for
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 *  1.0    LiuSM    2024-10-10 14:24:23  First version
 */

#include "fassert.h"
#include "fdrivers_port.h"
#include "fdebug.h"
#include "fuart_msg_hw.h"

#define FUART_DEBUG_TAG "FUART-MSG"
#define FUART_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FUART_DEBUG_TAG, format, ##__VA_ARGS__)
#define FUART_WARN(format, ...) FT_DEBUG_PRINT_W(FUART_DEBUG_TAG, format, ##__VA_ARGS__)
#define FUART_INFO(format, ...) FT_DEBUG_PRINT_I(FUART_DEBUG_TAG, format, ##__VA_ARGS__)
#define FUART_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FUART_DEBUG_TAG, format, ##__VA_ARGS__)

#define FUART_HW_TIMEOUT 1000000

static inline unsigned int FUartMsgRead(uintptr addr, u32 reg)
{
    return FUART_MSG_READREG32(addr, reg);
}

static inline void FUartMsgWrite(uintptr addr, u32 reg, u32 value)
{
    FUART_MSG_WRITEREG32(addr, reg, value);
    return;
}

u8 FGetHexValue(enum types_set_subid subid)
{
    return (u8)subid;
}

void FUartMsgFill(FUartMsgInfo *msgInfo, u8 module_id, u8 cmd_id, u8 cmd_subid, u8 complete)
{
    msgInfo->seq = module_id;
    msgInfo->cmd_type = cmd_id;
    msgInfo->cmd_subid = cmd_subid;
    msgInfo->status0 = complete;
    memset(msgInfo->para, 0, sizeof(msgInfo->para));
}

/**
 * @name: FUaartMsgTxRingBufferIsFull
 * @msg:  Used to confirm whether the tx ring buffer is full.
 * @param addr contains the MSG regfile base address of the device.
 * @return {bool} true if the tx ring buffer is full, false otherwise.
 *
 */
int FUartMsgTxRingBufferIsFull(uintptr addr)
{
    u16 tx_head, tx_tail;

    tx_head = FUartMsgRead(addr, FMSG_TX_HEAD) & BUFFER_POINTER_MASK;
    tx_tail = FUartMsgRead(addr, FMSG_TX_TAIL) & BUFFER_POINTER_MASK;

    if (((tx_tail + 1) % TX_BUFFER_SIZE) == tx_head)
    {
        return 1;
    }
    return 0;
}

/**
 * @name: FUartMsgTxRingBufferIsEmpty
 * @msg:  Used to confirm whether the tx ring buffer is empty.
 * @param addr contains the MSG regfile base address of the device.
 * @return {bool} true if the tx ring buffer is empty, false otherwise.
 */
int FUartMsgTxRingBufferIsEmpty(uintptr addr)
{
    u16 tx_head, tx_tail;

    tx_head = FUartMsgRead(addr, FMSG_TX_HEAD) & BUFFER_POINTER_MASK;
    tx_tail = FUartMsgRead(addr, FMSG_TX_TAIL) & BUFFER_POINTER_MASK;

    if (tx_tail == tx_head)
    {
        return 1;
    }
    return 0;
}

/**
 * @name: FM2sMsgInsert
 * @msg:  Used to insert a message into the M2S message queue.
 * @param msg_p contains the MSG device message pointer.
 * @param msgInfo contains the message information to be inserted.
 * @return {void}
 */
void FM2sMsgInsert(FDevMsg *msg_p, FUartMsgInfo *msgInfo)
{
    u16 tx_head, tx_tail;

    tx_head = FUartMsgRead(msg_p->regfile, FMSG_TX_HEAD) & BUFFER_POINTER_MASK;
    tx_tail = FUartMsgRead(msg_p->regfile, FMSG_TX_TAIL) & BUFFER_POINTER_MASK;

    FUART_DEBUG("tx_head = %x, tx_tail = %x", tx_head, tx_tail);
    while (FUartMsgTxRingBufferIsFull(msg_p->regfile))
    {
        ; /* 需要超时机制 */
    }

    memcpy((FUartMsgInfo *)(msg_p->shmem + TX_MSG_SIZE * tx_tail), msgInfo, sizeof(FUartMsgInfo));

    /* update tx_tail pointer */
    tx_tail = (tx_tail + 1) % TX_BUFFER_SIZE;
    FUartMsgWrite(msg_p->regfile, FMSG_TX_TAIL, tx_tail);

    FUartMsgWrite(msg_p->regfile, FMSG_M2S_INT_STATE, M2S_INT_TRIGGER_BIT);

    return;
}

/**
 * @name: FUartMsgRxRingBufferIsEmpty
 * @msg:  Used to confirm whether the rx ring buffer is empty.
 * @param addr contains the MSG regfile base address of the device.
 * @return {bool} true if the rx ring buffer is empty, false otherwise.
 */
int FUartMsgRxRingBufferIsEmpty(uintptr addr)
{
    u16 rx_head, rx_tail;

    rx_head = FUartMsgRead(addr, FMSG_RX_HEAD) & BUFFER_POINTER_MASK;
    rx_tail = FUartMsgRead(addr, FMSG_RX_TAIL) & BUFFER_POINTER_MASK;

    if (rx_tail == rx_head)
    {
        return 1;
    }
    return 0;
}

/**
 * @name: FUartMsgRxRingBufferIsFull
 * @msg:  Used to confirm whether the rx ring buffer is full.
 * @param addr contains the MSG regfile base address of the device.
 * @return {bool} true if the rx ring buffer is full, false otherwise.
 */
int FUartMsgRxRingBufferIsFull(uintptr addr)
{
    u16 rx_head, rx_tail;
    rx_head = FUartMsgRead(addr, FMSG_RX_HEAD) & BUFFER_POINTER_MASK;
    rx_tail = FUartMsgRead(addr, FMSG_RX_TAIL) & BUFFER_POINTER_MASK;

    if (((rx_tail + 1) % RX_BUFFER_SIZE) == rx_head)
    {
        return 1;
    }
    return 0;
}

/**
 * @name: FUartMsgRxChars
 * @msg:  Used to receive characters from the rx ring buffer.
 * @param msg_p contains the MSG device message pointer.
 * @param char_data contains the buffer to store the received characters.
 * @param len contains the length of the buffer to store the received characters.
 * @return u32 the number of characters received.
 */
u32 FUartMsgRxChars(FDevMsg *msg_p, u8 *char_data, u32 len)
{
    FASSERT(char_data != NULL);
    FUartMsgInfo *pMsgInfo;
    u16 count = 0;
    u16 rx_head, rx_tail;
    u16 data_char = 0;
    u32 rec_count = 0;

    while (!FUartMsgRxRingBufferIsEmpty(msg_p->regfile))
    {
        rx_head = FUartMsgRead(msg_p->regfile, FMSG_RX_HEAD) & BUFFER_POINTER_MASK;
        rx_tail = FUartMsgRead(msg_p->regfile, FMSG_RX_TAIL) & BUFFER_POINTER_MASK;

        pMsgInfo = (FUartMsgInfo *)(msg_p->shmem + TX_MSG_SIZE * TX_BUFFER_SIZE + RX_MSG_SIZE * rx_head);
        FUART_DEBUG("rx_head = %x, rx_tail = %x, pMsgInfo = %p", rx_head, rx_tail, pMsgInfo);
        if (!pMsgInfo)
        {
            FUART_ERROR("%s cannot find pMsgInfo", __func__);
            return 0;
        }

        count = pMsgInfo->len / 2; /*传输是16位数据*/
        if (count > len)           /*收到的数据比要获取的长*/
        {
            FUART_WARN("count %d > len %d", count, len);
            count = len;
        }

        FUART_DEBUG("count = %d", count);
        rx_head = (rx_head + 1) % RX_BUFFER_SIZE;
        FUartMsgWrite(msg_p->regfile, FMSG_RX_HEAD, rx_head);

        u16 data_count = 0;
        for (data_count = 0; data_count < RX_DATA_MAXINUM; data_count++)
        {
            if (count == 0)
            {
                FUART_DEBUG("recevie data count is %d", data_count);
                break;
            }
            count--;
            /* Take chars max number 60*2 = 120 bytes from each message*/
            data_char = pMsgInfo->para[2 * data_count];
            data_char |= pMsgInfo->para[2 * data_count + 1] << 8;
            data_char |= DATA_DUMMY_RX;

            // 检查是否有数据错误
            FUART_DEBUG("data_char[%d] = 0x%x", data_count, data_char);
            if (data_char & DATA_ERROR)
            {
                /*
                // 处理数据错误，例如记录日志或丢弃数据
                if (data_char & DATA_BE)
                {
                    // data_char &= ~(DATA_PE | DATA_FE | DATA_OE);
                    data_char &= ~(DATA_PE | DATA_FE);
                    FUART_ERROR("receive data error, data_char = %x", data_char);
                }
                else if (data_char & DATA_PE)
                {
                    data_char &= ~(DATA_FE | DATA_OE);
                    FUART_ERROR("receive parity error, data_char = %x", data_char);
                }
                else if (data_char & DATA_FE)
                {
                    data_char &= ~DATA_OE;
                    FUART_ERROR("receive framing error, data_char = %x", data_char);
                }
                else if (data_char & DATA_OE)
                {
                    FUART_ERROR("receive overrun error, data_char = %x", data_char);
                }
                */
            }
            char_data[data_count] = data_char & DATA_MASK;
        }
        rec_count += data_count;
    }
    return rec_count;
}

/**
 * @name: FUartMsgStopTx
 * @msg:  Used to stop the tx process.
 * @param FDevMsg contains the MSG device message pointer.
 * @return {void}
 */
void FUartMsgStopTx(FDevMsg *msg_p)
{
    FASSERT(msg_p != NULL);
    u32 int_mask;
    while (!FUartMsgTxRingBufferIsEmpty(msg_p->regfile))
    {
    }
    int_mask = FUartMsgRead(msg_p->regfile, FMSG_S2M_INT_MASK);
    FUartMsgWrite(msg_p->regfile, FMSG_S2M_INT_MASK, int_mask | M2S_MSG_DATA_COMPLETED);
}

/**
 * @name: FUartMsgStopRx
 * @msg:  Used to stop the rx process.
 * @param FDevMsg contains the MSG device message pointer.
 * @return {void}
 */
void FUartMsgStopRx(FDevMsg *msg_p)
{
    FASSERT(msg_p != NULL);
    FUartMsgInfo msgInfo;

    u8 cmd_subid;
    u32 int_mask;

    enum types_set_subid cmd = MSG_UART_CMD_SET_ERROR_IM;

    cmd_subid = FGetHexValue(cmd);

    int_mask = FUartMsgRead(msg_p->regfile, FMSG_S2M_INT_MASK);
    FUartMsgWrite(msg_p->regfile, FMSG_S2M_INT_MASK, int_mask | RX_TAIL_INT);
    FUartMsgFill(&msgInfo, UART_MODULE_ID, MSG_SET, cmd_subid, 0);
    FM2sMsgInsert(msg_p, &msgInfo);
}

/**
 * @name: FUartMsgTxChar
 * @msg:  Used to transmit a character.
 * @param FDevMsg contains the MSG device message pointer.
 * @param char_data contains the character to be transmitted.
 * @return {int} 0 if the character is successfully transmitted, -1 otherwise.
 */
int FUartMsgTxChar(FDevMsg *msg_p, u8 char_data)
{
    FUartMsgInfo msgInfo;
    FUartMsgInfo *pMsgInfo;

    u16 count = 1;

    FUartMsgFill(&msgInfo, UART_MODULE_ID, MSG_DATA, MSG_TX_DATA, 0);

    pMsgInfo = (FUartMsgInfo *)msg_p->shmem;
    if (!pMsgInfo->status0 & MSG_COMPLETE)
    {
        FUART_DEBUG("%s: tx ring buffer is not empty, please wait for tx complete", __func__);
        return -1;
    }
    msgInfo.para[0] = char_data;
    msgInfo.len = count;
    FUartMsgWrite(msg_p->regfile, REG_CHECK_TX, 1);

    FM2sMsgInsert(msg_p, &msgInfo);

    return 0;
}

/**
 * @name: FUartMsgTxChars
 * @msg:  Used to transmit characters.
 * @param FDevMsg contains the MSG device message pointer.
 * @param char_data contains the characters to be transmitted.
 * @param len contains the length of the characters to be transmitted,no more than TX_DATA_MAXINUM.
 * @return u32 the number of characters transmitted.
 */
u32 FUartMsgTxChars(FDevMsg *msg_p, u8 *char_data, u32 len)
{
    FUartMsgInfo msgInfo;
    u16 count = TX_DATA_MAXINUM;
    u16 data_id = 0;
    u32 send;

    FUartMsgFill(&msgInfo, UART_MODULE_ID, MSG_DATA, MSG_TX_DATA, 0);

    send = len;
    while (send > 0)
    {
        if (count > send)
        {
            count = send;
        }
        msgInfo.len = count;
        for (data_id = 0; data_id < count; data_id++)
        {
            msgInfo.para[data_id] = char_data[data_id];
        }
        if (FUartMsgTxRingBufferIsFull(msg_p->regfile)) /* 发送缓冲区满，等待下次进入 */
        {
            return len - send;
        }

        FM2sMsgInsert(msg_p, &msgInfo);
        send -= count;
        char_data += count;
    }
    //while (FUartMsgTxRingBufferIsEmpty(msg_p->regfile));/* 需要超时退出机制 */
    return len - send;
}

/**
 * @name: FUartMsgModemStatus
 * @msg:  Used to get the modem status.
 * @param FDevMsg contains the MSG device message pointer.
 * @return {u32} the modem status,0 if the modem status is not available.
 */
u32 FUartMsgModemStatus(FDevMsg *msg_p)
{
    FUartMsgInfo msgInfo;
    FUartMsgInfo *pMsgInfo;

    u32 status;
    u16 tx_tail;

    FUartMsgFill(&msgInfo, UART_MODULE_ID, MSG_GET, MSG_GET_MODEM, 0);

    tx_tail = FUartMsgRead(msg_p->regfile, FMSG_TX_TAIL) & BUFFER_POINTER_MASK;

    FM2sMsgInsert(msg_p, &msgInfo);

    pMsgInfo = (FUartMsgInfo *)(msg_p->shmem + TX_MSG_SIZE * tx_tail);

    while (FUartMsgTxRingBufferIsEmpty(msg_p->regfile))
    {
        ;
    }

    status = pMsgInfo->para[0];

    return status;
}

/**
 * @name: FUartMsgEnableModemStatus
 * @msg:  Used to enable the modem status interrupt.
 * @param FDevMsg contains the MSG device message pointer.
 * @return {void}
 */
void FUartMsgEnableModemStatus(FDevMsg *msg_p)
{
    FUartMsgInfo msgInfo;
    u8 cmd_subid;

    enum types_set_subid cmd = MSG_UART_CMD_SET_MODEM_IM;

    cmd_subid = FGetHexValue(cmd);
    FUartMsgFill(&msgInfo, UART_MODULE_ID, MSG_SET, cmd_subid, 0);
    msgInfo.para[0] = 0xf;
    FM2sMsgInsert(msg_p, &msgInfo);
    return;
}

/**
 * @name: FUartMsgHwInit
 * @msg:  Used to initialize the hardware.
 * @param FDevMsg contains the MSG device message pointer.
 * @return {void}
 */
void FUartMsgHwInit(FDevMsg *msg_p)
{
    FUartMsgInfo msgInfo;
    u8 cmd_subid;

    enum types_set_subid cmd = MSG_UART_CMD_SET_HWINIT;
    cmd_subid = FGetHexValue(cmd);
    FUartMsgFill(&msgInfo, UART_MODULE_ID, MSG_SET, cmd_subid, 0);
    FM2sMsgInsert(msg_p, &msgInfo);
    return;
}


/**
 * @name: FUartMsgDisable
 * @msg:  Used to disable the uart.
 * @param FDevMsg contains the MSG device message pointer.
 * @return {void}
 */
void FUartMsgDisable(FDevMsg *msg_p)
{
    FUartMsgInfo msgInfo;
    u8 cmd_subid;

    enum types_set_subid cmd = MSG_UART_CMD_SET_DISABLE_UART;
    cmd_subid = FGetHexValue(cmd);
    FUartMsgFill(&msgInfo, UART_MODULE_ID, MSG_SET, cmd_subid, 0);
    FM2sMsgInsert(msg_p, &msgInfo);
    return;
}
