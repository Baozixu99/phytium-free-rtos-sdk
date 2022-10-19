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
 * FilePath: cmd_usb.c
 * Date: 2022-09-19 14:34:44
 * LastEditTime: 2022-09-19 14:34:45
 * Description:  This files is for 
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/9/20  init commit
 */
#include <string.h>
#include <stdio.h>
#include "strto.h"
#include "sdkconfig.h"

#include "FreeRTOS.h"

#include "../src/shell.h"
#include "usb_host.h"

static int USBCmdEntry(int argc, char *argv[])
{
    int ret = 0;

    if (!strcmp(argv[1], "init"))
    {
        ret = FFreeRTOSInitUsb();
    }
    else if (!strcmp(argv[1], "disk"))
    {
        ret = FFreeRTOSRunUsbDisk();
    }
    else if (!strcmp(argv[1], "input"))
    {
        ret = FFreeRTOSRunUsbInput();
    }
    else if (!strcmp(argv[1], "lsusb"))
    {
        ret = FFreeRTOSListUsbDev(argc - 1, &argv[1]);
    }

    return ret;
}
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), usb, USBCmdEntry, test freertos usb driver);