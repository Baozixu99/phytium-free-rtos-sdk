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
* FilePath: fsdif_msg_common.c
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
#define FSDIF_MSG_ERROR(expr) (((expr) == 0) ? FSDIF_SUCCESS : FSDIF_ERR_AP_MSG_FAIL)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define FSDIF_DEBUG_TAG       "FSDIF-MSG"
#define FSDIF_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FSDIF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSDIF_WARN(format, ...) FT_DEBUG_PRINT_W(FSDIF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSDIF_INFO(format, ...) FT_DEBUG_PRINT_I(FSDIF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSDIF_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FSDIF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSDIF_VERBOS(format, ...) \
    FT_DEBUG_PRINT_V(FSDIF_DEBUG_TAG, format, ##__VA_ARGS__)
/************************** Function Prototypes ******************************/

/*****************************************************************************/
void *FSdifMsgMemcpy(void *dest, const void *src, u32 len)
{
    while (len >= 8)
    {
        *(u64 *)dest = *(u64 *)src;
        dest += 8;
        src += 8;
        len -= 8;
    }
    while (len)
    {
        *(u8 *)dest = *(u8 *)src;
        dest++;
        src++;
        len--;
    }
    return dest;
}

static int FSdifSendMessage(FSdifMsgCtrl *const instance, FSdifMsgInfo *msg)
{
    FSdifMsgDev *dev_msg = &(instance->config.dev_msg);
    u32 tx_tail;
    u32 tx_head;
    u32 tx_curr;
    int status1 = 0;
    int i;

    tx_tail = FSDIF_READ_REG32(dev_msg->regfile, FMSG_TX_TAIL) & 0xffff;
    tx_head = FSDIF_READ_REG32(dev_msg->regfile, FMSG_TX_HEAD) & 0xffff;

    if (((tx_tail + 1) % FMSG_MAX_RING_ITEMS) == tx_head)
    {
        FSDIF_ERROR("mci ring buff full");
        return -1;
    }

    tx_curr = tx_tail;
    tx_tail = (tx_tail + 1) % FMSG_MAX_RING_ITEMS;

    dev_msg->tx_shmem[tx_curr] = (FSdifMsgInfo *)(*(dev_msg->tx_shmem) + tx_curr);
    FSDIF_DEBUG("MCIAP shmem[%d] %x", tx_curr, dev_msg->tx_shmem[tx_curr]);
    FSDIF_DEBUG("MCIAP MSG CMD:%d, SCMD:%d", msg->cmd_type, msg->cmd_subid);

    /* write msg to shared memory */
    FSdifMsgMemcpy(dev_msg->tx_shmem[tx_curr], msg, sizeof(FSdifMsgInfo));

    /* upate tx tail pointer */
    FSDIF_WRITE_REG32(dev_msg->regfile, FMSG_TX_TAIL, tx_tail | FMSG_TX_TAIL_INT_BIT);

    /* wait and check status */
    for (i = 0; i < 200U; i++)
    {
        FSDIF_DATA_BARRIER();

        if ((dev_msg->tx_shmem[tx_curr]->status0 == FSDIF_MSG_STATUS0_GOING) ||
            (dev_msg->tx_shmem[tx_curr]->status0 == FSDIF_MSG_STATUS0_NOT_READY))
        {
            FSDIF_VERBOS("MCIAP [%d] not complete %x", tx_curr,
                         dev_msg->tx_shmem[tx_curr]->status0);
        }
        else if (dev_msg->tx_shmem[tx_curr]->status0 == FSDIF_MSG_STATUS0_SUCCESS)
        {
            status1 = (int)dev_msg->tx_shmem[tx_curr]->status1;
            FSDIF_DEBUG("MCIAP [%d] complete and return %d", tx_curr, status1);
            dev_msg->tx_shmem[tx_curr]->status0 = 0;
            dev_msg->tx_shmem[tx_curr]->status1 = 0;
            break;
        }
        else if (dev_msg->tx_shmem[tx_curr]->status0 == FSDIF_MSG_STATUS0_GENERIC_ERROR)
        {
            FSDIF_ERROR("MCIAP [%d] status error %x", tx_curr,
                        dev_msg->tx_shmem[tx_curr]->status0);
            status1 = -1;
            dev_msg->tx_shmem[tx_curr]->status0 = 0;
            dev_msg->tx_shmem[tx_curr]->status1 = 0;
            break;
        }

        FDriverUdelay(40);
    }

    FSDIF_DEBUG("MCIAP %s end [%d] times:%d status0/1:%d/%d", __func__, tx_curr, i,
                dev_msg->tx_shmem[tx_curr]->status0, status1);
    return status1;
}

void FSdifMsgSetIntMask(FSdifMsgCtrl *const instance)
{
    u32 int_mask;
    FSdifMsgDev *dev_msg = &(instance->config.dev_msg);

    int_mask = (u32)(~FMSG_RX_TAIL_INT_BIT);
    FSDIF_WRITE_REG32(dev_msg->regfile, FMSG_S2M_INT_MASK, int_mask);
    FSDIF_DEBUG("set int mask 0x%x", int_mask);
}

FError FSdifMsgHostInit(FSdifMsgCtrl *const instance)
{
    FSdifMsgInfo *msg = &(instance->sdif_msg);
    FSdifMsgDataInit *data = (FSdifMsgDataInit *)msg->para;

    memset(msg, 0U, sizeof(*msg));

    msg->cmd_type = FMSG_CMD_SET;
    msg->cmd_subid = FSDIF_MSG_CMD_SET_INIT;

    memcpy(data, &(instance->config.init), sizeof(FSdifMsgDataInit));

    FSDIF_DEBUG("+++++++[HOST INIT]+++++++");
    FSDIF_DEBUG("MCIAP host->mmc->caps %x host->clk_rate %ld", data->caps, data->clk_rate);

    return FSDIF_MSG_ERROR(FSdifSendMessage(instance, msg));
}

FError FSdifMsgHostInitHW(FSdifMsgCtrl *const instance)
{
    FSdifMsgInfo *msg = &(instance->sdif_msg);
    FError err;

    memset(msg, 0U, sizeof(*msg));

    msg->cmd_type = FMSG_CMD_SET;
    msg->cmd_subid = FSDIF_MSG_CMD_SET_INIT_HW;

    FSDIF_DEBUG("+++++++[HOST INIT HW]+++++++");
    err = FSDIF_MSG_ERROR(FSdifSendMessage(instance, msg));
    FSDIF_DEBUG("init hardware done!");
    return err;
}

FError FSdifMsgHostResetHW(FSdifMsgCtrl *const instance)
{
    FSdifMsgInfo *msg = &(instance->sdif_msg);

    memset(msg, 0U, sizeof(*msg));

    msg->cmd_type = FMSG_MSG_CMD_DEFAULT;
    msg->cmd_subid = FSDIF_CMD_DEFAULT_HW_RESET;

    FSDIF_DEBUG("+++++++[HOST RESET HW]+++++++");
    return FSDIF_MSG_ERROR(FSdifSendMessage(instance, msg));
}

FError FSdifMsgHostDeInitHw(FSdifMsgCtrl *const instance)
{
    FSdifMsgInfo *msg = &(instance->sdif_msg);

    memset(msg, 0U, sizeof(*msg));

    msg->cmd_type = FMSG_CMD_SET;
    msg->cmd_subid = FSDIF_MSG_CMD_SET_DEINIT_HW;

    FSDIF_DEBUG("+++++++[HOST DEINIT HW]+++++++");
    return FSDIF_MSG_ERROR(FSdifSendMessage(instance, msg));
}

FError FSdifMsgRestDMA(FSdifMsgCtrl *const instance)
{
    FSdifMsgInfo *msg = &(instance->sdif_msg);

    memset(msg, 0U, sizeof(*msg));

    msg->cmd_type = FMSG_MSG_CMD_DEFAULT;
    msg->cmd_subid = FSDIF_CMD_DEFAULT_ADMA_RESET;

    FSDIF_DEBUG("+++++++[RESET DMA]+++++++");
    return FSDIF_MSG_ERROR(FSdifSendMessage(instance, msg));
}

boolean FSdifMsgCheckifCardExists(FSdifMsgCtrl *const instance)
{
    FASSERT(instance && (instance->is_ready == FT_COMPONENT_IS_READY));
    FSdifMsgDev *dev_msg = &(instance->config.dev_msg);

    if (instance->config.non_removable)
    {
        return TRUE;
    }

    u32 status = FSDIF_READ_REG32(dev_msg->regfile, FSDIF_CARD_DETECT_OFFSET);
    return ((status & 0x1) == 0x1) ? FALSE : TRUE;
}

boolean FSdifMsgCheckifCardBusy(FSdifMsgCtrl *const instance)
{
    FASSERT(instance && (instance->is_ready == FT_COMPONENT_IS_READY));
    FSdifMsgInfo *msg = &(instance->sdif_msg);

    memset(msg, 0U, sizeof(*msg));

    msg->cmd_type = FMSG_CMD_GET;
    msg->cmd_subid = FSDIF_MSG_CMD_GET_CARD_BUSY;

    FSDIF_DEBUG("+++++++[CARD BUSY]+++++++");
    return FSdifSendMessage(instance, msg) ? TRUE : FALSE;
}

boolean FSdifMsgCheckifCardReadonly(FSdifMsgCtrl *const instance)
{
    FASSERT(instance && (instance->is_ready == FT_COMPONENT_IS_READY));
    FSdifMsgInfo *msg = &(instance->sdif_msg);

    memset(msg, 0U, sizeof(*msg));

    msg->cmd_type = FMSG_CMD_GET;
    msg->cmd_subid = FSDIF_MSG_CMD_GET_READ_ONLY;

    FSDIF_DEBUG("+++++++[CARD RO]+++++++");
    return FSdifSendMessage(instance, msg) ? TRUE : FALSE;
}

FError FSdifMsgSetIos(FSdifMsgCtrl *const instance, FSdifMsgDataSetIos *ios)
{
    FASSERT(instance && (instance->is_ready == FT_COMPONENT_IS_READY));
    FASSERT(ios);
    FSdifMsgInfo *msg = &(instance->sdif_msg);
    FASSERT(sizeof(FSdifMsgDataSetIos) <= sizeof(msg->para));
    FSdifMsgDataSetIos *data = (FSdifMsgDataSetIos *)msg->para;
    FError ret;

    memset(msg, 0U, sizeof(*msg));

    msg->cmd_type = FMSG_CMD_SET;
    msg->cmd_subid = FSDIF_MSG_CMD_SET_SET_IOS;

    memcpy(data, ios, sizeof(*ios));
    FSDIF_DEBUG("+++++++[SET IOS]+++++++");
    FSDIF_DEBUG("MCIAP clock%d timing%x buswidth%d powermode%d", data->ios_clock,
                data->ios_timing, data->ios_bus_width, data->ios_power_mode);

    ret = FSDIF_MSG_ERROR(FSdifSendMessage(instance, msg));
    if (FSDIF_SUCCESS != ret)
    {
        FSDIF_ERROR("set ios failed !!!");
        return ret;
    }

    memcpy(&instance->cur_ios, ios, sizeof(*ios));
    return ret;
}

FError FSdifMsgSetVoltage(FSdifMsgCtrl *const instance, FSdifMsgDataSwitchVolt *volt)
{
    FASSERT(instance && (instance->is_ready == FT_COMPONENT_IS_READY));
    FSdifMsgInfo *msg = &(instance->sdif_msg);
    FError ret;

    memset(msg, 0U, sizeof(*msg));

    msg->cmd_type = FMSG_CMD_SET;
    msg->cmd_subid = FSDIF_MSG_CMD_SET_SWITCH_VOLT;
    msg->para[0] = volt->signal_voltage & 0xFF;

    FSDIF_DEBUG("+++++++[SET VOLT]+++++++");
    ret = FSDIF_MSG_ERROR(FSdifSendMessage(instance, msg));
    FSDIF_DEBUG("MCIAP %s %d voltage%d", __func__, ret, volt->signal_voltage);

    if (FSDIF_SUCCESS == ret)
    {
        memcpy(&instance->cur_volt, volt, sizeof(*volt));
    }

    return ret;
}

FError FSdifMsgEnableSdioIrq(FSdifMsgCtrl *const instance, boolean enable)
{
    FASSERT(instance && (instance->is_ready == FT_COMPONENT_IS_READY));
    FSdifMsgInfo *msg = &(instance->sdif_msg);

    memset(msg, 0U, sizeof(*msg));

    msg->cmd_type = FMSG_CMD_SET;
    msg->cmd_subid = FSDIF_MSG_CMD_SET_EN_SDIO_IRQ;
    msg->para[0] = enable & 0xFF;

    FSDIF_DEBUG("+++++++[SET SDIO IRQ]+++++++");
    return FSDIF_MSG_ERROR(FSdifSendMessage(instance, msg));
}

FError FSdifMsgStartCommand(FSdifMsgCtrl *const instance, FSdifMsgCommand *cmd)
{
    FSdifMsgInfo *msg = &(instance->sdif_msg);
    FSdifMsgDataStartCmd *data = (FSdifMsgDataStartCmd *)msg->para;

    memset(msg, 0U, sizeof(*msg));

    msg->cmd_type = FMSG_CMD_SET;
    msg->cmd_subid = FSDIF_MSG_CMD_SET_START_CMD;

    memcpy(data, &cmd->cmdinfo, sizeof(FSdifMsgDataStartCmd));
    instance->cur_cmd = cmd;

    FSDIF_DEBUG("+++++++[TRANS CMD]+++++++");
    FSDIF_DEBUG("MCIAP arg%x flags%x opcode%d rawcmd%x", data->cmd_arg, data->flags,
                data->opcode, data->raw_cmd);
    return FSDIF_MSG_ERROR(FSdifSendMessage(instance, msg));
}

FError FSdifMsgStartData(FSdifMsgCtrl *const instance, FSdifMsgData *cmddata)
{
    FSdifMsgInfo *msg = &(instance->sdif_msg);
    FSdifMsgDataStartData *data = (FSdifMsgDataStartData *)msg->para;

    memset(msg, 0U, sizeof(*msg));

    msg->cmd_type = FMSG_CMD_SET;
    msg->cmd_subid = FSDIF_MSG_CMD_SET_START_DATA;

    memcpy(data, &cmddata->datainfo, sizeof(FSdifMsgDataStartData));
    instance->cur_data = cmddata;

    FSDIF_DEBUG("+++++++[TRANS DATA]+++++++");
    FSDIF_DEBUG("MCIAP arg%x flags%x adtc%d dma%x blksz%d blknum%d rawcmd%x",
                data->cmd_arg, data->data_flags, data->adtc_type, data->adma_addr,
                data->mrq_data_blksz, data->mrq_data_blocks, data->raw_cmd);
    return FSDIF_MSG_ERROR(FSdifSendMessage(instance, msg));
}