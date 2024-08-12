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
 * FilePath: fatfs_examples.c
 * Date: 2022-07-11 11:32:48
 * LastEditTime: 2022-04-28 11:32:48
 * Description:  This file is for the fatfs test example functions.
 *
 * Modify History:
 *  Ver   Who         Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/12/7    init commit
 * 2.0   liyilun    2024/04/28   add no letter shell mode, adapt to auto test system
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "fatfs_examples.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"

#include "fkernel.h"
#include "strto.h"
#include "fassert.h"
#include "fdebug.h"
#include "fparameters.h"
#include "sdkconfig.h"

#include "ff_utils.h"

/************************** Constant Definitions *****************************/
#define FATFS_EVT_INIT_DONE        (0x1 << 0)
#define FATFS_EVT_CYC_TEST_DONE    (0x1 << 1)

/*The macro WR_SECTOR, RAM_WR_SECTOR, CYCLE_TIMES affects the test time.
*If you want to increase the macro value, increase WAIT_TIMEOUT as well*/
#define WR_SECTOR                  3000U
#define RAM_WR_SECTOR              300000U
#define CYCLE_TIMES                1
#define WAIT_TIMEOUT               pdMS_TO_TICKS(3000000U)
/************************** Variable Definitions *****************************/
static const char *mount_points[FFREERTOS_DISK_TYPE_NUM] =
{
    [FFREERTOS_FATFS_RAM_DISK] = FF_RAM_DISK_MOUNT_POINT,
    [FFREERTOS_FATFS_TF_CARD] = FF_FSDIF_TF_DISK_MOUNT_POINT,
    [FFREERTOS_FATFS_EMMC_CARD] = FF_FSDIF_EMMC_DISK_MOUNT_POINT,
    [FFREERTOS_FATFS_USB_DISK] = FF_USB_DISK_MOUNT_POINT,
    [FFREERTOS_FATFS_SATA_DISK] = FF_SATA_DISK_MOUNT_POINT,
    [FFREERTOS_FATFS_SATA_PCIE_DISK] = FF_SATA_PCIE_DISK_MOUNT_POINT
};
static const MKFS_PARM fs_option =
{
    .fmt = FM_EXFAT, /* format file system as exFAT to support > 4GB storage */
    .n_fat = 0, /* use default setting for other options */
    .align = 0,
    .n_root = 0,
    .au_size = 0
};

static ff_fatfs file_sys[FFREERTOS_DISK_TYPE_NUM];
static QueueHandle_t xQueue = NULL;

/***************** Macros (Inline Functions) Definitions *********************/
#define FF_DEBUG_TAG "FATFS"
#define FF_ERROR(format, ...)   FT_DEBUG_PRINT_E(FF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FF_WARN(format, ...)    FT_DEBUG_PRINT_W(FF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FF_INFO(format, ...)    FT_DEBUG_PRINT_I(FF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FF_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FF_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/

/*****************************************************************************/

static FRESULT FatfsInit(void)
{
    FRESULT fr = FR_OK;

#ifdef CONFIG_FATFS_RAM_DISK
    printf("RAM init...\r\n");
    fr = ff_setup(&file_sys[FFREERTOS_FATFS_RAM_DISK],
                  mount_points[FFREERTOS_FATFS_RAM_DISK],
                  &fs_option, pdTRUE);

    if (FR_OK != fr)
    {
        FF_ERROR("RAM disk init failed, err = %d.", fr);
        return fr;
    }
#endif

#ifdef CONFIG_FATFS_SDMMC_FSDIF_TF
    printf("SDIF TF init...\r\n");
    fr = ff_setup(&file_sys[FFREERTOS_FATFS_TF_CARD],
                  mount_points[FFREERTOS_FATFS_TF_CARD],
                  &fs_option, pdTRUE);

    if (FR_OK != fr)
    {
        FF_ERROR("TF card init failed, err = %d.", fr);
        return fr;
    }
#endif

#ifdef CONFIG_FATFS_SDMMC_FSDIF_EMMC
    printf("SDIF EMMC init...\r\n");
    fr = ff_setup(&file_sys[FFREERTOS_FATFS_EMMC_CARD],
                  mount_points[FFREERTOS_FATFS_EMMC_CARD],
                  &fs_option, pdTRUE);

    if (FR_OK != fr)
    {
        FF_ERROR("eMMC card init failed, err = %d.", fr);
        return fr;
    }
#endif

#ifdef CONFIG_FATFS_USB
    printf("USB init...\r\n");
    fr = ff_setup(&file_sys[FFREERTOS_FATFS_USB_DISK],
                  mount_points[FFREERTOS_FATFS_USB_DISK],
                  &fs_option, pdTRUE);

    if (FR_OK != fr)
    {
        FF_ERROR("USB init failed, err = %d.", fr);
        return fr;
    }
#endif

#ifdef CONFIG_FATFS_FSATA
    printf("SATA init...\r\n");
    fr = ff_setup(&file_sys[FFREERTOS_FATFS_SATA_DISK],
                  mount_points[FFREERTOS_FATFS_SATA_DISK],
                  &fs_option, pdTRUE);

    if (FR_OK != fr)
    {
        FF_ERROR("SATA init failed, err = %d.", fr);
        return fr;
    }
#endif
#ifdef CONFIG_FATFS_FSATA_PCIE
    printf("SATA PCIE...\r\n");
    fr = ff_setup(&file_sys[FFREERTOS_FATFS_SATA_PCIE_DISK],
                  mount_points[FFREERTOS_FATFS_SATA_PCIE_DISK],
                  &fs_option, pdTRUE);

    if (FR_OK != fr)
    {
        FF_ERROR("SATA PCIE init failed, err = %d.", fr);
        return fr;
    }
#endif
    printf("Storage device init finished !!!\r\n");
    return FR_OK;
}


static FRESULT FatfsBasicTest(void)
{
    FRESULT fr = FR_OK;
#ifdef CONFIG_FATFS_RAM_DISK
        printf("\r\n========Basic test for RAM Disk=================\r\n");
        fr = ff_basic_test(mount_points[FFREERTOS_FATFS_RAM_DISK], "logfile.txt");
        if (FR_OK != fr)
        {
            FF_ERROR("RAM disk basic test failed, err = %d.", fr);
            return fr;
        }
#endif

#ifdef CONFIG_FATFS_SDMMC_FSDIF_TF
        printf("\r\n========Basic test for TF Card=================\r\n");
        fr = ff_basic_test(mount_points[FFREERTOS_FATFS_TF_CARD], "logfile.txt");
        if (FR_OK != fr)
        {
            FF_ERROR("TF card basic test failed, err = %d.", fr);
            return fr;
        }
#endif

#ifdef CONFIG_FATFS_SDMMC_FSDIF_EMMC
        printf("\r\n========Basic test for eMMC=================\r\n");
        fr = ff_basic_test(mount_points[FFREERTOS_FATFS_EMMC_CARD], "logfile.txt");
        if (FR_OK != fr)
        {
            FF_ERROR("SDIO basic test failed, err = %d.", fr);
            return fr;
        }
#endif

#ifdef CONFIG_FATFS_USB
        printf("\r\n========Basic test for USB Disk=================\r\n");
        fr = ff_basic_test(mount_points[FFREERTOS_FATFS_USB_DISK], "logfile.txt");
        if (FR_OK != fr)
        {
            FF_ERROR("USB basic test failed, err = %d.", fr);
            return fr;
        }
#endif

#ifdef CONFIG_FATFS_FSATA
        printf("\r\n========Basic test for SATA Disk=================\r\n");
        fr = ff_basic_test(mount_points[FFREERTOS_FATFS_SATA_DISK], "logfile.txt");
        if (FR_OK != fr)
        {
            FF_ERROR("SATA basic test failed, err = %d.", fr);
            return fr;
        }
#endif

#ifdef CONFIG_FATFS_FSATA_PCIE
        printf("\r\n========Basic test for SATA PCIE Disk=================\r\n");
        fr = ff_basic_test(mount_points[FFREERTOS_FATFS_SATA_PCIE_DISK], "logfile.txt");
        if (FR_OK != fr)
        {
            FF_ERROR("SATA PCIE basic test failed, err = %d.", fr);
            return fr;
        }
#endif
    return fr;
}

/* cycle test will test diskio and destory file system */
static FRESULT FatfsCycleTest(void)
{
    FRESULT fr = FR_OK;
#ifdef CONFIG_FATFS_RAM_DISK
        printf("\r\n========Cycle test for RAM Disk=================\r\n");
        fr = ff_cycle_test(mount_points[FFREERTOS_FATFS_RAM_DISK], CYCLE_TIMES);
        if (FR_OK != fr)
        {
            FF_ERROR("RAM cycle test failed, err = %d.", fr);
            return fr;
        }
#endif

#ifdef CONFIG_FATFS_SDMMC_FSDIF_TF
        printf("\r\n========Cycle test for TF Disk=================\r\n");
        fr = ff_cycle_test(mount_points[FFREERTOS_FATFS_TF_CARD], CYCLE_TIMES);
        if (FR_OK != fr)
        {
            FF_ERROR("TF cycle test failed, err = %d.", fr);
            return fr;
        }
#endif

#ifdef CONFIG_FATFS_SDMMC_FSDIF_EMMC
        printf("\r\n========Cycle test for SDIO Disk=================\r\n");
        fr = ff_cycle_test(mount_points[FFREERTOS_FATFS_EMMC_CARD], CYCLE_TIMES);
        if (FR_OK != fr)
        {
            FF_ERROR("SDIO cycle test failed, err = %d.", fr);
            return fr;
        }
        
#endif

#ifdef CONFIG_FATFS_USB
        printf("\r\n========Cycle test for USB Disk=================\r\n");
        fr = ff_cycle_test(mount_points[FFREERTOS_FATFS_USB_DISK], CYCLE_TIMES);
        if (FR_OK != fr)
        {
            FF_ERROR("USB cycle test failed, err = %d.", fr);
            return fr;
        }
#endif

#ifdef CONFIG_FATFS_FSATA
        printf("\r\n========Cycle test for SATA Disk=================\r\n");
        fr = ff_cycle_test(mount_points[FFREERTOS_FATFS_SATA_DISK], CYCLE_TIMES);
        if (FR_OK != fr)
        {
            FF_ERROR("SATA cycle test failed, err = %d.", fr);
            return fr;
        }
#endif

#ifdef CONFIG_FATFS_FSATA_PCIE
        printf("\r\n========Cycle test for SATA PCIE Disk=================\r\n");
        fr = ff_cycle_test(mount_points[FFREERTOS_FATFS_SATA_PCIE_DISK], CYCLE_TIMES);
        if (FR_OK != fr)
        {
            FF_ERROR("SATA pcie cycle test failed, err = %d.", fr);
            return fr;
        }
#endif
    return fr;
}

void FatfsRunTask(void)
{
    FRESULT fret = FR_OK;
    fret = FatfsInit();
    if(FR_OK != fret)
    {
        goto task_ret;
    }

#ifdef CONFIG_FATFS_BASIC_TEST
    fret = FatfsBasicTest();
    if(FR_OK != fret)
    {
        goto task_ret;
    }
#endif

#ifdef CONFIG_FATFS_CYCLE_TEST
    fret = FatfsCycleTest();
    if(FR_OK != fret)
    {
        goto task_ret;
    }
#endif
task_ret:
    xQueueSend(xQueue, &fret, 0);
    vTaskDelete(NULL);
}

BaseType_t FFreeRTOSFatfsTest(void)
{
    BaseType_t ret = pdPASS;
    FRESULT task_ret = FR_OK;
    xQueue = xQueueCreate(1, sizeof(FRESULT));

    ret = xTaskCreate((TaskFunction_t)FatfsRunTask,
                      (const char *)"FatfsRunTask",
                      2048,
                      NULL,
                      (UBaseType_t)configMAX_PRIORITIES - 1,
                      NULL);
    FASSERT_MSG(pdPASS == ret, "Create task failed.");

    ret = xQueueReceive(xQueue, &task_ret, WAIT_TIMEOUT);
    FASSERT_MSG(pdPASS == ret, "xQueue Receive failed.\r\n");

    vQueueDelete(xQueue);
    if(task_ret != FR_OK)
    {
        printf("%s@%d: fatfs example [failure].\r\n", __func__, __LINE__);
        return pdFAIL;
    }
    else
    {
        printf("%s@%d: fatfs example [success].\r\n", __func__, __LINE__);
        return pdTRUE;
    }
}