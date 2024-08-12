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
 * FilePath: xhci_usb_disk_read_write_example.c
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
#include "task.h"

#include "fassert.h"
#include "fdebug.h"
#include "fmemory_pool.h"

#include "usbh_core.h"
#include "usbh_msc.h"

#include "xhci_host_example.h"
/************************** Constant Definitions *****************************/
#define TIMER_OUT               (pdMS_TO_TICKS(10000UL))

/**************************** Type Definitions *******************************/
enum
{
    XHCI_USB_DISK_RW_SUCCESS = 0,
    XHCI_USB_DISK_RW_FAIL    = 1,
};

/************************** Variable Definitions *****************************/
static uint8_t rd_table[512] = {0};
static uint8_t wr_table[512] = {0};
static uintptr_t usb_id = 0;
static QueueHandle_t xQueue = NULL;

/***************** Macros (Inline Functions) Definitions *********************/
#define FUSB_DEBUG_TAG "XHCI-USB-DISK"
#define FUSB_ERROR(format, ...) FT_DEBUG_PRINT_E(FUSB_DEBUG_TAG, format, ##__VA_ARGS__)
#define FUSB_WARN(format, ...)  FT_DEBUG_PRINT_W(FUSB_DEBUG_TAG, format, ##__VA_ARGS__)
#define FUSB_INFO(format, ...)  FT_DEBUG_PRINT_I(FUSB_DEBUG_TAG, format, ##__VA_ARGS__)
#define FUSB_DEBUG(format, ...) FT_DEBUG_PRINT_D(FUSB_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/
void usb_hc_dcache_invalidate(void *addr, unsigned long len);
void usb_hc_dcache_flush(void *addr, unsigned long len);
/*****************************************************************************/
static void XhciUsbMscTask(void *args)
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
        USB_LOG_RAW("Do not find %s. \r\n", devname);
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
            USB_LOG_ERR("Error in scsi_write10 error, ret:%d", ret);
            goto task_exit;
        }

        /* get the partition table */
        ret = usbh_msc_scsi_read10(msc_class, start_blk, rd_table, blk_num);
        if (ret < 0)
        {
            USB_LOG_ERR("Error in scsi_read10, ret:%d", ret);
            goto task_exit;
        }

        /* check if read table == write table */
        if (0 != memcmp(wr_table, rd_table, sizeof(rd_table)))
        {
            USB_LOG_ERR("Failed to check read and write.\r\n");
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
    ret = (ret == 0) ? XHCI_USB_DISK_RW_SUCCESS : XHCI_USB_DISK_RW_FAIL;
    xQueueSend(xQueue, &ret, 0);
    vTaskDelete(NULL);
}

BaseType_t FFreeRTOSRunXhciDisk(u32 id, const char *devname)
{
    BaseType_t ret = pdPASS;
    int task_res = XHCI_USB_DISK_RW_SUCCESS;
    xQueue = xQueueCreate(1, sizeof(int));
    if (xQueue == NULL)
    {
        FUSB_ERROR("xQueue create failed.\r\n");
        goto exit;
    }

    usb_id = id;

    ret = xTaskCreate((TaskFunction_t)XhciUsbMscTask,
                      (const char *)"XhciUsbMscTask",
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

    if (task_res != XHCI_USB_DISK_RW_SUCCESS)
    {
        printf("%s@%d: XHCI usb disk read/write example [failure], task_res = %d\r\n", __func__, __LINE__, task_res);
        return pdFAIL;
    }
    else
    {
        printf("%s@%d: XHCI usb disk read/write example [success].\r\n", __func__, __LINE__);
        return pdTRUE;
    }
}
