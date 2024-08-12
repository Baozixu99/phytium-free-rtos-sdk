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
 * FilePath: canfd_intr_loopback_mode_example.c
 * Date: 2023-10-20 11:32:48
 * LastEditTime: 2023-10-20 11:32:48
 * Description:  This file is for canfd intr loopback mode example task implementations 
 *
 * Modify History:
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huangjin   2023/10/7   first commit
 * 2.0   huangjin   2024/04/25  add no letter shell mode, adapt to auto-test system
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

#define FCAN_TEST_DEBUG_TAG "FCANFD_FREERTOS_INTR_TEST"
#define FCAN_TEST_DEBUG(format, ...) FT_DEBUG_PRINT_D(FCAN_TEST_DEBUG_TAG, format, ##__VA_ARGS__)
#define FCAN_TEST_INFO(format, ...) FT_DEBUG_PRINT_I(FCAN_TEST_DEBUG_TAG, format, ##__VA_ARGS__)
#define FCAN_TEST_WARN(format, ...) FT_DEBUG_PRINT_W(FCAN_TEST_DEBUG_TAG, format, ##__VA_ARGS__)
#define FCAN_TEST_ERROR(format, ...) FT_DEBUG_PRINT_E(FCAN_TEST_DEBUG_TAG, format, ##__VA_ARGS__)

enum 
{
    CANFD_TEST_SUCCESS = 0,  /*Canfd intr loopback mode test success*/
    CANFD_INIT_FAILURE = 1,  /*Canfd init step failure */
    CANFD_SEND_FAILURE = 2,  /*Canfd send step failure */
    CANFD_RECV_FAILURE = 3,  /*Canfd recv step failure */
    CANFD_DATA_FAILURE = 4,  /*Canfd data is not equal */       
    CANFD_UNKNOWN_STATE = 5, /*Canfd example unknown state */                
};

/* canfd frame config */
#define CANFD_ENABLE TRUE
#define FCAN_SEND_STID 0x000007FDU
#define FCAN_SEND_EXID 0x1FFFFFFDU
#define FCAN_SEND_LENGTH 16
#define FCANFD_SEND_CNT 3
#define TIMER_OUT ( pdMS_TO_TICKS( 3000UL ) )
#define CANFD_INTR_TASK_PRIORITY  3

/* canfd baudrate */
#define ARB_BAUD_RATE  1000000
#define DATA_BAUD_RATE 1000000

typedef struct
{
    FFreeRTOSCan *os_can_p;
} FCanQueueData;

/* Declare a variable of type QueueHandle_t.  This is used to store the queue that is accessed by all three tasks. */
static QueueHandle_t xQueue_irq;
static QueueHandle_t xQueue_task;

static FFreeRTOSCan *os_can_ctrl_p[FCAN_NUM];

static FCanFrame send_frame[FCAN_NUM];
static FCanFrame recv_frame[FCAN_NUM];

static FError FFreeRTOSCanfdSendThenRecvData(int ide);
static FError FFreeRTOSCanfdRecvData(FCanQueueData *xReceiveStructure);
static void FFreeRTOSCanDelete(void);

static void FCanRxIrqCallback(void *args)
{
    FFreeRTOSCan *os_can_p = (FFreeRTOSCan *)args;
    FCAN_TEST_DEBUG("Can%d irq recv frame callback.", os_can_p->can_ctrl.config.instance_id);

    static FCanQueueData xSendStructure;
    xSendStructure.os_can_p = os_can_p;

    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    xQueueSendToBackFromISR(xQueue_irq, &xSendStructure, &xHigherPriorityTaskWoken);

    /* never call taskYIELD() form ISR! */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static FError FFreeRTOSCanIntrSet(FFreeRTOSCan *os_can_p)
{
    FError ret = FCAN_SUCCESS;
    FCanIntrEventConfig intr_event;
    memset(&intr_event, 0, sizeof(intr_event));

    intr_event.type = FCAN_INTR_EVENT_RECV;
    intr_event.handler = FCanRxIrqCallback;
    intr_event.param = (void *)os_can_p;
    ret = FFreeRTOSCanControl(os_can_p, FREERTOS_CAN_CTRL_INTR_SET, &intr_event);
    if (FCAN_SUCCESS != ret)
    {
        FCAN_TEST_ERROR("FFreeRTOSCanControl FCAN_INTR_EVENT_RECV failed.");
        return ret;
    }

    u32 cpu_id;
    GetCpuId(&cpu_id);
    FCanCtrl *instance_p = &os_can_p->can_ctrl;
    InterruptSetTargetCpus(instance_p->config.irq_num, cpu_id);
    InterruptSetPriority(instance_p->config.irq_num, instance_p->config.irq_prority);
    InterruptInstall(instance_p->config.irq_num, FCanIntrHandler, instance_p, "can");
    InterruptUmask(instance_p->config.irq_num);

    return ret;
}

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

        /* enable canfd */
        init_ret = FFreeRTOSCanControl(os_can_ctrl_p[instance_id], FREERTOS_CAN_CTRL_FD_ENABLE, (void *)CANFD_ENABLE);
        if (FCAN_SUCCESS != init_ret)
        {
            FCAN_TEST_ERROR("FFreeRTOSCanControl FREERTOS_CAN_CTRL_FD_ENABLE failed.");
            return init_ret;
        }

        /* set canfd baudrate */
        init_ret = FFreeRTOSCanBaudrateSet(os_can_ctrl_p[instance_id]);
        if (FCAN_SUCCESS != init_ret)
        {
            FCAN_TEST_ERROR("FFreeRTOSCanInit FFreeRTOSCanBaudrateSet failed!!!");
            return init_ret;
        }

        /* set canfd id mask */
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

        /* init canfd interrupt handler */
        init_ret = FFreeRTOSCanIntrSet(os_can_ctrl_p[instance_id]);
        if (FCAN_SUCCESS != init_ret)
        {
            FCAN_TEST_ERROR("FFreeRTOSCanInit FFreeRTOSCanIntrSet failed!!!");
            return init_ret;
        }

        /* set can transfer mode */
        init_ret = FFreeRTOSCanControl(os_can_ctrl_p[instance_id], FREERTOS_CAN_CTRL_MODE_SET, &tran_mode);
        if (FCAN_SUCCESS != init_ret)
        {
            FCAN_TEST_ERROR("FFreeRTOSCanControl FREERTOS_CAN_CTRL_MODE_SET failed.");
            return init_ret;
        }

        /* enable canfd transfer */
        init_ret = FFreeRTOSCanControl(os_can_ctrl_p[instance_id], FREERTOS_CAN_CTRL_ENABLE, NULL);
        if (FCAN_SUCCESS != init_ret)
        {
            FCAN_TEST_ERROR("FFreeRTOSCanControl FREERTOS_CAN_CTRL_ENABLE failed.");
            return init_ret;
        }
    }

    return init_ret;
}

static void FFreeRTOSCanfdIntrTask(void *pvParameters)
{
    FError ret = FCAN_SUCCESS;
    int task_res = CANFD_TEST_SUCCESS;
    int ide = (int)(uintptr)pvParameters;

    /* The queue is created to hold a maximum of 32 structures of type xData. */
    xQueue_irq = xQueueCreate(32, sizeof(FCanQueueData));
    if (xQueue_irq == NULL)
    {
        FCAN_TEST_ERROR("xQueue_irq create failed.");
    }

    /* init can controller */
    ret = CanInit(ide);
    if (CANFD_TEST_SUCCESS != ret)
    {
        FCAN_TEST_ERROR("Can init failed.");
        task_res = CANFD_INIT_FAILURE;
        goto canfd_test_exit;
    }

    /* canfd send data then recv data */
    ret = FFreeRTOSCanfdSendThenRecvData(ide);
    if (CANFD_TEST_SUCCESS != ret)
    {
        FCAN_TEST_ERROR("Canfd send then recv data failed.");
        task_res = CANFD_DATA_FAILURE;
        goto canfd_test_exit;
    }

canfd_test_exit:
    xQueueSend(xQueue_task, &task_res, 0);
    if (task_res != CANFD_INIT_FAILURE)
    {
        FFreeRTOSCanDelete();
    }
    vTaskDelete(NULL);
}

static FError FFreeRTOSCanfdSendThenRecvData(int ide)
{
    FError ret = FCAN_SUCCESS;
    u32 count[FCAN_NUM] = {0};
    BaseType_t xReturn = pdPASS;
    FCanQueueData xReceiveStructure;

    for (int j = 0; j < FCANFD_SEND_CNT; j++)
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
                FCAN_TEST_ERROR("canfd%d send failed.", instance_id);
                ret = FCAN_INVAL_PARAM;
                return ret;
            }
            count[instance_id]++;

            /* wait recv interrupt */
            xReturn = xQueueReceive(xQueue_irq, &xReceiveStructure, TIMER_OUT);
            if (xReturn == pdFAIL)
            {
                FCAN_TEST_ERROR("xQueue_irq receive timeout.");
                ret = CANFD_RECV_FAILURE;
                return ret;
            } 

            /* canfd recv data */
            ret = FFreeRTOSCanfdRecvData(&xReceiveStructure);
            if (CANFD_TEST_SUCCESS != ret)
            {
                FCAN_TEST_ERROR("Canfd recv data failed.");
                return ret;
            }
        }
    }

    return ret;
}

static FError FFreeRTOSCanfdRecvData(FCanQueueData * xReceiveStructure)
{
    FError ret = FCAN_SUCCESS;
    FFreeRTOSCan *os_can_p;
    u32 instance_id = FCAN0_ID;
    static u32 recv_count[FCAN_NUM] = {0};

    os_can_p = xReceiveStructure->os_can_p;
    instance_id = os_can_p->can_ctrl.config.instance_id;
    ret = FFreeRTOSCanRecv(os_can_p, &recv_frame[instance_id]);
    if (FCAN_SUCCESS == ret)
    {
        FCAN_TEST_DEBUG("canfd%d recv id is 0x%02x.", instance_id, recv_frame[instance_id].canid);
        FCAN_TEST_DEBUG("canfd%d recv dlc is %d.", instance_id, recv_frame[instance_id].candlc);
        FCAN_TEST_DEBUG("canfd%d recv data is ", instance_id);
        for (int i = 0; i < recv_frame[instance_id].candlc; i++)
        {
            FCAN_TEST_DEBUG("0x%02x ", recv_frame[instance_id].data[i]);
            if (recv_frame[instance_id].data[i] != send_frame[FCAN1_ID - instance_id].data[i])
            {
                FCAN_TEST_ERROR("count%d = %d: canfd%d recv is not equal to canfd%d send!!!", instance_id,  recv_count[instance_id], instance_id, FCAN1_ID - instance_id);
            }
        }
        recv_count[instance_id]++;
        FCAN_TEST_DEBUG("count%d = %d: canfd%d recv is equal to canfd%d send!!!", instance_id, recv_count[instance_id], instance_id, FCAN1_ID - instance_id);
    }
    if (recv_count[instance_id] == FCANFD_SEND_CNT)
    {
        if (recv_frame[instance_id].canid & CAN_EFF_FLAG)
        {
            printf("canfd%d -> canfd%d: Extended frame interrupt loop test completed.\r\n", FCAN1_ID - instance_id, instance_id);
        }
        else
        {
            printf("canfd%d -> canfd%d: Standard frame interrupt loop test completed.\r\n", FCAN1_ID - instance_id, instance_id);
        }
        recv_count[instance_id] = 0;
    }
    
    return ret;
}

/* create canfd intr test, can0 and can1 loopback */
BaseType_t FFreeRTOSCreateCanfdIntrTestTask(void)
{
    int task_res = CANFD_UNKNOWN_STATE;
    BaseType_t xReturn = pdPASS;

    xQueue_task = xQueueCreate(1, sizeof(int));
    if (xQueue_task == NULL)
    {
        FCAN_TEST_ERROR("xQueue_task create failed.");
        goto exit;
    }

    /* canfd intr example standard frame task */
    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSCanfdIntrTask,  /* 任务入口函数 */
                          (const char *)"FFreeRTOSCanfdIntrStandardTask",/* 任务名字 */
                          1024,  /* 任务栈大小 */
                          (void *)STANDARD_FRAME,/* 任务入口函数参数 */
                          (UBaseType_t)CANFD_INTR_TASK_PRIORITY,  /* 任务的优先级 */
                          NULL); /* 任务控制 */
    if (xReturn == pdFAIL)
    {
        FCAN_TEST_ERROR("xTaskCreate FFreeRTOSCanfdIntrStandardTask failed.");
        goto exit;
    }    

    xReturn = xQueueReceive(xQueue_task, &task_res, TIMER_OUT);
    if (xReturn == pdFAIL)
    {
        FCAN_TEST_ERROR("xQueue_task receive timeout.");
        goto exit;
    }                            

    /* canfd intr example extended frame task */
    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSCanfdIntrTask,  /* 任务入口函数 */
                          (const char *)"FFreeRTOSCanfdIntrExtendedTask",/* 任务名字 */
                          1024,  /* 任务栈大小 */
                          (void *)EXTEND_FRAME,/* 任务入口函数参数 */
                          (UBaseType_t)CANFD_INTR_TASK_PRIORITY,  /* 任务的优先级 */
                          NULL); /* 任务控制 */
    if (xReturn == pdFAIL)
    {
        FCAN_TEST_ERROR("xTaskCreate FFreeRTOSCanIntrTask failed.");
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
    
    if (task_res != CANFD_TEST_SUCCESS)
    {
        printf("%s@%d: Canfd intr loopback mode example [failure], task_res = %d\r\n", __func__, __LINE__, task_res);
        return pdFAIL;
    }
    else
    {
        printf("%s@%d: Canfd intr loopback mode example [success].\r\n", __func__, __LINE__);
        return pdPASS;
    }
}

static void FFreeRTOSCanDelete(void)
{
    /* deinit canfd os instance */
    FFreeRTOSCanDeinit(os_can_ctrl_p[FCAN1_ID]);
    FFreeRTOSCanDeinit(os_can_ctrl_p[FCAN0_ID]);

    /*iopad deinit */
    FIOMuxDeInit();

    /* delete queue */
    vQueueDelete(xQueue_irq);
}