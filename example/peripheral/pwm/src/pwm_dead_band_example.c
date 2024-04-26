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
 * FilePath: pwm_dead_band_example.c
 * Date: 2022-07-11 11:32:48
 * LastEditTime: 2022-07-11 11:32:48
 * Description:  This file is for pwm test example functions.
 *
 * Modify History:
 *  Ver   Who           Date           Changes
 * ----- ------       --------      --------------------------------------
 * 1.0  wangxiaodong  2022/8/24      first release
 * 1.1  zhangyan      2024/4/23      refactoring 
 */
#include <string.h>
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "fpwm_os.h"
#include "fio_mux.h"
#include "pwm_example.h"
#include "fparameters.h"

#define FPWM_DEBUG_TAG "PWM_DEAD_BAND_TEST"
#define FPWM_ERROR(format, ...)   FT_DEBUG_PRINT_E(FPWM_DEBUG_TAG, format, ##__VA_ARGS__)
#define FPWM_WARN(format, ...)    FT_DEBUG_PRINT_W(FPWM_DEBUG_TAG, format, ##__VA_ARGS__)
#define FPWM_INFO(format, ...)    FT_DEBUG_PRINT_I(FPWM_DEBUG_TAG, format, ##__VA_ARGS__)
#define FPWM_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FPWM_DEBUG_TAG, format, ##__VA_ARGS__)

/* pwm pulse change period */
#define PWM_CHANGE_PERIOD           (pdMS_TO_TICKS(500UL))
#define TIMER_OUT                   (pdMS_TO_TICKS(4000UL))
/* pwm pulse amplitude of periodic variation */
#define PWM_PULSE_CHANGE    1000

/* pwm primary config */
#define PWM_DIV             500
#define PWM_PERIOD          10000
#define PWM_PULSE           2000

enum
{
    PWM_TEST_SUCCESS = 0,
    PWM_TEST_UNKNOWN = 1,
    PWM_INIT_FAILED   = 2,
    PWM_CHANGE_FAILED = 3,              
};

static QueueHandle_t xQueue = NULL;
static FFreeRTOSPwm *os_pwm_ctrl_p;

static void PwmDelete(FFreeRTOSPwm *os_pwm_p)
{
    FFreeRTOSPwmEnable(os_pwm_p, PWM_TEST_CHANNEL, FALSE);
    FFreeRTOSPwmDeinit(os_pwm_p);
    FIOMuxDeInit();
}

static FError PwmInit(u32 pwm_id)
{
    FError ret = FPWM_SUCCESS;

    /* init iomux */
    FIOMuxInit();
    FIOPadSetPwmMux(pwm_id, FPWM_CHANNEL_0);
    FIOPadSetPwmMux(pwm_id, FPWM_CHANNEL_1);

    /* init pwm controller */
    os_pwm_ctrl_p = FFreeRTOSPwmInit(pwm_id);
    if (os_pwm_ctrl_p == NULL)
    {
        FPWM_ERROR("FFreeRTOSPwmInit failed.\n");
        ret = FPWM_ERR_INVAL_PARM;
        return ret;
    }

    /* set pwm db config */
    FPwmDbVariableConfig db_cfg;
    memset(&db_cfg, 0, sizeof(db_cfg));
    db_cfg.db_rise_cycle = 800;
    db_cfg.db_fall_cycle = 800;
    db_cfg.db_polarity_sel = FPWM_DB_AH;
    /*将死区输入源设置为当前测试使用的CHANNEL*/
    db_cfg.db_in_mode = PWM_TEST_CHANNEL == FPWM_CHANNEL_0 ? FPWM_DB_IN_MODE_PWM0 : FPWM_DB_IN_MODE_PWM1;
    db_cfg.db_out_mode = FPWM_DB_OUT_MODE_ENABLE_RISE_FALL;
    ret = FFreeRTOSPwmDbSet(os_pwm_ctrl_p, &db_cfg);
    if (FPWM_SUCCESS != ret)
    {
        FPWM_ERROR("FFreeRTOSPwmDbSet failed.\n");
        return ret;
    }
    /* start pwm config */
    FPwmVariableConfig pwm_cfg;
    memset(&pwm_cfg, 0, sizeof(pwm_cfg));
    pwm_cfg.tim_ctrl_mode = FPWM_MODULO;
    pwm_cfg.tim_ctrl_div = PWM_DIV - 1;
    pwm_cfg.pwm_period = PWM_PERIOD;
    pwm_cfg.pwm_pulse = PWM_PULSE;
    pwm_cfg.pwm_mode = FPWM_OUTPUT_COMPARE;
    pwm_cfg.pwm_polarity = FPWM_POLARITY_NORMAL;
    pwm_cfg.pwm_duty_source_mode = FPWM_DUTY_CCR;
    ret = FFreeRTOSPwmSet(os_pwm_ctrl_p, PWM_TEST_CHANNEL, &pwm_cfg);
    if (FPWM_SUCCESS != ret)
    {
        FPWM_ERROR("FFreeRTOSPwmSet failed.\n");
        return ret;
    }

    memset(&db_cfg, 0, sizeof(db_cfg));
    memset(&pwm_cfg, 0, sizeof(pwm_cfg));

    FFreeRTOSPwmDbGet(os_pwm_ctrl_p, &db_cfg);
    printf("FFreeRTOSPwmDbGet:\n");
    printf("db_cfg.db_rise_cycle = %d\n", db_cfg.db_rise_cycle);
    printf("db_cfg.db_fall_cycle = %d\n", db_cfg.db_fall_cycle);
    printf("db_cfg.db_polarity_sel = %d\n", db_cfg.db_polarity_sel);
    printf("db_cfg.db_in_mode = %d\n", db_cfg.db_in_mode);
    printf("db_cfg.db_out_mode = %d\n", db_cfg.db_out_mode);

    FFreeRTOSPwmGet(os_pwm_ctrl_p, PWM_TEST_CHANNEL, &pwm_cfg);
    printf("FPwmVariableGet:\n");
    printf("pwm_cfg.tim_ctrl_mode = %d\n", pwm_cfg.tim_ctrl_mode);
    printf("pwm_cfg.tim_ctrl_div = %d\n", pwm_cfg.tim_ctrl_div);
    printf("pwm_cfg.pwm_period = %d\n", pwm_cfg.pwm_period);
    printf("pwm_cfg.pwm_pulse = %d\n", pwm_cfg.pwm_pulse);
    printf("pwm_cfg.pwm_mode = %d\n", pwm_cfg.pwm_mode);
    printf("pwm_cfg.pwm_polarity = %d\n", pwm_cfg.pwm_polarity);
    printf("pwm_cfg.pwm_duty_source_mode = %d\n", pwm_cfg.pwm_duty_source_mode);

    ret = FFreeRTOSPwmEnable(os_pwm_ctrl_p, PWM_TEST_CHANNEL, TRUE);
    if (FPWM_SUCCESS != ret)
    {
        FPWM_ERROR("FFreeRTOSPwmEnable failed.\n");
        return ret;
    }

    return ret;
}

static FError PwmChange()
{
    FError ret = FPWM_SUCCESS;
    u32 pwm_pulse = PWM_PULSE;
    /* As per most tasks, this task is implemented in an infinite loop. */
    for (int i = 0; i < 2; i++)
    {
        ret = FFreeRTOSPwmPulseSet(os_pwm_ctrl_p, PWM_TEST_CHANNEL, pwm_pulse);
        if (FPWM_SUCCESS != ret)
        {
            FPWM_ERROR("FFreeRTOSPwmPulseSet failed.");
            return ret;
        }
        printf("pwm_pulse: %d\r\n", pwm_pulse);
        pwm_pulse = (pwm_pulse + PWM_PULSE_CHANGE) % PWM_PERIOD;
        vTaskDelay(PWM_CHANGE_PERIOD);
    }

    return ret;
}

static void FFreeRTOSPwmDeadBandTask()
{
    FError ret = FPWM_SUCCESS;
    int task_res = PWM_TEST_SUCCESS;

    ret = PwmInit(PWM_TEST_ID);
    if (ret != FPWM_SUCCESS)
    {
        FPWM_ERROR("PwmInit failed.");
        task_res = PWM_INIT_FAILED;
        goto task_exit;
    }

    ret = PwmChange();
    if (ret != FPWM_SUCCESS)
    {
        FPWM_ERROR("PwmChange failed.");
        task_res = PWM_CHANGE_FAILED;
        goto task_exit;
    }

task_exit:
    xQueueSend(xQueue, &task_res, 0);

    vTaskDelete(NULL);
}

/* create pwm test, id is pwm module number */
BaseType_t FFreeRTOSPwmDeadBandTaskCreate()
{
    BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为 pdPASS */
    int task_res = PWM_TEST_UNKNOWN;

    xQueue = xQueueCreate(1, sizeof(int)); /* 创建消息队列 */
    if (xQueue == NULL)
    {
        FPWM_ERROR("xQueue create failed.");
        goto exit;
    }

    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSPwmDeadBandTask,         /* 任务入口函数 */
                          (const char *)"FFreeRTOSPwmDeadBandTask",         /* 任务名字 */
                          (uint16_t)4096,                         /* 任务栈大小 */
                          NULL,                                   /* 任务入口函数参数 */
                          (UBaseType_t)2, /* 任务优先级 */
                          NULL);                                  /* 任务句柄 */
    if (xReturn == pdFAIL)
    {
        FPWM_ERROR("xTaskCreate FFreeRTOSPwmDeadBandTask failed.");
        goto exit;
    }

    xReturn = xQueueReceive(xQueue, &task_res, TIMER_OUT);
    if (xReturn == pdFAIL)
    {
        FPWM_ERROR("xQueue receive timeout.");
        goto exit;
    }

exit:
    PwmDelete(os_pwm_ctrl_p);
    vQueueDelete(xQueue);
    if (task_res != PWM_TEST_SUCCESS)
    {
        printf("%s@%d: pwm dead band example [failure], task_res = %d\r\n", __func__, __LINE__, task_res);
        return pdFAIL;
    }
    else
    {
        printf("%s@%d: pwm dead band example [success].\r\n", __func__, __LINE__);
        return pdTRUE;
    }
}
