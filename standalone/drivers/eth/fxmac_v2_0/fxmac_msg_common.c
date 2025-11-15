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
 * FilePath: fxmac_msg_msg.c
 * Date: 2024-10-16 15:32:34
 * LastEditTime: 2024-10-16 15:32:35
 * Description:  This file is for xmac msg register function definition
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huangjin   2024/10/16    init commit
 */
/***************************** Include Files *********************************/
#include "fio.h"
#include "fdrivers_port.h"
#include "fassert.h"
#include "ftypes.h"
#include "fswap.h"

#include "fxmac_msg_hw.h"
#include "fxmac_msg.h"
#include "fxmac_msg_common.h"
#include "fmsg_common.h"

#include "faarch.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define FXMAC_MSG_DEBUG_TAG "FXMAC_MSG_COMMON"
#define FXMAC_MSG_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FXMAC_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
#define FXMAC_MSG_WARN(format, ...) \
    FT_DEBUG_PRINT_W(FXMAC_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
#define FXMAC_MSG_INFO(format, ...) \
    FT_DEBUG_PRINT_I(FXMAC_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
#define FXMAC_MSG_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FXMAC_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
/************************** Function Prototypes ******************************/

/*****************************************************************************/
unsigned int FXmacMsgTxRingWrap(FXmacMsgCtrl *pdata, unsigned int index)
{
    return index & (pdata->msg_ring.tx_msg_ring_size - 1);
}

int FXmacMsgInitRing(FXmacMsgCtrl *pdata)
{
    u32 tx_msg_tail;

    pdata->msg_ring.tx_msg_ring_size = FXMAC_MSG_READ_BITS(pdata, FXMAC_MSG_SIZE, TXRING_SIZE);
    tx_msg_tail = FXMAC_MSG_READ(pdata, FXMAC_MSG_TX_MSG_TAIL) & 0xff;
    pdata->msg_ring.tx_msg_wr_tail = FXmacMsgTxRingWrap(pdata, tx_msg_tail);
    pdata->msg_ring.tx_msg_rd_tail = pdata->msg_ring.tx_msg_wr_tail;

    FXMAC_MSG_WRITE(pdata, FXMAC_MSG_IMR, 0xfffffffe);
    FXMAC_MSG_INFO("Msg ring size:%d, tx msg tail=%d", pdata->msg_ring.tx_msg_ring_size, tx_msg_tail);
    FXMAC_MSG_INFO("mac_regs= 0x%x, msg_regs = 0x%x", pdata->mac_regs, pdata->msg_regs);
    return 0;
}

int FXmacMsgSendMessage(FXmacMsgCtrl *pdata, u16 cmd_id, u16 cmd_subid, void *data, int len, int wait)
{
    u32 tx_head, tx_tail, ring_size;
    FXmacMsgInfo msg;
    FXmacMsgInfo msg_rx;
    int ret = 0;

    tx_head = FXMAC_MSG_READ(pdata, FXMAC_MSG_TX_MSG_HEAD) & 0xff;
    tx_tail = FXmacMsgTxRingWrap(pdata, pdata->msg_ring.tx_msg_wr_tail);
    pdata->msg_ring.tx_msg_rd_tail = tx_tail;
    ring_size = pdata->msg_ring.tx_msg_ring_size;

    while ((tx_tail + 1) % ring_size == tx_head)
    {
        FDriverUdelay(1);
        FXMAC_MSG_INFO("Tx msg ring is overrun, tx_tail:0x%x, tx_head:0x%x", tx_tail, tx_head);
        tx_head = FXMAC_MSG_READ(pdata, FXMAC_MSG_TX_MSG_HEAD) & 0xff;
    }

    wait = 1;
    memset(&msg, 0, sizeof(msg));
    memset(&msg_rx, 0, sizeof(msg_rx));
    msg.cmd_type = cmd_id;
    msg.cmd_subid = cmd_subid;

    if (len > 0 && len <= FXMAC_MSG_DATA_LEN)
    {
        memcpy((void *)&msg.para[0], data, len);
    }
    else if (len > FXMAC_MSG_DATA_LEN)
    {
        FXMAC_MSG_ERROR("Tx msg para len %d is greater than the max len %d", len, FXMAC_MSG_DATA_LEN);
        return -1;
    }
    FXMAC_MSG_INFO("Tx msg: cmdid:%d, subid:%d, status0:%d, len:%d, tail:%d",
                   msg.cmd_type, msg.cmd_subid, msg.status0, len, tx_tail);
    memcpy((void *)(pdata->msg_regs + FXMAC_MSG(tx_tail)), &msg, sizeof(msg));
    tx_tail = FXmacMsgTxRingWrap(pdata, ++tx_tail);
    FXMAC_MSG_WRITE(pdata, FXMAC_MSG_TX_MSG_TAIL, tx_tail | FXMAC_MSG_BIT(TX_MSG_INT));
    pdata->msg_ring.tx_msg_wr_tail = tx_tail;
    if (wait)
    {
        tx_head = FXMAC_MSG_READ(pdata, FXMAC_MSG_TX_MSG_HEAD) & 0xff;
        while (tx_head != tx_tail)
        {
            FDriverUdelay(1);
            tx_head = FXMAC_MSG_READ(pdata, FXMAC_MSG_TX_MSG_HEAD) & 0xff;
        }
        memcpy((void *)&msg_rx,
               (void *)(pdata->msg_regs + FXMAC_MSG(pdata->msg_ring.tx_msg_rd_tail)), MSG_HDR_LEN);
        if (!(msg_rx.status0 & FXMAC_MSG_FLAGS_MSG_COMP))
        {
            FXMAC_MSG_ERROR("Msg process error, cmdid:%d, subid:%d, status0:%d, "
                            "tail:%d",
                            msg.cmd_type, msg.cmd_subid, msg.status0, tx_tail);
            return -1;
        }
    }
    return ret;
}
