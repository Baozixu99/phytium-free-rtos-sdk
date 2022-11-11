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
 * FilePath: fsdio_os.h
 * Date: 2022-07-25 09:14:34
 * LastEditTime: 2022-07-25 09:14:34
 * Description:  This files is for 
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/7/27   init commit
 */
#ifndef  DRIVERS_FSDIO_OS_H
#define  DRIVERS_FSDIO_OS_H

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/
#include <FreeRTOS.h>
#include <semphr.h>
#include <event_groups.h>

#include "fkernel.h"
#include "fparameters.h"
#include "fsdio.h"

/************************** Constant Definitions *****************************/
#define FFREERTOS_SDIO_OK                   FT_SUCCESS
#define FFREERTOS_SDIO_SEMA_ERR             FT_CODE_ERR(ErrModPort, ErrBspMmc, 1)
#define FFREERTOS_SDIO_HOST_ERR             FT_CODE_ERR(ErrModPort, ErrBspMmc, 2)
#define FFREERTOS_SDIO_EVT_ERR              FT_CODE_ERR(ErrModPort, ErrBspMmc, 3)
#define FFREERTOS_SDIO_READ_ERR             FT_CODE_ERR(ErrModPort, ErrBspMmc, 4)
#define FFREERTOS_SDIO_WRITE_ERR            FT_CODE_ERR(ErrModPort, ErrBspMmc, 5)

#define FFREERTOS_SDIO_IRQ_PRIORITY         IRQ_PRIORITY_VALUE_12

#define FFREERTOS_SDIO_MEDIUM_TF            0U /* sdio medium is tf card */
#define FFREERTOS_SDIO_MEDIUM_EMMC          1U /* sdio medium is emmc */

#define FFREERTOS_SDIO_TRANS_READ           0U /* sdio trans direction is read */
#define FFREERTOS_SDIO_TRANS_WRITE          1U /* sdio trans direction is write */

#define FFREERTOS_SDIO_MAX_TRANS_BLOCK      64U /* max num of block involved in one transfer */
#define FFREERTOS_SDIO_BLOCK_SIZE           FSDIO_DEFAULT_BLOCK_SZ

#define FFREERTOS_SDIO_CMD_TIMEOUT          5000U
/**************************** Type Definitions *******************************/
typedef struct
{
    boolean en_dma;  /* TRUE: data transfer in DMA mode, FALSE: PIO mode */
    u32 medium_type; /* use FFREERTOS_SDIO_MEDIUM_* */
    FSdioEvtHandler card_detect_handler; /* callback for card detect status change */
    void *card_detect_args;   
} FFreeRTOSSdioConifg; /* freertos sdio config */

typedef struct
{
    FSdio ctrl;
    FFreeRTOSSdioConifg config;
    SemaphoreHandle_t locker;
    EventGroupHandle_t evt;
#define FFREERTOS_SDIO_CMD_TRANS_DONE             (0x1 << 0) /* evt bit when command / data finished */
#define FFREERTOS_SDIO_DAT_TRANS_DONE             (0x1 << 1)
#define FFREERTOS_SDIO_ERROR_OCCURRED             (0x1 << 2) /* evt bit when ctrl in error occurred */
    void *host_data; /* store private data of card and host */
    volatile FSdioIDmaDesc rw_desc[FFREERTOS_SDIO_MAX_TRANS_BLOCK]; /* dma descriptor entry */
    FSdioCmdData cmd_data; /* command information */
    FSdioData trans_data; /* data information */
} FFreeRTOSSdio; /* freertos sdio instance */

typedef struct
{
    uintptr start_block; /* block to start read/write */
    fsize_t block_num; /* num of block to read/write */
    u8 *buf; /* buffer read from / write to  */
    fsize_t buf_len; /* bytes of buffer contents */
    u32 trans_type; /* use FFREERTOS_SDIO_TRANS_* */
} FFreeRTOSSdioMessage; /* freertos sdio transfer message */

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/* init and get sdio instance */
FFreeRTOSSdio *FFreeRTOSSdioInit(u32 instance_id, const FFreeRTOSSdioConifg *config);

/* deinit sdio instance */
FError FFreeRTOSSdioDeInit(FFreeRTOSSdio *const instance);

/* start sdio transfer and wait transfer done in this function */
FError FFreeRTOSSdioTransfer(FFreeRTOSSdio *const instance, const FFreeRTOSSdioMessage *message);


#ifdef __cplusplus
}
#endif

#endif