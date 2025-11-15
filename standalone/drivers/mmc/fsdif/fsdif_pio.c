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
 * FilePath: fsdif_pio.c
 * Date: 2022-06-01 14:21:47
 * LastEditTime: 2022-06-01 14:21:47
 * Description:  This file is for PIO transfer related function implementation
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.1   zhugengyu  2022/6/6     modify according to tech manual.
 * 2.0   zhugengyu  2023/9/16    rename as sdif, support SD 3.0 and rework clock timing
 */
/***************************** Include Files *********************************/

#include "fdrivers_port.h"
#include "fsdif_hw.h"
#include "fsdif.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define FSDIF_DEBUG_TAG "FSDIF-PIO"
#define FSDIF_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FSDIF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSDIF_WARN(format, ...) FT_DEBUG_PRINT_W(FSDIF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSDIF_INFO(format, ...) FT_DEBUG_PRINT_I(FSDIF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSDIF_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FSDIF_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/
extern FError FSdifTransferCmd(FSdif *const instance_p, FSdifCmdData *const cmd_data_p);
extern FError FSdifPollWaitBusyCard(FSdif *const instance_p);

/*****************************************************************************/
/**
 * @name: FSdifPIOWriteData
 * @msg: Write data to fifo
 * @return {FError} FSDIF_SUCCESS if write success
 * @param {FSdif} *instance_p, SDIF controller instance
 * @param {FSdifData} *data_p, contents of transfer data
 */
static FError FSdifPIOWriteData(FSdif *const instance_p, FSdifData *data_p)
{
    FASSERT(data_p);
    FError ret = FSDIF_SUCCESS;
    u32 loop;
    uintptr base_addr = instance_p->config.base_addr;
    FASSERT((data_p->datalen) % sizeof(u32) == 0);
    const u32 wr_times = data_p->datalen / sizeof(u32); /* u8 --> u32 */
    u32 *wr_buf = (u32 *)data_p->buf;

    /* write fifo data */
    FSDIF_WRITE_REG(base_addr, FSDIF_CMD_OFFSET, FSDIF_CMD_DAT_WRITE);
    for (loop = 0; loop < wr_times; loop++)
    {
        FSDIF_WRITE_REG(base_addr, FSDIF_DATA_OFFSET, wr_buf[loop]);
    }

    return ret;
}

/**
 * @name: FSdifPIOReadData
 * @msg: Read data from fifo
 * @return {FError} FSDIF_SUCCESS if read success
 * @param {FSdif} *instance_p, SDIF controller instance
 * @param {FSdifData} *data_p, contents of transfer data
 */
FError FSdifPIOReadData(FSdif *const instance_p, FSdifData *data_p)
{
    FASSERT(data_p);
    FError ret = FSDIF_SUCCESS;
    u32 loop;
    uintptr base_addr = instance_p->config.base_addr;
    const u32 rd_times = data_p->datalen / sizeof(u32); /* u8 --> u32 */
    u32 *rd_buf = (u32 *)data_p->buf;

    /* while in PIO mode, max data transferred is 0x800 */
    if (data_p->datalen > FSDIF_MAX_FIFO_CNT)
    {
        FSDIF_ERROR("Fifo do not support writing more than 0x%x.", FSDIF_MAX_FIFO_CNT);
        return FSDIF_ERR_NOT_SUPPORT;
    }

    /* read data from fifo */
    for (loop = 0; loop < rd_times; loop++)
    {
        rd_buf[loop] = FSDIF_READ_REG(base_addr, FSDIF_DATA_OFFSET);
    }

    return ret;
}

/**
 * @name: FSdifPIOTransfer
 * @msg: Start command and data transfer in PIO mode
 * @return {FError} FSDIF_SUCCESS if transfer success, otherwise failed
 * @param {FSdif} *instance_p, SDIF controller instance
 * @param {FSdifCmdData} *cmd_data_p, contents of transfer command and data
 */
FError FSdifPIOTransfer(FSdif *const instance_p, FSdifCmdData *const cmd_data_p)
{
    FASSERT(instance_p);
    FASSERT(cmd_data_p);
    FError ret = FSDIF_SUCCESS;
    const boolean read = !(cmd_data_p->rawcmd & FSDIF_CMD_DAT_WRITE);
    uintptr base_addr = instance_p->config.base_addr;

    instance_p->cur_cmd = cmd_data_p;

    if (FT_COMPONENT_IS_READY != instance_p->is_ready)
    {
        FSDIF_ERROR("device is not yet initialized!!!");
        return FSDIF_ERR_NOT_INIT;
    }

    if (FSDIF_PIO_TRANS_MODE != instance_p->config.trans_mode)
    {
        FSDIF_ERROR("device is not configure in PIO transfer mode.");
        return FSDIF_ERR_INVALID_STATE;
    }

    /* for removable media, check if card exists */
    if ((FALSE == instance_p->config.non_removable) && (FALSE == FSdifCheckIfCardExists(base_addr)))
    {
        FSDIF_ERROR("card is not detected !!!");
        return FSDIF_ERR_NO_CARD;
    }

    /* check if previous command finished and card not busy */
    if (FSdifCheckCardBusy(instance_p))
    {
        return FSDIF_ERR_BUSY;
    }

    FSDIF_WRITE_REG(base_addr, FSDIF_RAW_INTS_OFFSET, 0xffffe);

    if ((NULL != cmd_data_p->data_p) && (cmd_data_p->data_p->buf))
    {
        /* while in PIO mode, max data transferred is 0x800 */
        if (cmd_data_p->data_p->datalen > FSDIF_MAX_FIFO_CNT)
        {
            FSDIF_ERROR("Fifo do not support writing more than 0x%x.", FSDIF_MAX_FIFO_CNT);
            return FSDIF_ERR_NOT_SUPPORT;
        }

        /* reset fifo and not use DMA */
        ret = FSdifResetCtrl(base_addr, FSDIF_CNTRL_FIFO_RESET | FSDIF_CNTRL_DMA_RESET);
        if (FSDIF_SUCCESS != ret)
        {
            return ret;
        }

        FSDIF_CLR_BIT(base_addr, FSDIF_CNTRL_OFFSET, FSDIF_CNTRL_USE_INTERNAL_DMAC);
        FSDIF_CLR_BIT(base_addr, FSDIF_BUS_MODE_OFFSET, FSDIF_BUS_MODE_DE);

        /* set transfer data length and block size */
        FSdifSetTransBytes(base_addr, cmd_data_p->data_p->datalen);
        FSdifSetBlockSize(base_addr, cmd_data_p->data_p->blksz);

        if (FALSE == read) /* if need to write, write to fifo before send command */
        {
            /* invalide buffer for data to write */

            FSDIF_DATA_BARRIER();

            ret = FSdifPIOWriteData(instance_p, cmd_data_p->data_p);
        }

        FSdifSetInterruptMask(instance_p, FSDIF_GENERAL_INTR, FSDIF_INTS_DATA_MASK, TRUE);
    }

    if (FSDIF_SUCCESS == ret) /* send command */
    {
        ret = FSdifTransferCmd(instance_p, cmd_data_p);
    }

    return ret;
}