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
 * FilePath: fuart_msg.h
 * Created Date: 2024-10-10 19:19:22
 * Last Modified: 2025-06-12 16:13:55
 * Description:  This file is for
 * 
 * Modify History:
 *   Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 */
#ifndef FUART_MSG_H
#define FUART_MSG_H

#include "ftypes.h"
#include "sdkconfig.h"
#include "fassert.h"
#include "fmsg_common.h"
#include "fparameters.h"

#ifdef __cplusplus
extern "C"
{
#endif

/***************** Macros (Inline Functions) Definitions *********************/

#define FUART_ERROR_PARAM                  FT_CODE_ERR(ErrModBsp, ErrBspUart, 0x1u)

#define FUART_MSG_DEFAULT_BAUDRATE         115200U

/* Data format values */
#define FUART_FORMAT_WORDLENGTH_8BIT       0x3
#define FUART_FORMAT_WORDLENGTH_7BIT       0x2
#define FUART_FORMAT_WORDLENGTH_6BIT       0x1
#define FUART_FORMAT_WORDLENGTH_5BIT       0x0

#define FUART_FORMAT_NO_PARITY             0U /* No parity */
#define FUART_FORMAT_EN_PARITY             1U /* Enable parity */
#define FUART_FORMAT_EVEN_PARITY           2U /* Even parity */
#define FUART_FORMAT_ODD_PARITY            0U /* Odd parity */
#define FUART_FORMAT_EN_STICK_PARITY       4U /* Stick parity */
#define FUART_FORMAT_NO_STICK_PARITY       0U /* Stick parity */

#define FUART_FORMAT_PARITY_MASK           7U /* Format parity mask */

#define FUART_FORMAT_EVEN_PARITY_SHIFT     1U /* Even parity shift */
#define FUART_FORMAT_EN_STICK_PARITY_SHIFT 5U /* Stick parity shift */

#define FUART_FORMAT_2_STOP_BIT            1U
#define FUART_FORMAT_1_STOP_BIT            0U


/* Callback events  */
#define FUART_EVENT_RECV_DATA              1U /* Data receiving done */
#define FUART_EVENT_SENT_DATA              3U /* Data transmission done */
#define FUART_EVENT_MODEM                  5U /* Modem status changed */

#define FUART_EVENT_RECV_ORERR             7U /* A receive overrun error detected */


/**************************** Type Definitions ******************************/

/**
 * Keep track of data format setting of a device.
 */
typedef struct
{
    u32 data_bits; /* Number of data bits */
    u32 parity;    /* Parity */
    u8 stopbits;   /* Number of stop bits */
} FUartMsgFormat;


typedef struct
{
    u32 instance_id; /* Id of device*/
    FDevMsg msg;
    u32 irq_num;
    u32 baudrate;
    u32 ref_clk_freq;
} FUartMsgConfig;

typedef struct
{
    u8 *byte_p;
    u32 requested_bytes;
    u32 remaining_bytes;
} FUartMsgBuffer;

typedef void (*FUartMsgEventHandler)(void *args, u32 event, u32 event_data);

typedef struct
{
    FUartMsgConfig config; /* Configuration data structure  */
    u32 is_ready;          /* Device is ininitialized and ready*/
    FUartMsgBuffer send_buffer;
    FUartMsgBuffer receive_buffer;
    FUartMsgEventHandler handler;
    void *args;
    uint8_t rxbs_error; /* An error occurs during receiving. 0 has no error and 1 has an error */
} FUartMsg;

/************************** Function Prototypes ******************************/

/* FUartMsg_uart_sinit.c */
const FUartMsgConfig *FUartMsgLookupConfig(u32 instance_id);
/* FUartMsg_uart.c */
FError FUartMsgCfgInitialize(FUartMsg *uart_p, FUartMsgConfig *config);
void FUartMsgBlockSend(FUartMsg *uart_p, u8 *byte_p, u32 length);
u32 FUartMsgSend(FUartMsg *uart_p, u8 *byte_p, u32 length);
u32 FUartMsgReceive(FUartMsg *uart_p, u8 *byte_p, u32 length);
u8 FUartMsgBlockReceive(FUartMsg *uart_p);

/* FUartMsg_uart_options.c */
void FUartMsgBreakCtl(FDevMsg *msg_p, u32 break_state);
u32 FUartMsgGetMctrl(FDevMsg *msg_p);
void FUartMsgSetMctrl(FDevMsg *msg_p, u32 mctrl);
FError FUartMsgSetBaudRate(FDevMsg *msg_p, u32 baudrate, u32 ref_clk_freq);
FError FUartMsgSetDataFormat(FDevMsg *msg_p, FUartMsgFormat *format_p);
u8 FUartMsgSetStartUp(FUartMsg *uart_p);
void FUartMsgSetShutdown(FUartMsg *uart_p);

/* FUartMsg_uart_intr.c */
u32 FUartMsgGetInterruptMask(FUartMsg *uart_p);
void FUartMsgEnableInterrups(FUartMsg *uart_p);
void FUartMsgInterruptDisable(FUartMsg *uart_p);
void FUartMsgInterruptHandler(s32 vector, void *param);
void FUartMsgSetHandler(FUartMsg *uart_p, FUartMsgEventHandler fun_p, void *args);
void FUartMsgSetInterruptMask(FUartMsg *uart_p, u32 mask);
void FUartMsgInterruptClearAll(FUartMsg *uart_p);

#ifdef __cplusplus
}
#endif

#endif /* FUART_MSG_H */