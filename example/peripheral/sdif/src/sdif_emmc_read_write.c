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
 * FilePath: sdif_emmc_read_write.c
 * Date: 2022-07-25 15:58:24
 * LastEditTime: 2022-07-25 15:58:25
 * Description: This file is for providing functions used in cmd_sd.c file.
 *
 * Modify History:
 *  Ver      Who           Date         Changes
 * -----    ------       --------      --------------------------------------
 *  1.0    zhugengyu     2022/8/26     first commit
 *  2.0    liqiaozhong   2024/4/22     add no letter shell mode, adapt to auto-test system
 */

/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "fdebug.h"
#include "fkernel.h"

#include "fsdif_timing.h"
#include "fsl_sdmmc.h"
/************************** Constant Definitions *****************************/
#define FSD_EXAMPLE_TAG "FSD_EXAMPLE"
#define FSD_ERROR(format, ...)   FT_DEBUG_PRINT_E(FSD_EXAMPLE_TAG, format, ##__VA_ARGS__)
#define FSD_WARN(format, ...)    FT_DEBUG_PRINT_W(FSD_EXAMPLE_TAG, format, ##__VA_ARGS__)
#define FSD_INFO(format, ...)    FT_DEBUG_PRINT_I(FSD_EXAMPLE_TAG, format, ##__VA_ARGS__)
#define FSD_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FSD_EXAMPLE_TAG, format, ##__VA_ARGS__)

#ifdef CONFIG_TARGET_PHYTIUMPI
#define SD_CONTROLLER_ID            FSDIO0_ID
#else
#define SD_CONTROLLER_ID            FSDIO1_ID
#endif

/* user-define */
#define SD_WORK_IRQ                 TRUE /* 0 for POLL mode, 1 for IRQ mode */
#define SD_WORK_DMA                 TRUE /* 0 for PIO mode, 1 for DMA mode */
#define SD_BLOCK_SIZE               512U
#define SD_START_BLOCK              0U
#define SD_USE_BLOCK                3U
#define RESULT_DISPLAY_LEN          64U /* 用于展示的结果长度 */
#define SDIF_EXAMPLE_TASK_PRIORITY  3U
#define TIMER_OUT                   (pdMS_TO_TICKS(4000U))

#if SD_WORK_DMA
#define SD_MAX_RW_BLK               10U
#else
#define SD_MAX_RW_BLK               4U
#endif
/**************************** Type Definitions *******************************/
enum
{
    SDIF_EXAMPLE_SUCCESS = 0,
    SDIF_EXAMPLE_UNKNOWN_STATE,
    SDIF_EXAMPLE_INIT_FAILURE,
    SDIF_EXAMPLE_WRITE_READ_FAILURE,             
};
/************************** Variable Definitions *****************************/
static sdmmchost_config_t s_inst_config;
static sdmmc_mmc_t s_inst;

static QueueHandle_t xQueue = NULL;

static u8 write_buf[SD_MAX_RW_BLK * SD_BLOCK_SIZE] __attribute((aligned(SD_BLOCK_SIZE))) = {0};
static u8 read_buf[SD_MAX_RW_BLK * SD_BLOCK_SIZE] __attribute((aligned(SD_BLOCK_SIZE))) = {0};
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/
static status_t EmmcInit(void)
{
    status_t err = kStatus_Success;

    err = MMC_CfgInitialize(&s_inst, &s_inst_config);
    if (err != kStatus_Success)
    {
        FSD_ERROR("Init MMC failed, err = %d !!!", err);
        goto task_exit;
    }

task_exit:
    FSD_INFO("%s return with err: %d", __func__, err);
    return err;
}

static status_t EmmcWriteRead(void)
{
    status_t err = kStatus_Success;

    memset(write_buf, 0, sizeof(write_buf));
    memset(read_buf, 0, sizeof(write_buf));

    for (u32 i = 0; i < SD_USE_BLOCK; i++) /* copy string to write buffer as each block */
    {
        memset((write_buf + i * SD_BLOCK_SIZE), (SD_START_BLOCK + i + 1), SD_BLOCK_SIZE);
    }

    err = MMC_WriteBlocks(&s_inst.card, write_buf, SD_START_BLOCK, SD_USE_BLOCK);
    if (err != kStatus_Success)
    {
        FSD_ERROR("eMMC write fail.");
        goto write_read_exit;
    }

    err = MMC_ReadBlocks(&s_inst.card, read_buf, SD_START_BLOCK, SD_USE_BLOCK);
    if (err == kStatus_Success)
    {   
        if (memcmp(write_buf, read_buf, (SD_MAX_RW_BLK * SD_BLOCK_SIZE)) == 0)
        {
            taskENTER_CRITICAL();
            printf("write_buf...\r\n");
            FtDumpHexByte(write_buf, RESULT_DISPLAY_LEN);
            printf("read_buf...\r\n");
            FtDumpHexByte(read_buf, RESULT_DISPLAY_LEN);
            taskEXIT_CRITICAL(); 
        }
        else
        {
            err = kStatus_Fail;
            FSD_ERROR("write_buf != read_buf, eMMC write or read failed.");
            goto write_read_exit;
        }
    }
    else
    {
        FSD_ERROR("eMMC read fail.");
        goto write_read_exit;
    }

write_read_exit:
    FSD_INFO("%s return with err: %d", __func__, err);
    return err;
}

static void SdifEmmcReadWriteTask(void)
{
    FError err = kStatus_Success;
    int task_res = SDIF_EXAMPLE_SUCCESS;

    err = EmmcInit();
    if (err != kStatus_Success)
    {
        task_res = SDIF_EXAMPLE_INIT_FAILURE;
        FSD_ERROR("EmmcInit failed.");
        goto task_exit;
    }

    err = EmmcWriteRead();
    if (err != kStatus_Success)
    {
        task_res = SDIF_EXAMPLE_WRITE_READ_FAILURE;
        FSD_ERROR("EmmcWriteRead failed.");
        goto task_exit;
    }

task_exit:
    xQueueSend(xQueue, &task_res, 0);

    if (&s_inst.card != NULL)
    {
        MMC_Deinit(&s_inst.card);
    }
    SDMMC_OSADeInit();
    FSdifTimingDeinit();

    vTaskDelete(NULL);
}

int FFreeRTOSEmmcWriteRead(void)
{
    BaseType_t xReturn = pdPASS; /* 定义一个创建信息返回值，默认为 pdPASS */
    int task_res = SDIF_EXAMPLE_UNKNOWN_STATE;

    /* 进入操作任务前的准备工作 */
    xQueue = xQueueCreate(1, sizeof(int)); /* 创建消息队列 */
    if (xQueue == NULL)
    {
        FSD_ERROR("xQueue create failed.");
        goto exit;
    }

    FSdifTimingInit();
    SDMMC_OSAInit();

    memset(&s_inst_config, 0, sizeof(s_inst_config));
    memset(&s_inst, 0, sizeof(s_inst));

    s_inst_config.hostId = FSDIF0_ID;
    s_inst_config.hostType = kSDMMCHOST_TYPE_FSDIF;
    s_inst_config.cardType = kSDMMCHOST_CARD_TYPE_EMMC;
    s_inst_config.enableDMA = SD_WORK_DMA;
    s_inst_config.enableIrq = SD_WORK_IRQ;
    s_inst_config.timeTuner = FSdifGetTimingSetting;
    s_inst_config.endianMode = kSDMMCHOST_EndianModeLittle;
    s_inst_config.maxTransSize = SD_MAX_RW_BLK * SD_BLOCK_SIZE;
    s_inst_config.defBlockSize = SD_BLOCK_SIZE;
    s_inst_config.cardClock = MMC_CLOCK_52MHZ;

    /* 创建主要操作任务 */
    xReturn = xTaskCreate((TaskFunction_t)SdifEmmcReadWriteTask,
                          (const char *)"SdifEmmcReadWriteTask",
                          (uint16_t)4096,
                          NULL,
                          (UBaseType_t)SDIF_EXAMPLE_TASK_PRIORITY,
                          NULL);
    if (xReturn == pdFAIL)
    {
        FSD_ERROR("xTaskCreate SdifTFReadWriteTask failed.");
        goto exit;
    }

    xReturn = xQueueReceive(xQueue, &task_res, TIMER_OUT);
    if (xReturn == pdFAIL)
    {
        FSD_ERROR("xQueue receive timeout.");
        goto exit;
    }

exit:
    if (xQueue != NULL)
    {
        vQueueDelete(xQueue);
    }

    if (task_res != SDIF_EXAMPLE_SUCCESS)
    {
        printf("%s@%d: SDIF eMMC write and read example [failure], task_res = %d\r\n", __func__, __LINE__, task_res);
        return task_res;
    }
    else
    {
        printf("%s@%d: SDIF eMMC write and read example [success].\r\n", __func__, __LINE__);
        return task_res;
    }
}