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
 * FilePath: i2c_example.c
 * Date: 2022-11-10 11:35:23
 * LastEditTime: 2022-11-10 11:35:24
 * Description:  This file is for i2c rtc test example functions.
 *
 * Modify History:
 *  Ver       Who            Date                 Changes
 * -----    ------         --------     --------------------------------------
 *  1.0    liushengming   2022/11/25             init commit
 *  1.0    liushengming   2023/09/25             rtc commit
 *  1.1   zhangyan       2024/4/18     add no letter shell mode, adapt to auto-test system
 */
#include <string.h>
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "fi2c_os.h"
#include "i2c_example.h"
#include "fio_mux.h"
#include "sdkconfig.h"
#include "fdebug.h"
#include "ftypes.h"
#include "finterrupt.h"

#define FI2C_DEBUG_TAG "I2C_RTC_TEST"
#define FI2C_ERROR(format, ...)   FT_DEBUG_PRINT_E(FI2C_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI2C_WARN(format, ...)    FT_DEBUG_PRINT_W(FI2C_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI2C_INFO(format, ...)    FT_DEBUG_PRINT_I(FI2C_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI2C_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FI2C_DEBUG_TAG, format, ##__VA_ARGS__)

#if defined(CONFIG_E2000D_DEMO_BOARD) || defined(CONFIG_E2000Q_DEMO_BOARD)
#define DS_1339_MIO FMIO9_ID
#endif

/*
 * RTC register addresses
 */
#define DS1339_SEC_REG 0x0
#define DS1339_MIN_REG 0x1
#define DS1339_HOUR_REG 0x2
#define DS1339_DAY_REG 0x3
#define DS1339_DATE_REG 0x4
#define DS1339_MONTH_REG 0x5
#define DS1339_YEAR_REG 0x6

#define BCD_TO_BIN(bcd) ((((((bcd) & 0xf0) >> 4) * 10) + ((bcd) & 0xf)) & 0xff)
#define BIN_TO_BCD(bin) (((((bin) / 10) << 4) + ((bin) % 10)) & 0xff)

/* write and read task delay in milliseconds */
#define TASK_DELAY_MS 1000UL
#define I2C_RTC_TEST_TASK_PRIORITY 3
#define RTC_ADDR 0x68 /* rtc address */
#define DAT_LENGTH 15
static char data_r0[DAT_LENGTH];

#define TIMER_OUT (pdMS_TO_TICKS(5000UL))
enum
{
    I2C_RTC_TEST_SUCCESS = 0,
    I2C_RTC_TEST_UNKNOWN = 1,
    I2C_RTC_SET_FAILURE = 2,
    I2C_RTC_WRITE_FAILURE = 3,
    I2C_RTC_READ_FAILURE = 4,
};
static QueueHandle_t xQueue = NULL;
static FFreeRTOSI2c *os_i2c_master;

typedef struct
{
    u16 year;        /* year */
    u8 month;        /* month */
    u8 day_of_month; /* day of month */
    u8 day_of_week;  /* day of week */
    u8 hour;         /* hour */
    u8 minute;       /* minute */
    u8 second;       /* second */
} FRtcDateTimer;

static FError DsRtcDateCheck(FRtcDateTimer *rtc_time)
{
    FASSERT(rtc_time != NULL);

    /* Check validity of seconds value */
    if (rtc_time->second > 59)
    {
        return FI2C_ERR_INVAL_PARM;
    }

    /* Check validity of minutes value */
    if (rtc_time->minute > 59)
    {
        return FI2C_ERR_INVAL_PARM;
    }

    /* Check validity of day of week value */
    if (rtc_time->day_of_week < 1 || rtc_time->day_of_week > 7)
    {
        return FI2C_ERR_INVAL_PARM;
    }

    /* Check validity of hours value */
    if (rtc_time->hour > 23)
    {
        return FI2C_ERR_INVAL_PARM;
    }

    /* Check validity of day of month value */
    if (rtc_time->day_of_month < 1 || rtc_time->day_of_month > 31)
    {
        return FI2C_ERR_INVAL_PARM;
    }

    /* Check validity of month value */
    if (rtc_time->month < 1 || rtc_time->month > 12)
    {
        return FI2C_ERR_INVAL_PARM;
    }

    /* Check validity of year value */
    if (rtc_time->year > 2099)
    {
        return FI2C_ERR_INVAL_PARM;
    }
    return FREERTOS_I2C_SUCCESS;
}
/*
 * @name: FI2cIntrTxDone
 * @msg:user transmit FIFO done interrupt callback.
 * @param {void} *instance_p
 */
static void FI2cIntrTxDonecallback(void *instance, void *param)
{
    BaseType_t x_result = pdFALSE;
    BaseType_t xhigher_priority_task_woken = pdFALSE;
    FI2c *instance_p = (FI2c *)instance;

    x_result = xEventGroupSetBitsFromISR(os_i2c_master[instance_p->config.instance_id].trx_event, RTOS_I2C_WRITE_DONE, &xhigher_priority_task_woken);
    if (x_result != pdFAIL)
    {
        portYIELD_FROM_ISR(xhigher_priority_task_woken);
    }
}

/*
 * @name: FI2cIntrRxDonecallback
 * @msg:user receive fifo level done interrupt callback.
 * @param {void} *instance_p
 */
static void FI2cIntrRxDonecallback(void *instance, void *param)
{
    BaseType_t x_result = pdFALSE;
    BaseType_t xhigher_priority_task_woken = pdFALSE;
    FI2c *instance_p = (FI2c *)instance;

    x_result = xEventGroupSetBitsFromISR(os_i2c_master[instance_p->config.instance_id].trx_event, RTOS_I2C_READ_DONE, &xhigher_priority_task_woken);
    if (x_result != pdFAIL)
    {
        portYIELD_FROM_ISR(xhigher_priority_task_woken);
    }
}

/*
 * @name: FI2cIntrTxAbrtcallback
 * @msg:user transmit abort interrupt callback.
 * @param {void} *instance_p
 */
static void FI2cIntrTxAbrtcallback(void *instance, void *param)
{
    BaseType_t x_result = pdFALSE;
    BaseType_t xhigher_priority_task_woken = pdFALSE;
    FI2c *instance_p = (FI2c *)instance;

    x_result = xEventGroupSetBitsFromISR(os_i2c_master[instance_p->config.instance_id].trx_event, RTOS_I2C_TRANS_ABORTED, &xhigher_priority_task_woken);
    if (x_result != pdFAIL)
    {
        portYIELD_FROM_ISR(xhigher_priority_task_woken);
    }
}

static FError I2cRead(FFreeRTOSI2c *os_i2c_read_p)
{
    const TickType_t xDelay = pdMS_TO_TICKS(TASK_DELAY_MS);
    vTaskDelay(xDelay);
    const char *pcReadTaskName = "\r\n*****I2cRead is running...\r\n";
    vPrintf(pcReadTaskName);
    FError ret = FREERTOS_I2C_SUCCESS;
    /* Master mode for send or receive data */
    FFreeRTOSI2cMessage message;

    /* The FFreeRTOSI2c to use is passed in via the parameter.
    Cast this to a FFreeRTOSI2c pointer. */
    message.slave_addr = os_i2c_read_p->i2c_device.config.slave_addr;
    message.mem_byte_len = 1;
    message.mem_addr = 0x0; /* 地址偏移0x0的位置poll方式读取数据 */
    message.buf_length = 7;
    message.buf = data_r0;
    message.mode = FI2C_READ_DATA_POLL;

    ret = FFreeRTOSI2cTransfer(os_i2c_read_p, &message);
    if (ret != FREERTOS_I2C_SUCCESS)
    {
        FI2C_ERROR("FFreeRTOSI2cTransfer read poll task error,i2c id:%d.\r\n", os_i2c_read_p->i2c_device.config.instance_id);
        return ret;
    }
    u16 year;
    if (data_r0[5] & 0x80)
    {
        year = BCD_TO_BIN(data_r0[6]) + 2000;
    }
    else
    {
        year = BCD_TO_BIN(data_r0[6]) + 1900;
    }
    printf("Date_time: %d-%d-%d week:%d time:%d:%d:%d\r\n",
           year,
           BCD_TO_BIN(data_r0[5] & 0x1F),
           BCD_TO_BIN(data_r0[4] & 0x3F),
           BCD_TO_BIN((data_r0[3] - 1) & 0x7),
           BCD_TO_BIN(data_r0[2] & 0x3F),
           BCD_TO_BIN(data_r0[1] & 0x7F),
           BCD_TO_BIN(data_r0[0] & 0x7F));

    return ret;
}

static FError I2cWrite(FFreeRTOSI2c *os_i2c_write_p)
{
    int task_res = 0;
    const char *pcWriteTaskName = "\r\n*****I2cWrite is running...\r\n";
    vPrintf(pcWriteTaskName);
    FError ret = FREERTOS_I2C_SUCCESS;
    u8 i, century;
    u8 data_buf[7] = {0};
    /* Master mode for send or receive data */
    FFreeRTOSI2cMessage message;

    /* The FFreeRTOSI2c to use is passed in via the parameter.
    Cast this to a FFreeRTOSI2c pointer. */

    message.slave_addr = os_i2c_write_p->i2c_device.config.slave_addr;
    FRtcDateTimer date_time = {2024, 4, 26, 5, 11, 22, 30};
    printf("Set:year: %d, month: %d, day: %d,week: %d, hour: %d, minute: %d, second: %d\r\n",
           date_time.year,
           date_time.month,
           date_time.day_of_month,
           date_time.day_of_week,
           date_time.hour,
           date_time.minute,
           date_time.second);
    ret = DsRtcDateCheck(&date_time);
    if (FREERTOS_I2C_SUCCESS != ret)
    {
        FI2C_ERROR("Time data param error.\r\n");
        return ret;
    }
    data_buf[0] = BIN_TO_BCD(date_time.second);

    data_buf[1] = BIN_TO_BCD(date_time.minute);

    data_buf[2] = BIN_TO_BCD(date_time.hour);

    data_buf[3] = BIN_TO_BCD(date_time.day_of_week + 1);

    data_buf[4] = BIN_TO_BCD(date_time.day_of_month);

    if (date_time.year >= 2000)
    {
        century = 0x80;
    }
    else
    {
        century = 0x0;
    }
    data_buf[5] = (BIN_TO_BCD(date_time.month) | century);

    data_buf[6] = BIN_TO_BCD(date_time.year % 100);

    /*8位地址*/
    message.mem_byte_len = 1;
    message.buf = &data_buf;
    message.buf_length = sizeof(data_buf);
    message.mem_addr = 0x0; /* 地址偏移0x0的位置poll方式写入数据 */
    message.mode = FI2C_WRITE_DATA_POLL;
    ret = FFreeRTOSI2cTransfer(os_i2c_write_p, &message);
    if (ret != FREERTOS_I2C_SUCCESS)
    {
        FI2C_ERROR("FFreeRTOSI2cTransfer write poll task error,i2c id:%d.\r\n", os_i2c_write_p->i2c_device.config.instance_id);
        return ret;
    }

    return ret;
}

static FError FFreeRTOSI2cInitSet(uint32_t id, uint32_t work_mode, uint32_t slave_address)
{
    FError ret = FREERTOS_I2C_SUCCESS;
    /* init iomux */
    FIOMuxInit();

    FIOPadSetMioMux(id);
    /* init i2c controller */
    if (work_mode == FI2C_MASTER) /* 主机初始化默认使用poll模式 */
    {
        os_i2c_master = FFreeRTOSI2cInit(id, work_mode, slave_address, FI2C_SPEED_STANDARD_RATE);
        /* register intr callback */
        InterruptInstall(os_i2c_master->i2c_device.config.irq_num, FI2cMasterIntrHandler, &os_i2c_master->i2c_device, "fi2cmaster");
        /* register intr handler func */
        FI2cMasterRegisterIntrHandler(&os_i2c_master->i2c_device, FI2C_EVT_MASTER_TRANS_ABORTED, FI2cIntrTxAbrtcallback);
        FI2cMasterRegisterIntrHandler(&os_i2c_master->i2c_device, FI2C_EVT_MASTER_READ_DONE, FI2cIntrRxDonecallback);
        FI2cMasterRegisterIntrHandler(&os_i2c_master->i2c_device, FI2C_EVT_MASTER_WRITE_DONE, FI2cIntrTxDonecallback);
    }
    else
    {
        FI2C_ERROR("error_work_mode!\r\n");
        ret = FREERTOS_I2C_INVAILD_PARAM_ERROR;
    }

    return ret;
}

static void FFreeRTOSI2cDelete(void)
{
    FIOMuxDeInit();

    FFreeRTOSI2cDeinit(os_i2c_master);
}

static void FFreeRTOSI2cRtcTask(void *pvParameters)
{
    FError ret;
    int task_res = I2C_RTC_TEST_SUCCESS;

    ret = FFreeRTOSI2cInitSet(DS_1339_MIO, FI2C_MASTER, RTC_ADDR);
    if (ret != FREERTOS_I2C_SUCCESS)
    {
        FI2C_ERROR("I2c FFreeRTOSI2cInitSet failed.\r\n");
        task_res = I2C_RTC_SET_FAILURE;
        goto task_exit;
    }

    ret = I2cWrite(os_i2c_master);
    if (ret != FREERTOS_I2C_SUCCESS)
    {
        FI2C_ERROR("I2cWrite failed.\r\n");
        task_res = I2C_RTC_WRITE_FAILURE;
        goto task_exit;
    }
    for (int i = 0; i < 2; i++)
    {
        ret = I2cRead(os_i2c_master);
        if (ret != FREERTOS_I2C_SUCCESS)
        {
            FI2C_ERROR("I2cRead failed.\r\n");
            task_res = I2C_RTC_READ_FAILURE;
            goto task_exit;
        }
    }

task_exit:
    xQueueSend(xQueue, &task_res, 0);

    vTaskDelete(NULL);
}

BaseType_t FFreeRTOSI2cRtcCreate(void)
{
    BaseType_t xReturn = pdPASS; /* 定义一个创建信息返回值，默认为 pdPASS */
    BaseType_t xTimerStarted = pdPASS;
    int task_res = I2C_RTC_TEST_UNKNOWN;

    xQueue = xQueueCreate(1, sizeof(int));
    if (xQueue == NULL)
    {
        FI2C_ERROR("xQueue create failed.\r\n");
        goto exit;
    }
    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSI2cRtcTask,   /* 任务入口函数 */
                          (const char *)"FFreeRTOSI2cRtcTask",   /* 任务名字 */
                          (uint16_t)1024,                        /* 任务栈大小 */
                          (void *)NULL,                          /* 任务入口函数参数 */
                          (UBaseType_t)I2C_RTC_TEST_TASK_PRIORITY, /* 任务的优先级 */
                          NULL);                                 /* 任务控制 */
    FASSERT_MSG(xReturn == pdPASS, "FFreeRTOSI2cRtcTask creation is failed.");

    xReturn = xQueueReceive(xQueue, &task_res, TIMER_OUT);
    if (xReturn == pdFAIL)
    {
        FI2C_ERROR("xQueue receive timeout.\r\n");
        goto exit;
    }

exit:
    vQueueDelete(xQueue);
    FFreeRTOSI2cDelete();
    if (task_res != I2C_RTC_TEST_SUCCESS)
    {
        printf("%s@%d: I2C rtc example [failure], task_res = %d\r\n", __func__, __LINE__, task_res);
        return pdFAIL;
    }
    else
    {
        printf("%s@%d: I2C rtc indirect example [success].\r\n", __func__, __LINE__);
        return pdTRUE;
    }
}
