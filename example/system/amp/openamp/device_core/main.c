/*
 * Copyright : (C) 2024 Phytium Information Technology, Inc.
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
 * Created Date: 2022-02-25 13:25:14
 * Last Modified: 2024-07-19 14:47:05
 * Description:  This file is for main
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  huanghe    2022/03/25  first commit
 *  1.1  liusm      2024/03/05  update openamp for freertos
 */


#include "ftypes.h"
#include "fpsci.h"
#include "fsleep.h"
#include "fprintk.h"
#include "fdebug.h"
#include "FreeRTOS.h"
#include "task.h"
#include "rpmsg-demo-listening.h"

#define OPENAMP_MAIN_DEBUG_TAG "OPENAMP_MAIN"
#define OPENAMP_MAIN_DEBUG_I(format, ...) FT_DEBUG_PRINT_I(OPENAMP_MAIN_DEBUG_TAG, format, ##__VA_ARGS__)
#define OPENAMP_MAIN_DEBUG_W(format, ...) FT_DEBUG_PRINT_W(OPENAMP_MAIN_DEBUG_TAG, format, ##__VA_ARGS__)
#define OPENAMP_MAIN_DEBUG_E(format, ...) FT_DEBUG_PRINT_E(OPENAMP_MAIN_DEBUG_TAG, format, ##__VA_ARGS__)

int main(void)
{
    BaseType_t ret;
    ret = rpmsg_listening_func();
    if(ret != pdPASS)
        goto FAIL_EXIT;
    OPENAMP_MAIN_DEBUG_I("Create task OK!");
    /* 开启任务调度前确保主核不会提前发送消息过来，否则会导致IPI模式中断被屏蔽，无法接收到消息中断 */
    vTaskStartScheduler(); /* 启动任务，开启调度 */   
    while (1); /* 正常不会执行到这里 */

FAIL_EXIT:
    printf("failed 0x%x \r\n", ret);  
    return 0;
}


