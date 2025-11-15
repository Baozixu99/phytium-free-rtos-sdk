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
 * FilePath: fuart_msg_options.c
 * Created Date: 2024-10-10 19:18:58
 * Last Modified: 2025-06-09 16:54:26
 * Description:  This file is for
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 */

/***************************** Include Files *********************************/

#include "fuart_msg.h"
#include "fuart_msg_hw.h"

/************************** Constant Definitions *****************************/

#define FUART_DEBUG_TAG "FUART-MSG-OPT"
#define FUART_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FUART_DEBUG_TAG, format, ##__VA_ARGS__)
#define FUART_WARN(format, ...) FT_DEBUG_PRINT_W(FUART_DEBUG_TAG, format, ##__VA_ARGS__)
#define FUART_INFO(format, ...) FT_DEBUG_PRINT_I(FUART_DEBUG_TAG, format, ##__VA_ARGS__)
#define FUART_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FUART_DEBUG_TAG, format, ##__VA_ARGS__)

/***************** Macros (Inline Functions) Definitions *********************/

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

/**
 * @name: FUartMsgBreakCtl
 * @msg:  Used to control the break signal.
 * @param FDevMsg contains the MSG device message pointer.
 * @param break_state contains the break signal state, 1 to enable the break signal, 0 to disable the break signal.
 * @return {void}
 */
void FUartMsgBreakCtl(FDevMsg *msg_p, u32 break_state)
{
    FUartMsgInfo msgInfo;
    u32 ctrl = 1;
    u8 cmd_subid;

    enum types_set_subid cmd = MSG_UART_CMD_SET_BREAK_EN;

    cmd_subid = FGetHexValue(cmd);
    FUartMsgFill(&msgInfo, UART_MODULE_ID, MSG_SET, cmd_subid, 0);
    if (break_state == 1)
    {
        msgInfo.para[0] = (ctrl)&DATA_MASK;
        msgInfo.para[1] = (ctrl >> 8) & DATA_MASK;
        msgInfo.para[2] = (ctrl >> 16) & DATA_MASK;
        msgInfo.para[3] = (ctrl >> 24) & DATA_MASK;
    }
    FM2sMsgInsert(msg_p, &msgInfo);
}

/**
 * @name: FUartMsgGetMctrl
 * @msg:  Used to get the modem control register.
 * @param FDevMsg contains the MSG device message pointer.
 * @return {u32} modem control register value.
 */
u32 FUartMsgGetMctrl(FDevMsg *msg_p)
{
    FASSERT(msg_p != NULL);
    FUartMsgInfo msgInfo;
    FUartMsgInfo *pMsgInfo;
    u32 status;
    u16 tx_tail;

    tx_tail = FUartMsgRead(msg_p->regfile, FMSG_TX_TAIL) & BUFFER_POINTER_MASK;
    FUartMsgFill(&msgInfo, UART_MODULE_ID, MSG_GET, MSG_GET_MODEM, 0);
    FM2sMsgInsert(msg_p, &msgInfo);

    pMsgInfo = (FUartMsgInfo *)(msg_p->shmem + TX_MSG_SIZE * tx_tail);
    while (!(FUartMsgTxRingBufferIsEmpty(msg_p->regfile)))
    {
        ; /* 需要超时退出机制 */
    }
    status = pMsgInfo->para[0];

    return status;
}

/**
 * @name: FUartMsgSetMctrl
 * @msg:  Used to set the modem control register.
 * @param FDevMsg contains the MSG device message pointer.
 * @param mctrl contains the modem control register value.
 * @return {void}
 */
void FUartMsgSetMctrl(FDevMsg *msg_p, u32 mctrl)
{
    FASSERT(msg_p != NULL);
    FUartMsgInfo msgInfo;
    u8 cmd_subid;

    enum types_set_subid cmd = MSG_UART_CMD_SET_MCTRL;

    cmd_subid = FGetHexValue(cmd);

    FUartMsgFill(&msgInfo, UART_MODULE_ID, MSG_SET, cmd_subid, 0);

    msgInfo.para[0] = mctrl & DATA_MASK;
    msgInfo.para[1] = (mctrl >> 8) & DATA_MASK;
    msgInfo.para[2] = (mctrl >> 16) & DATA_MASK;
    msgInfo.para[3] = (mctrl >> 24) & DATA_MASK;

    FM2sMsgInsert(msg_p, &msgInfo);
    return;
}

/**
 * @name: FUartMsgSetBaudRate
 * @msg:  Used to set the baud rate.
 * @param FDevMsg contains the MSG device message pointer.
 * @param baudrate contains the baud rate value.
 * @param ref_clk_freq contains the reference clock frequency.
 * @return {FError} FUART_ERROR_PARAM if the baud rate is out of range.
 */
FError FUartMsgSetBaudRate(FDevMsg *msg_p, u32 baudrate, u32 ref_clk_freq)
{
    FASSERT(NULL != msg_p);
    FUartMsgInfo msgInfo;
    u8 cmd_subid;

    enum types_set_subid cmd = MSG_UART_CMD_SET_BAUD_RATE;
    cmd_subid = FGetHexValue(cmd);
    FUartMsgFill(&msgInfo, UART_MODULE_ID, MSG_SET, cmd_subid, 0);

    msgInfo.para[0] = baudrate & DATA_MASK;
    msgInfo.para[1] = (baudrate >> 8) & DATA_MASK;
    msgInfo.para[2] = (baudrate >> 16) & DATA_MASK;
    msgInfo.para[3] = (baudrate >> 24) & DATA_MASK;

    msgInfo.para[4] = ref_clk_freq & DATA_MASK;
    msgInfo.para[5] = (ref_clk_freq >> 8) & DATA_MASK;
    msgInfo.para[6] = (ref_clk_freq >> 16) & DATA_MASK;
    msgInfo.para[7] = (ref_clk_freq >> 24) & DATA_MASK;

    FM2sMsgInsert(msg_p, &msgInfo);

    return FT_SUCCESS;
}

/**
 * @name: FUartMsgSetDataFormat
 * @msg:  Sets the data format for the specified UART.
 * @param msg_p    is a pointer to the uart instance.
 * @param format_p  is a pointer to a format structure that will
 *      contain the data format after this call completes.
 * @return
 *          FT_SUCCESS if everything configured as expected
 *          FT_ERROR_PARAM if one of the parameters was not valid.
 */
FError FUartMsgSetDataFormat(FDevMsg *msg_p, FUartMsgFormat *format_p)
{
    FError ret = FT_SUCCESS;
    FASSERT(NULL != msg_p);
    FUartMsgInfo msgInfo;
    u8 cmd_subid;
    u8 parity_en = 0;
    u8 parity_odd = 0;
    u8 cmspar = 0;
    u8 crtscts = 0;

    parity_en = (format_p->parity & FUART_FORMAT_EN_PARITY) >> (FUART_FORMAT_EN_PARITY - 1);
    parity_odd = (format_p->parity & FUART_FORMAT_EVEN_PARITY) >>
                 (FUART_FORMAT_EVEN_PARITY_SHIFT - 1);

    enum types_set_subid cmd = MSG_UART_CMD_SET_TERMIOS;
    cmd_subid = FGetHexValue(cmd);
    FUartMsgFill(&msgInfo, UART_MODULE_ID, MSG_SET, cmd_subid, 0);

    msgInfo.para[0] = format_p->data_bits & DATA_MASK;
    msgInfo.para[1] = format_p->stopbits & DATA_MASK;
    msgInfo.para[2] = parity_en & DATA_MASK;
    msgInfo.para[3] = parity_odd & DATA_MASK;
    msgInfo.para[4] = cmspar & DATA_MASK;
    msgInfo.para[5] = crtscts & DATA_MASK;

    FM2sMsgInsert(msg_p, &msgInfo);

    return ret;
}

/**
 * @name: FUartMsgSetStartUp
 * @msg:  Sets the startup parameters for the specified UART.
 * @param uart_p  is a pointer to the uart instance.
 * @return GET_MODEM 
 */
u8 FUartMsgSetStartUp(FUartMsg *uart_p)
{
    FASSERT(uart_p != NULL);
    FDevMsg *msg_p = &(uart_p->config.msg);

    FUartMsgInfo msgInfo;
    FUartMsgInfo *pMsgInfo;
    u8 cmd_subid;
    u16 tx_tail;
    enum types_set_subid cmd = MSG_UART_CMD_SET_STARTUP;

    cmd_subid = FGetHexValue(cmd);

    FUartMsgHwInit(msg_p);

    FUartMsgFill(&msgInfo, UART_MODULE_ID, MSG_SET, cmd_subid, 0);

    FM2sMsgInsert(msg_p, &msgInfo);

    FUartMsgFill(&msgInfo, UART_MODULE_ID, MSG_GET, MSG_GET_MODEM, 0);
    tx_tail = FUartMsgRead(msg_p->regfile, FMSG_TX_TAIL) & BUFFER_POINTER_MASK;
    FM2sMsgInsert(msg_p, &msgInfo);

    pMsgInfo = (FUartMsgInfo *)(msg_p->shmem + TX_MSG_SIZE * tx_tail);
    while (!(FUartMsgTxRingBufferIsEmpty(msg_p->regfile)))
    {
        ; /* 需要超时退出机制 */
    }

    return pMsgInfo->para[0]; //GET_MODEM
}

void FUartMsgSetShutdown(FUartMsg *uart_p)
{
    FASSERT(uart_p != NULL);
    FDevMsg *msg_p = &(uart_p->config.msg);
    FUartMsgDisable(msg_p);
}