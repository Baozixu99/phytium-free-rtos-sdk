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
#include "fkernel.h"
#include "sfud_read_write.h"
#include "sfud.h"

/************************** Constant Definitions *****************************/
#define SFUD_WR_BUF_LEN   64

/************************** Variable Definitions *****************************/
static u32 flash_addr = 0x10;/*flash read and write addr*/
static u8 write_flash_buffer[SFUD_WR_BUF_LEN];
static u8 read_flash_buffer[SFUD_WR_BUF_LEN];
static QueueHandle_t xQueue = NULL;

/***************** Macros (Inline Functions) Definitions *********************/
#define TIMER_OUT       ( pdMS_TO_TICKS( 3000UL ) )
#define SFUD_WR_TEST_TASK_PRIORITY 3
#define FSPIM_DEBUG_TAG "SFUD-DEMO"
#define FSPIM_ERROR(format, ...)   FT_DEBUG_PRINT_E(FSPIM_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSPIM_WARN(format, ...)    FT_DEBUG_PRINT_W(FSPIM_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSPIM_INFO(format, ...)    FT_DEBUG_PRINT_I(FSPIM_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSPIM_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FSPIM_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/

/*****************************************************************************/
enum 
{
    SPI_SFUD_TEST_SUCCESS = 0,  /*SPI sufd test success*/
    SPI_SFUD_TEST_UNKNOWN = 1,  /*SPI sufd test success*/
    SPI_SFUD_INIT_FAILURE = 2,  /*SPI init step failure*/
    SPI_SFUD_WRITE_FAILURE = 3, /*SPI write step failure*/
    SPI_SFUD_READ_FAILURE = 4,  /*SPI read step failure*/                        
};

static sfud_err SpiSfudInit(void)
{
    sfud_err sfud_ret = sfud_init();
    if (SFUD_SUCCESS != sfud_ret)
    {
        return sfud_ret;
    }

    const sfud_flash *flash = sfud_get_device(SFUD_FLASH_INDEX);
    if (NULL == flash)
    {
        FSPIM_ERROR("Flash not found.");
        sfud_ret = SFUD_ERR_NOT_FOUND;
        return sfud_ret;
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

    return sfud_ret;
}

static sfud_err SpiSfudWrite(void)
{
    sfud_err sfud_ret = SFUD_SUCCESS;
    const sfud_flash *flash = NULL;
    u8 status = 0;

    flash = sfud_get_device(SFUD_FLASH_INDEX);
    if (NULL == flash)
    {
        FSPIM_ERROR("Flash not found.");
        sfud_ret = SFUD_ERR_NOT_FOUND;
        return sfud_ret;
    }

    /* remove flash write protect */
    sfud_ret = sfud_write_status(flash, TRUE, status);
    if (SFUD_SUCCESS != sfud_ret)
    {
        FSPIM_ERROR("Write flash status failed.");
        return sfud_ret;
    }

    /* get flash status */
    sfud_ret = sfud_read_status(flash, &status);
    if (SFUD_SUCCESS != sfud_ret)
    {
        FSPIM_ERROR("Read flash status failed.");
        return sfud_ret;
    }
    else
    {
        printf("Flash status: 0x%x\r\n", status);
    }

    /* write to flash */
    printf("Data to write @0x%x...\r\n", flash_addr);
    FtDumpHexByte(write_flash_buffer, SFUD_WR_BUF_LEN);

    /* erase before write */
    sfud_ret = sfud_erase(flash, flash_addr, SFUD_WR_BUF_LEN);
    if (SFUD_SUCCESS != sfud_ret)
    {
        FSPIM_ERROR("Erase flash failed.");
        return sfud_ret;
    }

    sfud_ret = sfud_write(flash, flash_addr, SFUD_WR_BUF_LEN, write_flash_buffer);
    if (SFUD_SUCCESS != sfud_ret)
    {
        FSPIM_ERROR("Write flash failed.");
        return sfud_ret;
    }
    
    return sfud_ret;
}

static sfud_err SpiSfudRead(void)
{
    sfud_err sfud_ret = SFUD_SUCCESS;
    const sfud_flash *flash = NULL;
    u8 status = 0;

    flash = sfud_get_device(SFUD_FLASH_INDEX);
    if (NULL == flash)
    {
        FSPIM_ERROR("Flash not found.");
        sfud_ret = SFUD_ERR_NOT_FOUND;
        return sfud_ret;
    }

    /* get flash status */
    sfud_ret = sfud_read_status(flash, &status);
    if (SFUD_SUCCESS != sfud_ret)
    {
        FSPIM_ERROR("Read flash status failed.");
        return sfud_ret;
    }
    else
    {
        printf("Flash status: 0x%x\r\n", status);
    }

    /* read from flash */
    memset(read_flash_buffer, 0, SFUD_WR_BUF_LEN);
    sfud_ret = sfud_read(flash, flash_addr, SFUD_WR_BUF_LEN, read_flash_buffer);
    if (SFUD_SUCCESS != sfud_ret)
    {
        FSPIM_ERROR("Read flash failed.");
        return sfud_ret;
    }

    if (memcmp(write_flash_buffer, read_flash_buffer, SFUD_WR_BUF_LEN)!=0)
    {
        FSPIM_ERROR("Read flash failed. read_flash_buffer != write_flash_buffer\r\n");
        sfud_ret = SFUD_ERR_READ;
        return sfud_ret;
    }
    
    printf("Data read from flash @0x%x...\r\n", flash_addr);
    FtDumpHexByte(read_flash_buffer, SFUD_WR_BUF_LEN);

    return sfud_ret;;
}

static void SpiSfudWriteThenReadTask(void)
{
    sfud_err sfud_ret = SFUD_SUCCESS;
    int task_res = SPI_SFUD_TEST_SUCCESS;

    sfud_ret = SpiSfudInit();
    if (SFUD_SUCCESS != sfud_ret)
    {
        FSPIM_ERROR("Flash init failed.\r\n");
        task_res = SPI_SFUD_INIT_FAILURE;
        goto task_exit;
    }

    sfud_ret = SpiSfudWrite();
    if (SFUD_SUCCESS != sfud_ret)
    {
        FSPIM_ERROR("Write flash failed.\r\n");
        task_res = SPI_SFUD_WRITE_FAILURE;
        goto task_exit;
    }

    sfud_ret = SpiSfudRead();
    if (SFUD_SUCCESS != sfud_ret)
    {
        FSPIM_ERROR("Read flash failed.\r\n");
        task_res = SPI_SFUD_READ_FAILURE;
        goto task_exit;
    }

task_exit:
    xQueueSend(xQueue, &task_res, 0);

    vTaskDelete(NULL);
}

BaseType_t FFreeRTOSSfudWriteThenRead(void)
{
    BaseType_t xReturn = pdPASS;
    int task_res = SPI_SFUD_TEST_UNKNOWN;
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
    xReturn = xTaskCreate((TaskFunction_t)SpiSfudWriteThenReadTask,  /* 任务入口函数 */
                          (const char *)"SpiSfudWriteThenReadTask",/* 任务名字 */
                          4096,  /* 任务栈大小 */
                          NULL,/* 任务入口函数参数 */
                          (UBaseType_t)SFUD_WR_TEST_TASK_PRIORITY,  /* 任务的优先级 */
                          NULL);
    if (xReturn == pdFAIL)
    {
        FSPIM_ERROR("xTaskCreate SpiSfudWriteThenReadTask failed.");
        goto exit;
    }
    
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
