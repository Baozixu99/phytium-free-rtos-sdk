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
 * FilePath: usb_host_mass_storage_bench_example.c
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
#include "fkernel.h"

#include "usbh_core.h"
#include "usbh_msc.h"

#include "usb_host_mass_storage_bench_example.h"
/************************** Constant Definitions *****************************/
#define TIMER_OUT               (0xFFFFFFFF)

#define USB_MAX_RW_BLK          1000U
#define USB_DISK_BLK_SIZE       512U
#define USB_DISK_START_BLK      0U
#define USB_BENCH_TIMES         6U
#define USB_BENCH_SIZE          (u64)(SZ_1M * 200ULL)
#define USB_BENCH_BLKS          (USB_BENCH_SIZE / USB_DISK_BLK_SIZE)     
/**************************** Type Definitions *******************************/
enum
{
    USB_HOST_OP_SUCCESS = 0,
    USB_HOST_OP_FAILED    = 1,
};

/************************** Variable Definitions *****************************/
static uint8_t rw_buf[USB_DISK_BLK_SIZE * USB_MAX_RW_BLK] = {0};
static uintptr_t usb_id = 0;
static QueueHandle_t xQueue = NULL;

/***************** Macros (Inline Functions) Definitions *********************/
#define FUSB_DEBUG_TAG "USB-HOST-DISK"
#define FUSB_ERROR(format, ...) FT_DEBUG_PRINT_E(FUSB_DEBUG_TAG, format, ##__VA_ARGS__)
#define FUSB_WARN(format, ...)  FT_DEBUG_PRINT_W(FUSB_DEBUG_TAG, format, ##__VA_ARGS__)
#define FUSB_INFO(format, ...)  FT_DEBUG_PRINT_I(FUSB_DEBUG_TAG, format, ##__VA_ARGS__)
#define FUSB_DEBUG(format, ...) FT_DEBUG_PRINT_D(FUSB_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/

/*****************************************************************************/
static void UsbMscBenchTask(void *args)
{
    int ret;
    u8 id = (u8)usb_id;
    struct usbh_msc *msc_class;
    u32 loop = 0;
    const char *devname = (const char *)args;
    TickType_t tick_count, tick_spend;
    double time_in_seconds, rw_speed;
    u64 start_blk = USB_DISK_START_BLK;
    u64 bench_blk;
    u64 bench_rw;
    int run_times = USB_BENCH_TIMES;

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

    while (run_times-- > 0)
    {
        
        /* do write bench */
        {
            for (u32 i = 0; i < USB_MAX_RW_BLK; i++) /* fill some none-zero value into write buffer */
            {
                memset(rw_buf + i * USB_DISK_BLK_SIZE, ((USB_DISK_START_BLK + i) & 0xff), USB_DISK_BLK_SIZE);
            }

            bench_blk = start_blk;

            if ((bench_blk + USB_BENCH_BLKS) > msc_class->blocknum)
            {
                break;
            }

            printf("Writing the %ld MB data (Block %ld ~ %ld), waiting...\r\n", 
                    USB_BENCH_SIZE / SZ_1M, bench_blk, bench_blk + USB_BENCH_BLKS);        
            tick_count = xTaskGetTickCount();

            while (bench_blk < (start_blk + USB_BENCH_BLKS))
            {
                /* how many blocks to be wrote this time */
                bench_rw = (bench_blk + USB_MAX_RW_BLK) <= (start_blk + USB_BENCH_BLKS) ?
                            USB_MAX_RW_BLK : (start_blk + USB_BENCH_BLKS - bench_blk);
            
                ret = usbh_msc_scsi_write10(msc_class, bench_blk, rw_buf, USB_MAX_RW_BLK);
                if (ret < 0)
                {
                    printf("Error in scsi_write10 error, ret:%d", ret);
                    goto task_exit;
                }

                bench_blk += bench_rw;
            }

            tick_spend = xTaskGetTickCount() - tick_count;
            time_in_seconds = ((double)tick_spend / configTICK_RATE_HZ);
            rw_speed = ((double)USB_BENCH_SIZE / SZ_1M) / time_in_seconds;
            printf("Write benchmark success, total time: %.2f Sec, total size: %d MB, speed: %.2f MB/sec\r\n",
                    time_in_seconds,
                    (int)(USB_BENCH_SIZE / SZ_1M),
                    rw_speed); /* this forced type casting is to avoid floating-point printing error */        
        }

        /* do read bench */
        {
            printf("Reading the %ld MB data (%ld Blocks), waiting...\r\n", 
                    USB_BENCH_SIZE / SZ_1M, USB_BENCH_BLKS);        
            tick_count = xTaskGetTickCount();

            bench_blk = start_blk;
            while (bench_blk < (start_blk + USB_BENCH_BLKS))
            {
                /* how many blocks to be wrote this time */
                bench_rw = (bench_blk + USB_MAX_RW_BLK) <= (start_blk + USB_BENCH_BLKS) ?
                            USB_MAX_RW_BLK : (start_blk + USB_BENCH_BLKS - bench_blk);
            
                ret = usbh_msc_scsi_read10(msc_class, bench_blk, rw_buf, USB_MAX_RW_BLK);
                if (ret < 0)
                {
                    printf("Error in scsi_write10 error, ret:%d", ret);
                    goto task_exit;
                }

                bench_blk += bench_rw;
            }

            tick_spend = xTaskGetTickCount() - tick_count;
            time_in_seconds = ((double)tick_spend / configTICK_RATE_HZ);
            rw_speed = ((double)USB_BENCH_SIZE / SZ_1M) / time_in_seconds;
            printf("Read benchmark success, total time: %.2f Sec, total size: %d MB, speed: %.2f MB/sec\r\n",
                    time_in_seconds,
                    (int)(USB_BENCH_SIZE / SZ_1M),
                    rw_speed); /* this forced type casting is to avoid floating-point printing error */     
        }

        start_blk += USB_BENCH_BLKS;
    }

task_exit:
    ret = (ret == 0) ? USB_HOST_OP_SUCCESS : USB_HOST_OP_FAILED;
    xQueueSend(xQueue, &ret, 0);
    vTaskDelete(NULL);
}

BaseType_t FFreeRTOSBenchMassStorage(u32 id, const char *devname)
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

    ret = xTaskCreate((TaskFunction_t)UsbMscBenchTask,
                      (const char *)"UsbMscBenchTask",
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