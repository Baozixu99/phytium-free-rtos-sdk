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
 * FilePath: qspi_dual_flash_stack_example.c
 * Date: 2023-11-20 11:32:48
 * LastEditTime: 2023-11-20 11:32:48
 * Description:  This file is for qspi dual flash stack example function implmentation
 *
 * Modify History:
 *  Ver   Who           Date           Changes
 * ----- ------       --------      --------------------------------------
 * 1.0   huangjin     2023/11/20    first release
 * 1.1   zhangyan       2024/4/18     add no letter shell mode, adapt to auto-test system
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

#define FQSPI_DEBUG_TAG "QSPI_DUAL_FLASH_TEST"
#define FQSPI_ERROR(format, ...) FT_DEBUG_PRINT_E(FQSPI_DEBUG_TAG, format, ##__VA_ARGS__)
#define FQSPI_WARN(format, ...) FT_DEBUG_PRINT_W(FQSPI_DEBUG_TAG, format, ##__VA_ARGS__)
#define FQSPI_INFO(format, ...) FT_DEBUG_PRINT_I(FQSPI_DEBUG_TAG, format, ##__VA_ARGS__)
#define FQSPI_DEBUG(format, ...) FT_DEBUG_PRINT_D(FQSPI_DEBUG_TAG, format, ##__VA_ARGS__)

/* write and read task delay in milliseconds */
#define TASK_DELAY_MS 1000UL
#define TIMER_OUT (pdMS_TO_TICKS(5000UL))
#define QSPI_DUAL_FLASH_TEST_TASK_PRIORITY 3
/* Offset 1M from flash maximum capacity*/
#define FLASH_WR_OFFSET 0x100
/* write and read start address */
static u32 flash_wr_start = 0;

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

static FError QspiRead(int channel)
{
    const TickType_t xDelay = pdMS_TO_TICKS(TASK_DELAY_MS);
    FError ret = FQSPI_SUCCESS;
    int i = 0;
    FFreeRTOSQspiMessage message;
    memset(&message, 0, sizeof(message));

    printf("CSN%d QspiReadTask is running\r\n", channel);
    message.read_buf = rd_buf;
    message.length = DAT_LENGTH;
    message.addr = flash_wr_start;
    message.cmd = FQSPI_FLASH_CMD_READ;
    message.cs = channel;
    ret = FFreeRTOSQspiTransfer(os_qspi_ctrl_p, &message);
    if (FQSPI_SUCCESS != ret)
    {
        FQSPI_ERROR("QspiReadTask FFreeRTOSQspiTransfer failed, return value: 0x%x\r\n", ret);
        return ret;
    }
    FtDumpHexByte(rd_buf, DAT_LENGTH);

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

static FError QspiWrite(int channel)
{
    FError ret = FQSPI_SUCCESS;
    char *pcTaskName;
    const TickType_t xDelay = pdMS_TO_TICKS(TASK_DELAY_MS);
    int i = 0;
    FFreeRTOSQspiMessage message;
    memset(&message, 0, sizeof(message));

    if (channel == 0)
    {
        pcTaskName = "CSN0 write content successfully";
    }
    else if (channel == 1)
    {
        pcTaskName = "CSN1 write content successfully";
    }

    printf("CSN%d QspiWrite is running\r\n", channel);

    /*set the write buffer content*/
    u8 len = strlen(pcTaskName) + 1;
    memcpy(&wr_buf, pcTaskName, len);
    message.addr = flash_wr_start;
    message.cmd = FQSPI_FLASH_CMD_SE;
    message.cs = channel;
    ret = FFreeRTOSQspiTransfer(os_qspi_ctrl_p, &message);
    if (FQSPI_SUCCESS != ret)
    {
        FQSPI_ERROR("FFreeRTOSQspiTransfer failed, return value: 0x%x\r\n", ret);
        return ret;
    }
    message.write_buf = wr_buf;
    message.length = DAT_LENGTH;
    message.addr = flash_wr_start;
    message.cmd = FQSPI_FLASH_CMD_PP;
    message.cs = channel;
    ret = FFreeRTOSQspiTransfer(os_qspi_ctrl_p, &message);
    if (FQSPI_SUCCESS != ret)
    {
        FQSPI_ERROR("FFreeRTOSQspiTransfer failed, return value: 0x%x\r\n", ret);
        return ret;
    }

    return ret;
}

static FError QspiInit(void)
{
    FError ret = FQSPI_SUCCESS;

    /*init iomux*/
    FIOMuxInit();
    FIOPadSetQspiMux(QSPI_TEST_ID, FQSPI_CS_0);
    FIOPadSetQspiMux(QSPI_TEST_ID, FQSPI_CS_1);

    /* init qspi controller */
    os_qspi_ctrl_p = FFreeRTOSQspiInit(QSPI_TEST_ID);
    flash_wr_start = os_qspi_ctrl_p->qspi_ctrl.flash_size[FQSPI_CS_0] - FLASH_WR_OFFSET;
    if (os_qspi_ctrl_p == NULL)
    {
        FQSPI_ERROR("FFreeRTOSQspiInit failed.\n");
        ret = FQSPI_INVAL_PARAM;
    }

    return ret;
}

static void FFreeRTOSQspiDelete(void)
{
    BaseType_t xReturn = pdPASS;

    FIOMuxDeInit();
    FFreeRTOSQspiDeinit(os_qspi_ctrl_p);
}

static void FFreeRTOSQspiDualFlashTask(void)
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

    for (int i = FQSPI_CS_0; i < 2; i++)
    {
        ret = QspiWrite(i);
        if (FQSPI_SUCCESS != ret)
        {
            FQSPI_ERROR("CSN%d Flash write failed.\r\n", i);
            task_res = QSPI_WRITE_FAILURE;
            goto task_exit;
        }

        ret = QspiRead(i);
        if (FQSPI_SUCCESS != ret)
        {
            FQSPI_ERROR("CSN%d Flash read failed.\r\n", i);
            task_res = QSPI_READ_FAILURE;
            goto task_exit;
        }
    }

task_exit:
    xQueueSend(xQueue, &task_res, 0);

    vTaskDelete(NULL);
}
BaseType_t FFreeRTOSQspiDualFlashTaskCreate(void)
{
    BaseType_t xReturn = pdPASS; /* 定义一个创建信息返回值，默认为 pdPASS */
    int task_res = QSPI_TEST_UNKNOWN;

    xQueue = xQueueCreate(1, sizeof(int));
    if (xQueue == NULL)
    {
        FQSPI_ERROR("xQueue create failed.\r\n");
        goto exit;
    }

    taskENTER_CRITICAL();                                             /*进入临界区*/
    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSQspiDualFlashTask, /* 任务入口函数 */
                          (const char *)"FFreeRTOSQspiDualFlashTask", /* 任务名字 */
                          1024,                             /* 任务栈大小 */
                          NULL,                                          /* 任务入口函数参数 */
                          (UBaseType_t)QSPI_DUAL_FLASH_TEST_TASK_PRIORITY,      /* 任务的优先级 */
                          NULL);                                      /* 任务控制 */
    taskEXIT_CRITICAL();                                              /*退出临界区*/

    if (xReturn == pdFAIL)
    {
        FQSPI_ERROR("xTaskCreate FFreeRTOSQspiDualFlashTask failed.");
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
        printf("%s@%d: Qspi dual flash example [failure], task_res = %d\r\n", __func__, __LINE__, task_res);
        return pdFAIL;
    }
    else
    {
        printf("%s@%d: Qspi dual flash example [success].\r\n", __func__, __LINE__);
        return pdTRUE;
    }
}