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

#include "usbh_core.h"

#ifdef CONFIG_USE_LETTER_SHELL
#include "../src/shell.h"
#include "xhci_host_example.h"
#endif

#ifdef CONFIG_USE_LETTER_SHELL
BaseType_t FFreeRTOSInitUsb(u32 id);
BaseType_t FFreeRTOSDeInitUsb(u32 id);
BaseType_t FFreeRTOSListUsbDev(int argc, char *argv[]);

static void USBCmdUsage(void)
{
    printf("Usage:\r\n");
    printf("usb start <id>\r\n");
    printf("-- Start usb bus and enumrate devices\r\n");
    printf("usb stop <id>\r\n");
    printf("-- Stop usb bus and deattach devices\r\n");
    printf("usb lsusb\r\n");
    printf("-- List all attached devices on usb bus\r\n");      
    printf("usb disk <id> <dev>\r\n");
    printf("-- Read and write usb disk device\r\n");
    printf("usb diskbench <id> <dev>\r\n");
    printf("-- Bench read and write usb disk device\r\n");
    printf("usb keyboard <id> <dev>\r\n");
    printf("-- Get usb keyboard input\r\n");
    printf("usb mouse <id> <dev>\r\n");
    printf("-- Get usb mouse input\r\n");
}

static int USBCmdEntry(int argc, char *argv[])
{
    int ret = 0;
    u32 usb_id = 0;
    const char *devname;

    if (!strcmp(argv[1], "start"))
    {
        if (argc < 2) 
        {
            usb_id = 0;
        }
        else
        {
            usb_id = (uint8_t)simple_strtoul(argv[2], NULL, 10);
        }

        ret = usbh_initialize(usb_id, usb_hc_get_register_base(usb_id));
    }
    else if (!strcmp(argv[1], "stop"))
    {
        if (argc < 2) 
        {
            usb_id = 0;
        }
        else
        {
            usb_id = (uint8_t)simple_strtoul(argv[2], NULL, 10);
        }

        ret = usbh_deinitialize(usb_id);
    }
    else if (!strcmp(argv[1], "lsusb"))
    {
        ret = lsusb(argc - 1, &argv[1]);
    }
    else if (!strcmp(argv[1], "disk"))
    {
        if (argc < 2) 
        {
            usb_id = 0;
        }
        else
        {
            usb_id = (uint8_t)simple_strtoul(argv[2], NULL, 10);
        }
        
        if (argc < 3)
        {
            devname = "/dev/sda";
        }
        else
        {
            devname = argv[3];
        }

        ret = FFreeRTOSRunXhciDisk(usb_id, devname);
    }
    else if (!strcmp(argv[1], "diskbench"))
    {
        if (argc < 2) 
        {
            usb_id = 0;
        }
        else
        {
            usb_id = (uint8_t)simple_strtoul(argv[2], NULL, 10);
        }
        
        if (argc < 3)
        {
            devname = "/dev/sda";
        }
        else
        {
            devname = argv[3];
        }

        ret = FFreeRTOSRunXhciDiskBench(usb_id, devname);
    }
    else if (!strcmp(argv[1], "keyboard"))
    {
        if (argc < 2) 
        {
            usb_id = 0;
        }
        else
        {
            usb_id = (uint8_t)simple_strtoul(argv[2], NULL, 10);
        }
        
        if (argc < 3)
        {
            devname = "/dev/input0";
        }
        else
        {
            devname = argv[3];
        }

        ret = FFreeRTOSRunXhciKeyboard(usb_id, devname);
    }
    else if (!strcmp(argv[1], "mouse"))
    {
        if (argc < 2) 
        {
            usb_id = 0;
        }
        else
        {
            usb_id = (uint8_t)simple_strtoul(argv[2], NULL, 10);
        }
        
        if (argc < 3)
        {
            devname = "/dev/input0";
        }
        else
        {
            devname = argv[3];
        }

        ret = FFreeRTOSRunXhciMouse(usb_id, devname);
    }

    return ret;
}
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), usb, USBCmdEntry, test freertos usb driver);
#endif