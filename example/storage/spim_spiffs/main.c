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
 * FilePath: main.c
 * Date: 2022-06-17 08:17:59
 * LastEditTime: 2022-06-17 08:17:59
 * Description:  This file is for the main functions.
 * 
 * Modify History: 
 *  Modify History: 
 *  Ver     Who      Date         Changes
 * -----   ------  --------   --------------------------------------
 * 1.0 liqiaozhong 2022/11/2  first commit
 * 2.0  liyilun     2024/04/25 add no letter shell mode, adapt to auto test system
 */

#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "spim_spiffs_example.h"

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
#include "shell_port.h"
#endif

int main(void)
{
    BaseType_t ret;
#ifdef CONFIG_USE_LETTER_SHELL
    ret = LSUserShellTask() ;
    if(ret != pdPASS)
        goto FAIL_EXIT;
#else

    ret = xTaskCreate((TaskFunction_t)SpimSpiffsExampleEntry,
                    (const char *)"SpimSpiffsExampleEntry",
                    (uint16_t)4096,
                    NULL,
                    (UBaseType_t)2,
                    NULL);
#endif
    vTaskStartScheduler(); /* 启动任务，开启调度 */   
    while (1); /* 正常不会执行到这里 */
    
FAIL_EXIT:
    printf("failed 0x%x \r\n", ret);  
    return 0;
}
