/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: i2c_msg_wr_example.c
 * Date: 2025-04-21 11:35:23
 * LastEditTime: 2025-04-21 11:35:24
 * Description:  This file is for i2c msg master and slave test example functions.
 *
 * Modify History:
 *  Ver       Who            Date              Changes
 * -----    ------         --------          --------------------------------------
 *  1.0    zhangyan        2025/04/21         init commit
 */

#include <string.h>
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "fi2c_msg_os.h"
#include "i2c_msg_example.h"
#include "fio_mux.h"
#include "sdkconfig.h"
#include "fdebug.h"
#include "ftypes.h"
#include "finterrupt.h"
#include "fassert.h"

#define FI2C_MSG_DEBUG_TAG "I2C_MSG_WR_TEST"
#define FI2C_MSG_ERROR(format, ...)   FT_DEBUG_PRINT_E(FI2C_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI2C_MSG_WARN(format, ...)    FT_DEBUG_PRINT_W(FI2C_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI2C_MSG_INFO(format, ...)    FT_DEBUG_PRINT_I(FI2C_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI2C_MSG_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FI2C_MSG_DEBUG_TAG, format, ##__VA_ARGS__)

#define I2C_MS_TEST_TASK_PRIORITY 3
#define TIMER_OUT (pdMS_TO_TICKS(5000UL))
/* write and read task delay in milliseconds */
#define I2C_MASTER_INSTANCE_ID FI2C3_MSG_ID
#define I2C_DEVICE_ADDR 0x53
#define I2C_DEVICE_MEMADDR 0x0

static FFreeRTOSI2cMsg *os_i2c;
static QueueHandle_t xQueue = NULL;

static u8 read_buf[60] __attribute__((aligned(4)));
static u8 write_buf[60] = {"Phytium FreeRtos I2C Msg Driver Test Successfully !!"};

enum
{
    I2C_MSG_MR_TEST_SUCCESS = 0,
    I2C_MSG_MR_TEST_UNKNOWN = 1,
    I2C_MSG_MR_SET_FAILURE = 2,
    I2C_MSG_MR_WRITE_FAILURE = 3,
    I2C_MSG_MR_READ_FAILURE = 4,
};

static FError FFreeRTOSI2cMsgInitSet(uint32_t id)
{
    printf("FFreeRTOSI2cMsgInitSet \n");
    FError ret = FREERTOS_I2C_MSG_SUCCESS;

    os_i2c = FFreeRTOSI2cMsgInit(id, FI2C_STANDARD_SPEED);
}

static FError I2cMsgWrite(FFreeRTOSI2cMsg *os_i2c_p)
{
    FError ret = FREERTOS_I2C_MSG_SUCCESS;
    /* Master mode for send or receive data */
    FFreeRTOSI2cMsgMessage write_message;
    write_message.slave_addr = I2C_DEVICE_ADDR;
    write_message.buf_length = sizeof(write_buf);
    write_message.buf = write_buf;
    write_message.mem_byte_len = 2;
    write_message.mem_addr = I2C_DEVICE_MEMADDR;
    write_message.mode = FI2C_WRITE_DATA;

    ret = FFreeRTOSI2cMsgTransfer(os_i2c_p, &write_message);
    if (ret != FREERTOS_I2C_MSG_SUCCESS)
    {
        FI2C_MSG_ERROR("FFreeRTOSI2cMsgTransfer func failed");
    }
    
    return ret;
}

static FError I2cMsgRead(FFreeRTOSI2cMsg *os_i2c_p)
{
    FError ret = FREERTOS_I2C_MSG_SUCCESS;
    FFreeRTOSI2cMsgMessage read_message;
    read_message.slave_addr = I2C_DEVICE_ADDR;
    read_message.buf_length = sizeof(write_buf);
    read_message.buf = read_buf;

    read_message.mem_addr = I2C_DEVICE_MEMADDR;
    read_message.mem_byte_len = 2;
    read_message.mode = FI2C_READ_DATA;

    ret = FFreeRTOSI2cMsgTransfer(os_i2c_p, &read_message);
    if (ret != FREERTOS_I2C_MSG_SUCCESS)
    {
        FI2C_MSG_ERROR("FFreeRTOSI2cMsgTransfer func failed");
    }
    
    FtDumpHexByte(read_buf, 64);
    return ret;
}

static void FFreeRTOSI2cMsgDelete(void)
{
    FIOMuxDeInit();

    FFreeRTOSI2cMsgDeinit(os_i2c);
}

static void FFreeRTOSI2cMsgWriteThenReadTask(void *pvParameters)
{
    printf("FFreeRTOSI2cMsgWriteThenReadTask \n");
    FError ret;
    BaseType_t xReturn = pdPASS;
    int task_res = I2C_MSG_MR_TEST_SUCCESS;
    FIOMuxInit();
    ret = FFreeRTOSI2cMsgInitSet(I2C_MASTER_INSTANCE_ID);
    if (ret != FREERTOS_I2C_MSG_SUCCESS)
    {
        FI2C_MSG_ERROR("I2c FFreeRTOSI2cInitSet failed.\r\n");
        task_res = I2C_MSG_MR_SET_FAILURE;
        goto task_exit;
    }
    
    ret = I2cMsgWrite(os_i2c);
    if (ret != FREERTOS_I2C_MSG_SUCCESS)
    {
        FI2C_MSG_ERROR("I2cWrite failed.\r\n");
        task_res = I2C_MSG_MR_WRITE_FAILURE;
        goto task_exit;
    }

    ret = I2cMsgRead(os_i2c);
    if (ret != FREERTOS_I2C_MSG_SUCCESS)
    {
        FI2C_MSG_ERROR("I2cRead failed.\r\n");
        task_res = I2C_MSG_MR_READ_FAILURE;
        goto task_exit;
    }

task_exit:
    xQueueSend(xQueue, &task_res, 0);

    vTaskDelete(NULL);
}

BaseType_t FFreeRTOSI2cMsgWriteThenReadCreate(void)
{
    BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为 pdPASS */
    BaseType_t xTimerStarted = pdPASS;
    int task_res = I2C_MSG_MR_TEST_UNKNOWN;

    xQueue = xQueueCreate(1, sizeof(int));
    if (xQueue == NULL)
    {
        printf("xQueue create failed.\r\n");
        goto exit;
    }

    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSI2cMsgWriteThenReadTask,  /* 任务入口函数 */
                          (const char *)"FFreeRTOSI2cMsgWriteThenReadTask",/* 任务名字 */
                          1024,  /* 任务栈大小 */
                          (void *)NULL,/* 任务入口函数参数 */
                          (UBaseType_t)I2C_MS_TEST_TASK_PRIORITY,  /* 任务的优先级 */
                          NULL); /* 任务控制 */
    FASSERT_MSG(xReturn == pdPASS, "FFreeRTOSI2cMsgWriteThenReadTask creation is failed.");
    
    xReturn = xQueueReceive(xQueue, &task_res, TIMER_OUT);
    if (xReturn == pdFAIL)
    {
        FI2C_MSG_ERROR("xQueue receive timeout.\r\n");
        goto exit;
    }

exit:
    vQueueDelete(xQueue);
    FFreeRTOSI2cMsgDelete();
    if (task_res != I2C_MSG_MR_TEST_SUCCESS)
    {
        printf("%s@%d: I2C msg wr example [failure], task_res = %d\r\n", __func__, __LINE__, task_res);
        return pdFAIL;
    }
    else
    {
        printf("%s@%d: I2C msg wr example [success].\r\n", __func__, __LINE__);
        return pdTRUE;
    }
}




