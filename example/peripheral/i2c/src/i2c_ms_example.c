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
 * FilePath: i2c_ms_example.c
 * Date: 2022-11-10 11:35:23
 * LastEditTime: 2022-11-10 11:35:24
 * Description:  This file is for i2c master and slave test example functions.
 *
 * Modify History:
 *  Ver       Who            Date                 Changes
 * -----    ------         --------     --------------------------------------
 *  1.0    liushengming   2023/09/25             init commit
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

#define FI2C_DEBUG_TAG "I2C_MS_TEST"
#define FI2C_ERROR(format, ...)   FT_DEBUG_PRINT_E(FI2C_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI2C_WARN(format, ...)    FT_DEBUG_PRINT_W(FI2C_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI2C_INFO(format, ...)    FT_DEBUG_PRINT_I(FI2C_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI2C_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FI2C_DEBUG_TAG, format, ##__VA_ARGS__)

#if defined(CONFIG_FIREFLY_DEMO_BOARD)
#define I2C_MS_TEST_MASTER_DEVICE FMIO1_ID
#define I2C_MS_TEST_SLAVE_DEVICE  FMIO2_ID
#endif

/* write and read task delay in milliseconds */
#define TASK_DELAY_MS   1000UL
#define I2C_MS_TEST_TASK_PRIORITY 3
#define TIMER_OUT (pdMS_TO_TICKS(5000UL))
/* slave address */
/* Notice! Using addresses above 0x50 may cause the loopback test to fail  */
#define MASTER_SLAVE_ADDR   0x30

#define DAT_LENGTH  16
static char data_w[DAT_LENGTH] = {0};
static char data_r0[DAT_LENGTH];
static char data_r1[DAT_LENGTH];

static FFreeRTOSI2c *os_i2c_master;
static FFreeRTOSI2c *os_i2c_slave;

enum
{
    I2C_MS_TEST_SUCCESS = 0,
    I2C_MS_TEST_UNKNOWN = 1,
    I2C_MS_SET_FAILURE = 2,
    I2C_MS_WRITE_FAILURE = 3,
    I2C_MS_READ_FAILURE = 4,
    I2C_MS_EEPROM_DUMP_FAILURE = 5,
};
static QueueHandle_t xQueue = NULL;

typedef struct data
{
    boolean first_write;/*IIC首次写入，在初始化时置位，用来指示当前传输的首个字节数据是用户需要读写的地址偏移*/
    u32 buff_idx;/* PC 指向的地址偏移 */
    u8 buff[IO_BUF_LEN];/*虚拟内存块*/
} FI2cSlaveData;
/* Slave mode for virtual eeprom memory ，size: IO_BUF_LEN in fi2c_os.h*/
static FI2cSlaveData slave;

/**
 * @name: FI2cSlaveCb
 * @msg: 从机内存操作
 * @return {*},无
 * @param {void} *instance_p
 * @param {void} *para
 * @param {u32} evt
 */
void FI2cOsSlaveCb(void *instance_p, void *para, u32 evt)
{
    FI2cSlaveData *slave_p = &slave;
    u8 *val = (u8 *)para;
    /*
    *Do not increment buffer_idx here,because we set maximum lenth is IO_BUF_LEN
    */
    if (slave_p->buff_idx >= IO_BUF_LEN)
    {
        slave_p->buff_idx = slave_p->buff_idx % IO_BUF_LEN;
    }
    switch (evt)
    {
        case FI2C_EVT_SLAVE_WRITE_RECEIVED:
            if (slave_p->first_write)
            {
                /*第一个传入的数据是内部偏移地址，而非待写入数据*/
                slave_p->buff_idx = *val;
                slave_p->first_write = FALSE;
            }
            else
            {
                slave_p->buff[slave_p->buff_idx++] = *val;
            }

            break;
        case FI2C_EVT_SLAVE_READ_PROCESSED:
            /* The previous byte made it to the bus, get next one */
            slave_p->buff_idx++;
            /* fallthrough */
            break;
        case FI2C_EVT_SLAVE_READ_REQUESTED:
            *val = slave_p->buff[slave_p->buff_idx++];
            break;
        case FI2C_EVT_SLAVE_STOP:
        case FI2C_EVT_SLAVE_WRITE_REQUESTED:
            slave_p->first_write = TRUE;
            break;   
        case FI2C_EVT_SLAVE_ABORT:
            break;
        default:
            break;
    }

    return;
}

/**
 * @name: FI2cSlaveWriteReceived
 * @msg: Slave收到主机发送的数据，需要存下
 * @return {*} 无
 * @param {void} *instance_p
 * @param {void} *para
 */
void FI2cOsSlaveWriteReceived(void *instance_p, void *para)
{
    FI2cOsSlaveCb(instance_p, para, FI2C_EVT_SLAVE_WRITE_RECEIVED);
}

/**
 * @name: FI2cSlaveReadProcessed
 * @msg: 在Slave发送模式下，发送完数据的最后一个字节后，在规定时间内没有收到 Master 端的回应
 * @return {*} 无
 * @param {void} *instance_p
 * @param {void} *para
 */
void FI2cOsSlaveReadProcessed(void *instance_p, void *para)
{
    FI2cOsSlaveCb(instance_p, para, FI2C_EVT_SLAVE_READ_PROCESSED);
}

/**
 * @name: FI2cSlaveReadRequest
 * @msg: slave收到主机读取内容的请求
 * @return {*} 无
 * @param {void} *instance_p
 * @param {void} *para
 */
void FI2cOsSlaveReadRequest(void *instance_p, void *para)
{
    FI2cOsSlaveCb(instance_p, para, FI2C_EVT_SLAVE_READ_REQUESTED);
}

/**
 * @name: FI2cSlaveStop
 * @msg: I2C总线接口上是否产生了STOP。与控制器工作在Master模式还是 Slave 模式无关。
 * @return {*}
 * @param {void} *instance_p
 * @param {void} *para
 */
void FI2cOsSlaveStop(void *instance_p, void *para)
{
    FI2cOsSlaveCb(instance_p, para, FI2C_EVT_SLAVE_STOP);
}

/**
 * @name: FI2cSlaveWriteRequest
 * @msg: slave收到主机发送的写请求
 * @return {*}
 * @param {void} *instance_p
 * @param {void} *para
 */
void FI2cOsSlaveWriteRequest(void *instance_p, void *para)
{
    FI2cOsSlaveCb(instance_p, para, FI2C_EVT_SLAVE_WRITE_REQUESTED);
}

/**
 * @name: FI2cOsSlaveAbort
 * @msg: slave出现异常
 * @return {*}
 * @param {void} *instance_p
 * @param {void} *para
 */

void FI2cOsSlaveAbort(void *instance_p, void *para)
{
    FI2cOsSlaveCb(instance_p, para, FI2C_EVT_SLAVE_ABORT);
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

/**
 * @name: EepromDump
 * @msg:  打印模拟的Eeprom内部数据
 * @return {FError}
 * @param {FFreeRTOSI2c} *os_i2c_read_p
 */
static FError EepromDump(FFreeRTOSI2c *os_i2c_read_p)
{
    const TickType_t xDelay = pdMS_TO_TICKS(TASK_DELAY_MS);
    vTaskDelay(xDelay);
    FError ret = FREERTOS_I2C_SUCCESS;
    FI2cSlaveData *slave_p = &slave;
    FASSERT(os_i2c_read_p);
    FASSERT(os_i2c_read_p->wr_semaphore != NULL);

    printf("Eeprom dump: \r\n");
    /*只打印前半部分数据*/
    FtDumpHexByte(slave_p->buff, IO_BUF_LEN/2);
    return ret;
}

static FError I2cRead(FFreeRTOSI2c *os_i2c_read_p)
{
    const TickType_t xDelay = pdMS_TO_TICKS(TASK_DELAY_MS);
    vTaskDelay(xDelay);
    FError ret = FREERTOS_I2C_SUCCESS;
    /* Master mode for send or receive data */
    FFreeRTOSI2cMessage message;

    message.slave_addr = os_i2c_read_p->i2c_device.config.slave_addr;
    message.mem_byte_len = 1;    /* 8位地址 */
    message.mem_addr = 0x01;     /* 地址偏移0x1的位置poll方式读取数据 */
    message.buf_length = DAT_LENGTH;
    message.buf = data_r0;
    message.mode = FI2C_READ_DATA_POLL;

    ret = FFreeRTOSI2cTransfer(os_i2c_read_p, &message);
    if (ret != FREERTOS_I2C_SUCCESS)
    {
        FI2C_ERROR("FFreeRTOSI2cTransfer read poll task error,i2c id:%d.\r\n", os_i2c_read_p->i2c_device.config.instance_id);
        return ret;
    }
    message.mem_addr = 0x31;/* 地址偏移0x31的位置poll方式读取数据 */
    message.buf = data_r1;
    message.buf_length = DAT_LENGTH;
    ret = FFreeRTOSI2cTransfer(os_i2c_read_p, &message);
    if (ret != FREERTOS_I2C_SUCCESS)
    {
        FI2C_ERROR("FFreeRTOSI2cTransfer read poll task error,i2c id:%d.\r\n", os_i2c_read_p->i2c_device.config.instance_id);
        return ret;
    }

    for (int i = 0; i < DAT_LENGTH; i++)
    {
        if (data_r0[i] != data_w[i] && data_r1[i] != data_w[i])
        {
            FI2C_ERROR("The read and write data is inconsistent.\r\n");
            ret = FREERTOS_I2C_INVAILD_PARAM_ERROR;
            return ret;
        }
    }
    
    printf("data_r0:\r\n");
    FtDumpHexByte(data_r0, DAT_LENGTH);
    printf("data_r1:\r\n");
    FtDumpHexByte(data_r1, DAT_LENGTH);

    return ret;
}

static FError I2cWrite(FFreeRTOSI2c *os_i2c_write_p)
{
    FError ret = FREERTOS_I2C_SUCCESS;
    /* Master mode for send or receive data */
    FFreeRTOSI2cMessage message;
    message.slave_addr = os_i2c_write_p->i2c_device.config.slave_addr;
    for (u8 i = 0; i < DAT_LENGTH; i++)
    {
        data_w[i] = i + 1;
    }
    /*8位地址*/
    message.mem_byte_len = 1;
    message.buf = data_w;
    message.buf_length = DAT_LENGTH;
    message.mem_addr = 0x01;/* 地址偏移0x1的位置poll方式写入数据 */
    message.mode = FI2C_WRITE_DATA_POLL;
    ret = FFreeRTOSI2cTransfer(os_i2c_write_p, &message);
    if (ret != FREERTOS_I2C_SUCCESS)
    {
        FI2C_ERROR("FFreeRTOSI2cTransfer write poll task error,i2c id:%d.\r\n", os_i2c_write_p->i2c_device.config.instance_id);
        return ret;
    }

    message.mem_addr = 0x31;/* 地址偏移0x31的位置poll方式写入数据 */
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
    else if (work_mode == FI2C_SLAVE)
    {
        os_i2c_slave = FFreeRTOSI2cInit(id, work_mode, slave_address, FI2C_SPEED_STANDARD_RATE);
        /* register intr callback */
        InterruptInstall(os_i2c_slave->i2c_device.config.irq_num, FI2cSlaveIntrHandler, &os_i2c_slave->i2c_device, "fi2cslave");
        /* slave mode intr set,must set before master data come in. */
        ret = FI2cSlaveSetupIntr(&os_i2c_slave->i2c_device);
        if (ret != FREERTOS_I2C_SUCCESS)
        {
            FI2C_ERROR("I2c slave intr init failed.\r\n");
            return ret;
        }
        FI2cSlaveData *slave_p = &slave;
        memset(slave_p, 0, sizeof(*slave_p));
        slave_p->first_write = TRUE;
        FI2cSlaveRegisterIntrHandler(&os_i2c_slave->i2c_device, FI2C_EVT_SLAVE_WRITE_RECEIVED, FI2cOsSlaveWriteReceived);
        FI2cSlaveRegisterIntrHandler(&os_i2c_slave->i2c_device, FI2C_EVT_SLAVE_READ_PROCESSED, FI2cOsSlaveReadProcessed);
        FI2cSlaveRegisterIntrHandler(&os_i2c_slave->i2c_device, FI2C_EVT_SLAVE_READ_REQUESTED, FI2cOsSlaveReadRequest);
        FI2cSlaveRegisterIntrHandler(&os_i2c_slave->i2c_device, FI2C_EVT_SLAVE_STOP, FI2cOsSlaveStop);
        FI2cSlaveRegisterIntrHandler(&os_i2c_slave->i2c_device, FI2C_EVT_SLAVE_WRITE_REQUESTED, FI2cOsSlaveWriteRequest);
        FI2cSlaveRegisterIntrHandler(&os_i2c_slave->i2c_device, FI2C_EVT_SLAVE_ABORT, FI2cOsSlaveAbort);
    }

    return ret;
}

static void FFreeRTOSI2cDelete(void)
{
    FIOMuxDeInit();

    FFreeRTOSI2cDeinit(os_i2c_master);
    FFreeRTOSI2cDeinit(os_i2c_slave);
}

static void FFreeRTOSI2cLoopbackTask(void *pvParameters)
{
    FError ret;
    BaseType_t xReturn = pdPASS;
    int task_res = I2C_MS_TEST_SUCCESS;
    FIOMuxInit();

    ret = FFreeRTOSI2cInitSet(I2C_MS_TEST_MASTER_DEVICE, FI2C_MASTER, MASTER_SLAVE_ADDR);
    if (ret != FREERTOS_I2C_SUCCESS)
    {
        FI2C_ERROR("I2c FFreeRTOSI2cInitSet failed.\r\n");
        task_res = I2C_MS_SET_FAILURE;
        goto task_exit;
    }

    ret = FFreeRTOSI2cInitSet(I2C_MS_TEST_SLAVE_DEVICE, FI2C_SLAVE, MASTER_SLAVE_ADDR);
    if (ret != FREERTOS_I2C_SUCCESS)
    {
        FI2C_ERROR("I2c FFreeRTOSI2cInitSet failed.\r\n");
        task_res = I2C_MS_SET_FAILURE;
        goto task_exit;
    }

    ret = I2cWrite(os_i2c_master);
    if (ret != FREERTOS_I2C_SUCCESS)
    {
        FI2C_ERROR("I2cWrite failed.\r\n");
        task_res = I2C_MS_WRITE_FAILURE;
        goto task_exit;
    }

    ret = I2cRead(os_i2c_master);
    if (ret != FREERTOS_I2C_SUCCESS)
    {
        FI2C_ERROR("I2cRead failed.\r\n");
        task_res = I2C_MS_READ_FAILURE;
        goto task_exit;
    }

    ret = EepromDump(os_i2c_slave);
    if (ret != FREERTOS_I2C_SUCCESS)
    {
        FI2C_ERROR("EepromDump failed.\r\n");
        task_res = I2C_MS_EEPROM_DUMP_FAILURE;
        goto task_exit;
    }

task_exit:
    xQueueSend(xQueue, &task_res, 0);

    vTaskDelete(NULL);
}

BaseType_t FFreeRTOSI2cLoopbackCreate(void)
{
    BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为 pdPASS */
    BaseType_t xTimerStarted = pdPASS;
    int task_res = I2C_MS_TEST_UNKNOWN;

    xQueue = xQueueCreate(1, sizeof(int));
    if (xQueue == NULL)
    {
        printf("xQueue create failed.\r\n");
        goto exit;
    }

    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSI2cLoopbackTask,  /* 任务入口函数 */
                          (const char *)"FFreeRTOSI2cLoopbackTask",/* 任务名字 */
                          (uint16_t)1024,  /* 任务栈大小 */
                          (void *)NULL,/* 任务入口函数参数 */
                          (UBaseType_t)I2C_MS_TEST_TASK_PRIORITY,  /* 任务的优先级 */
                          NULL); /* 任务控制 */
    FASSERT_MSG(xReturn == pdPASS, "FFreeRTOSI2cLoopbackTask creation is failed.");
    
    xReturn = xQueueReceive(xQueue, &task_res, TIMER_OUT);
    if (xReturn == pdFAIL)
    {
        FI2C_ERROR("xQueue receive timeout.\r\n");
        goto exit;
    }

exit:
    vQueueDelete(xQueue);
    FFreeRTOSI2cDelete();
    if (task_res != I2C_MS_TEST_SUCCESS)
    {
        printf("%s@%d: I2C ms simulate eeprom example [failure], task_res = %d\r\n", __func__, __LINE__, task_res);
        return pdFAIL;
    }
    else
    {
        printf("%s@%d: I2C ms simulate eeprom example [success].\r\n", __func__, __LINE__);
        return pdTRUE;
    }
}




