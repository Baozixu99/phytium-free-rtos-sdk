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
 * FilePath: fuart_msg_hw.h
 * Created Date: 2024-10-10 14:24:39
 * Last Modified: 2025-06-06 16:36:59
 * Description:  This file is for
 * 
 * Modify History:
 *   Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 *  0.1    LiuSM     2024-10-10 14:24:39  file created
 */

#ifndef FUART_MSG_HW_H
#define FUART_MSG_HW_H

#include "ftypes.h"
#include "sdkconfig.h"
#include "fmsg_common.h"
#include "fio.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define FUART_MAX_DATA_LENGTH     120
#define FUART_MAX_RING_ITEMS      20

#define MSG_COMPLETE              0x1
#define UART_NR                   14

#define TX_DATA_MAXINUM           120
#define RX_DATA_MAXINUM           ((TX_DATA_MAXINUM / 2) + 1)

#define DATA_OE                   (1 << 11)
#define DATA_BE                   (1 << 10)
#define DATA_PE                   (1 << 9)
#define DATA_FE                   (1 << 8)
#define DATA_MASK                 (0xff)
#define DATA_ERROR                (DATA_OE | DATA_BE | DATA_PE | DATA_FE)
#define DATA_DUMMY_RX             (1 << 16)

#define REG_CHECK_TX              0x4c

#define MODEM_CTS                 0x1
#define MODEM_DSR                 0x2
#define MODEM_DCD                 0x4
#define MODEM_CAR                 MODEM_DCD
#define MODEM_RNG                 0x8
#define MODEM_RTS                 0x10
#define MODEM_DTR                 0x20
#define MODEM_OUT1                0x40
#define MODEM_OUT2                0x80

/* interrupt mask */
#define TX_HEAD_INT               0x1
#define RX_TAIL_INT               0x2
#define M2S_INT_TRIGGER_BIT       0x10
#define M2S_MSG_DATA_COMPLETED    0x10
#define MODEM_INT                 0x20
#define INTR_MASK_ALL             0x33

#define UART_MODULE_ID            0x1
#define MSG_DEFAULT               0x0
#define MSG_DEFAULT_SUBID         0x10
#define MSG_SET                   0x1
#define MSG_GET                   0x2
#define MSG_DATA                  0x3

#define MSG_GET_MODEM             0x3
#define MSG_GET_RX_EMPTY          0x4
#define MSG_TX_DATA               0x0


#define RX_TAIL_INT_ENABLE        0x10000
#define MSG_DATA_MASK             0xff
#define BUFFER_POINTER_MASK       0xffff
#define CHAR_MASK                 255

#define TX_BUFFER_SIZE            8
#define RX_BUFFER_SIZE            8
#define BUFFER_SIZE               20
#define UART_FIFOSIZE             64
#define TX_MSG_SIZE               0x80

#define RX_MSG_SIZE               0x80

#define RX_CHARS_MAX              28

/* uart debug mechanism */
#define DBG_REG                   0x58
/* uart debug register mask bit */
#define DBG_ENABLE_MASK           0x1

#define DBG_HEARTBEAT_ENABLE_MASK (0x1 << 1)
#define DBG_HEARTBEAT_MASK        (0x1 << 2)
#define DBG_LOG_EXIST_MASK        (0x1 << 3)
#define DBG_SIZE_MASK             (0xf << 4)
#define DBG_ADDR_MASK             (0x3fff << 8)


/* enum all types_set subid */
enum types_set_subid
{
    MSG_UART_CMD_SET_DEVICE_EN = 0x00, /* enable/disable uart device */
    MSG_UART_CMD_SET_BAUD_RATE,        /* set uart baud rate */
    MSG_UART_CMD_SET_DATA_BITS,        /* set trans bit width 0:5, 1:6, 2:7, 3:8 */
    MSG_UART_CMD_SET_STOP_BITS,        /* set stop bit width 0:1bit, 1:2bit */
    MSG_UART_CMD_SET_PARITY_EN,        /* set parity 0:disable, 1:enable */
    MSG_UART_CMD_SET_PARITY_TYPE,      /* set parity type 0:odd, 2:even */
    MSG_UART_CMD_SET_PARITY_STICK_SET, /* set parity stick 0:off, 1:on */
    MSG_UART_CMD_SET_BREAK_EN,         /* set break signal, 0: no break, 1:send break */
    MSG_UART_CMD_SET_RX_BUFFER_EN,     /* set rx en, 0:disable,single byte, 1:enable */
    MSG_UART_CMD_SET_TX_BUFFER_EN,     /* set tx en, 0:disable,single byte, 1:enable */
    MSG_UART_CMD_SET_TX_EN,            /* enable/disable tx, 0:disable, 1:enable */
    MSG_UART_CMD_SET_RX_EN,            /* enable/disable rx, 0:disable, 1:enable */
    MSG_UART_CMD_SET_LOOP_EN,     /* enable/disable loopback, 0:disable, 1:enable */
    MSG_UART_CMD_SET_RTS_EN,      /* enable/disable rts, 0:disable, 1:enable */
    MSG_UART_CMD_SET_CTS_EN,      /* enable/disable cts, 0:disable, 1:enable */
    MSG_UART_CMD_SET_DTR_SET,     /* Set DTR,set 1 is low  */
    MSG_UART_CMD_SET_RTS_SET,     /* Set RTS,set 1 is low  */
    MSG_UART_CMD_SET_DTE_DCD_SET, /* Set DCD,set 1 is low  */
    MSG_UART_CMD_SET_DTE_RI_SET,  /* Set RI,set 1 is low  */
    MSG_UART_CMD_SET_DTE_RX_IM,   /* Set rx interrupt mask,0:disable, 1:enable */
    MSG_UART_CMD_SET_DTE_TX_IM,   /* Set tx interrupt mask,0:disable, 1:enable */
    MSG_UART_CMD_SET_ERROR_IM, /* Set error interrupt mask,bit 0:frame error, 1:parity error, 2:break error, 3:overrun error */
    MSG_UART_CMD_SET_MODEM_IM, /* Set modem interrupt mask,bit 0:cts, 1:dsr, 2:dcd, 3:ri */
    MSG_UART_CMD_SET_STARTUP,      /* Set uart startup */
    MSG_UART_CMD_SET_HWINIT,       /* Set uart init */
    MSG_UART_CMD_SET_MCTRL,        /* Set modem */
    MSG_UART_CMD_SET_TERMIOS,      /* Set termios */
    MSG_UART_CMD_SET_DISABLE_UART, /* Set disable uart */
};

u8 FGetHexValue(enum types_set_subid subid);

typedef struct
{
    u8 reserved; /*取消模块id，作为保留字段，必须为0值*/
    u8 seq;      /*由发起方使用或维护，对端不改变该值，默认为0*/
    u8 cmd_type; /*功能性命令字段，可以用极少位表示，当前保留为8bit*/
    u8 cmd_subid; /*子命令id，与命令实现有关，最多为256个，搭配cmd_type补充*/
    u16 len;    /*在交互数据时，表示payload的长度，交互命令时为0*/
    u8 status1; /*保留字段，各模块根据需要定义，可返回某些函数执行后的返回值*/
    u8 status0; /*协议命令状态，表示命令执行相应*/
    u8 para[FUART_MAX_DATA_LENGTH]; /*传输数据*/
} FUartMsgInfo;

/***************** Macros (Inline Functions) Definitions *********************/

#define FUART_MSG_READREG32(addr, reg_offset) FtIn32(addr + (u32)reg_offset)
#define FUART_MSG_WRITEREG32(addr, reg_offset, reg_value) \
    FtOut32(addr + (u32)reg_offset, (u32)reg_value)

void FUartMsgFill(FUartMsgInfo *msgInfo, u8 module_id, u8 cmd_id, u8 cmd_subid, u8 complete);

int FUartMsgTxRingBufferIsEmpty(uintptr addr);
int FUartMsgRxRingBufferIsEmpty(uintptr addr);
int FUartMsgTxRingBufferIsFull(uintptr addr);
int FUartMsgRxRingBufferIsFull(uintptr addr);

u32 FUartMsgRxChars(FDevMsg *msg_p, u8 *char_data, u32 len);
void FUartMsgStopTx(FDevMsg *msg_p);
void FUartMsgStopRx(FDevMsg *msg_p);
int FUartMsgTxChar(FDevMsg *msg_p, u8 char_data);
u32 FUartMsgTxChars(FDevMsg *msg_p, u8 *char_data, u32 len);
u32 FUartMsgModemStatus(FDevMsg *msg_p);
void FM2sMsgInsert(FDevMsg *msg_p, FUartMsgInfo *msgInfo);
void FUartMsgEnableModemStatus(FDevMsg *msg_p);
void FUartMsgHwInit(FDevMsg *msg_p);
void FUartMsgDisable(FDevMsg *msg_p);
/************************** Function Prototypes ******************************/
#ifdef __cplusplus
}
#endif

#endif /* FUART_MSG_HW_H */