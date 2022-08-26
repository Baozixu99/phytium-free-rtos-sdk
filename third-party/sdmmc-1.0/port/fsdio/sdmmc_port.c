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
 * FilePath: sdmmc_port.c
 * Date: 2022-07-25 09:58:29
 * LastEditTime: 2022-07-25 09:58:30
 * Description:  This files is for 
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */

/***************************** Include Files *********************************/
#include <FreeRTOS.h>
#include <semphr.h>
#include <event_groups.h>

#include "fmemory_pool.h"
#include "parameters.h"

#include "sdmmc_port.h"
#include "sdmmc_cmd.h"
#include "sdmmc_defs.h"

/************************** Constant Definitions *****************************/
#define SDMMC_ALIGNMENT         32U
#define SDMMC_DMA_BUF_SIZE      512U

/* reserve more bytes for 64-bit */
#if defined(__aarch64__)
#define SDMMC_MEM_BUF_SIZE      8192U * 2U
#else
#define SDMMC_MEM_BUF_SIZE      8192U
#endif
/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
static FMemp sdmmc_mem_pool;
static u8 mem_buf[SDMMC_MEM_BUF_SIZE];
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/
sdmmc_err_t sdmmc_port_init(void)
{
    FError err = FT_SUCCESS;
    if (FT_COMPONENT_IS_READY == sdmmc_mem_pool.is_ready)
        return err;

    taskENTER_CRITICAL(); /* no schedule when init */

    err = FMempInit(&sdmmc_mem_pool,  mem_buf, mem_buf + sizeof(mem_buf)); /* init memory pool */;

    taskEXIT_CRITICAL(); /* allow schedule after init */

    return (FT_SUCCESS == err) ? SDMMC_OK : SDMMC_FAIL;
}

sdmmc_err_t sdmmc_port_deinit(void)
{
    FError err = FT_SUCCESS;
    if (FT_COMPONENT_IS_READY != sdmmc_mem_pool.is_ready)
        return err;

    taskENTER_CRITICAL(); /* no schedule when deinit */

    FMempDeinit(&sdmmc_mem_pool); /* remove memroy pool */

    taskEXIT_CRITICAL(); /* allow schedule after deinit */

    return (FT_SUCCESS == err) ? SDMMC_OK : SDMMC_FAIL;
}

void *sdmmc_port_align_malloc(size_t want_size, boolean dma_capable)
{
    void *pv = NULL;
    FASSERT(FT_COMPONENT_IS_READY == sdmmc_mem_pool.is_ready);

    if (dma_capable)
    {
        if (want_size < SDMMC_DMA_BUF_SIZE) /* if allocated memory is needed by DMA, allocate at leaset 512 bytes */
        {
            want_size = SDMMC_DMA_BUF_SIZE;
        }
        else if (want_size % SDMMC_DMA_BUF_SIZE) /* round up size to 512 */
        {
            want_size = roundup(want_size, (size_t)SDMMC_DMA_BUF_SIZE);
        }
    }

    pv = FMempMallocAlign(&sdmmc_mem_pool, want_size, SDMMC_ALIGNMENT); /* allocate aligned memory */
    return pv;
}

void sdmmc_port_align_free(void *pv)
{
    FASSERT(FT_COMPONENT_IS_READY == sdmmc_mem_pool.is_ready);

    FMempFree(&sdmmc_mem_pool, pv);   
}

boolean sdmmc_port_is_aligned_memory(const void *buf_p)
{
    return ((uintptr)buf_p % SDMMC_ALIGNMENT == 0);
}