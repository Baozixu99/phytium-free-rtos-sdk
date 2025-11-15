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
* FilePath: fsdif_msg.c
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
#define FSDIF_DEBUG_TAG "FSDIF-MSG"
#define FSDIF_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FSDIF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSDIF_WARN(format, ...) FT_DEBUG_PRINT_W(FSDIF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSDIF_INFO(format, ...) FT_DEBUG_PRINT_I(FSDIF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSDIF_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FSDIF_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/
void FSdifMsgSetIntMask(FSdifMsgCtrl *const instance);
FError FSdifMsgHostInit(FSdifMsgCtrl *const instance);
FError FSdifMsgHostInitHW(FSdifMsgCtrl *const instance);
FError FSdifMsgHostResetHW(FSdifMsgCtrl *const instance);
FError FSdifMsgHostDeInitHw(FSdifMsgCtrl *const instance);
FError FSdifMsgRestDMA(FSdifMsgCtrl *const instance);
FError FSdifMsgStartCommand(FSdifMsgCtrl *const instance, FSdifMsgCommand *cmd);
FError FSdifMsgStartData(FSdifMsgCtrl *const instance, FSdifMsgData *data);
FError FSdifMsgEnableSdioIrq(FSdifMsgCtrl *const instance, boolean enable);

/*****************************************************************************/
/**
 * @name: FSdifMsgCfgInitialize
 * @msg: initialization SDIF controller instance
 * @return {FError} FSDIF_SUCCESS if initialization success, otherwise failed
 * @param {FSdifMsgCtrl} *instance, SDIF controller instance
 * @param {FSdifMsgConfig} *input_config, SDIF controller configure
 * @note get into card-detect mode after initialization, bus width = 1, card freq = 400kHz
 */
FError FSdifMsgCfgInitialize(FSdifMsgCtrl *const instance, const FSdifMsgConfig *input_config)
{
    FASSERT(instance && input_config);
    FError ret = FSDIF_SUCCESS;
    FSdifMsgDev *dev_msg = &(instance->config.dev_msg);

    if (FT_COMPONENT_IS_READY == instance->is_ready)
    {
        FSDIF_WARN("Device is already initialized!!!");
    }

    if (&instance->config != input_config)
    {
        instance->config = *input_config;
    }

    *dev_msg->tx_shmem = (FSdifMsgInfo *)dev_msg->shmem;
    *dev_msg->rv_shmem = (FSdifMsgInfo *)(*dev_msg->tx_shmem + FMSG_MAX_RING_ITEMS);
    FSDIF_DEBUG("tx_shmem = 0x%x rv_shmem = 0x%x", *dev_msg->tx_shmem, *dev_msg->rv_shmem);

    FSdifMsgSetIntMask(instance);

    ret = FSdifMsgHostInit(instance);
    if (FSDIF_SUCCESS != ret)
    {
        FSDIF_ERROR("host init failed !!!");
        return ret;
    }

    ret = FSdifMsgHostInitHW(instance);
    if (FSDIF_SUCCESS != ret)
    {
        FSDIF_ERROR("IOP host init hw failed !!!");
        return ret;
    }

    instance->is_ready = FT_COMPONENT_IS_READY;
    FSDIF_INFO("Device initialize success !!!");
    return ret;
}

void FSdifMsgDeInitialize(FSdifMsgCtrl *const instance)
{
    FASSERT(instance);

    if (instance->is_ready == FT_COMPONENT_IS_READY)
    {
        (void)FSdifMsgHostDeInitHw(instance);
        instance->is_ready = 0;
    }
}

FError FSdifMsgSetIDMAList(FSdifMsgCtrl *const instance, FSdifMsgIDmaDesc *desc,
                           uintptr desc_dma, u32 desc_num)
{
    FASSERT(instance && desc);

    if (FT_COMPONENT_IS_READY != instance->is_ready)
    {
        FSDIF_ERROR("Device is not yet initialized!!!");
        return FSDIF_ERR_NOT_INIT;
    }

    instance->desc_list.first_desc = desc; /* virtual address of first descriptors */
    instance->desc_list.first_desc_dma = desc_dma; /* physical address equals with virtual address */
    instance->desc_list.desc_num = desc_num; /* num of descriptors in the array */
    instance->desc_list.desc_trans_sz = FSDIF_IDMAC_MAX_BUF_SIZE; /* each descriptor can transfer at most 0x1000 bytes data */

    return FSdifMsgRestDMA(instance);
}

static void FSdifMsgDumpDMADescriptor(FSdifMsgCtrl *const instance, u32 desc_in_use)
{
    u32 loop;
    volatile FSdifMsgIDmaDesc *cur_desc = NULL;
    FUNUSED(cur_desc);
    if (instance->desc_list.first_desc)
    {
        for (loop = 0; loop < desc_in_use; loop++)
        {
            cur_desc = &(instance->desc_list.first_desc[loop]);
            FSDIF_DEBUG("descriptor@%p", cur_desc);
            FSDIF_DEBUG("\tattribute: 0x%x", cur_desc->attribute);
            FSDIF_DEBUG("\tnon1: 0x%x", cur_desc->non1);
            FSDIF_DEBUG("\tlen: 0x%x", cur_desc->len);
            FSDIF_DEBUG("\tnon2: 0x%x", cur_desc->non2);
            FSDIF_DEBUG("\taddr_lo: 0x%x", cur_desc->addr_lo);
            FSDIF_DEBUG("\taddr_hi: 0x%x", cur_desc->addr_hi);
            FSDIF_DEBUG("\tdesc_lo: 0x%x", cur_desc->desc_lo);
            FSDIF_DEBUG("\tdesc_hi: 0x%x", cur_desc->desc_hi);
        }
    }
}

#ifndef FIO_SCATTERED_DMA
FError FSdifMsgSetupDMADescriptor(FSdifMsgCtrl *const instance, FSdifMsgData *data)
{
    FASSERT(instance && data);
    FASSERT(instance->desc_list.first_desc);
    FSdifMsgIDmaDescList *desc_list = &instance->desc_list;
    volatile FSdifMsgIDmaDesc *cur_desc = NULL;
    u32 desc_num = 1U;
    u32 blksz = data->datainfo.mrq_data_blksz;
    u32 blknum = data->datainfo.mrq_data_blocks;
    u32 bytes_total = blksz * blknum;
    /* num of blocks can be transferred by one descriptor */
    u32 desc_blocks = instance->desc_list.desc_trans_sz / blksz;
    u32 remain_blocks = blknum; /* blocks still need to transfer */
    u32 trans_blocks;           /* blocks that will be transferred this round */
    uintptr next_desc_addr = 0U;
    uintptr buff_addr = data->buf_dma;
    boolean is_first = TRUE;
    boolean is_last = FALSE;
    u32 loop = 0;

    if (bytes_total > instance->desc_list.desc_trans_sz)
    {
        desc_num = ((bytes_total / instance->desc_list.desc_trans_sz) +
                    ((bytes_total % instance->desc_list.desc_trans_sz) ? 1 : 0));
    }

    if (desc_num > desc_list->desc_num)
    {
        FSDIF_ERROR("Transfer descriptor are not enough %d < %d !!!", desc_list->desc_num, desc_num);
        return FSDIF_ERR_SHORT_BUF;
    }

    FSDIF_DEBUG("DMA transfer 0x%x use %d desc, total %d", data->buf_dma, desc_num,
                desc_list->desc_num);

    /* setup DMA descriptor list, so that we just need to update buffer address
       in each transaction */
    memset((void *)desc_list->first_desc, 0, sizeof(FSdifMsgIDmaDesc) * desc_list->desc_num);

    for (loop = 0; loop < desc_num; loop++)
    {
        trans_blocks = (remain_blocks <= desc_blocks) ? remain_blocks : desc_blocks;
        cur_desc = &(desc_list->first_desc[loop]); /* virtual addr of current descriptor */
        next_desc_addr = desc_list->first_desc_dma + (loop + 1) * sizeof(FSdifMsgIDmaDesc); /* physical addr of next descriptor */

        is_first = (0U == loop) ? TRUE : FALSE;
        is_last = ((desc_num - 1U) == loop) ? TRUE : FALSE;

        /* set properity of descriptor entry */
        cur_desc->attribute = FSDIF_IDMAC_DES0_CH | FSDIF_IDMAC_DES0_OWN; /* descriptor list in chain, and set OWN bit  */
        cur_desc->attribute |= (is_first) ? FSDIF_IDMAC_DES0_FD : 0; /* is it the first entry ? */
        cur_desc->attribute |= (is_last) ? (FSDIF_IDMAC_DES0_LD | FSDIF_IDMAC_DES0_ER) : 0; /* is it the last entry ? */

        /* set data length in transfer */
        cur_desc->non1 = 0U;
        cur_desc->len = (u32)(trans_blocks * blksz);

        /* set data buffer for transfer */
        if (buff_addr % FSDIF_DMA_BUF_ALIGMENT) /* make sure buffer aligned and not cross page boundary */
        {
            FSDIF_ERROR("Data buffer 0x%x do not align to %d.", buff_addr, blksz);
            return FSDIF_ERR_DMA_BUF_UNALIGN;
        }

#ifdef __aarch64__
        cur_desc->addr_hi = UPPER_32_BITS(buff_addr);
        cur_desc->addr_lo = LOWER_32_BITS(buff_addr);
#else
        cur_desc->addr_hi = 0U;
        cur_desc->addr_lo = (u32)(buff_addr);
#endif

        /* set address of next descriptor entry, NULL for last entry */
        next_desc_addr = is_last ? 0U : next_desc_addr;
        if (next_desc_addr % sizeof(FSdifMsgIDmaDesc)) /* make sure dma descriptor aligned and not cross page boundary */
        {
            FSDIF_ERROR("DMA descriptor 0x%x do not align.", next_desc_addr);
            return FSDIF_ERR_DMA_BUF_UNALIGN;
        }

#ifdef __aarch64__
        cur_desc->desc_hi = UPPER_32_BITS(next_desc_addr);
        cur_desc->desc_lo = LOWER_32_BITS(next_desc_addr);
#else
        cur_desc->desc_hi = 0U;
        cur_desc->desc_lo = (u32)(next_desc_addr);
#endif

        buff_addr += cur_desc->len;
        remain_blocks -= trans_blocks;
    }

    /* flush cache of descripor list and transfer buffer */
    FSDIF_DATA_BARRIER();

    FSdifMsgDumpDMADescriptor(instance, desc_num);
    return FSDIF_SUCCESS;
}
#endif

FError FSdifMsgDMATransfer(FSdifMsgCtrl *const instance, FSdifMsgRequest *const mrq)
{
    FASSERT(instance);
    FASSERT(mrq);
    FError ret = FSDIF_SUCCESS;

    if (FT_COMPONENT_IS_READY != instance->is_ready)
    {
        FSDIF_ERROR("Device is not yet initialized!!!");
        return FSDIF_ERR_NOT_INIT;
    }

    /* for removable media, check if card exists */
    if ((FALSE == instance->config.non_removable) && (FALSE == FSdifMsgCheckifCardExists(instance)))
    {
        FSDIF_ERROR("Card is not detected !!!");
        return FSDIF_ERR_NO_CARD;
    }

    if (TRUE == FSdifMsgCheckifCardBusy(instance))
    {
        FSDIF_ERROR("Card is busy !!!");
        return FSDIF_ERR_CARD_BUSY;
    }

    FSDIF_DEBUG("============[%s-%d] begin ============", "CMD", mrq->command->cmdinfo.opcode);

    if (mrq->data != NULL)
    {
        ret = FSdifMsgRestDMA(instance);
        if (FSDIF_SUCCESS != ret)
        {
            FSDIF_ERROR("Reset DMA failed !!!");
            return ret;
        }

        /* Start data transaction */
        mrq->data->datainfo.adma_addr = instance->desc_list.first_desc_dma;
        return FSdifMsgStartData(instance, mrq->data);
    }

    /* Start command transaction */
    return ret = FSdifMsgStartCommand(instance, mrq->command);
}