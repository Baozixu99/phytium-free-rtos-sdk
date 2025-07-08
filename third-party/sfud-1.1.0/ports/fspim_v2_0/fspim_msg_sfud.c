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
 * FilePath: fspim_msg_sfud.c
 * Date: 2025-04-15 10:09:31
 * LastEditTime: 2025-04-15 10:09:31
 * Description:  This files is for spi msg sfud implementation.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   liyilun  2025-04-15   first release
 */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <string.h>

#include "fparameters.h"
#include "fassert.h"
#include "fsleep.h"

#include "stdbool.h"
#include "fspim_msg_os.h"

#include "fspim_msg_sfud.h"

#define           FOUR_BYTE_ADDR_MODE                       4
#define           THREE_BYTE_ADDR_MODE                      3


typedef struct
{
    u32 spi_id;
    FFreeRTOSSpim *spi;
    FFreeRTOSSpimConifg spi_cfg;
    FFreeRTOSSpiMessage spi_msg;
    boolean is_inited;
} FSpimSfudOs;

static FSpimSfudOs sfud_instance[FSPI_MSG_NUM] =
{
    [FSPI0_MSG_ID] =
    {
        .spi_id = FSPI0_MSG_ID,
        .spi = NULL,
        .spi_cfg =
        {
            /* you may need to assign spi mode according to flash spec. */
            .spi_mode = FFREERTOS_SPIM_MODE_0,
            .en_dma = FALSE,
            .inner_loopback = FALSE
        },
        .spi_msg =
        {
            .rx_buf = NULL,
            .rx_len = 0U,
            .tx_buf = NULL,
            .tx_len = 0U
        },
        .is_inited = FALSE
    },
    [FSPI1_MSG_ID] =
    {
        .spi_id = FSPI1_MSG_ID,
        .spi = NULL,
        .spi_cfg =
        {
            /* you may need to assign spi mode according to flash spec. */
            .spi_mode = FFREERTOS_SPIM_MODE_0,
            .en_dma = FALSE,
            .inner_loopback = FALSE
        },
        .spi_msg =
        {
            .rx_buf = NULL,
            .rx_len = 0U,
            .tx_buf = NULL,
            .tx_len = 0U
        },
        .is_inited = FALSE
    },
};


static sfud_err FSpimMsgReadWrite(const sfud_spi *spi, const uint8_t *write_buf, size_t write_size, uint8_t *read_buf,
                               size_t read_size)
{
    sfud_err result = SFUD_SUCCESS;
    FError err = FT_SUCCESS;
    FSpimSfudOs *user_data = (FSpimSfudOs *)spi->user_data;

    SFUD_DEBUG("spi_write_read@%p beg+++++++++++++++++++++++++++++++++++++++++++++++++++", spi);
    if (NULL != write_buf)
    {
        SFUD_DEBUG("++++  Write %d Bytes @%p: 0x%x, 0x%x, 0x%x",
                   write_size, write_buf,
                   ((write_size > 0)) ? write_buf[0] : 0xff,
                   ((write_size > 1)) ? write_buf[1] : 0xff,
                   ((write_size > 2)) ? write_buf[2] : 0xff);
    }

    /**
     * add your spi write and read code
     */
    user_data->spi_msg.rx_buf = read_buf;
    user_data->spi_msg.rx_len = read_size;
    user_data->spi_msg.tx_buf =(void *) write_buf;
    user_data->spi_msg.tx_len = write_size;
    err = FFreeRTOSSpimMsgTransfer(user_data->spi, &user_data->spi_msg);
    if (FT_SUCCESS != err)
    {
        result = write_size > 0 ? SFUD_ERR_WRITE : SFUD_ERR_READ;
        SFUD_ERROR("sfud transfer failed !!!");
    }

    if (NULL != read_buf)
    {
        SFUD_DEBUG("++++  Read %d Bytes @%p: 0x%x, 0x%x, 0x%x",
                   read_size, read_buf,
                   ((read_size > 0)) ? read_buf[0] : 0xff,
                   ((read_size > 1)) ? read_buf[1] : 0xff,
                   ((read_size > 2)) ? read_buf[2] : 0xff);
    }
    SFUD_DEBUG("spi_write_read@%p end+++++++++++++++++++++++++++++++++++++++++++++++++++", spi);

    return result;
}

sfud_err FSpimMsgProbe(sfud_flash *flash)
{
    sfud_spi *spi_p = &flash->spi;
    u32 spim_id;

    
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
    FSpimSfudOs *user_data = &sfud_instance[spim_id];
    /* sfud_spi_port_init will be called for each flash candidate,
    and we just do controller init for the first time */
    if (FALSE == user_data->is_inited)
    {
        /* init spi controller */
        user_data->spi = FFreeRTOSSpimMsgInit(user_data->spi_id, &user_data->spi_cfg);
        if (NULL == user_data->spi)
        {
            return SFUD_ERR_NOT_FOUND;
        }
        user_data->is_inited = TRUE;
    }
        user_data->spi->ctrl.addr_bytes = flash->addr_in_4_byte ? FOUR_BYTE_ADDR_MODE : THREE_BYTE_ADDR_MODE;
        flash->spi.user_data = user_data;
        flash->spi.wr = FSpimMsgReadWrite;

    return SFUD_SUCCESS;
}