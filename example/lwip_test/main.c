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
 * Date: 2022-02-24 13:42:19
 * LastEditTime: 2022-03-21 17:01:57
 * Description:  This file is for 
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */

#include "parameters.h"
#include "ft_types.h"
#include "FreeRTOS.h"
#include "task.h"
#include "ft_os_gmac.h"

#define OS_GMAC0_NAME "gmac0"

extern BaseType_t TestCpuStatsEntry();
extern void LwipRawTest(FtOsGmac *Os_Gmac);

FtOsGmac os_gmac = {0};

int main()
{
    BaseType_t ret = pdPASS;

    FtOsGmacConfig os_config = {
        .gmac_instance = FT_OS_GMAC0_ID,
        .isr_priority = 0, /* irq Priority */
        .address = {{192,
                     168,
                     4,
                     20},
                    {255, 255, 255, 0},
                    {192, 168, 4, 1}},
        .mac_input_thread = {
            .thread_name = OS_GMAC0_NAME,
            .stack_depth = 4096, /* The number of words the stack */
            .priority = configMAX_PRIORITIES-1, /* Defines the priority at which the task will execute. */
            .thread_handle = NULL,
        }, /* Gmac input thread */
        
    };
    
    /* !!! make sure eth in-place before init gmac */
    FtOsGmacObjectInit(&os_gmac, &os_config);
    
    LwipRawTest(&os_gmac);
    
    ret = TestCpuStatsEntry();
   
    if (pdPASS != ret)
    {
        goto FAIL_EXIT;
    }
    
    vTaskStartScheduler(); /* 启动任务，开启调度 */   
    while (1); /* 正常不会执行到这里 */
    
FAIL_EXIT:
    printf("failed 0x%x \r\n", ret);  

    return 0;
}