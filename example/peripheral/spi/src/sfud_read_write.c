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
 * FilePath: sfud_read_write.c
 * Date: 2022-07-12 09:53:00
 * LastEditTime: 2022-07-12 09:53:02
 * Description:  This file is for providing functions used in cmd_sf.c file.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  zhugengyu  2022/8/26    first commit
 */
/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "fdebug.h"
#include "fsleep.h"
#include "fkernel.h"
#include "timers.h"

#include "sfud.h"

/************************** Constant Definitions *****************************/
#define SFUD_WR_BUF_LEN   64
#if defined(CONFIG_TARGET_E2000D)||defined(CONFIG_TARGET_E2000Q)
#define SFUD_FLASH_INDEX  SFUD_FSPIM2_INDEX
#elif defined(CONFIG_TARGET_PHYTIUMPI)
#define SFUD_FLASH_INDEX  SFUD_FSPIM0_INDEX
#endif

/************************** Variable Definitions *****************************/
static u32 flash_addr = 0x0;
static u8 write_flash_buffer[SFUD_WR_BUF_LEN];
static u8 read_flash_buffer[SFUD_WR_BUF_LEN];
static QueueHandle_t xQueue = NULL;

/***************** Macros (Inline Functions) Definitions *********************/
#define TIMER_OUT       ( pdMS_TO_TICKS( 3000UL ) )

#define FSPIM_DEBUG_TAG "SFUD-DEMO"
#define FSPIM_ERROR(format, ...)   FT_DEBUG_PRINT_E(FSPIM_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSPIM_WARN(format, ...)    FT_DEBUG_PRINT_W(FSPIM_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSPIM_INFO(format, ...)    FT_DEBUG_PRINT_I(FSPIM_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSPIM_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FSPIM_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/

/*****************************************************************************/
enum 
{
    SPI_SFUD_TEST_SUCCESS = 1,  /*SPI sufd test success*/
    SPI_SFUD_INIT_FAILURE = 2,  /*SPI init step failure*/
    SPI_SFUD_WRITE_FAILURE = 3, /*SPI write step failure*/
    SPI_SFUD_READ_FAILURE = 4,  /*SPI read step failure*/                        
};

static sfud_err SpiSfudInit(void)
{
    int task_res = 0;
    sfud_err sfud_ret = sfud_init();
    if (SFUD_SUCCESS != sfud_ret)
    {
        goto init_exit;
    }

    const sfud_flash *flash = sfud_get_device(SFUD_FLASH_INDEX);
    if (NULL == flash)
    {
        FSPIM_ERROR("Flash not found.");
        sfud_ret = SFUD_ERR_NOT_FOUND;
        goto init_exit;
    }

    /* print flash info */
    printf("Flash %s is found.\r\n", flash->name);
    printf("    manufacturer id: 0x%x \r\n", flash->chip.mf_id);
    printf("    memory-type id: 0x%x \r\n", flash->chip.type_id);
    printf("    capacity id: 0x%x \r\n", flash->chip.capacity_id);

    if (flash->chip.capacity < SZ_1M)
    {
        printf("    cacpity: %d KB \r\n", flash->chip.capacity / SZ_1K);
    }
    else
    {
        printf("    cacpity: %d MB\r\n", flash->chip.capacity / SZ_1M);
    }

    printf("    Erase granularity: %d Bytes\r\n", flash->chip.erase_gran);

init_exit:
    if (SFUD_SUCCESS != sfud_ret)
    {
        task_res = SPI_SFUD_INIT_FAILURE;
        xQueueSend(xQueue, &task_res, 0);
    }

    return sfud_ret;
}

static sfud_err SpiSfudWrite(void)
{
    sfud_err sfud_ret = SFUD_SUCCESS;
    int task_res = 0;
    u32 in_chip_addr = flash_addr;
    const sfud_flash *flash = NULL;
    u8 status = 0;
    u8 *write_buf = write_flash_buffer;

    flash = sfud_get_device(SFUD_FLASH_INDEX);
    if (NULL == flash)
    {
        FSPIM_ERROR("Flash not found.");
        sfud_ret = SFUD_ERR_NOT_FOUND;
        goto write_exit;
    }

    /* remove flash write protect */
    sfud_ret = sfud_write_status(flash, TRUE, status);
    if (SFUD_SUCCESS != sfud_ret)
    {
        FSPIM_ERROR("Write flash status failed.");
        goto write_exit;
    }

    /* get flash status */
    sfud_ret = sfud_read_status(flash, &status);
    if (SFUD_SUCCESS != sfud_ret)
    {
        FSPIM_ERROR("Read flash status failed.");
        goto write_exit;
    }
    else
    {
        printf("Flash status: 0x%x\r\n", status);
    }

    /* write to flash */
    taskENTER_CRITICAL(); /* no schedule when printf bulk */
    printf("Data to write @0x%x...\r\n", in_chip_addr);
    FtDumpHexByte(write_buf, SFUD_WR_BUF_LEN);
    taskEXIT_CRITICAL();

    /* erase before write */
    sfud_ret = sfud_erase(flash, in_chip_addr, SFUD_WR_BUF_LEN);
    if (SFUD_SUCCESS != sfud_ret)
    {
        FSPIM_ERROR("Erase flash failed.");
        goto write_exit;
    }

    sfud_ret = sfud_write(flash, in_chip_addr, SFUD_WR_BUF_LEN, write_buf);
    if (SFUD_SUCCESS != sfud_ret)
    {
        FSPIM_ERROR("Write flash failed.");
        goto write_exit;
    }

write_exit:
    if (SFUD_SUCCESS != sfud_ret)
    {
        task_res = SPI_SFUD_WRITE_FAILURE;
        xQueueSend(xQueue, &task_res, 0);
    }
    
    return sfud_ret;
}

static sfud_err SpiSfudRead(void)
{
    sfud_err sfud_ret = SFUD_SUCCESS;
    int task_res = 0;
    u32 in_chip_addr = flash_addr;
    const sfud_flash *flash = NULL;
    u8 status = 0;
    u8 *read_buf = read_flash_buffer;

    flash = sfud_get_device(SFUD_FLASH_INDEX);
    if (NULL == flash)
    {
        FSPIM_ERROR("Flash not found.");
        sfud_ret = SFUD_ERR_NOT_FOUND;
        goto read_exit;
    }

    /* get flash status */
    sfud_ret = sfud_read_status(flash, &status);
    if (SFUD_SUCCESS != sfud_ret)
    {
        FSPIM_ERROR("Read flash status failed.");
        goto read_exit;
    }
    else
    {
        printf("Flash status: 0x%x\r\n", status);
    }

    /* read from flash */
    memset(read_buf, 0, SFUD_WR_BUF_LEN);
    sfud_ret = sfud_read(flash, in_chip_addr, SFUD_WR_BUF_LEN, read_buf);
    if (SFUD_SUCCESS != sfud_ret)
    {
        FSPIM_ERROR("Read flash failed.");
        goto read_exit;
    }

    taskENTER_CRITICAL(); /* no schedule when printf bulk */
    for (u32 i = 0; i < SFUD_WR_BUF_LEN; i++)
    {
        if (read_buf[i] != write_flash_buffer[i])
        {
            FSPIM_ERROR("Read flash failed.read_buf != write_flash_buffer\r\n");
            sfud_ret = SFUD_ERR_READ;
            goto read_exit;
        }
    }
    
    printf("Data read from flash @0x%x...\r\n", in_chip_addr);
    FtDumpHexByte(read_buf, SFUD_WR_BUF_LEN);
    taskEXIT_CRITICAL();

read_exit:
    if (SFUD_SUCCESS != sfud_ret)
    {
        task_res = SPI_SFUD_READ_FAILURE;
        xQueueSend(xQueue, &task_res, 0);
    }
    
    return sfud_ret;
}

static void SpiSfudWriteThenReadTask(void)
{
    sfud_err sfud_ret = SFUD_SUCCESS;
    int task_res = 0;

    sfud_ret = SpiSfudInit();
    if (SFUD_SUCCESS != sfud_ret)
    {
        FSPIM_ERROR("Flash init failed.\r\n");
        goto task_exit;
    }

    sfud_ret = SpiSfudWrite();
    if (SFUD_SUCCESS != sfud_ret)
    {
        FSPIM_ERROR("Write flash failed.\r\n");
        goto task_exit;
    }

    sfud_ret = SpiSfudRead();
    if (SFUD_SUCCESS != sfud_ret)
    {
        FSPIM_ERROR("Read flash failed.\r\n");
        goto task_exit;
    }

task_exit:
    if (sfud_ret == SFUD_SUCCESS)
    {
        task_res = SPI_SFUD_TEST_SUCCESS;
        xQueueSend(xQueue, &task_res, 0);
    }

    vTaskDelete(NULL);
}

BaseType_t FFreeRTOSSfudWriteThenRead()
{
    BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为 pdPASS */
    int task_res = 0;
    flash_addr = 0x10;
    const char *content = "write-spi-nor-flash-from-freertos-sfud";
    if (strlen(content) + 1 > SFUD_WR_BUF_LEN)
    {
        FSPIM_ERROR("write content too long.\r\n");
        goto exit;
    }

    memset(write_flash_buffer, 0, sizeof(write_flash_buffer));
    memcpy(write_flash_buffer, content, strlen(content) + 1);
    xQueue = xQueueCreate(1, sizeof(int));
    if (xQueue == NULL)
    {
        FSPIM_ERROR("xQueue create failed.\r\n");
        goto exit;
    }
    
    taskENTER_CRITICAL(); /*进入临界区*/
    xReturn = xTaskCreate((TaskFunction_t)SpiSfudWriteThenReadTask,  /* 任务入口函数 */
                          (const char *)"SpiSfudWriteThenReadTask",/* 任务名字 */
                          (uint16_t)4096,  /* 任务栈大小 */
                          NULL,/* 任务入口函数参数 */
                          (UBaseType_t)2,  /* 任务的优先级 */
                          NULL);
    taskEXIT_CRITICAL(); /*退出临界区*/
    xReturn = xQueueReceive(xQueue, &task_res, TIMER_OUT);

    if (xReturn == pdFAIL)
    {
        FSPIM_ERROR("xQueue receive timeout.\r\n");
        goto exit;
    }
    
exit:
    vQueueDelete(xQueue);
    if (task_res != SPI_SFUD_TEST_SUCCESS)
    {
        printf("%s@%d: Spi sfud read then write example [failure], task_res = %d\r\n", __func__, __LINE__, task_res);
        return pdFAIL;
    }
    else
    {
        printf("%s@%d: Spi sfud read then write example [success].\r\n", __func__, __LINE__);
        return pdTRUE;
    }
}
