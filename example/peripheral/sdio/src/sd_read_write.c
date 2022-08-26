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
 * FilePath: sd_read_write.c
 * Date: 2022-07-25 15:58:24
 * LastEditTime: 2022-07-25 15:58:25
 * Description:  This files is for 
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */
/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include "ft_assert.h"
#include "ft_debug.h"
#include "fsleep.h"
#include "kernel.h"
#include "cache.h"

#include "fsdio_os.h"
/************************** Constant Definitions *****************************/
#define SD_WR_BUF_LEN       4096
#define SD_EVT_INIT_DONE    (0x1 << 0)
#define SD_EVT_WRITE_DONE   (0x1 << 1)
#define SD_EVT_READ_DONE    (0x1 << 2)
/************************** Variable Definitions *****************************/
static u8 sd_write_buffer[SD_WR_BUF_LEN] = {0};
static u8 sd_read_buffer[SD_WR_BUF_LEN] = {0};
static u32 sd_slot = 0U;
static EventGroupHandle_t sync = NULL;
static TaskHandle_t write_task = NULL;
static TaskHandle_t read_task = NULL;
static TimerHandle_t exit_timer = NULL;
static FFreeRTOSSdio *sdio = NULL;
static FFreeRTOSSdioConifg sdio_config =
{
    .en_dma = TRUE,
    .medium_type = FFREERTOS_SDIO_MEDIUM_TF,
    .card_detect_handler = NULL,
    .card_detect_args = NULL
};
static FFreeRTOSSdioMessage read_message =
{
    .buf = sd_read_buffer,
    .buf_len = SD_WR_BUF_LEN,
    .start_block = 0U,
    .block_num = 3U,
    .trans_type = FFREERTOS_SDIO_TRANS_READ
};
static FFreeRTOSSdioMessage write_message =
{
    .buf = sd_write_buffer,
    .buf_len = SD_WR_BUF_LEN,
    .start_block = 0U,
    .block_num = 3U,
    .trans_type = FFREERTOS_SDIO_TRANS_WRITE
};
static u32 run_times = 3U;
static boolean is_running = FALSE;
/***************** Macros (Inline Functions) Definitions *********************/
#define FSDIO_DEBUG_TAG "FSDIO-SD"
#define FSDIO_ERROR(format, ...)   FT_DEBUG_PRINT_E(FSDIO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSDIO_WARN(format, ...)    FT_DEBUG_PRINT_W(FSDIO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSDIO_INFO(format, ...)    FT_DEBUG_PRINT_I(FSDIO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSDIO_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FSDIO_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/

/*****************************************************************************/
static void SDExitCallback(TimerHandle_t timer)
{
    FError err = FT_SUCCESS;
    printf("exiting.....\r\n");

    if (write_task)
    {
        vTaskDelete(write_task);
        write_task = NULL;
    }

    if (read_task)
    {
        vTaskDelete(read_task);
        read_task = NULL;
    }

    if (sync)
    {
        vEventGroupDelete(sync);
        sync = NULL;
    }

    if (sdio)
    {
        err = FFreeRTOSSdioDeInit(sdio);
        sdio = NULL;
    }

    if (pdPASS != xTimerDelete(timer, 0)) /* delete timer ifself */
    {
        FSDIO_ERROR("delete exit timer failed !!!");
        exit_timer = NULL;
    }

    is_running = FALSE;        
}

static void SDSendEvent(u32 evt_bits)
{
    FASSERT(sync);
    BaseType_t x_result = pdFALSE;

    FSDIO_DEBUG("ack evt 0x%x", evt_bits);
    x_result = xEventGroupSetBits(sync, evt_bits);
}

static boolean SDWaitEvent(u32 evt_bits, TickType_t wait_delay)
{
    FASSERT(sync);
    EventBits_t ev;
    ev = xEventGroupWaitBits(sync, evt_bits,
                             pdTRUE, pdFALSE, wait_delay); /* wait for cmd/data done */
    if (ev & evt_bits)
    {
        return TRUE;
    }

    return FALSE;
}

static void SDInitTask(void * args)
{
    sdio = FFreeRTOSSdioInit(sd_slot, &sdio_config);
    if (NULL == sdio)
    {
        FSDIO_ERROR("init sdio failed !!!");
        goto task_exit;
    }

    SDSendEvent(SD_EVT_INIT_DONE);

task_exit:
    vTaskDelete(NULL); /* delete task itself */ 
}

static void SDReadTask(void * args)
{
    u32 times = 0U;
    const TickType_t wait_delay = pdMS_TO_TICKS(5000UL); /* wait for 5 seconds */
    const uintptr trans_len = read_message.block_num * FFREERTOS_SDIO_BLOCK_SIZE;

    FASSERT_MSG(trans_len <= SD_WR_BUF_LEN, "trans length exceed buffer limits");
    
    for (;;)
    {
        /* wait write finish to get the updated contents */
        if (!SDWaitEvent(SD_EVT_WRITE_DONE, portMAX_DELAY))
        {
            FSDIO_ERROR("sdio read timeout !!!");
            goto task_exit;
        }        

        printf("start read ...\r\n");

        memset(read_message.buf, 0U, trans_len);
        FError err = FFreeRTOSSdioTransfer(sdio, &read_message);
        if (FFREERTOS_SDIO_OK != err)
        {
            FSDIO_ERROR("sdio read failed !!!");
            goto task_exit;
        }

        FCacheDCacheFlushRange((uintptr)(void *)read_message.buf, trans_len);
        printf("==>Read from Block [%d:%d]\r\n", 
                read_message.start_block, 
                read_message.start_block + read_message.block_num);
        
        FtDumpHexByte(read_message.buf, trans_len);

        SDSendEvent(SD_EVT_READ_DONE); /* send read finish singal */
        vTaskDelay(wait_delay);

        if (++times > run_times)
            break;
    }

task_exit:
    printf("exit from read task \r\n");
    vTaskSuspend(NULL); /* suspend task */
}

static void SDWriteTask(void * args)
{
    u32 times = 0U;
    const TickType_t wait_delay = pdMS_TO_TICKS(2000UL); /* wait for 2 seconds */
    FError err;
    const uintptr trans_len = write_message.block_num * FFREERTOS_SDIO_BLOCK_SIZE;
    char ch = 'A';

    FASSERT_MSG(trans_len <= SD_WR_BUF_LEN, "trans length exceed buffer limits");
    SDWaitEvent(SD_EVT_INIT_DONE, portMAX_DELAY);

    for (;;)
    {
        printf("start write ...\r\n");
        memset(write_message.buf, (ch + times), trans_len);
        FCacheDCacheInvalidateRange((uintptr)(void *)write_message.buf, trans_len);
        printf("==>Write %c to Block [%d:%d]\r\n", 
                ch, write_message.start_block, 
                write_message.start_block + write_message.block_num);

        err = FFreeRTOSSdioTransfer(sdio, &write_message);
        if (FFREERTOS_SDIO_OK != err)
        {
            FSDIO_ERROR("sdio write failed !!!");
            goto task_exit;
        }

        SDSendEvent(SD_EVT_WRITE_DONE); /* send write finish signal */
        vTaskDelay(wait_delay);
        SDWaitEvent(SD_EVT_READ_DONE, portMAX_DELAY); /* wait until read done and go on next write */

        if (++times > run_times)
            break;
    }

task_exit:
    printf("exit from write task \r\n");
    vTaskSuspend(NULL); /* suspend task */
}

BaseType_t FFreeRTOSSdWriteRead(u32 slot_id, boolean is_emmc, u32 start_blk, u32 blk_num)
{
    BaseType_t ret = pdPASS;
    const TickType_t total_run_time = pdMS_TO_TICKS(30000UL); /* run for 10 secs deadline */

    if (is_running)
    {
        FSDIO_ERROR("task is running !!!!");
        return pdPASS;
    }

    is_running = TRUE;

    printf("sd write read task\r\n");

    FASSERT_MSG(NULL == sync, "event group exists !!!");
    FASSERT_MSG((sync = xEventGroupCreate()) != NULL, "create event group failed !!!");

    sd_slot = slot_id;
    sdio_config.medium_type = is_emmc ? FFREERTOS_SDIO_MEDIUM_EMMC: FFREERTOS_SDIO_MEDIUM_TF;

    read_message.start_block = start_blk;
    read_message.block_num = blk_num;
    write_message.start_block = start_blk;
    write_message.block_num = blk_num;
    taskENTER_CRITICAL(); /* no schedule when create task */

    ret = xTaskCreate((TaskFunction_t )SDInitTask,
                            (const char* )"SDInitTask",
                            (uint16_t )2048,
                            NULL,
                            (UBaseType_t )configMAX_PRIORITIES - 1,
                            NULL);
    FASSERT_MSG(pdPASS == ret, "create task failed");

    ret = xTaskCreate((TaskFunction_t )SDWriteTask,
                            (const char* )"SDWriteTask",
                            (uint16_t )2048,
                            NULL,
                            (UBaseType_t )configMAX_PRIORITIES - 2,
                            &write_task);

    FASSERT_MSG(pdPASS == ret, "create task failed");
    
    ret = xTaskCreate((TaskFunction_t )SDReadTask,
                            (const char* )"SDReadTask",
                            (uint16_t )2048,
                            NULL,
                            (UBaseType_t )configMAX_PRIORITIES - 2,
                            &read_task);

    FASSERT_MSG(pdPASS == ret, "create task failed");

    exit_timer = xTimerCreate("Exit-Timer",		            /* Text name for the software timer - not used by FreeRTOS. */
                            total_run_time,		            /* The software timer's period in ticks. */
                            pdFALSE,						/* Setting uxAutoRealod to pdFALSE creates a one-shot software timer. */
                            NULL,				            /* use timer id to pass task data for reference. */
                            SDExitCallback);                /* The callback function to be used by the software timer being created. */

    FASSERT_MSG(NULL != exit_timer, "create exit timer failed");

    taskEXIT_CRITICAL(); /* allow schedule since task created */

    ret = xTimerStart(exit_timer, 0); /* start */

    FASSERT_MSG(pdPASS == ret, "start exit timer failed");

    return ret;
}