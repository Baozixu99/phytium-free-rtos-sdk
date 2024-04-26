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
 * FilePath: qspi_flash_connection_check_example.c
 * Date: 2023-11-16 11:32:48
 * LastEditTime: 2023-11-16 11:32:48
 * Description:  This file is for qspi flash connection check example function implmentation
 *
 * Modify History:
 *  Ver   Who           Date           Changes
 * ----- ------       --------      --------------------------------------
 * 1.0  huangjin     2023/11/16    first release
 * 1.1  zhangyan       2024/4/18     add no letter shell mode, adapt to auto-test system
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

#define FQSPI_DEBUG_TAG "QSPI_CONNECT_CHECK_TEST"
#define FQSPI_ERROR(format, ...) FT_DEBUG_PRINT_E(FQSPI_DEBUG_TAG, format, ##__VA_ARGS__)
#define FQSPI_WARN(format, ...) FT_DEBUG_PRINT_W(FQSPI_DEBUG_TAG, format, ##__VA_ARGS__)
#define FQSPI_INFO(format, ...) FT_DEBUG_PRINT_I(FQSPI_DEBUG_TAG, format, ##__VA_ARGS__)
#define FQSPI_DEBUG(format, ...) FT_DEBUG_PRINT_D(FQSPI_DEBUG_TAG, format, ##__VA_ARGS__)

#define TIMER_OUT (pdMS_TO_TICKS(5000UL))
enum
{
    QSPI_TEST_SUCCESS = 0,
    QSPI_TEST_UNKNOWN = 1,
    QSPI_INIT_FAILURE = 2,
};
static QueueHandle_t xQueue = NULL;
static FFreeRTOSQspi *os_qspi_ctrl_p = NULL;

static void FFreeRTOSQspiDelete(void)
{
    BaseType_t xReturn = pdPASS;

    FIOMuxDeInit();
    FFreeRTOSQspiDeinit(os_qspi_ctrl_p);
}

static void QspiConnectCheckTask(void)
{
    FError ret = FQSPI_SUCCESS;
    int task_res = QSPI_TEST_SUCCESS;
    /*init iomux*/
    FIOMuxInit();
    FIOPadSetQspiMux(QSPI_TEST_ID, FQSPI_CS_0);
    FIOPadSetQspiMux(QSPI_TEST_ID, FQSPI_CS_1);

    /* init qspi controller */
    os_qspi_ctrl_p = FFreeRTOSQspiInit(QSPI_TEST_ID);
    if (os_qspi_ctrl_p == NULL)
    {
        FQSPI_ERROR("FFreeRTOSWdtInit failed.\n");
        ret = FQSPI_INVAL_PARAM;
        task_res = QSPI_INIT_FAILURE;
        goto qspi_init_exit;
    }

    FFreeRTOSQspiDelete();

qspi_init_exit:
    xQueueSend(xQueue, &task_res, 0);

    vTaskDelete(NULL);
}

BaseType_t FFreeRTOSQspiCheckTaskCreate(void)
{
    BaseType_t xReturn = pdPASS; /* 定义一个创建信息返回值，默认为 pdPASS */
    int task_res = QSPI_TEST_UNKNOWN;
    xQueue = xQueueCreate(1, sizeof(int));
    if (xQueue == NULL)
    {
        FQSPI_ERROR("xQueue create failed.\r\n");
        goto exit;
    }

    taskENTER_CRITICAL();                                       /*进入临界区*/
    xReturn = xTaskCreate((TaskFunction_t)QspiConnectCheckTask, /* 任务入口函数 */
                          (const char *)"QspiConnectCheckTask", /* 任务名字 */
                          (uint16_t)1024,                       /* 任务栈大小 */
                          NULL,
                          (UBaseType_t)2, /* 任务的优先级 */
                          NULL);
    taskEXIT_CRITICAL(); /*退出临界区*/
    if (xReturn == pdFAIL)
    {
        FQSPI_ERROR("xTaskCreate QspiConnectCheckTask failed.");
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
    if (task_res != QSPI_TEST_SUCCESS)
    {
        printf("%s@%d: Qspi flash indirect example [failure], task_res = %d\r\n", __func__, __LINE__, task_res);
        return pdFAIL;
    }
    else
    {
        printf("%s@%d: Qspi flash indirect example [success].\r\n", __func__, __LINE__);
        return pdTRUE;
    }
}
