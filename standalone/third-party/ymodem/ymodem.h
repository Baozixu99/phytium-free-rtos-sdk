/*
 * Copyright (C) 2022, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: ymodem.h
 * Date: 2022-02-10 14:53:45
 * LastEditTime: 2022-02-25 11:46:57
 * Description: This file is for ymodem Protocol
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe    2022/6/4     first release
 */

#ifndef  _YMODEM_H
#define  _YMODEM_H

#include "ftypes.h"

#ifdef __cplusplus
extern "C"
{
#endif


/* The word "FYM" is stand for "YModem". */

typedef enum
{
    FYM_CODE_NONE = 0x00,
    FYM_CODE_SOH  = 0x01,
    FYM_CODE_STX  = 0x02,
    FYM_CODE_EOT  = 0x04,
    FYM_CODE_ACK  = 0x06,
    FYM_CODE_NAK  = 0x15,
    FYM_CODE_CAN  = 0x18,
    FYM_CODE_C    = 0x43,
}FtYmodemCode;

/* FYM error code
 *
 * We use the rt_err_t to return error values. We take use of current error
 * codes available in RTT and append ourselves.
 */
/* timeout on handshake */
#define FYM_ERR_TMO  0x70
/* wrong code, wrong SOH, STX etc. */
#define FYM_ERR_CODE 0x71
/* wrong sequence number */
#define FYM_ERR_SEQ  0x72
/* wrong CRC checksum */
#define FYM_ERR_CRC  0x73
/* not enough data received */
#define FYM_ERR_DSZ  0x74
/* the transmission is aborted by user */
#define FYM_ERR_CAN  0x75
/* wrong answer, wrong ACK or C */
#define FYM_ERR_ACK  0x76
/* transmit file invalid */
#define FYM_ERR_FILE 0x77

#define FILE_NAME_LENGTH        64
#define FILE_SIZE_LENGTH        16

/* Packet structure defines */
#define PACKET_HEADER_SIZE      3
#define PACKET_DATA_INDEX       3
#define PACKET_START_INDEX      0
#define PACKET_NUMBER_INDEX     1
#define PACKET_CNUMBER_INDEX    2
#define PACKET_TRAILER_SIZE     ((uint32_t)2)
#define PACKET_OVERHEAD_SIZE    (PACKET_HEADER_SIZE + PACKET_TRAILER_SIZE - 1)
#define PACKET_SIZE             ((uint32_t)128)
#define PACKET_1K_SIZE          ((uint32_t)1024)

int YmodemTransmit(u8 *p_buf, const u8 *p_file_name, u32 file_size);

#ifdef __cplusplus
}
#endif

#endif