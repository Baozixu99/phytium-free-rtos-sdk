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
 * FilePath: gpio_intr_example.c
 * Date: 2022-07-22 13:57:42
 * LastEditTime: 2022-07-22 13:57:43
 * Description:  This file is for gpio irq implementation.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  zhugengyu  2022/8/26    init commit
 *  2.0  wangzq     2024/4/22     add no letter shell mode, adapt to auto-test system
 *  3.0  zhugengyu  2024/5/10    rework gpio example
 */
/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "event_groups.h"
#include "task.h"

#include "fdebug.h"
#include "fio_mux.h"

#include "fgpio_hw.h"
#include "fgpio.h"
/************************** Constant Definitions *****************************/
#define GPIO_IRQ_TEST_TASK_PRIORITY      3U
#define GPIO_IRQ_TEST_TASK_STACK_SIZE    4096U
#define GPIO_IRQ_TEST_TIMEOUT            (pdMS_TO_TICKS(4000UL))

#define GPIO_IRQ_EVENT_OCCIRRED          (1 << 0)
/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
#if defined(CONFIG_FIREFLY_DEMO_BOARD)
static const u32 input_pin_index = FGPIO_ID(FGPIO_CTRL_0, FGPIO_PIN_0);
static const u32 output_pin_index = FGPIO_ID(FGPIO_CTRL_4, FGPIO_PIN_13);
#elif defined(CONFIG_E2000D_DEMO_BOARD) || defined(CONFIG_E2000Q_DEMO_BOARD)
static const u32 input_pin_index = FGPIO_ID(FGPIO_CTRL_4, FGPIO_PIN_11);
static const u32 output_pin_index = FGPIO_ID(FGPIO_CTRL_4, FGPIO_PIN_12);
#elif defined(CONFIG_D2000_TEST_BOARD)
static const u32 input_pin_index = FGPIO_ID(FGPIO_CTRL_1, FGPIO_PORT_A, FGPIO_PIN_6);
static const u32 output_pin_index = FGPIO_ID(FGPIO_CTRL_1, FGPIO_PORT_A, FGPIO_PIN_7);
#endif

static FGpioIrqType irq_type = FGPIO_IRQ_TYPE_LEVEL_HIGH;
static const char *irq_type_names[] = 
{
    [FGPIO_IRQ_TYPE_EDGE_FALLING] = "falling edge",
    [FGPIO_IRQ_TYPE_EDGE_RISING] = "rising edge",
    [FGPIO_IRQ_TYPE_LEVEL_LOW] = "level low",
    [FGPIO_IRQ_TYPE_LEVEL_HIGH] = "level high"
};

static QueueHandle_t gpio_queue = NULL;
static EventGroupHandle_t gpio_event = NULL;
static boolean is_running = FALSE;

static FGpio input_pin_instance;
static FGpioConfig input_cfg;
static FGpio output_pin_instance;
static FGpioConfig output_cfg;
/***************** Macros (Inline Functions) Definitions *********************/
#define FGPIO_DEBUG_TAG "GPIO-IRQ-EXAMPLE"
#define FGPIO_ERROR(format, ...) FT_DEBUG_PRINT_E(FGPIO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGPIO_WARN(format, ...)  FT_DEBUG_PRINT_W(FGPIO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGPIO_INFO(format, ...)  FT_DEBUG_PRINT_I(FGPIO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGPIO_DEBUG(format, ...) FT_DEBUG_PRINT_D(FGPIO_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/

/*****************************************************************************/
static void GpioAckPinIrq(s32 vector, void *param)
{
    BaseType_t xhigher_priority_task_woken = pdFALSE;
    BaseType_t x_result = pdFALSE;
    FASSERT(gpio_event);
    x_result = xEventGroupSetBitsFromISR(gpio_event, GPIO_IRQ_EVENT_OCCIRRED,
                                         &xhigher_priority_task_woken);
    printf("Ack pin irq\r\n");
    return;
}

static FError GpioInit(void)
{
    FError ret = FT_SUCCESS;
    u32 cpu_id;
    u32 irq_num;
    u32 irq_priority = IRQ_PRIORITY_VALUE_12; /* change priority for FreeRTOS */

    GetCpuId(&cpu_id);

    memset(&input_pin_instance, 0, sizeof(input_pin_instance));
    memset(&output_pin_instance, 0, sizeof(output_pin_instance));

    input_cfg = *FGpioLookupConfig(input_pin_index);
    output_cfg = *FGpioLookupConfig(output_pin_index);

    FIOMuxInit();

    /* init input/output GPIO pins */
    ret = FGpioCfgInitialize(&input_pin_instance, &input_cfg);
    if (ret != FT_SUCCESS)
    {
        return ret;
    }

    ret = FGpioCfgInitialize(&output_pin_instance, &output_cfg);
    if (ret != FT_SUCCESS)
    {
        return ret;
    }

    /* set iomux for GPIO pins */
#if defined(CONFIG_D2000_TEST_BOARD)
    FIOPadSetGpioMux(input_pin_instance.config.ctrl, input_pin_instance.config.port, (u32)input_pin_instance.config.pin);
    FIOPadSetGpioMux(output_pin_instance.config.ctrl, output_pin_instance.config.port, (u32)output_pin_instance.config.pin);   
#else
    FIOPadSetGpioMux(input_pin_instance.config.ctrl, (u32)input_pin_instance.config.pin);
    FIOPadSetGpioMux(output_pin_instance.config.ctrl, (u32)output_pin_instance.config.pin);
#endif

    /* set direction of GPIO pins */
    FGpioSetDirection(&input_pin_instance, FGPIO_DIR_INPUT);
    FGpioSetDirection(&output_pin_instance, FGPIO_DIR_OUTPUT);

    /* set input interrupt trigger type */
    FGpioSetInterruptType(&input_pin_instance, FGPIO_IRQ_TYPE_EDGE_RISING);

    /* set outpu pin as low level first, then raise output pin level to trigger the interrupt */
    FGpioSetOutputValue(&output_pin_instance, FGPIO_PIN_LOW);

    /* input pin irq set */
    irq_num = input_pin_instance.config.irq_num;
    FGpioSetInterruptMask(&input_pin_instance, FALSE); /* disable pin irq */
    if (input_pin_instance.config.cap & FGPIO_CAP_IRQ_NONE)
    {
        printf("input gpio %u-%u-%u do not support interrupt\r\n", 
                                input_pin_instance.config.ctrl,
                                input_pin_instance.config.port,
                                input_pin_instance.config.pin);
        return FGPIO_ERR_INVALID_PARA;
    }

    InterruptSetTargetCpus(irq_num, cpu_id);
    InterruptSetPriority(irq_num, irq_priority); /* setup interrupt */

    FGpioRegisterInterruptCB(&input_pin_instance, 
                             GpioAckPinIrq, 
                             NULL); /* register intr callback to intr map */

    InterruptInstall(irq_num,
                    FGpioInterruptHandler,
                    NULL,
                    NULL); /* register intr handler */

    InterruptUmask(irq_num);

    return ret;
}

static void GpioDeInit(void)
{
    FIOMuxDeInit();

    if (input_pin_instance.is_ready == FT_COMPONENT_IS_READY)
    {
        InterruptMask(input_pin_instance.config.irq_num);

        FGpioSetInterruptMask(&input_pin_instance, FALSE);

        /* deinit ctrl and pin instance */  
        FGpioDeInitialize(&input_pin_instance);
    }

    if (output_pin_instance.is_ready == FT_COMPONENT_IS_READY)
    {
        FGpioDeInitialize(&output_pin_instance);
    }
}

static FError GpioToggleOutput(void)
{
    FError ret = FT_SUCCESS;

    FGpioSetInterruptMask(&input_pin_instance, TRUE);

    /* create rising edge to trigger the input interrupt */
    ret = FGpioSetOutputValue(&output_pin_instance, FGPIO_PIN_HIGH);
    if (ret != FT_SUCCESS)
    {
        return ret;
    }

    vTaskDelay(pdMS_TO_TICKS(2000U));
    
    FGpioSetInterruptMask(&input_pin_instance, FALSE);

    ret = FGpioSetOutputValue(&output_pin_instance, FGPIO_PIN_LOW);
    if (ret != FT_SUCCESS)
    {
        return ret;
    }

    return ret;
}

static FError GpioWaitIrqOccur(void)
{
    const TickType_t wait_delay = pdMS_TO_TICKS(2000U); /* just check 2sec wait */
    FError ret = TRUE;
    EventBits_t ev = xEventGroupWaitBits(gpio_event,
                                         GPIO_IRQ_EVENT_OCCIRRED,
                                         pdTRUE, pdFALSE, wait_delay);

    if ((ev & GPIO_IRQ_EVENT_OCCIRRED))
    {
        ret = FT_SUCCESS;
    }
    else
    {
        FGPIO_ERROR("Gpio irq example failed.");
        goto exit;
    }

exit:
    return ret;
}

static void GpioIrqTestTask(void)
{
    FError ret = FT_SUCCESS;

    ret = GpioInit();
    if (ret != FT_SUCCESS)
    {
        FGPIO_ERROR("GpioInit failed.");
        goto task_exit;
    }

    ret = GpioToggleOutput();
    if (ret != FT_SUCCESS)
    {
        FGPIO_ERROR("GpioToggleOutput failed.");
        goto task_exit;
    }

    ret = GpioWaitIrqOccur();
    if (ret != FT_SUCCESS)
    {
        FGPIO_ERROR("GpioWaitIrqOccur failed.");
        goto task_exit;
    }

task_exit:
    GpioDeInit();

    xQueueSend(gpio_queue, &ret, 0);
    vTaskDelete(NULL);
}

/*gpio irq example entry function*/
BaseType_t FFreeRTOSRunGpioIrq(void)
{
    BaseType_t ret = pdPASS; /*  pdPASS */
    FError task_res = FT_SUCCESS;
    if (is_running)
    {
        FGPIO_ERROR("The task is running.");
        return ret;
    }

    is_running = TRUE;

    FASSERT_MSG(NULL == gpio_event, "Event group exists.");
    FASSERT_MSG((gpio_event = xEventGroupCreate()) != NULL, "Create event group failed.");  
    FASSERT_MSG(NULL == gpio_queue, "Task queue exists.");
    FASSERT_MSG((gpio_queue = xQueueCreate(1, sizeof(FError))) != NULL, "Create task group failed.");  

    ret = xTaskCreate((TaskFunction_t)GpioIrqTestTask,         /* 任务入口函数 */
                      (const char *)"GpioIrqTestTask",         /* 任务名字 */
                      (uint16_t)GPIO_IRQ_TEST_TASK_STACK_SIZE, /* 任务栈大小 */
                      NULL,                                    /* 任务入口函数参数 */
                      (UBaseType_t)GPIO_IRQ_TEST_TASK_PRIORITY, /* 任务优先级 */
                      NULL);                                   /* 任务句柄 */
    if (ret == pdFAIL)
    {
        FGPIO_ERROR("Create GpioIrqTestTask failed !!!");
        goto exit;
    }

    ret = xQueueReceive(gpio_queue, &task_res, GPIO_IRQ_TEST_TIMEOUT);
    if (ret == pdFAIL)
    {
        FGPIO_ERROR("wait GpioIrqTestTask timeout !!!");
        goto exit;
    }

exit:
    if (gpio_event != NULL)
    {
        vEventGroupDelete(gpio_event);
        gpio_event = NULL;
    }

    if (gpio_queue != NULL)
    {
        vQueueDelete(gpio_queue);
        gpio_queue = NULL;
    }

    is_running = FALSE;

    printf("task_res = %d\r\n", task_res);
    if (task_res != FT_SUCCESS)
    {
        printf("%s@%d: Gpio irq example [failure], task_res = %d\r\n", __func__, __LINE__, task_res);
        return pdFAIL;
    }
    else
    {
        printf("%s@%d: Gpio irq example [success].\r\n", __func__, __LINE__);
        return pdTRUE;
    }
}