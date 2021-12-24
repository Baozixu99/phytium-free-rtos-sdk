/*
 * @ : Copyright (c) 2021 Phytium Information Technology, Inc. 
 *  
 * SPDX-License-Identifier: Apache-2.0.
 * 
 * @Date: 2021-07-09 08:08:39
 * @LastEditTime: 2021-12-15 16:57:31
 * @Description:  This files is for 
 * 
 * @Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"

extern BaseType_t TestCpuStatsEntry();
extern BaseType_t TestMsgQueueEntry();
int main()
{
    BaseType_t xReturn = pdPASS;
    printf("main is start \r\n") ;
    xReturn = TestCpuStatsEntry();
    if (pdPASS != xReturn)
    {
        goto FAIL_EXIT;
    }

    xReturn = TestMsgQueueEntry();
    if (pdPASS != xReturn)
    {
        goto FAIL_EXIT;
    }

    printf("vTaskStartScheduler \r\n") ;
    vTaskStartScheduler(); /* 启动任务，开启调度 */   
    while (1); /* 正常不会执行到这里 */

FAIL_EXIT:
    printf("failed 0x%x \r\n", xReturn);  
    return 0;
}