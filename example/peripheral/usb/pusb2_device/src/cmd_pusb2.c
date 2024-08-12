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
 * Description:  This file is for USB shell command.
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
#include "pusb2_mass_storage_example.h"
#include "pusb2_virtual_serial_example.h"

/* usage info function for pusb2 example */
static void FPUsb2ExampleUsage(void)
{
    printf("Usage:\r\n");
    printf("pusb2 mass_storage\r\n");
    printf("-- run pusb2 massage storage example\r\n");
    printf("pusb2 virtual_serial\r\n");
    printf("-- run pusb2 virtual serial example\r\n");
}

static int USBCmdEntry(int argc, char *argv[])
{
    int ret = 0;
    u32 usb_id = 0;
    const char *devname;

    if (argc < 2)
    {
        FPUsb2ExampleUsage();
        return -1;
    }

    if (!strcmp(argv[1], "mass_storage"))
    {
        ret = FPUsb2MassStorageExample(usb_id);
    }
    else if (!strcmp(argv[1], "virtual_serial"))
    {
        ret = FPUsb2VirtualSerialExample(usb_id);
    }

    return ret;
}
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), pusb2, USBCmdEntry, test freertos usb driver);