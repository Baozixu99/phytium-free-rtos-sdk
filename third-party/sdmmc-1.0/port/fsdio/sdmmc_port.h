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
 * FilePath: sdmmc_port.h
 * Date: 2022-07-25 09:58:09
 * LastEditTime: 2022-07-25 09:58:09
 * Description:  This files is for 
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */
#ifndef  THIRD_PARTY_SDMMC_PORT_H
#define  THIRD_PARTY_SDMMC_PORT_H

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/
#include <FreeRTOS.h>

#include "ft_types.h"
#include "fsleep.h"
#include "kernel.h"
#include "ft_assert.h"
#include "ft_debug.h"

/************************** Constant Definitions *****************************/
#define true TRUE 
#define false FALSE

/**************************** Type Definitions *******************************/
typedef s32 sdmmc_err_t; 
typedef boolean bool;

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define SDMMC_DELAY(m)          fsleep_microsec(m)
#define SDMMC_ASSERT(a)         FASSERT(a)
#define WORD_ALIGNED_ATTR       __attribute__((aligned(4)))
#define SDMMC_PTR_DMA_CAP(a)    sdmmc_port_is_aligned_memory(a)

#define SDMMC_LOGE( tag, format, ... ) LOG_EARLY_IMPL(tag, format, FT_LOG_ERROR, E, ##__VA_ARGS__)
#define SDMMC_LOGW( tag, format, ... ) LOG_EARLY_IMPL(tag, format, FT_LOG_WARN, W, ##__VA_ARGS__)
#define SDMMC_LOGI( tag, format, ... ) LOG_EARLY_IMPL(tag, format, FT_LOG_INFO, I, ##__VA_ARGS__)
#define SDMMC_LOGD( tag, format, ... ) LOG_EARLY_IMPL(tag, format, FT_LOG_DEBUG, D, ##__VA_ARGS__)
#define SDMMC_LOGV( tag, format, ... ) LOG_EARLY_IMPL(tag, format, FT_LOG_VERBOSE, W, ##__VA_ARGS__)

#define SDMMC_LOG_BUFFER_HEXDUMP( tag, buffer, buff_len, level ) FtDumpHexWord((u32 *)(buffer), (u32)(buff_len))

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/* init sdmmc before use */
sdmmc_err_t sdmmc_port_init(void);

/* deinit sdmmc */
sdmmc_err_t sdmmc_port_deinit(void);

/* allocate aligned memory from local memory pool */
void *sdmmc_port_align_malloc(size_t want_size, boolean dma_capable);

/* free memory to local memory pool */
void sdmmc_port_align_free(void *pv);

/* check if address of buffer is aligned */
boolean sdmmc_port_is_aligned_memory(const void *buf_p);

#ifdef __cplusplus
}
#endif

#endif