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
 * FilePath: main.c
 * Date: 2022-02-24 16:56:46
 * LastEditTime: 2022-03-21 17:00:56
 * Description:  This file is for AMP example that running rpmsg_echo_task and open scheduler
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0 huanghe    2022/03/25  first commit
 *  1.1 huanghe    2023/03/09  Adapt OpenAMP routines based on e2000D/Q
 */

#include <stdio.h>
#include "FreeRTOS.h"
#include "ftypes.h"
#include "sdkconfig.h"
#ifdef CONFIG_USE_LETTER_SHELL
#include "shell_port.h"
#include "shell.h"
#else
#include "task.h"
#include "rpmsg_demo_manager.h"

void OpenampExampleTaskEntry(void)
{
    /*Demo read and write by sfud*/
    FFreeRTOSOpenampExample();

    printf("[test_end]\r\n");

    vTaskDelete(NULL);
}
#endif

extern int FOpenampCmdEntry(int argc, char *argv[]) ;

int main(void)
{
    BaseType_t ret = pdPASS; /* 定义一个创建信息返回值，默认为 pdPASS */
#ifdef CONFIG_USE_LETTER_SHELL
    ret = LSUserShellTask();
    if(ret != pdPASS)
        goto FAIL_EXIT;
#else
    taskENTER_CRITICAL(); /* no schedule when create task */
    ret = xTaskCreate((TaskFunction_t )OpenampExampleTaskEntry, /* 任务入口函数 */
                        (const char* )"OpenampExampleTaskEntry",/* 任务名字 */
                        (4096*2), /* 任务栈大小 */
                        (void* )NULL,/* 任务入口函数参数 */
                        (UBaseType_t )4, /* 任务的优先级 */
                        NULL); /* 任务控制块指针 */
    taskEXIT_CRITICAL(); /* allow schedule since task created */
#endif
    vTaskStartScheduler(); /* 启动任务，开启调度 */   
    while (1); /* 正常不会执行到这里 */

FAIL_EXIT:
    printf("failed 0x%x \r\n", ret);
    return 0;
}

