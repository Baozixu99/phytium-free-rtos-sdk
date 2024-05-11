/*
 * Copyright : (C) 2023 Phytium Information Technology, Inc.
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
 * FilePath: main.c
 * Created Date: 2023-11-20 18:26:15
 * Last Modified: 2024-04-28 11:47:17
 * Description:  This file is for lwip startup example main functions.
 * 
 * Modify History:
 *  Ver      Who         Date               Changes
 * -----  ----------   --------  ---------------------------------
 *  1.0   liuzhihong   2023/12/26          first release
 *  2.0   liuzhihong   2024/4/26   add no letter shell mode, adapt to auto-test system
 */

#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "lwip/tcpip.h"
#include "sdkconfig.h"
#ifdef CONFIG_USE_LETTER_SHELL
#include "shell_port.h"
#else
#include "lwip_dhcp_example.h"
#include "lwip_ipv4_example.h"
#include "lwip_ipv6_example.h"

#define LWIP_STARTUP_EXAMPLE_TASK_PRIORITY 2

void LwipStartupExampleTaskEntry()
{
    /* example functions */
    FFreeRTOSLwipIpv4TaskCreate();
    LwipIpv4TestDeinit();
    
    FFreeRTOSLwipIpv6TaskCreate();
    LwipIpv6TestDeinit();

    FFreeRTOSLwipDhcpTaskCreate();
    LwipDhcpTestDeinit();

    /* end flag */
    printf("[test_end]\r\n");
    vTaskDelete(NULL);
}
#endif

int main(void)
{
    BaseType_t ret = pdPASS; /* 定义一个创建信息返回值，默认为 pdPASS */

#ifdef CONFIG_USE_LETTER_SHELL
    ret = LSUserShellTask();
#else
    /* used in no-letter-shell mode */
    ret = xTaskCreate((TaskFunction_t)LwipStartupExampleTaskEntry,    /* 任务入口函数 */
                      (const char *)"LwipStartupExampleTaskEntry",    /* 任务名字 */
                      (uint16_t)4096,                          /* 任务栈大小 */
                      NULL,                                    /* 任务入口函数参数 */
                      (UBaseType_t)LWIP_STARTUP_EXAMPLE_TASK_PRIORITY, /* 任务优先级 */
                      NULL);                                   /* 任务句柄 */
#endif
    if (ret != pdPASS)
    {
        goto FAIL_EXIT;
    }
    
    tcpip_init(NULL, NULL);
    /* 启动任务，开启调度 */
    vTaskStartScheduler(); 
    while (1); /* 正常不会执行到这里 */

FAIL_EXIT:
    printf("Failed,the ret value is 0x%x. \r\n", ret);
    return -2;
}