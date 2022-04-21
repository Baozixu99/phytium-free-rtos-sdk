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
 * FilePath: wdt_isr.c
 * Date: 2022-02-24 13:42:19
 * LastEditTime: 2022-03-21 17:01:23
 * Description:  This file is for 
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */

#include <stdio.h>
#include <fsleep.h>
#include <generic_timer.h>
#include "interrupt.h"
#include "gicv3.h"
#include "parameters.h"
#include "fwdt.h"
#include "fwdt_hw.h"
#include "FreeRTOS.h"
#include "task.h"
#include "ft_types.h"
#include "math.h"

FWdtCtrl wdt_ctrl;

/* wdt num: 0 or 1 */
u8 wdt_id = 0;

static TaskHandle_t wdtTaskCreateHandle = NULL;

/**
 * @name: WdtInterrupt
 * @msg:  This function handle wdt timeout interrupt, use it to refresh wdt.
 * @return {void} 
 * @param  {s32} vector, the interrupt number
 * @param  {void} *param, pointer to a WdtCtrl structure that contains
  *                the configuration information for the specified wdt module.
 */
void FWdtInterrupt(s32 vector, void *param)
{
    FWdtRefresh((FWdtCtrl *)param);

    static float expected = 0.5f;

    expected = expected * 1.1f * cosf(0.0f);

    printf("wdt isr expected=%f\n", expected);
}

/**
 * @name: WdtRefreshTest
 * @msg:  Set wdt interrupt to refresh wdt, set timeout value, start wdt.
 * @return {void} 
 * @param {WdtCtrl} *pctrl, pointer to a WdtCtrl structure that contains
  *                the configuration information for the specified wdt module.
 */

void FWdtRefreshTest(FWdtCtrl *pctrl)
{
    FWdtConfig *pconfig = &pctrl->config;

    /* interrupt init */
    InterruptSetPriority(pconfig->irq_num, pconfig->irq_prority);
    InterruptInstall(pconfig->irq_num, FWdtInterrupt, (void*)pctrl, pconfig->instance_name);
    InterruptUmask(pconfig->irq_num);

    FWdtSetTimeout(pctrl, 2);

    FWdtStart(pctrl);
   
}

/**
 * @name: WdtNoRefreshTest
 * @msg:  Set wdt timeout value, start wdt, no refresh.
 * @return {void} 
 * @param {WdtCtrl} *pctrl,  pointer to a WdtCtrl structure that contains
  *                the configuration information for the specified wdt module.
 */
void WdtTaskCreate(void)
{
    FWdtCtrl *pctrl = &wdt_ctrl;
   
    pctrl->config = *FWdtLookupConfig(wdt_id);

    FWdtRefreshTest(pctrl);

    while (1)
    {
        vTaskDelay(100);
    }
}

BaseType_t TestWdtIsrFloatEntry(void)
{
    u32 count = 0;
    printf("ft wdt isr float test.\n");

    BaseType_t ret = pdPASS;/* 定义一个创建信息返回值，默认为 pdPASS */

    taskENTER_CRITICAL(); //进入临界区

    ret = xTaskCreate((TaskFunction_t )WdtTaskCreate, /* 任务入口函数 */
                            (const char* )"WdtTaskCreate",/* 任务名字 */
                            (uint16_t )512, /* 任务栈大小 */
                            (void* )NULL,/* 任务入口函数参数 */
                            (UBaseType_t )1, /* 任务的优先级 */
                            (TaskHandle_t* )&wdtTaskCreateHandle); /* 任务控制 */

    taskEXIT_CRITICAL(); //退出临界区
                            
    return ret;
    
}
