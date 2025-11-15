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
 * FilePath: fspim_msg_sfud.c
 * Date: 2025-3-10 14:53:44
 * LastEditTime: 2025-06-06 09:01:56
 * Description:  This file is for providing sfud func based on spi_msg.
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   liyilun    2025/06/06   first commit
 * */

#include "fspim_msg_sfud.h"
#include "fparameters.h"
#include "sfud_def.h"
#include "sdkconfig.h"
#include "fcpu_info.h"
#include "fio_mux.h"
#include "finterrupt.h"

#include "fspim_msg.h"
#include "fspim_msg_hw.h"

#ifndef SDK_CONFIG_H__
#warning "Please include sdkconfig.h"
#endif

#define FOUR_BYTE_ADDR_MODE  4
#define THREE_BYTE_ADDR_MODE 3


static u32 device_select_mask; /* 每一位用于指示那个设备被选择，如0x3 ,则 fspim0 ，fspim1 被选择 */

static FSpiMsgCtrl fspim[FSPI_MSG_NUM] = {0};


static sfud_err FSpiMsgWriteRead(const sfud_spi *spi, const uint8_t *write_buf,
                                 size_t write_size, uint8_t *read_buf, size_t read_size)
{
    sfud_err result = SFUD_SUCCESS;
    FSpiMsgCtrl *spi_msg_ctrl = (FSpiMsgCtrl *)spi->user_data;
    if (write_size)
    {
        SFUD_ASSERT(write_buf);
    }
    if (read_size)
    {
        SFUD_ASSERT(read_buf);
    }

    FSpiMsgSetChipSelection(spi_msg_ctrl, TRUE);

    SFUD_DEBUG("\r\nspim@%p beg+++++++++++++++++++++++++++++++++++++++++++++++++++", spi_msg_ctrl);
    SFUD_DEBUG("++++  Write %d Bytes @%p: 0x%x, 0x%x, 0x%x, 0x%x", write_size, write_buf,
               ((NULL != write_buf) && (write_size > 0)) ? write_buf[0] : 0xff,
               ((NULL != write_buf) && (write_size > 1)) ? write_buf[1] : 0xff,
               ((NULL != write_buf) && (write_size > 2)) ? write_buf[2] : 0xff,
               ((NULL != write_buf) && (write_size > 3)) ? write_buf[3] : 0xff);

    if (write_size)
    {
        result = FSpiMsgTransfer(spi_msg_ctrl, (void *)write_buf, NULL, write_size);
        if (result != SFUD_SUCCESS)
        {
            SFUD_ERROR("Write data failed");
            goto exit;
        }
    }


    if (read_size)
    {
        result = FSpiMsgTransfer(spi_msg_ctrl, NULL, (void *)read_buf, read_size);

        if (result != SFUD_SUCCESS)
        {
            SFUD_ERROR("Read data failed");
            goto exit;
        }
    }

    SFUD_DEBUG("++++  Read %d Bytes @%p: 0x%x, 0x%x", read_size, read_buf,
               ((NULL != read_buf) && (read_size > 0)) ? read_buf[0] : 0xff,
               ((NULL != read_buf) && (read_size > 0)) ? read_buf[1] : 0xff,
               ((NULL != read_buf) && (read_size > 0)) ? read_buf[2] : 0xff,
               ((NULL != read_buf) && (read_size > 1)) ? read_buf[3] : 0xff);

exit:
    FSpiMsgSetChipSelection(spi_msg_ctrl, FALSE);
    return result;
}

static void FSpiMsgIntrInit(FSpiMsgCtrl *ctrl, const FSpiMsgConfig *config)
{
    u32 cpu_id = 0;

    GetCpuId(&cpu_id);
    InterruptSetTargetCpus(config->irq_num, cpu_id);
    InterruptSetPriority(config->irq_num, config->irq_priority);

    ctrl->cmd_completion = CMD_MSG_NOT_COMPLETION;
    InterruptInstall(config->irq_num, FSpiMsgInterruptHandler, ctrl, NULL);

    InterruptUmask(config->irq_num);
}

sfud_err FSpiMsgProbe(sfud_flash *flash)
{
    sfud_spi *spi_p = &flash->spi;
    sfud_err result = SFUD_SUCCESS;
    FSpiMsgCtrl *spim_p;
    u32 spim_id = FSPI0_MSG_ID;

    if (!memcmp(FSPIM0_SFUD_NAME, spi_p->name, strlen(FSPIM0_SFUD_NAME)))
    {
        spim_id = FSPI0_MSG_ID;
    }
    else if (!memcmp(FSPIM1_SFUD_NAME, spi_p->name, strlen(FSPIM1_SFUD_NAME)))
    {
        spim_id = FSPI1_MSG_ID;
    }
    else
    {
        return SFUD_ERR_NOT_FOUND;
    }
    spim_p = &fspim[spim_id];

    FSpiMsgConfig input_cfg = *FSpiMsgLookupConfig(spim_id);

    FIOMuxInit();
    FIOPadSetSpimMux(spim_id);
    FSpiMsgIntrInit(spim_p, &input_cfg);

    memset(&fspim[spim_id], 0, sizeof(fspim[spim_id]));

    input_cfg.slave_dev_id = FSPI_MSG_SLAVE_DEV_0; // set slave device id
    input_cfg.n_bytes = FSPIM_1_BYTE;              // set transfer data width
    input_cfg.mode = FSPI_MSG_MODE_0;              // set spi mode (cpol and cpha)
    input_cfg.clk_div = FSPI_MSG_DEFAULT_CLK_DIV;  // set spi clock divider

    if (ERR_SPI_SUCEESS != FSpiMsgCfgInitialize(spim_p, &input_cfg))
    {
        SFUD_DEBUG("init spi failed");
        result = SFUD_ERR_INIT_FAILED;
        return result;
    }
    spim_p->addr_bytes = flash->addr_in_4_byte ? FOUR_BYTE_ADDR_MODE : THREE_BYTE_ADDR_MODE;
    flash->spi.wr = FSpiMsgWriteRead;
    flash->spi.user_data = &fspim[spim_id];

    device_select_mask |= 0x1;

    return result;
}