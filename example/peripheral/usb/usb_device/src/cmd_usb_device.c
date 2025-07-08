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
#include "usbd_core.h"

#ifdef CONFIG_USE_LETTER_SHELL
#include "../src/shell.h"
#include "usb_device_mass_storage_example.h"
#include "usb_device_virtual_serial_example.h"
#include "usb_device_virtual_keyboard_example.h"

/* usage info function for pusb2 example */
static void UsbdExampleUsage(void)
{
    printf("Usage:\r\n");
    printf("usbd msc\r\n");
    printf("-- run usbd stack as mass storage device\r\n");
    printf("usbd serial\r\n");
    printf("-- run usbd stack as virtual serial device\r\n");
    printf("usbd keyboard\r\n");
    printf("-- run usbd stack as virtual keyboard device\r\n");
    printf("usbd stop\r\n");
    printf("-- stop usbd stack and destory device\r\n");
}

static int USBCmdEntry(int argc, char *argv[])
{
    int ret = 0;
    u32 usb_id = 0;
    const char *devname;

    if (argc < 2)
    {
        UsbdExampleUsage();
        return -1;
    }

    if (!strcmp(argv[1], "msc"))
    {
        ret = FFreeRTOSCreateMassStorageDevice(usb_id);
    }
    else if (!strcmp(argv[1], "serial"))
    {
        ret = FFreeRTOSCreateVirtualSerialDevice(usb_id);
    }
    else if (!strcmp(argv[1], "keyboard"))
    {
        ret = FFreeRTOSCreateVirtualKeyboardDevice(usb_id);
    }
    else if (!strcmp(argv[1], "stop"))
    {
        ret = usbd_deinitialize(usb_id);
    }

    return ret;
}
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), usbd, USBCmdEntry, test freertos usb driver);
#endif