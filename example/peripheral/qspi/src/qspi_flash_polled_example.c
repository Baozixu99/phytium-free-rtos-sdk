/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc.
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
 * FilePath: qspi_flash_polled_example.c
 * Date: 2022-07-11 11:32:48
 * LastEditTime: 2022-07-11 11:32:48
 * Description:  This file is for qspi polled example functions.
 *
 * Modify History:
 *  Ver   Who           Date           Changes
 * ----- ------       --------      --------------------------------------
 * 1.0  wangxiaodong  2022/8/9      first release
 * 2.0  huangjin      2023/11/16    Modified function
 * 2.1  zhangyan      2024/4/18     add no letter shell mode, adapt to auto-test system
 */
#include <string.h>
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "fqspi_os.h"
#include "qspi_example.h"
#include "fqspi_flash.h"
#include "sdkconfig.h"
#include "fio_mux.h"

#define FQSPI_DEBUG_TAG "QSPI_FLASH_POLLED_TEST"
#define FQSPI_ERROR(format, ...) FT_DEBUG_PRINT_E(FQSPI_DEBUG_TAG, format, ##__VA_ARGS__)
#define FQSPI_WARN(format, ...) FT_DEBUG_PRINT_W(FQSPI_DEBUG_TAG, format, ##__VA_ARGS__)
#define FQSPI_INFO(format, ...) FT_DEBUG_PRINT_I(FQSPI_DEBUG_TAG, format, ##__VA_ARGS__)
#define FQSPI_DEBUG(format, ...) FT_DEBUG_PRINT_D(FQSPI_DEBUG_TAG, format, ##__VA_ARGS__)

/* write and read task delay in milliseconds */
#define TASK_DELAY_MS 1000UL
#define TIMER_OUT (pdMS_TO_TICKS(5000UL))
#define QSPI_POLLED_TEST_TASK_PRIORITY 3
/* Offset 1M from flash maximum capacity*/
#define FLASH_WR_OFFSET SZ_1M
/* write and read start address */
static u32 flash_wr_start = 0;

/* write and read cs channel */
#define QSPI_CS_CHANNEL 0

#define DAT_LENGTH 64
static u8 rd_buf[DAT_LENGTH] = {0};
static u8 wr_buf[DAT_LENGTH] = {0};

enum
{
    QSPI_TEST_SUCCESS = 0,
    QSPI_TEST_UNKNOWN = 1,
    QSPI_INIT_FAILURE = 2,
    QSPI_WRITE_FAILURE = 3,
    QSPI_READ_FAILURE = 4,
};

static QueueHandle_t xQueue = NULL;
static FFreeRTOSQspi *os_qspi_ctrl_p = NULL;

static FError QspiInit(void)
{
    /* The qspi_id to use is passed in via the parameter.
    Cast this to a qspi_id pointer. */
    FError ret = FQSPI_SUCCESS;

    /*init iomux*/
    FIOMuxInit();
    FIOPadSetQspiMux(QSPI_TEST_ID, FQSPI_CS_0);

    /* init qspi controller */
    os_qspi_ctrl_p = FFreeRTOSQspiInit(QSPI_TEST_ID);
    flash_wr_start = os_qspi_ctrl_p->qspi_ctrl.flash_size - FLASH_WR_OFFSET;
    if (os_qspi_ctrl_p == NULL)
    {
        FQSPI_ERROR("FFreeRTOSQspiInit failed.\n");
        ret = FQSPI_INVAL_PARAM;
        return ret;
    }

    return ret;
}

static FError QspiRead(void)
{
    const TickType_t xDelay = pdMS_TO_TICKS(TASK_DELAY_MS);
    FError ret = FQSPI_SUCCESS;
    FFreeRTOSQspiMessage message;
    memset(&message, 0, sizeof(message));
    int i = 0;

    message.read_buf = rd_buf;
    message.length = DAT_LENGTH;
    message.addr = flash_wr_start;
    message.cmd = FQSPI_FLASH_CMD_READ;
    message.cs = QSPI_CS_CHANNEL;
    ret = FFreeRTOSQspiTransfer(os_qspi_ctrl_p, &message);
    if (FQSPI_SUCCESS != ret)
    {
        FQSPI_ERROR("QspiRead FFreeRTOSQspiTransfer failed, return value: 0x%x\r\n", ret);
        return ret;
    }
    FtDumpHexByte(rd_buf, DAT_LENGTH);

    /* 判断读写内容是否一致 */
    for (i = 0; i < DAT_LENGTH; i++)
    {
        if (rd_buf[i] != wr_buf[i])
        {
            FQSPI_ERROR("The read and write data is inconsistent.\r\n");
            ret = FQSPI_INVAL_PARAM;
            return ret;
        }
    }

    return ret;
}

static FError QspiWrite(void)
{
    const char *pcTaskName = "QspiWrite is running\r\n";
    const TickType_t xDelay = pdMS_TO_TICKS(TASK_DELAY_MS);
    int i = 0;
    FError ret = FQSPI_SUCCESS;
    FFreeRTOSQspiMessage message;
    memset(&message, 0, sizeof(message));
    /* Print out the name of this task. */
    printf(pcTaskName);
    for (i = 0; i < DAT_LENGTH; i++)
    {
        wr_buf[i] = i;
    }
    message.addr = flash_wr_start;
    message.cmd = FQSPI_FLASH_CMD_SE;
    message.cs = QSPI_CS_CHANNEL;
    ret = FFreeRTOSQspiTransfer(os_qspi_ctrl_p, &message);
    if (FQSPI_SUCCESS != ret)
    {
        FQSPI_ERROR("Failed to erase sectors. return value: 0x%x\r\n", ret);
        return ret;
    }
    message.write_buf = wr_buf;
    message.length = DAT_LENGTH;
    message.addr = flash_wr_start;
    message.cmd = FQSPI_FLASH_CMD_PP;
    message.cs = QSPI_CS_CHANNEL;
    ret = FFreeRTOSQspiTransfer(os_qspi_ctrl_p, &message);
    if (FQSPI_SUCCESS != ret)
    {
        FQSPI_ERROR("QspiWrite FFreeRTOSQspiTransfer failed, return value: 0x%x\r\n", ret);
        return ret;
    }

    return ret;
}

static void FFreeRTOSQspiPolledTask(void)
{
    FError ret = FQSPI_SUCCESS;
    int task_res = QSPI_TEST_SUCCESS;

    ret = QspiInit();
    if (FQSPI_SUCCESS != ret)
    {
        FQSPI_ERROR("Flash init failed.\r\n");
        task_res = QSPI_INIT_FAILURE;
        goto task_exit;
    }

    ret = QspiWrite();
    if (FQSPI_SUCCESS != ret)
    {
        FQSPI_ERROR("Flash write failed.\r\n");
        task_res = QSPI_WRITE_FAILURE;
        goto task_exit;
    }

    ret = QspiRead();
    if (FQSPI_SUCCESS != ret)
    {
        FQSPI_ERROR("Flash read failed.\r\n");
        task_res = QSPI_READ_FAILURE;
        goto task_exit;
    }

task_exit:
    xQueueSend(xQueue, &task_res, 0);

    vTaskDelete(NULL);
}

static void FFreeRTOSQspiDelete(void)
{
    FIOMuxDeInit();

    FFreeRTOSQspiDeinit(os_qspi_ctrl_p);
}

BaseType_t FFreeRTOSQspiPolledTaskCreate(void)
{
    BaseType_t xReturn = pdPASS; /* 定义一个创建信息返回值，默认为 pdPASS */
    int task_res = QSPI_TEST_UNKNOWN;

    xQueue = xQueueCreate(1, sizeof(int));
    if (xQueue == NULL)
    {
        FQSPI_ERROR("xQueue create failed.\r\n");
        goto exit;
    }

    taskENTER_CRITICAL();                                          /*进入临界区*/
    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSQspiPolledTask, /* 任务入口函数 */
                          (const char *)"FFreeRTOSQspiPolledTask", /* 任务名字 */
                          (uint16_t)1024,                          /* 任务栈大小 */
                          NULL,                                       /* 任务入口函数参数 */
                          (UBaseType_t)QSPI_POLLED_TEST_TASK_PRIORITY,   /* 任务的优先级 */
                          NULL);                                   /* 任务控制 */
    taskEXIT_CRITICAL();                                           /*退出临界区*/
    if (xReturn == pdFAIL)
    {
        FQSPI_ERROR("xTaskCreate FFreeRTOSQspiPolledTask failed.");
        goto exit;
    }

    xReturn = xQueueReceive(xQueue, &task_res, TIMER_OUT);
    if (xReturn == pdFAIL)
    {
        FQSPI_ERROR("xQueue receive timeout.\r\n");
        goto exit;
    }

exit:
    vQueueDelete(xQueue);
    FFreeRTOSQspiDelete();
    if (task_res != QSPI_TEST_SUCCESS)
    {
        printf("%s@%d: Qspi flash polled example [failure], task_res = %d\r\n", __func__, __LINE__, task_res);
        return pdFAIL;
    }
    else
    {
        printf("%s@%d: Qspi flash polled example [success].\r\n", __func__, __LINE__);
        return pdTRUE;
    }
}
