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
 * Description:   This file is for cherry-usb test main entry.
 *
 * Modify History:
 *  Ver   Who         Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/10/19   init commit
 * 2.0   zhugengyu  2024/7/26    support auto-run
 */

#include <stdio.h>
#include "FreeRTOS.h"
#include "fparameters.h"

#include "pusb2_mass_storage_example.h"
#include "pusb2_virtual_serial_example.h"

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
#include "shell_port.h"
#endif

int main(void)
{
    BaseType_t ret;

#ifdef CONFIG_USE_LETTER_SHELL
    ret = LSUserShellTask();
    if (ret != pdPASS)
    {
        goto FAIL_EXIT;
    }
#else
    ret = FPUsb2VirtualSerialExample(FUSB2_ID_VHUB_0);
#endif
    vTaskStartScheduler(); /* 启动任务，开启调度 */
    while (1); /* 正常不会执行到这里 */

FAIL_EXIT:
    printf("Failed,the ret value is 0x%x. \r\n", ret);
    return 0;
}
