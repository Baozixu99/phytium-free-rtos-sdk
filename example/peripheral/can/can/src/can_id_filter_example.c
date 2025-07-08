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
 * FilePath: can_id_filter_example.c
 * Date: 2023-10-07 14:53:42
 * LastEditTime: 2023-10-20 17:46:03
 * Description:  This file is for can id filter example function implmentation
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huangjin   2023/10/7   first release
 * 2.0   huangjin   2024/04/24  add no letter shell mode, adapt to auto-test system
 */

/***************************** Include Files *********************************/
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

/************************** Constant Definitions *****************************/
#define FCAN_TEST_DEBUG_TAG "FCAN_FREERTOS_FILTER_TEST"
#define FCAN_TEST_DEBUG(format, ...) FT_DEBUG_PRINT_D(FCAN_TEST_DEBUG_TAG, format, ##__VA_ARGS__)
#define FCAN_TEST_INFO(format, ...) FT_DEBUG_PRINT_I(FCAN_TEST_DEBUG_TAG, format, ##__VA_ARGS__)
#define FCAN_TEST_WARN(format, ...) FT_DEBUG_PRINT_W(FCAN_TEST_DEBUG_TAG, format, ##__VA_ARGS__)
#define FCAN_TEST_ERROR(format, ...) FT_DEBUG_PRINT_E(FCAN_TEST_DEBUG_TAG, format, ##__VA_ARGS__)

enum
{
    CAN_TEST_SUCCESS = 0, /*Can intr loopback mode test success*/
    CAN_INIT_FAILURE = 1, /*Can init step failure */
    CAN_SEND_FAILURE = 2, /*Can send step failure */
    CAN_RECV_FAILURE = 3, /*Can recv step failure */
    CAN_DATA_FAILURE = 4, /*Can data is not equal */
    CAN_UNKNOWN_STATE = 5, /*Can example unknown state */        
};

/* can frame config */
#define CAN_FILTER_MODE_1 1
#define CAN_FILTER_MODE_2 2
#define FCAN_SEND_LENGTH 8
#define FCAN_FILTER_MODE1_ID 0x0F
#define FCAN_FILTER_MODE1_MASK 0x00
#define FCAN_FILTER_MODE2_ID 0x0C
#define FCAN_FILTER_MODE2_MASK 0x03
#define FCAN_FILTER_MODE2_CNT 4
#define TIMER_OUT (pdMS_TO_TICKS(10000UL))
#define RECV_TIMER_OUT (pdMS_TO_TICKS(100UL))
#define CAN_FILTER_TASK_PRIORITY 3
#define FCAN_SEND_CNT 0x0F

/* can baudrate */
#define ARB_BAUD_RATE 1000000
#define DATA_BAUD_RATE 1000000

/**************************** Type Definitions *******************************/
typedef struct
{
    FFreeRTOSCan *os_can_p;
} FCanQueueData;
/************************** Variable Definitions *****************************/
/* Declare a variable of type QueueHandle_t.  This is used to store the queue that is accessed by all three tasks. */
static QueueHandle_t xQueue_irq;
static QueueHandle_t xQueue_task;

static FFreeRTOSCan *os_can_ctrl_p[FCAN_NUM];

static FCanFrame send_frame[FCAN_NUM];
static FCanFrame recv_frame[FCAN_NUM];

static FError FFreeRTOSCanSendThenRecvData(int mode);
static FError FFreeRTOSCanRecvData(int mode, FCanQueueData * xReceiveStructure);
static void FFreeRTOSCanDelete(void);

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Function *****************************************/
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

static FError FFreeRTOSCanIdMaskSet(FFreeRTOSCan *os_can_p, int mode)
{
    FError ret = FCAN_SUCCESS;

    if (mode == 1)
    {
        FCanIdMaskConfig id_mask;
        memset(&id_mask, 0, sizeof(id_mask));
        for (int i = 0; i < FCAN_ACC_ID_REG_NUM; i++)
        {
            id_mask.filter_index = i;
            id_mask.id = FCAN_FILTER_MODE1_ID;   // 只接收id的消息
            id_mask.mask = FCAN_FILTER_MODE1_MASK; // 掩码 FCAN_ACC_IDN_MASK   对应位为 1:则忽略 0：则比较
            ret = FFreeRTOSCanControl(os_can_p, FREERTOS_CAN_CTRL_ID_MASK_SET, &id_mask);
            if (FCAN_SUCCESS != ret)
            {
                FCAN_TEST_ERROR("FFreeRTOSCanControl FREERTOS_CAN_CTRL_ID_MASK_SET %d failed.", i);
                return ret;
            }
        }
    }
    else if (mode == 2)
    {
        FCanIdMaskConfig id_mask;
        memset(&id_mask, 0, sizeof(id_mask));
        for (int i = 0; i < FCAN_ACC_ID_REG_NUM; i++)
        {
            id_mask.filter_index = i;
            id_mask.id = FCAN_FILTER_MODE2_ID;        //(canid&maskid)与canid比较
            id_mask.mask = FCAN_FILTER_MODE2_MASK;    // 0011 比较高两位，忽略低两位
            ret = FFreeRTOSCanControl(os_can_p, FREERTOS_CAN_CTRL_ID_MASK_SET, &id_mask);
            if (FCAN_SUCCESS != ret)
            {
                FCAN_TEST_ERROR("FFreeRTOSCanControl FREERTOS_CAN_CTRL_ID_MASK_SET %d failed.", i);
                return ret;
            }
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

        /* init can interrupt handler */
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
    1:Only one id is received
    2:Filter a group of ids
 */
static void FFreeRTOSCanFilterTask(void *pvParameters)
{
    FError ret = FCAN_SUCCESS;
    int task_res = 0;
    int mode = (int)(uintptr)pvParameters;

    /* The queue is created to hold a maximum of 32 structures of type xData. */
    xQueue_irq = xQueueCreate(32, sizeof(FCanQueueData));
    if (xQueue_irq == NULL)
    {
        FCAN_TEST_ERROR("xQueue_irq create failed.");
    }

    /* init can controller */
    ret = CanInit(mode);
    if (CAN_TEST_SUCCESS != ret)
    {
        FCAN_TEST_ERROR("Can init failed.");
        task_res = CAN_INIT_FAILURE;
        goto can_test_exit;
    }

    /* can send data */
    ret = FFreeRTOSCanSendThenRecvData(mode);
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

static FError FFreeRTOSCanSendThenRecvData(int mode)
{
    FError ret = FCAN_SUCCESS;
    u32 instance_id = FCAN0_ID;
    u32 count[FCAN_NUM] = {0};
    BaseType_t xReturn = pdPASS;
    FCanQueueData xReceiveStructure;

    for (u32 instance_id = FCAN0_ID; instance_id <= FCAN1_ID; instance_id++)
    {
        /* can id递增发送 */
        for (u32 id = 0x00; id <= FCAN_SEND_CNT; id++)
        {
            send_frame[instance_id].canid = id;
            send_frame[instance_id].canid &= CAN_SFF_MASK;
            send_frame[instance_id].candlc = FCAN_SEND_LENGTH;
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
            ret = CAN_UNKNOWN_STATE; 
            /* wait recv interrupt */
            xReturn = xQueueReceive(xQueue_irq, &xReceiveStructure, RECV_TIMER_OUT);
            if (xReturn == pdPASS)
            {
                /* can recv data */
                ret = FFreeRTOSCanRecvData(mode, &xReceiveStructure);
                if (CAN_TEST_SUCCESS != ret)
                {
                        FCAN_TEST_ERROR("Can recv data failed.");
                        return ret;
                }
            }
            else
            {
                continue;
            }
        }
    }

    return ret;
}

static FError FFreeRTOSCanRecvData(int mode, FCanQueueData * xReceiveStructure)
{
    FError ret = FCAN_SUCCESS;
    FFreeRTOSCan *os_can_p;
    u32 instance_id = FCAN0_ID;
    static u32 recv_count[FCAN_NUM] = {0};

    os_can_p = xReceiveStructure->os_can_p;
    instance_id = os_can_p->can_ctrl.config.instance_id;
    ret = FFreeRTOSCanRecv(os_can_ctrl_p[instance_id], &recv_frame[instance_id]);
    if (FCAN_SUCCESS == ret)
    {
        FCAN_TEST_DEBUG("can%d recv id is 0x%02x.", instance_id, recv_frame[instance_id].canid);
        if (recv_frame[instance_id].canid == send_frame[FCAN1_ID - instance_id].canid)
        {
            for (int i = 0; i < recv_frame[instance_id].candlc; i++)
            {
                if (recv_frame[instance_id].data[i] != send_frame[FCAN1_ID - instance_id].data[i])
                {
                    FCAN_TEST_ERROR("count=%d: can %d recv is not equal to can%d send!!!", recv_count[instance_id], instance_id, FCAN1_ID - instance_id);
                    ret = CAN_DATA_FAILURE;
                    return ret;
                }
            }
        }
        FCAN_TEST_DEBUG("The frame id:0x%02x was receved successfully.", recv_frame[instance_id].canid);

        if ( (mode == CAN_FILTER_MODE_1) && (recv_frame[instance_id].canid == FCAN_FILTER_MODE1_ID) )
        {
            printf("can%d -> can%d: Filter mode1 test completed.\r\n", FCAN1_ID - instance_id, instance_id);
            return ret;
        }
        else if ( (mode == CAN_FILTER_MODE_2) && ((recv_frame[instance_id].canid & FCAN_FILTER_MODE2_ID) == FCAN_FILTER_MODE2_ID) )
        {
            recv_count[instance_id]++;
            if ( recv_count[instance_id] == FCAN_FILTER_MODE2_CNT )
            {
                printf("can%d -> can%d: Filter mode2 test completed.\r\n", FCAN1_ID - instance_id, instance_id);
                recv_count[instance_id] = 0;
                return ret;
            }
        }
        else
        {
            /* 过滤失败 */
            FCAN_TEST_ERROR("can%d recv id 0x%02x should not be received!!!", instance_id, recv_frame[instance_id].canid);
            ret = CAN_DATA_FAILURE;
            return ret;            
        }
    }

    return ret;
}

/* function1,2 of can id filter example */
BaseType_t FFreeRTOSCanCreateFilterTestTask(void)
{
    int task_res = CAN_UNKNOWN_STATE;
    BaseType_t xReturn = pdPASS;

    xQueue_task = xQueueCreate(1, sizeof(int));
    if (xQueue_task == NULL)
    {
        FCAN_TEST_ERROR("xQueue_task create failed.");
        goto exit;
    }

    /* can filter test1 task */
    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSCanFilterTask,         /* 任务入口函数 */
                          (const char *)"FFreeRTOSCanFilterMode1Task", /* 任务名字 */
                          1024,                                 /* 任务栈大小 */
                          (void *)CAN_FILTER_MODE_1,                                      /* 任务入口函数参数 */
                          (UBaseType_t)CAN_FILTER_TASK_PRIORITY,          /* 任务的优先级 */
                          NULL);                                          /* 任务控制 */
    if (xReturn == pdFAIL)
    {
        FCAN_TEST_ERROR("xTaskCreate FFreeRTOSCanFilterMode1Task failed.");
        goto exit;
    }

    xReturn = xQueueReceive(xQueue_task, &task_res, TIMER_OUT);
    if (xReturn == pdFAIL)
    {
        FCAN_TEST_ERROR("xQueue_task receive timeout.");
        goto exit;
    }

    /* can filter test2 task */
    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSCanFilterTask,         /* 任务入口函数 */
                          (const char *)"FFreeRTOSCanFilterMode2Task", /* 任务名字 */
                          1024,                                 /* 任务栈大小 */
                          (void *)CAN_FILTER_MODE_2,                                      /* 任务入口函数参数 */
                          (UBaseType_t)CAN_FILTER_TASK_PRIORITY,          /* 任务的优先级 */
                          NULL);                                          /* 任务控制 */
    if (xReturn == pdFAIL)
    {
        FCAN_TEST_ERROR("xTaskCreate FFreeRTOSCanFilterMode2Task failed.");
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
        printf("%s@%d: Can Filter mode example [failure], task_res = %d\r\n", __func__, __LINE__, task_res);
        return pdFAIL;
    }
    else
    {
        printf("%s@%d: Can Filter mode example [success].\r\n", __func__, __LINE__);
        return pdPASS;
    }

    return xReturn;
}

static void FFreeRTOSCanDelete(void)
{
    /* deinit can os instance */
    FFreeRTOSCanDeinit(os_can_ctrl_p[FCAN1_ID]);
    FFreeRTOSCanDeinit(os_can_ctrl_p[FCAN0_ID]);

    /*iopad deinit */
    FIOMuxDeInit();

    /* delete queue */
    vQueueDelete(xQueue_irq);
}
