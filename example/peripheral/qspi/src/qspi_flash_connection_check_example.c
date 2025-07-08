/*
 * Copyright (C) 2023, Phytium Technology Co., Ltd.   All Rights Reserved.
 *
 * Licensed under the BSD 3-Clause License (the "License"); you may not use
 * this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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

#define QSPI_FLASH_CONNECT_TEST_TASK_PRIORITY 3
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
                          1024,                       /* 任务栈大小 */
                          NULL,
                          (UBaseType_t)QSPI_FLASH_CONNECT_TEST_TASK_PRIORITY, /* 任务的优先级 */
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
