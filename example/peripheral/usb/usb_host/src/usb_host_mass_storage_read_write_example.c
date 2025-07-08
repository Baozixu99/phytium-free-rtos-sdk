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
 * FilePath: usb_host_mass_storage_read_write_example.c
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
#include "fmemory_pool.h"

#include "usbh_core.h"
#include "usbh_msc.h"

#include "usb_host_mass_storage_read_write_example.h"
/************************** Constant Definitions *****************************/
#define TIMER_OUT               (pdMS_TO_TICKS(10000UL))

/**************************** Type Definitions *******************************/
enum
{
    USB_HOST_OP_SUCCESS = 0,
    USB_HOST_OP_FAILED    = 1,
};

/************************** Variable Definitions *****************************/
static uint8_t rd_table[512] = {0};
static uint8_t wr_table[512] = {0};
static uintptr_t usb_id = 0;
static uint8_t parent_hub_index = 0;
static uint8_t parent_hub_port = 0;
static QueueHandle_t xQueue = NULL;

/***************** Macros (Inline Functions) Definitions *********************/
#define FUSB_DEBUG_TAG "USB-DISK"
#define FUSB_ERROR(format, ...) FT_DEBUG_PRINT_E(FUSB_DEBUG_TAG, format, ##__VA_ARGS__)
#define FUSB_WARN(format, ...)  FT_DEBUG_PRINT_W(FUSB_DEBUG_TAG, format, ##__VA_ARGS__)
#define FUSB_INFO(format, ...)  FT_DEBUG_PRINT_I(FUSB_DEBUG_TAG, format, ##__VA_ARGS__)
#define FUSB_DEBUG(format, ...) FT_DEBUG_PRINT_D(FUSB_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/
void usb_hc_dcache_invalidate(void *addr, unsigned long len);
void usb_hc_dcache_flush(void *addr, unsigned long len);
/*****************************************************************************/
static void UsbMscReadWriteTask(void *args)
{
    int ret;
    u8 id = (u8)usb_id;
    struct usbh_msc *msc_class;
    u32 loop = 0;
    uint32_t start_blk = 0;
    uint32_t blk_num = 1;
    const char *devname = (const char *)args;

    msc_class = (struct usbh_msc *)usbh_find_class_instance(devname);
    if (msc_class == NULL)
    {
        printf("Do not find %s. \r\n", devname);
        goto task_exit;
    }

    ret = usbh_msc_scsi_init(msc_class);
    if (ret < 0) {
        printf("scsi_init error, ret:%d\r\n", ret);
        goto task_exit;
    }

    /* check msc device information */
    printf("MSC (/dev/sd%c) info:\r\n", (char)msc_class->sdchar);
    printf("    USB: 0x%04x\r\n", msc_class->hport->device_desc.bcdUSB);
    printf("    Class: 0x%02x\r\n", msc_class->hport->device_desc.bDeviceClass);
    printf("    Subclass: 0x%02x\r\n", msc_class->hport->device_desc.bDeviceSubClass);
    printf("    Protocol: 0x%02x\r\n", msc_class->hport->device_desc.bDeviceProtocol);
    printf("    Block size: %d\r\n", msc_class->blocksize);
    printf("    Total size: %ld GB\r\n", ((u64)msc_class->blocksize * msc_class->blocknum) / SZ_1G);
    printf("    Bulk in: %d(mps)\r\n", msc_class->bulkin->wMaxPacketSize);
    printf("    Bulk out: %d(mps)\r\n", msc_class->bulkout->wMaxPacketSize);

    while (TRUE)
    {
        /* write partition table */
        for (uint32_t i = 0; i < 512; i++)
        {
            wr_table[i] ^= 0xfffff;
        }

        ret = usbh_msc_scsi_write10(msc_class, start_blk, wr_table, blk_num);
        if (ret < 0)
        {
            printf("Error in scsi_write10 error, ret:%d", ret);
            goto task_exit;
        }

        /* get the partition table */
        ret = usbh_msc_scsi_read10(msc_class, start_blk, rd_table, blk_num);
        if (ret < 0)
        {
            printf("Error in scsi_read10, ret:%d", ret);
            goto task_exit;
        }

        /* check if read table == write table */
        if (0 != memcmp(wr_table, rd_table, sizeof(rd_table)))
        {
            printf("Failed to check read and write.\r\n");
            goto task_exit;
        }
        else
        {
            printf("[%d] disk read and write %d bytes successfully.\r\n", loop, sizeof(rd_table)); 
#if defined(CONFIG_LOG_DEBUG) || defined(CONFIG_LOG_VERBOS)
            FtDumpHexByte(rd_table, sizeof(rd_table));
#endif
        }

        loop++;
        if (loop > 5)
        {
            break;
        }

        start_blk += blk_num;

        vTaskDelay(1);
    }

task_exit:
    ret = (ret == 0) ? USB_HOST_OP_SUCCESS : USB_HOST_OP_FAILED;
    xQueueSend(xQueue, &ret, 0);
    vTaskDelete(NULL);
}

BaseType_t FFreeRTOSReadWriteMassStorage(u32 id, const char *devname)
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

    ret = xTaskCreate((TaskFunction_t)UsbMscReadWriteTask,
                      (const char *)"UsbMscReadWriteTask",
                      (uint16_t)4096,
                      (void *)devname,
                      (UBaseType_t)configMAX_PRIORITIES - 1,
                      NULL);
    if (ret != pdPASS)
    {
        FUSB_ERROR("xTaskCreate create failed.\r\n");
        goto exit;        
    }

    ret = xQueueReceive(xQueue, &task_res, TIMER_OUT);
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
        printf("%s@%d: USB disk read/write example [failure], task_res = %d\r\n", __func__, __LINE__, task_res);
        return pdFAIL;
    }
    else
    {
        printf("%s@%d: USB disk read/write example [success].\r\n", __func__, __LINE__);
        return pdTRUE;
    }
}
