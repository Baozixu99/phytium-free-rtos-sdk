/*
* Copyright : (C) 2025 Phytium Information Technology, Inc.
* All Rights Reserved.
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
* FilePath: fsdif_msg_intr.c
* Date: 2025-04-10 16:20:52
* LastEditTime: 2025-04-10 16:20:52
* Description:  This file is for sdif user interface definition
*
* Modify History:
*  Ver   Who        Date         Changes
* ----- ------     --------    --------------------------------------
* 1.0   zhugengyu  2025/4/10    init commit
*/
/***************************** Include Files *********************************/
#include "fdrivers_port.h"

#include "fsdif_msg_hw.h"
#include "fsdif_msg.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define FSDIF_DEBUG_TAG "FSDIF-INTR"
#define FSDIF_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FSDIF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSDIF_WARN(format, ...) FT_DEBUG_PRINT_W(FSDIF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSDIF_INFO(format, ...) FT_DEBUG_PRINT_I(FSDIF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSDIF_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FSDIF_DEBUG_TAG, format, ##__VA_ARGS__)

#define FSDIF_CALL_EVT_HANDLER(instance, evt, data)                           \
    if (instance->evt_handlers[evt])                                          \
    {                                                                         \
        instance->evt_handlers[evt](instance, instance->evt_args[evt], data); \
    }
/************************** Function Prototypes ******************************/
void *FSdifMsgMemcpy(void *dest, const void *src, u32 len);

/*****************************************************************************/

static boolean FSdifMsgHandleEvtCommandNext(FSdifMsgCtrl *const instance, FSdifMsgInfo *rxmsg)
{
    FSdifMsgDataCmdNext *data = (FSdifMsgDataCmdNext *)rxmsg->para;
    u32 events = data->events;

    if (!instance->cur_cmd)
    {
        return TRUE;
    }

    u32 opcode = instance->cur_cmd->cmdinfo.opcode;
    instance->cur_cmd->response[0] = (data->response0);
    instance->cur_cmd->response[1] = (data->response1);
    instance->cur_cmd->response[2] = (data->response2);
    instance->cur_cmd->response[3] = (data->response3);

    if (events & FSDIF_INTS_CMD_ERR_MASK)
    {
        FSDIF_ERROR("Cmd@0x%x, index:%d, arg: 0x%x", instance->cur_cmd,
                    instance->cur_cmd ? instance->cur_cmd->cmdinfo.opcode : -1,
                    instance->cur_cmd ? instance->cur_cmd->cmdinfo.cmd_arg : 0);
        FSDIF_ERROR("ERR:events:0x%x", events);
    }

    if ((events & FSDIF_INT_CMD_BIT) ||
        ((events & FSDIF_INT_HTO_BIT) && (SD_SWITCH_VOLTAGE == opcode)))
    {
        FSDIF_CALL_EVT_HANDLER(instance, FSDIF_EVT_CMD_DONE, data);
    }

    return TRUE;
}

static boolean FSdifMsgHandleEvtDataNext(FSdifMsgCtrl *const instance, FSdifMsgInfo *rxmsg)
{
    u32 events = ((u32 *)(rxmsg->para))[0];
    FASSERT(instance->cur_data);

    if (events & FSDIF_INTS_CMD_ERR_MASK)
    {
        FSDIF_ERROR("Data@0x%x, blks:%d, blk_sz:%d", instance->cur_data,
                    instance->cur_data ? instance->cur_data->datainfo.mrq_data_blocks : 0,
                    instance->cur_data ? instance->cur_data->datainfo.mrq_data_blksz : 0);
        FSDIF_ERROR("ERR:events:0x%x", events);
    }

    if (events & FSDIF_INT_DTO_BIT)
    {
        FSDIF_CALL_EVT_HANDLER(instance, FSDIF_EVT_DATA_DONE, NULL);
    }

    return TRUE;
}

static boolean FSdifMsgHandleEvtCardDetect(FSdifMsgCtrl *const instance, FSdifMsgInfo *rxmsg)
{
    u32 events = ((u32 *)(rxmsg->para))[0];

    FUNUSED(events);
    FSDIF_CALL_EVT_HANDLER(instance, FSDIF_EVT_CARD_DETECTED, NULL);
    return TRUE;
}

static boolean FSdifMsgHandleEvtErrOccurred(FSdifMsgCtrl *const instance, FSdifMsgInfo *rxmsg)
{
    FSdifMsgDataErrIrq *data = (FSdifMsgDataErrIrq *)rxmsg->para;

    u32 raw_ints = data->raw_ints;
    u32 dmac_status = data->dmac_status;
    u32 ints_mask = data->ints_mask;
    u32 dmac_mask = data->dmac_mask;
    u32 opcode = data->opcode;

    FUNUSED(dmac_status);
    FUNUSED(ints_mask);
    FUNUSED(dmac_mask);
    FUNUSED(opcode);

    FSDIF_ERROR("MCIAP ERR raw_ints:%x ints_mask:%x dmac_status:%x dmac_mask:%x cmd:%d",
                raw_ints, ints_mask, dmac_status, dmac_mask, opcode);

    if (raw_ints & FSDIF_INT_RE_BIT)
    {
        FSDIF_ERROR("Response err. 0x%x", FSDIF_INT_RE_BIT);
    }

    if (raw_ints & FSDIF_INT_RTO_BIT)
    {
        FSDIF_ERROR("Response timeout. 0x%x", FSDIF_INT_RTO_BIT);
    }

    if (dmac_status & FSDIF_DMAC_STATUS_DU)
    {
        FSDIF_ERROR("Descriptor un-readable. 0x%x", FSDIF_DMAC_STATUS_DU);
    }

    if (raw_ints & FSDIF_INT_RCRC_BIT)
    {
        FSDIF_ERROR("Response CRC. 0x%x", FSDIF_INT_RCRC_BIT);
    }

    if (raw_ints & FSDIF_INT_DCRC_BIT)
    {
        FSDIF_ERROR("Data CRC error. 0x%x", FSDIF_INT_DCRC_BIT);
    }

    FSDIF_CALL_EVT_HANDLER(instance, FSDIF_EVT_ERR_OCCURE, data);

    return TRUE;
}

static FError FSdifMsgRecvMessage(FSdifMsgCtrl *const instance)
{
    FASSERT(instance);
    FSdifMsgDev *dev_msg = &(instance->config.dev_msg);
    u32 rx_tail;
    u32 rx_head;
    u32 rx_curr;
    FSdifMsgInfo rx_msg = {0};
    u8 complete;

    rx_tail = FSDIF_READ_REG32(dev_msg->regfile, FMSG_RX_TAIL) & 0xffff;
    rx_head = FSDIF_READ_REG32(dev_msg->regfile, FMSG_RX_HEAD) & 0xffff;

    while (rx_tail != rx_head)
    {
        FSDIF_DEBUG("MCIAP %s rx_t:%d rx_h:%d", __func__, rx_tail, rx_head);

        rx_curr = rx_head;
        rx_head = (rx_head + 1) % FMSG_MAX_RX_RING_ITEMS;

        FSDIF_WRITE_REG32(dev_msg->regfile, FMSG_RX_HEAD, rx_head);

        dev_msg->rv_shmem[rx_curr] = (FSdifMsgInfo *)(*(dev_msg->rv_shmem) + rx_curr);
        FSDIF_DEBUG("MCIAP %s %d %x", __func__, rx_curr, dev_msg->rv_shmem[rx_curr]);

        /* read message from shared memory */
        FSdifMsgMemcpy(&rx_msg, dev_msg->rv_shmem[rx_curr], sizeof(FSdifMsgInfo));

        /* check recv message */
        switch (rx_msg.cmd_type)
        {
            case FMSG_CMD_REPORT:
                switch (rx_msg.cmd_subid)
                {
                    case FSDIF_MSG_CMD_REPORT_CMD_NEXT:
                        FSDIF_DEBUG("CMD NEXT");
                        complete = FSdifMsgHandleEvtCommandNext(instance, &rx_msg);
                        break;
                    case FSDIF_MSG_CMD_REPORT_DATA_NEXT:
                        FSDIF_DEBUG("DATA NEXT");
                        complete = FSdifMsgHandleEvtDataNext(instance, &rx_msg);
                        break;
                    case FSDIF_MSG_CMD_REPORT_CD_IRQ:
                        FSDIF_DEBUG("CARD DETECT");
                        complete = FSdifMsgHandleEvtCardDetect(instance, &rx_msg);
                        break;
                    case FSDIF_MSG_CMD_REPORT_ERR_IRQ:
                        FSDIF_ERROR("ERR OCCURRED");
                        complete = FSdifMsgHandleEvtErrOccurred(instance, &rx_msg);
                        break;
                    default:
                        FSDIF_ERROR("MCIAP invalid sub cmd %d", rx_msg.cmd_subid);
                        break;
                }
                break;
            default:
                FSDIF_ERROR("MCIAP invalid cmd %d", rx_msg.cmd_type);
                break;
        }

        dev_msg->rv_shmem[rx_curr]->status0 = complete;
    }

    return FSDIF_SUCCESS;
}

void FSdifMsgInterruptHandler(s32 vector, void *param)
{
    FSdifMsgCtrl *instance = (FSdifMsgCtrl *)param;
    FASSERT(instance && (instance->is_ready == FT_COMPONENT_IS_READY));
    FSdifMsgDev *dev_msg = &(instance->config.dev_msg);
    u32 s2m_event = FSDIF_READ_REG32(dev_msg->regfile, FMSG_S2M_INT_STATE);

    if (s2m_event & FMSG_TX_HEAD_INT_BIT)
    {
        s2m_event &= ~FMSG_TX_HEAD_INT_BIT;
        FSDIF_DEBUG("MCIAP %s MMC_TXRING_HEAD_INT %x", __func__, s2m_event);
        FSDIF_WRITE_REG32(dev_msg->regfile, FMSG_S2M_INT_STATE, s2m_event);
    }

    if (s2m_event & FMSG_RX_TAIL_INT_BIT)
    {
        s2m_event &= ~FMSG_RX_TAIL_INT_BIT;
        FSDIF_DEBUG("MCIAP %s MMC_RXRING_TAIL_INT %x", __func__, s2m_event);
        FSDIF_WRITE_REG32(dev_msg->regfile, FMSG_S2M_INT_STATE, s2m_event);
        (void)FSdifMsgRecvMessage(instance);
    }

    return;
}

void FSdifMsgRegisterEvtHandler(FSdifMsgCtrl *const instance, FSdifMsgEvtType evt,
                                FSdifMsgEvtHandler handler, void *handler_arg)
{
    FASSERT(instance);

    instance->evt_handlers[evt] = handler;
    instance->evt_args[evt] = handler_arg;
}