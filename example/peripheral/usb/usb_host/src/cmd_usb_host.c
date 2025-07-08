/*
 * Copyright (C) 2022, Phytium Technology Co., Ltd.   All Rights Reserved.
 *
 * Licensed under the BSD 3-Clause License (the "License"); you may not use
 * this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *
 * FilePath: cmd_usb_host.c
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
#include "usb_host_probe_devices_example.h"
#include "usb_host_mass_storage_read_write_example.h"
#include "usb_host_mass_storage_bench_example.h"
#include "usb_host_mouse_input_example.h"
#include "usb_host_keyboard_input_example.h"

static void USBCmdUsage(void)
{
    printf("Usage:\r\n");
    printf("usbh start [bus-id]\r\n");
    printf("-- Start usb bus and enumrate devices\r\n");
    printf("usbh list [bus-id] -t\r\n");
    printf("-- Stop usb bus and de-attach devices\r\n");
    printf("usbh mscrw [bus-id] [dev-name]\r\n");
    printf("-- Read and write mass storage devices\r\n");
    printf("usbh mscbench [bus-id] [dev-name]\r\n");
    printf("-- Bench read and write mass storage devices\r\n");
    printf("usbh mouse [bus-id] [dev-name]\r\n");
    printf("-- Get inputs from mouse devices\r\n");
    printf("usbh kbd [bus-id] [dev-name]\r\n");
    printf("-- Get inputs from keyboard devices\r\n");
    printf("usbh stop [bus-id]\r\n");
    printf("-- Stop usb bus and de-attach devices\r\n");
}

static int USBCmdEntry(int argc, char *argv[])
{
    int ret = 0;
    u8 usb_id = 0;
    const char *devname;

    if (!strcmp(argv[1], "start"))
    {
        if (argc > 2)
        {
            usb_id = (uint8_t)simple_strtoul(argv[2], NULL, 10);
        }

        ret = FFreeRTOSListAllUsbDevices(usb_id);
    }
    else if (!strcmp(argv[1], "mscrw"))
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

        ret = FFreeRTOSReadWriteMassStorage(usb_id, devname);
    }
    else if (!strcmp(argv[1], "mscbench"))
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

        ret = FFreeRTOSBenchMassStorage(usb_id, devname);
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

        ret = FFreeRTOSMouseInput(usb_id, devname);
    }
    else if (!strcmp(argv[1], "kbd"))
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

        ret = FFreeRTOSKeyboardInput(usb_id, devname);
    }
    else if (!strcmp(argv[1], "list"))
    {
        ret = lsusb(argc - 1, &argv[1]);
    }
    else if (!strcmp(argv[1], "stop"))
    {
        ret = usbh_deinitialize(usb_id);
    }

    return ret;
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), usbh, USBCmdEntry, test freertos usb driver);
#endif