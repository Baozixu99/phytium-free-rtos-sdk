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
 * FilePath: can_polled_loopback_mode_example.c
 * Date: 2023-10-11 11:13:48
 * LastEditTime: 2023-10-20 11:13:48
 * Description:  This file is for CAN task implementations 
 *
 * Modify History:
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huangjin   2023/10/7   first commit
 * 2.0   huangjin   2024/04/24  add no letter shell mode, adapt to auto-test system
 */
#include <string.h>
#include <stdio.h>
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "fcan.h"
#include "fcan_os.h"
#include "fcpu_info.h"
#include "fio_mux.h"
#include "fassert.h"
#include "fdebug.h"

#define FCAN_TEST_DEBUG_TAG "FCAN_FREERTOS_POLLED_TEST"
#define FCAN_TEST_DEBUG(format, ...) FT_DEBUG_PRINT_D(FCAN_TEST_DEBUG_TAG, format, ##__VA_ARGS__)
#define FCAN_TEST_INFO(format, ...) FT_DEBUG_PRINT_I(FCAN_TEST_DEBUG_TAG, format, ##__VA_ARGS__)
#define FCAN_TEST_WARN(format, ...) FT_DEBUG_PRINT_W(FCAN_TEST_DEBUG_TAG, format, ##__VA_ARGS__)
#define FCAN_TEST_ERROR(format, ...) FT_DEBUG_PRINT_E(FCAN_TEST_DEBUG_TAG, format, ##__VA_ARGS__)

enum 
{
    CAN_TEST_SUCCESS = 0,  /*Can intr loopback mode test success*/
    CAN_INIT_FAILURE = 1,  /*Can init step failure */
    CAN_SEND_FAILURE = 2,  /*Can send step failure */
    CAN_RECV_FAILURE = 3,  /*Can recv step failure */
    CAN_DATA_FAILURE = 4,  /*Can data is not equal */     
    CAN_UNKNOWN_STATE = 5, /*Can example unknown state */                     
};

/* can frame config */
#define FCAN_SEND_STID 0x000007FDU
#define FCAN_SEND_EXID 0x1FFFFFFDU
#define FCAN_SEND_LENGTH 8
#define FCAN_SEND_CNT 3
#define TIMER_OUT ( pdMS_TO_TICKS( 3000UL ) )
#define CAN_POLLED_TASK_PRIORITY 3

/* can send period */
#define CAN_SEND_PERIOD             ( pdMS_TO_TICKS( 100UL ) )

/* can baudrate */
#define ARB_BAUD_RATE  1000000
#define DATA_BAUD_RATE 1000000

/* Declare a variable of type QueueHandle_t.  This is used to store the queue that is accessed by all three tasks. */
static QueueHandle_t xQueue_task;

static FFreeRTOSCan *os_can_ctrl_p[FCAN_NUM];

static FCanFrame send_frame[FCAN_NUM];
static FCanFrame recv_frame[FCAN_NUM];

static FError FFreeRTOSCanSendThenRecvData(int ide);
static FError FFreeRTOSCanRecvData(u32 instance_id);
static void FFreeRTOSCanDelete(void);

static FError FFreeRTOSCanBaudrateSet(FFreeRTOSCan *os_can_p)
{
    FError ret = FCAN_SUCCESS;

    FCanBaudrateConfig arb_segment_config;
    FCanBaudrateConfig data_segment_config;
    memset(&arb_segment_config, 0, sizeof(arb_segment_config));
    memset(&data_segment_config, 0, sizeof(data_segment_config));
    arb_segment_config.baudrate = ARB_BAUD_RATE;
    arb_segment_config.auto_calc = TRUE;
    arb_segment_config.segment = FCAN_ARB_SEGMENT;

    data_segment_config.baudrate = DATA_BAUD_RATE;
    data_segment_config.auto_calc = TRUE;
    data_segment_config.segment = FCAN_DATA_SEGMENT;

    ret = FFreeRTOSCanControl(os_can_p, FREERTOS_CAN_CTRL_BAUDRATE_SET, &arb_segment_config);
    if (FCAN_SUCCESS != ret)
    {
        FCAN_TEST_ERROR("FFreeRTOSCanControl arb_segment_config failed.");
        return ret;
    }

    ret = FFreeRTOSCanControl(os_can_p, FREERTOS_CAN_CTRL_BAUDRATE_SET, &data_segment_config);
    if (FCAN_SUCCESS != ret)
    {
        FCAN_TEST_ERROR("FFreeRTOSCanControl data_segment_config failed.");
        return ret;
    }
    return ret;
}

static FError FFreeRTOSCanIdMaskSet(FFreeRTOSCan *os_can_p, int frame_type)
{
    FError ret = FCAN_SUCCESS;
    FCanIdMaskConfig id_mask;
    memset(&id_mask, 0, sizeof(id_mask));
    for (int i = 0; i < FCAN_ACC_ID_REG_NUM; i++)
    {
        id_mask.filter_index = i;
        id_mask.id = 0;
        id_mask.mask = FCAN_ACC_IDN_MASK;
        if ( frame_type == 1 )
        {
            id_mask.type = EXTEND_FRAME;
        }

        ret = FFreeRTOSCanControl(os_can_p, FREERTOS_CAN_CTRL_ID_MASK_SET, &id_mask);
        if (FCAN_SUCCESS != ret)
        {
            FCAN_TEST_ERROR("FFreeRTOSCanControl FREERTOS_CAN_CTRL_ID_MASK_SET %d failed.", i);
            return ret;
        }
    }

    return ret;
}

static FError CanInit(int ide)
{
    u32 instance_id = FCAN0_ID;
    FError init_ret = FCAN_FAILURE;
    u32 tran_mode = FCAN_PROBE_NORMAL_MODE;

    /*init iomux*/
    FIOMuxInit();

    for (instance_id = FCAN0_ID; instance_id <= FCAN1_ID; instance_id++)
    {
        FIOPadSetCanMux(instance_id);

        /* init can controller */
        os_can_ctrl_p[instance_id] = FFreeRTOSCanInit(instance_id);
        if (os_can_ctrl_p[instance_id] == NULL)
        {
            FCAN_TEST_ERROR("FFreeRTOSCanInit %d failed!!!", instance_id);
            return init_ret;
        }

        /* set can baudrate */
        init_ret = FFreeRTOSCanBaudrateSet(os_can_ctrl_p[instance_id]);
        if (FCAN_SUCCESS != init_ret)
        {
            FCAN_TEST_ERROR("FFreeRTOSCanInit FFreeRTOSCanBaudrateSet failed!!!");
            return init_ret;
        }

        /* set can id mask */
        init_ret = FFreeRTOSCanIdMaskSet(os_can_ctrl_p[instance_id], ide);
        if (FCAN_SUCCESS != init_ret)
        {
            FCAN_TEST_ERROR("FFreeRTOSCanInit FFreeRTOSCanIdMaskSet failed!!!");
            return init_ret;
        }

        /* Identifier mask enable */
        init_ret = FFreeRTOSCanControl(os_can_ctrl_p[instance_id], FREERTOS_CAN_CTRL_ID_MASK_ENABLE, NULL);
        if (FCAN_SUCCESS != init_ret)
        {
            FCAN_TEST_ERROR("FFreeRTOSCanControl FREERTOS_CAN_CTRL_ID_MASK_ENABLE failed.");
            return init_ret;
        }

        /* set can transfer mode */
        init_ret = FFreeRTOSCanControl(os_can_ctrl_p[instance_id], FREERTOS_CAN_CTRL_MODE_SET, &tran_mode);
        if (FCAN_SUCCESS != init_ret)
        {
            FCAN_TEST_ERROR("FFreeRTOSCanControl FREERTOS_CAN_CTRL_MODE_SET failed.");
            return init_ret;
        }

        /* enable can transfer */
        init_ret = FFreeRTOSCanControl(os_can_ctrl_p[instance_id], FREERTOS_CAN_CTRL_ENABLE, NULL);
        if (FCAN_SUCCESS != init_ret)
        {
            FCAN_TEST_ERROR("FFreeRTOSCanControl FREERTOS_CAN_CTRL_ENABLE failed.");
            return init_ret;
        }
    }

    return init_ret;
}

/*  
pvParameters 
    0:standard frame test task
    1:extended frame test task
 */
static void FFreeRTOSCanPolledTask(void *pvParameters)
{
    FError ret = FCAN_SUCCESS;
    int task_res = 0;
    int ide = (int)(uintptr)pvParameters;

    /* init can controller */
    ret = CanInit(ide);
    if (CAN_TEST_SUCCESS != ret)
    {
        FCAN_TEST_ERROR("Can init failed.");
        task_res = CAN_INIT_FAILURE;
        goto can_test_exit;
    }

    /* can send data */
    ret = FFreeRTOSCanSendThenRecvData(ide);
    if (CAN_TEST_SUCCESS != ret)
    {
        FCAN_TEST_ERROR("Can send then recv data failed.");
        task_res = CAN_DATA_FAILURE;
        goto can_test_exit;
    }

can_test_exit:
    FFreeRTOSCanDelete();
    xQueueSend(xQueue_task, &task_res, 0);
    vTaskDelete(NULL);
}

static FError FFreeRTOSCanSendThenRecvData(int ide)
{
    FError ret = FCAN_SUCCESS;
    u32 instance_id = FCAN0_ID;
    u32 count[FCAN_NUM] = {0};
    BaseType_t xReturn = pdPASS;

    for (int j = 0; j < FCAN_SEND_CNT; j++)
    {
        for (u32 instance_id = FCAN0_ID; instance_id <= FCAN1_ID; instance_id++)
        {
            send_frame[instance_id].candlc = FCAN_SEND_LENGTH;
            if (ide == EXTEND_FRAME)
            {
                send_frame[instance_id].canid = FCAN_SEND_EXID + count[instance_id];
                send_frame[instance_id].canid |= CAN_EFF_FLAG;
            }  
            else
            {
                send_frame[instance_id].canid = FCAN_SEND_STID + count[instance_id]; 
                send_frame[instance_id].canid &= CAN_SFF_MASK;
            }

            for (int i = 0; i < send_frame[instance_id].candlc; i++)
            {
                send_frame[instance_id].data[i] = i + (instance_id << 4);
            }
            ret = FFreeRTOSCanSend(os_can_ctrl_p[instance_id], &send_frame[instance_id]);
            if (ret != FCAN_SUCCESS)
            {
                FCAN_TEST_ERROR("can%d send failed.", instance_id);
                ret = FCAN_INVAL_PARAM;
                return ret;
            }
            count[instance_id]++;
            vTaskDelay(CAN_SEND_PERIOD);

            /* can recv data */
            ret = FFreeRTOSCanRecvData(instance_id);
            if (CAN_TEST_SUCCESS != ret)
            {
                FCAN_TEST_ERROR("Can recv data failed.");
                return ret;
            }
        }
    }

return ret;
}

static FError FFreeRTOSCanRecvData(u32 instance_id)
{
    FError ret = FCAN_SUCCESS;
    static u32 recv_count[FCAN_NUM] = {0};
    
    instance_id = FCAN1_ID - instance_id;
    ret = FFreeRTOSCanRecv(os_can_ctrl_p[instance_id], &recv_frame[instance_id]);
    if (FCAN_SUCCESS == ret)
    {
        FCAN_TEST_DEBUG("can%d recv id is 0x%02x.", instance_id, recv_frame[instance_id].canid);
        FCAN_TEST_DEBUG("can%d recv dlc is %d.", instance_id, recv_frame[instance_id].candlc);
        FCAN_TEST_DEBUG("can%d recv data is ", instance_id);
        for (int i = 0; i < recv_frame[instance_id].candlc; i++)
        {
            FCAN_TEST_DEBUG("0x%02x ", recv_frame[instance_id].data[i]);
            if (recv_frame[instance_id].data[i] != send_frame[FCAN1_ID - instance_id].data[i])
            {
                FCAN_TEST_ERROR("count%d = %d: can%d recv is not equal to can%d send!!!", instance_id,  recv_count[instance_id], instance_id, FCAN1_ID - instance_id);
                ret = CAN_DATA_FAILURE;
                return ret;
            }
        }
        recv_count[instance_id]++;
        FCAN_TEST_DEBUG("count%d = %d: can%d recv is equal to can%d send!!!", instance_id, recv_count[instance_id], instance_id, FCAN1_ID - instance_id);
    }
    if (recv_count[instance_id] == FCAN_SEND_CNT)
    {
        if (recv_frame[instance_id].canid & CAN_EFF_FLAG)
        {
            printf("can%d -> can%d: Extended frame polled loop test completed.\r\n", FCAN1_ID - instance_id, instance_id);
        }
        else
        {
            printf("can%d -> can%d: Standard frame polled loop test completed.\r\n", FCAN1_ID - instance_id, instance_id);
        }
        recv_count[instance_id] = 0;
    }    

    return ret;
}

/* create can polled test, can0 and can1 loopback */
BaseType_t FFreeRTOSCreateCanPolledTestTask(void)
{
    int task_res = CAN_UNKNOWN_STATE;
    BaseType_t xReturn = pdPASS;

    xQueue_task = xQueueCreate(1, sizeof(int));
    if (xQueue_task == NULL)
    {
        FCAN_TEST_ERROR("xQueue_task create failed.");
        goto exit;
    }

    /* can polled example standard frame task */
    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSCanPolledTask,  /* 任务入口函数 */
                          (const char *)"FFreeRTOSCanPolledStandardTask",/* 任务名字 */
                          1024,  /* 任务栈大小 */
                          (void *)STANDARD_FRAME,/* 任务入口函数参数 */
                          (UBaseType_t)CAN_POLLED_TASK_PRIORITY,  /* 任务的优先级 */
                          NULL); /* 任务控制 */
    if (xReturn == pdFAIL)
    {
        FCAN_TEST_ERROR("xTaskCreate FFreeRTOSCanPolledTask failed.");
        goto exit;
    }

    xReturn = xQueueReceive(xQueue_task, &task_res, TIMER_OUT);
    if (xReturn == pdFAIL)
    {
        FCAN_TEST_ERROR("xQueue_task receive timeout.");
        goto exit;
    }    

    /* can polled example extended frame task */
    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSCanPolledTask,  /* 任务入口函数 */
                          (const char *)"FFreeRTOSCanPolledExtendedTask",/* 任务名字 */
                          1024,  /* 任务栈大小 */
                          (void *)EXTEND_FRAME,/* 任务入口函数参数 */
                          (UBaseType_t)CAN_POLLED_TASK_PRIORITY,  /* 任务的优先级 */
                          NULL); /* 任务控制 */
    if (xReturn == pdFAIL)
    {
        FCAN_TEST_ERROR("xTaskCreate FFreeRTOSCanPolledTask failed.");
        goto exit;
    }

    xReturn = xQueueReceive(xQueue_task, &task_res, TIMER_OUT);                                       
    if (xReturn == pdFAIL)
    {
        FCAN_TEST_ERROR("xQueue_task receive timeout.");
        goto exit;
    }
    
exit:
    if (xQueue_task != NULL)
    {
        vQueueDelete(xQueue_task);
    }

    if (task_res != CAN_TEST_SUCCESS)
    {
        printf("%s@%d: Can polled loopback mode example [failure], task_res = %d\r\n", __func__, __LINE__, task_res);
        return pdFAIL;
    }
    else
    {
        printf("%s@%d: Can polled loopback mode example [success].\r\n", __func__, __LINE__);
        return pdPASS;
    }
}

static void FFreeRTOSCanDelete(void)
{
    /* deinit can os instance */
    FFreeRTOSCanDeinit(os_can_ctrl_p[FCAN1_ID]);
    FFreeRTOSCanDeinit(os_can_ctrl_p[FCAN0_ID]);

    /*iopad deinit */
    FIOMuxDeInit();
}
