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
 * FilePath: usb_host_probe_devices_example.c
 * Date: 2022-09-23 08:24:09
 * LastEditTime: 2022-09-23 08:24:10
 * Description:  This file is for the usb disk functions.
 *
 * Modify History:
 *  Ver   Who         Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/10/19   init commit
 * 2.0   zhugengyu  2024/7/1     support auto-run
 */
/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "fassert.h"
#include "fdebug.h"
#include "fsleep.h"

#include "usbh_core.h"
#include "usb_host_probe_devices_example.h"
/************************** Constant Definitions *****************************/
#define ENMU_TIMER_OUT_MS                 (10000UL)
#define TASK_TIMER_OUT_TICK               (pdMS_TO_TICKS(15000UL))

/**************************** Type Definitions *******************************/
enum
{
    USB_HOST_OP_SUCCESS = 0,
    USB_HOST_OP_FAILED    = 1,
};

/************************** Variable Definitions *****************************/
static uint8_t usb_id = 0U;
static QueueHandle_t xQueue = NULL;

/***************** Macros (Inline Functions) Definitions *********************/
#define FUSB_DEBUG_TAG "USB-LIST"
#define FUSB_ERROR(format, ...) FT_DEBUG_PRINT_E(FUSB_DEBUG_TAG, format, ##__VA_ARGS__)
#define FUSB_WARN(format, ...)  FT_DEBUG_PRINT_W(FUSB_DEBUG_TAG, format, ##__VA_ARGS__)
#define FUSB_INFO(format, ...)  FT_DEBUG_PRINT_I(FUSB_DEBUG_TAG, format, ##__VA_ARGS__)
#define FUSB_DEBUG(format, ...) FT_DEBUG_PRINT_D(FUSB_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/
unsigned long usb_hc_get_register_base(uint32_t id);

/*****************************************************************************/
static void UsbListTask(void *args)
{
    int ret;
    int argc;
    u8 id = usb_id;
    char id_buf[5];

    ret = usbh_initialize(usb_id, usb_hc_get_register_base(usb_id));
    if (ret) {
        FUSB_ERROR("init usb-%d@0x%x failed !!!", usb_id, usb_hc_get_register_base(usb_id));
        goto task_exit;
    }

    printf("waiting %dms for usb enumration...\r\n", ENMU_TIMER_OUT_MS);
    fsleep_millisec(ENMU_TIMER_OUT_MS);

task_exit:
    ret = (ret == 0) ? USB_HOST_OP_SUCCESS : USB_HOST_OP_FAILED;
    xQueueSend(xQueue, &ret, 0);
    vTaskDelete(NULL);
}

BaseType_t FFreeRTOSListAllUsbDevices(u8 id)
{
    BaseType_t ret = pdPASS;
    int task_res = USB_HOST_OP_SUCCESS;
    xQueue = xQueueCreate(1, sizeof(int));
    if (xQueue == NULL)
    {
        FUSB_ERROR("xQueue create failed.\r\n");
        goto exit;
    }

    usb_id = id;

    ret = xTaskCreate((TaskFunction_t)UsbListTask,
                      (const char *)"UsbListTask",
                      (uint16_t)4096,
                      (void *)NULL,
                      (UBaseType_t)configMAX_PRIORITIES - 1,
                      NULL);
    if (ret != pdPASS)
    {
        FUSB_ERROR("xTaskCreate create failed.\r\n");
        goto exit;        
    }

    ret = xQueueReceive(xQueue, &task_res, TASK_TIMER_OUT_TICK);
    if (ret != pdPASS)
    {
        FUSB_ERROR("xQueue receive timeout.\r\n");
        goto exit;        
    }

exit:
    if (xQueue)
    {
        vQueueDelete(xQueue);
    }

    if (task_res != USB_HOST_OP_SUCCESS)
    {
        printf("%s@%d: USB probe device example [failure], task_res = %d\r\n", __func__, __LINE__, task_res);
        return pdFAIL;
    }
    else
    {
        printf("%s@%d: USB probe device example [success].\r\n", __func__, __LINE__);
        return pdTRUE;
    }
}