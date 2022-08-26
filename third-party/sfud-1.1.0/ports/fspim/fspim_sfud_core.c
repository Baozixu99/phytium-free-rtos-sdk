/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc. 
 * All Rights Reserved.
 *  
 * This program is OPEN SOURCE software: you can redistribute it and/or modify it  
 * under the terms of the Phytium Public License as published by the Phytium Technology Co.,Ltd,  
 * either version 1.0 of the License, or (at your option) any later version. 
 *  
 * This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY;  
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the Phytium Public License for more details. 
 *  
 * 
 * FilePath: fspim_sfud_core.c
 * Date: 2022-08-19 10:09:23
 * LastEditTime: 2022-08-19 10:09:23
 * Description:  This files is for 
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "parameters.h"
#include "ft_assert.h"
#include "fsleep.h"

#include "fspim_os.h"

#include "fspim_sfud_core.h"

typedef struct
{
    u32 spi_id;
    FFreeRTOSSpim *spi;
    FFreeRTOSSpimConifg spi_cfg;
    FFreeRTOSSpiMessage spi_msg;
    boolean is_inited;
} FSpimSfudOs;

static FSpimSfudOs sfud_instance =
{
    .spi_id = FSPI2_ID,
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
};

static sfud_err FSpimReadWrite(const sfud_spi *flash, const uint8_t *write_buf, size_t write_size, uint8_t *read_buf,
                               size_t read_size)
{
    sfud_err result = SFUD_SUCCESS;
    FError err = FT_SUCCESS;
    FSpimSfudOs *user_data = (FSpimSfudOs *)flash->user_data;

    SFUD_DEBUG("spi_write_read@%p beg+++++++++++++++++++++++++++++++++++++++++++++++++++", flash);
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
    user_data->spi_msg.tx_buf = write_buf;
    user_data->spi_msg.tx_len = write_size;
    err = FFreeRTOSSpimTransfer(user_data->spi, &user_data->spi_msg);
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
    SFUD_DEBUG("spi_write_read@%p end+++++++++++++++++++++++++++++++++++++++++++++++++++", flash);

    return result;    
}

sfud_err FSpimProbe(sfud_flash *flash)
{
    FSpimSfudOs *user_data = &sfud_instance;

    /* sfud_spi_port_init will be called for each flash candidate, 
    and we just do controller init for the first time */
    if (FALSE == user_data->is_inited)
    {
        /* init spi controller */
        user_data->spi = FFreeRTOSSpimInit(user_data->spi_id, &user_data->spi_cfg);
        if (NULL == user_data->spi)
        {
            return SFUD_ERR_NOT_FOUND;
        }

        user_data->is_inited = TRUE;        
    }

    flash->user_data = user_data;
    flash->spi.wr = FSpimReadWrite;

    return SFUD_SUCCESS;
}