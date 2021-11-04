/*
 * @ : Copyright (c) 2021 Phytium Information Technology, Inc. 
 *  
 * SPDX-License-Identifier: Apache-2.0.
 * 
 * @Date: 2021-07-05 10:28:22
 * @LastEditTime: 2021-07-22 09:05:53
 * @Description:  This files is for 
 * 
 * @Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "math.h"

/* ---------------------Test 1: Backported Timer functions-----------------------
 * Test xTimerCreateStatic(), vTimerSetTimerId(), xTimerGetPeriod(), xTimerGetExpiryTime()
 *
 * This test creates a one-shot static timer, sets/checks the timer's id and period. Then ensures
 * the timer cb is executed in a timely fashion.
 */

#define TMR_PERIOD_TICKS  1000
#define TIMER_ID    0xFF
#define TICK_DELTA  5

static TaskHandle_t appTaskCreateHand = NULL;
static StaticTimer_t timer_buffer;
static TickType_t tmr_ideal_exp;

static void TimerHandler(TimerHandle_t xtimer)
{
    static float expected = 0.5f;
    /*Check cb is called in timely fashion*/
    printf("isr xTaskGetTickCount()=%d\n", xTaskGetTickCount());
    
    expected = expected * 1.002f * cosf(0.0f);
    
    printf("isr expected=%f\n", expected);
}

void TestFloatInIsr(void) 
{
    /*进入临界区*/
    taskENTER_CRITICAL(); 

    TimerHandle_t tmr_handle = xTimerCreateStatic("static_tmr", TMR_PERIOD_TICKS, pdTRUE, NULL, TimerHandler, &timer_buffer);
    
    printf("xTimerGetPeriod=%d\n",xTimerGetPeriod(tmr_handle));      

    vTimerSetTimerID(tmr_handle, (void *)TIMER_ID);

    /*Start Timer*/
    xTimerStart(tmr_handle, 1);      

    /*Calculate ideal expiration time*/
    tmr_ideal_exp = xTaskGetTickCount() + TMR_PERIOD_TICKS;     

    /*Need to yield to allow daemon task to process start command, or else expiration time will be NULL*/
    vTaskDelay(2);  

    printf("xTimerGetExpiryTime(tmr_handle)=%d\n", xTimerGetExpiryTime(tmr_handle)); 
    
    vTaskDelete(appTaskCreateHand); 
     
    taskEXIT_CRITICAL(); 
    
} 

BaseType_t TestFloatIsrEntry(void)
{
    BaseType_t ret = pdPASS;/* 定义一个创建信息返回值，默认为 pdPASS */
    
    ret = xTaskCreate((TaskFunction_t )TestFloatInIsr, /* 任务入口函数 */
                            (const char* )"FloatTaskCreate",/* 任务名字 */
                            (uint16_t )512, /* 任务栈大小 */
                            (void* )NULL,/* 任务入口函数参数 */
                            (UBaseType_t )1, /* 任务的优先级 */
                            (TaskHandle_t* )&appTaskCreateHand); /* 任务控制 */
                
    return ret;
}