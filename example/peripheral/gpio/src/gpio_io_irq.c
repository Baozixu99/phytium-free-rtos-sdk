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
 * FilePath: gpio_io_irq.c
 * Date: 2022-07-22 13:57:42
 * LastEditTime: 2022-07-22 13:57:43
 * Description:  This file is for gpio io irq implementation.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  zhugengyu  2022/8/26    init commit
 *  2.0  wangzq     2024/4/22     add no letter shell mode, adapt to auto-test system
 */
/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include "fdebug.h"
#include "fio_mux.h"

#include "fgpio_os.h"
#include "gpio_io_irq.h"
/************************** Constant Definitions *****************************/
#if defined(CONFIG_E2000D_DEMO_BOARD) || defined(CONFIG_E2000Q_DEMO_BOARD)
    #define IN_PIN_INDEX FFREERTOS_GPIO_PIN_INDEX(4, 0, 11) /* GPIO 4-A-11 */
    #define OUT_PIN_INDEX FFREERTOS_GPIO_PIN_INDEX(4, 0, 12)
#endif

#ifdef CONFIG_FIREFLY_DEMO_BOARD
    #define IN_PIN_INDEX FFREERTOS_GPIO_PIN_INDEX(1, 0, 12)
    #define OUT_PIN_INDEX FFREERTOS_GPIO_PIN_INDEX(1, 0, 11)
#endif

#define PIN_IRQ_OCCURED     BIT(0)
#define GPIO_IRQ_TEST_TASK_PRIORITY  3
#define TIMER_OUT                  (pdMS_TO_TICKS(4000UL))
/**************************** Type Definitions *******************************/
enum
{
    GPIO_IRQ_TEST_SUCCESS = 0,
    GPIO_IRQ_TEST_UNKNOWN = 1,
    GPIO_INIT_ERROR   = 2,
    GPIO_IRQ_TEST_ERROR = 3,
};
/************************** Variable Definitions *****************************/
static FFreeRTOSFGpio *in_gpio = NULL;
static FFreeRTOSGpioConfig in_gpio_cfg;
static FFreeRTOSFGpio *out_gpio = NULL;
static FFreeRTOSGpioConfig out_gpio_cfg;

static QueueHandle_t xQueue = NULL;
static EventGroupHandle_t event = NULL;
static boolean is_running = FALSE;
/***************** Macros (Inline Functions) Definitions *********************/
#define FGPIO_DEBUG_TAG "GPIO-IRQ"
#define FGPIO_ERROR(format, ...) FT_DEBUG_PRINT_E(FGPIO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGPIO_WARN(format, ...)  FT_DEBUG_PRINT_W(FGPIO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGPIO_INFO(format, ...)  FT_DEBUG_PRINT_I(FGPIO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGPIO_DEBUG(format, ...) FT_DEBUG_PRINT_D(FGPIO_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/*exit the gpio irq example task and deinit the gpio */
static void GpioIOIrqExit(void)
{
    printf("Exiting...\r\n");
    /* deinit iomux */
    FIOMuxDeInit();
    if (FT_SUCCESS != FFreeRTOSGpioDeInit(in_gpio))
    {
        FGPIO_ERROR("Delete gpio failed.");
    }
    in_gpio = NULL;

    if (FFREERTOS_GPIO_PIN_CTRL_ID(OUT_PIN_INDEX) != FFREERTOS_GPIO_PIN_CTRL_ID(IN_PIN_INDEX)) /* check if pin in diff ctrl */
    {
        if (FT_SUCCESS != FFreeRTOSGpioDeInit(out_gpio))
        {
            FGPIO_ERROR("Delete gpio failed.");
        }
    }
    out_gpio = NULL;

    if (event)
    {
        vEventGroupDelete(event);
        event = NULL;
    }

    is_running = FALSE;
}

/*gpio irq callback function*/
static void GpioIOAckPinIrq(s32 vector, void *param)
{
    BaseType_t xhigher_priority_task_woken = pdFALSE;
    BaseType_t x_result = pdFALSE;
    x_result = xEventGroupSetBitsFromISR(event, PIN_IRQ_OCCURED,
                                         &xhigher_priority_task_woken);
    printf("Ack pin irq\r\n");
    return;
}   

/*gpio init function*/
static FError GpioInit(void)
{
    FError ret = FT_SUCCESS;
    FFreeRTOSGpioPinConfig in_pin_config;
    memset(&in_pin_config, 0, sizeof(in_pin_config));
    /* init iomux fuction */
    FIOMuxInit();
    /* init output/input pin */
    FIOPadSetGpioMux(FFREERTOS_GPIO_PIN_CTRL_ID(OUT_PIN_INDEX), FFREERTOS_GPIO_PIN_ID(OUT_PIN_INDEX)); /* set io pad */
    FIOPadSetGpioMux(FFREERTOS_GPIO_PIN_CTRL_ID(IN_PIN_INDEX), FFREERTOS_GPIO_PIN_ID(IN_PIN_INDEX)); /* set io pad */

    out_gpio = FFreeRTOSGpioInit(FFREERTOS_GPIO_PIN_CTRL_ID(OUT_PIN_INDEX), &out_gpio_cfg);
    if (FFREERTOS_GPIO_PIN_CTRL_ID(OUT_PIN_INDEX) != FFREERTOS_GPIO_PIN_CTRL_ID(IN_PIN_INDEX))
    {
        in_gpio = FFreeRTOSGpioInit(FFREERTOS_GPIO_PIN_CTRL_ID(IN_PIN_INDEX), &in_gpio_cfg);
    }
    else
    {
        in_gpio = out_gpio; /* no need to init if in-pin and out-pin under same ctrl */
    }

    in_pin_config.pin_idx = IN_PIN_INDEX;
    in_pin_config.mode = FGPIO_DIR_INPUT;
    in_pin_config.en_irq = TRUE;
    in_pin_config.irq_type = FGPIO_IRQ_TYPE_EDGE_RISING;
    in_pin_config.irq_handler = GpioIOAckPinIrq;
    in_pin_config.irq_args = NULL;

    ret = FFreeRTOSSetupPin(in_gpio, &in_pin_config);/*set the input pin config*/
    if (ret != FT_SUCCESS)
    {
        FGPIO_ERROR("Init input gpio pin failed.");
        goto exit;
    }

exit:
    return ret;
}

/*set output pin high level and toggle it*/
static FError GpioIrqSetOutput(void)
{
    FError ret = FT_SUCCESS;
    const TickType_t toggle_delay = pdMS_TO_TICKS(500UL); /* toggle every 500 ms */
    FFreeRTOSGpioPinConfig out_pin_config;
    memset(&out_pin_config, 0, sizeof(out_pin_config));

    out_pin_config.pin_idx = OUT_PIN_INDEX;
    out_pin_config.mode = FGPIO_DIR_OUTPUT;
    out_pin_config.en_irq = FALSE;

    ret = FFreeRTOSSetupPin(out_gpio, &out_pin_config);/*set the output pin config*/

    if (ret != FT_SUCCESS)
    {
        FGPIO_ERROR("Init output gpio pin failed.");
        goto exit;
    }

    ret = FFreeRTOSPinWrite(out_gpio, OUT_PIN_INDEX, FGPIO_PIN_LOW); /* start with low level */
    if (ret != FT_SUCCESS)
    {
        FGPIO_ERROR("GpioSetOutput failed.");
        goto exit;
    }
    vTaskDelay(toggle_delay);

    ret = FFreeRTOSPinWrite(out_gpio, OUT_PIN_INDEX, FGPIO_PIN_HIGH); /* toggle */
    if (ret != FT_SUCCESS)
    {
        FGPIO_ERROR("GpioSetOutput toggle failed.");
        goto exit;
    }

exit:
    return ret;
}

/*wait for the input pin irq to occur*/
static FError GpioIOWaitIrqOccurr(void)
{
    const TickType_t wait_delay = pdMS_TO_TICKS(2000U); /* just check 2sec wait */
    FError ret = TRUE;
    EventBits_t ev = xEventGroupWaitBits(event,
                                         PIN_IRQ_OCCURED,
                                         pdTRUE, pdFALSE, wait_delay);

    if ((ev & PIN_IRQ_OCCURED))
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

/*gpio irq test task*/
static void GpioIrqTestTask(void)
{
    int task_res = GPIO_IRQ_TEST_SUCCESS;
    FError ret = FT_SUCCESS;
    ret = GpioInit();
    if (ret != FT_SUCCESS)
    {
        FGPIO_ERROR("GpioInitFunction failed.");
        task_res = GPIO_INIT_ERROR;
        goto task_exit;
    }
    ret = GpioIrqSetOutput();
    if (ret != FT_SUCCESS)
    {
        FGPIO_ERROR("GpioInitFunction failed.");
        task_res = GPIO_INIT_ERROR;
        goto task_exit;
    }
    ret = GpioIOWaitIrqOccurr();
    if (ret != FT_SUCCESS)
    {
        FGPIO_ERROR("GpioIOWaitIrqOccurr failed.");
        task_res = GPIO_IRQ_TEST_ERROR;
        goto task_exit;
    }

task_exit:
    xQueueSend(xQueue, &task_res, 0);
    vTaskDelete(NULL);
}

/*gpio irq example entry function*/
BaseType_t FFreeRTOSRunGpioIrq(void)
{
    BaseType_t xReturn = pdPASS; /*  pdPASS */
    int task_res = GPIO_IRQ_TEST_UNKNOWN;
    if (is_running)
    {
        FGPIO_ERROR("The task is running.");
        return pdPASS;
    }

    is_running = TRUE;

    FASSERT_MSG(NULL == event, "Event group exists.");
    FASSERT_MSG((event = xEventGroupCreate()) != NULL, "Create event group failed.");
    xQueue = xQueueCreate(1, sizeof(int)); /* create queue for task communication */
    if (xQueue == NULL)
    {
        FGPIO_ERROR("xQueue create failed.");
        goto exit;
    }

    xReturn = xTaskCreate((TaskFunction_t)GpioIrqTestTask,         /* 任务入口函数 */
                          (const char *)"GpioIrqTestTask",         /* 任务名字 */
                          (uint16_t)4096,                         /* 任务栈大小 */
                          NULL,                                   /* 任务入口函数参数 */
                          (UBaseType_t)GPIO_IRQ_TEST_TASK_PRIORITY, /* 任务优先级 */
                          NULL);                                  /* 任务句柄 */
    if (xReturn == pdFAIL)
    {
        FGPIO_ERROR("xTaskCreate GpioIrqTask failed.");
        goto exit;
    }
    xReturn = xQueueReceive(xQueue, &task_res, TIMER_OUT);
    if (xReturn == pdFAIL)
    {
        FGPIO_ERROR("xQueue receive timeout.");
        goto exit;
    }

exit:
    if (xQueue != NULL)
    {
        vQueueDelete(xQueue);
    }
    printf("task_res = %d\r\n", task_res);
    GpioIOIrqExit();
    if (task_res != GPIO_IRQ_TEST_SUCCESS)
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
