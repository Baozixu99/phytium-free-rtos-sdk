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
 * FilePath: qspi_spiffs_example.c
 * Date: 2022-07-11 11:32:48
 * LastEditTime: 2024-04-26 11:32:48
 * Description: This file is for the qspi_spiffs example functions.
 *
 * Modify History:
 *  Ver      Who            Date           Changes
 * -----   ------         --------    --------------------------------------
 * 1.0   wangxiaodong    2022/8/9       first release
 * 1.1   zhangyan        2023/2/9       improve functions
 * 1.2   huangjin        2024/4/26     add no letter shell mode, adapt to auto-test system
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "fkernel.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "fio_mux.h"
#include "qspi_spiffs_example.h"
#include "fassert.h"
#include "fdebug.h"
#include "fparameters.h"
#include "spiffs_port.h"
#include "sdkconfig.h"
#ifdef CONFIG_SPIFFS_ON_FSPIM_SFUD
    #include "fspim_spiffs_port.h"
#endif

#ifdef CONFIG_SPIFFS_ON_FQSPI_SFUD
    #include "fqspi_spiffs_port.h"
#endif


enum
{
    FSPIFFS_OPS_OK = 0,
    FSPIFFS_OPS_INIT_FAILED,
    FSPIFFS_OPS_ALREADY_INITED,
    FSPIFFS_OPS_MOUNT_FAILED,
    FSPIFFS_OPS_FORMAT_FAILED,
    FSPIFFS_OPS_NOT_YET_MOUNT,
    FSPIFFS_OPS_OPEN_FILE_FAILED,
    FSPIFFS_OPS_WRITE_FILE_FAILED,
    FSPIFFS_OPS_READ_FILE_FAILED,
    FSPIFFS_OPS_REMOVE_FILE_FAILED,
    FSPIFFS_OPS_CLOSE_FILE_FAILED,
    FSPIFFS_OPS_UNKNOWN_STATE,
    FSPIFFS_OPS_DATA_FAILED,
};

#define FSPIFFS_DEBUG_TAG "SPIFFS-QSPI"
#define FSPIFFS_ERROR(format, ...)   FT_DEBUG_PRINT_E(FSPIFFS_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSPIFFS_WARN(format, ...)    FT_DEBUG_PRINT_W(FSPIFFS_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSPIFFS_INFO(format, ...)    FT_DEBUG_PRINT_I(FSPIFFS_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSPIFFS_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FSPIFFS_DEBUG_TAG, format, ##__VA_ARGS__)

/* spiffs start address and size */
#if defined(CONFIG_E2000D_DEMO_BOARD) || defined(CONFIG_E2000Q_DEMO_BOARD) || defined(CONFIG_FIREFLY_DEMO_BOARD)
#define FSPIFFS_START_ADDR      (3 * SZ_1M)
#elif defined(CONFIG_FT2004_DSK_BOARD) || defined(CONFIG_D2000_TEST_BOARD)
#define FSPIFFS_START_ADDR      (7 * SZ_1M)
#endif

#define FSPIFFS_USE_SIZE        SZ_1M

#define FSPIFFS_RW_BUF_SIZE     64

/* if format flash, TRUE is need format, it tasks some time  */
#define FSPIFFS_IF_FORMAT       TRUE

#define QSPI_SPIFFS_WRITE_READ_TASK_PRIORITY  3

#define TIMER_OUT                  (pdMS_TO_TICKS(4000UL))

/* 一个页大小两倍的一个RAM缓冲区, 用来加载和维护SPIFFS的逻辑页 */
static volatile u8 fspiffs_work_buf[FSPIFFS_LOG_PAGE_SIZE * 2] = {0};
static volatile u8 fspiffs_fds_buf[32 * 4] = {0};
static volatile u8 fspiffs_cache_buf[(FSPIFFS_LOG_PAGE_SIZE + 32) * 4] = {0};
static u8 fspiffs_rd_buf[FSPIFFS_RW_BUF_SIZE] = {0};
static u8 fspiffs_wr_buf[FSPIFFS_RW_BUF_SIZE] = {0};
static FSpiffs instance;
static spiffs_config config;
static boolean spiffs_inited = FALSE;
const char *file_name = "test.txt";

/************************** Constant Definitions *****************************/
static QueueHandle_t xQueue = NULL;

static const char *xString = "FFreeRTOSQspiSpiffsWriteReadTask is running\r\n";

static void FFreeRTOSQspiSpiffsDelete(void);

static int FSpiffsOpsMount(boolean do_format)
{
    int result = 0;

    if (do_format)
    {
        result = SPIFFS_mount(&instance.fs,
                              &config,
                              (u8_t *)fspiffs_work_buf,
                              (u8_t *)fspiffs_fds_buf,
                              sizeof(fspiffs_fds_buf),
                              (u8_t *)fspiffs_cache_buf,
                              sizeof(fspiffs_cache_buf),
                              NULL);

        /* try mount to get status of filesystem  */
        if ((SPIFFS_OK != result) && (SPIFFS_ERR_NOT_A_FS != result))
        {
            /* if not a valid filesystem, continue to format,
                other error cannot handle, just exit */
            FSPIFFS_ERROR("Mount spiffs failed: %d", result);
            return FSPIFFS_OPS_MOUNT_FAILED;
        }

        /* must be unmounted prior to formatting */
        SPIFFS_unmount(&instance.fs);

        FSPIFFS_DEBUG("Spiffs formatting in progress ...");
        result = SPIFFS_format(&instance.fs);
        if (SPIFFS_OK != result)
        {
            FSPIFFS_ERROR("Spiffs formatting failed: %d", result);
            return FSPIFFS_OPS_FORMAT_FAILED;
        }
    }

    /* real mount */
    result = SPIFFS_mount(&instance.fs,
                          &config,
                          (u8_t *)fspiffs_work_buf,
                          (u8_t *)fspiffs_fds_buf,
                          sizeof(fspiffs_fds_buf),
                          (u8_t *)fspiffs_cache_buf,
                          sizeof(fspiffs_cache_buf),
                          NULL);
    if (SPIFFS_OK != result)
    {
        FSPIFFS_ERROR("Remount spiffs failed: %d, you may format the medium first.", result);
        return FSPIFFS_OPS_MOUNT_FAILED;
    }
    else
    {
        FSPIFFS_INFO("Mount spiffs success. \r\n");
        instance.fs_ready = TRUE;
    }

    return FSPIFFS_OPS_OK;
}

static int FSpiffsOpsListAll(void)
{
    int ret = FSPIFFS_OPS_OK;
    int result = SPIFFS_OK;

    if (FALSE == instance.fs_ready)
    {
        FSPIFFS_ERROR("Please mount file system first.");
        return FSPIFFS_OPS_NOT_YET_MOUNT;
    }

    static spiffs_DIR dir;
    static struct spiffs_dirent entry;

    memset(&dir, 0, sizeof(dir));
    memset(&entry, 0, sizeof(entry));

    struct spiffs_dirent *cur_entry = &entry;
    (void)SPIFFS_opendir(&instance.fs, "/", &dir);

    while (NULL != (cur_entry = SPIFFS_readdir(&dir, cur_entry)))
    {
        printf("-- %s file-id: [0x%04x] page-id: [%d] file-size: %d\r\n",
               cur_entry->name,
               cur_entry->pix,
               cur_entry->obj_id,
               cur_entry->size);
    }

    (void)SPIFFS_closedir(&dir);
    return ret;
}

int FSpiffsOpsCreateFile(const char *create_file_name)
{
    FASSERT((create_file_name) && (strlen(create_file_name) > 0));
    if (FALSE == instance.fs_ready)
    {
        FSPIFFS_ERROR("Please mount file system first.");
        return FSPIFFS_OPS_NOT_YET_MOUNT;
    }

    int ret = FSPIFFS_OPS_OK;

    /* create file */
    s32_t result = SPIFFS_creat(&instance.fs, create_file_name, 0);
    if (result < 0)
    {
        FSPIFFS_ERROR("Failed to create file %s", create_file_name);
        return FSPIFFS_OPS_OPEN_FILE_FAILED;
    }

    /* open file */
    spiffs_file fd = SPIFFS_open(&instance.fs, create_file_name, SPIFFS_RDONLY, 0);
    if (0 > fd)
    {
        FSPIFFS_ERROR("Failed to open file %s errno %d", create_file_name, SPIFFS_errno(&instance.fs));
        return FSPIFFS_OPS_OPEN_FILE_FAILED;
    }

    /* check file status */
    static spiffs_stat status;
    memset(&status, 0, sizeof(status));
    result = SPIFFS_fstat(&instance.fs, fd, &status);
    if (result < 0)
    {
        FSPIFFS_ERROR("Failed to get status of file %s, errno %d", create_file_name, SPIFFS_errno(&instance.fs));
        ret = FSPIFFS_OPS_OPEN_FILE_FAILED;
        goto err_exit;
    }

    if (0 != strcmp(status.name, create_file_name))
    {
        FSPIFFS_ERROR("Created file name %s != %s", status.name, create_file_name);
        ret = FSPIFFS_OPS_OPEN_FILE_FAILED;
        goto err_exit;
    }

    if (0 != status.size)
    {
        FSPIFFS_ERROR("Invalid file size %d", status.size);
        ret = FSPIFFS_OPS_OPEN_FILE_FAILED;
        goto err_exit;
    }

    FSPIFFS_INFO("File %s created successfully.\r\n", create_file_name);

err_exit:
    (void)SPIFFS_close(&instance.fs, fd);
    return ret;
}

int FSpiffsOpsWriteFile(const char *write_file_name, const char *str)
{
    FASSERT((write_file_name) && (strlen(write_file_name) > 0));
    FASSERT(str);
    int ret = FSPIFFS_OPS_OK;
    const u32 wr_len = strlen(str) + 1;

    spiffs_file fd = SPIFFS_open(&instance.fs, write_file_name, SPIFFS_RDWR | SPIFFS_TRUNC, 0);
    if (0 > fd)
    {
        FSPIFFS_ERROR("Failed to open file %s, errno %d", write_file_name, SPIFFS_errno(&instance.fs));
        return FSPIFFS_OPS_OPEN_FILE_FAILED;
    }

    int result = SPIFFS_write(&instance.fs, fd, (void *)str, wr_len);
    if (result < 0)
    {
        FSPIFFS_ERROR("Failed to write file %s, errno %d", write_file_name, SPIFFS_errno(&instance.fs));
        ret = FSPIFFS_OPS_WRITE_FILE_FAILED;
        goto err_exit;
    }

    /* check file status */
    static spiffs_stat status;
    memset(&status, 0, sizeof(status));
    result = SPIFFS_fstat(&instance.fs, fd, &status);
    if (result < 0)
    {
        FSPIFFS_ERROR("Failed to get status of file %s, errno %d", write_file_name, SPIFFS_errno(&instance.fs));
        ret = FSPIFFS_OPS_WRITE_FILE_FAILED;
        goto err_exit;
    }

    if (status.size != wr_len)
    {
        FSPIFFS_ERROR("File write size %ld != %ld", status.size, wr_len);
        ret = FSPIFFS_OPS_WRITE_FILE_FAILED;
        goto err_exit;
    }

    /* flush all pending writes from cache to flash */
    (void)SPIFFS_fflush(&instance.fs, fd);
    FSPIFFS_INFO("Write file %s with %d bytes successfully.\r\n", write_file_name, wr_len);
err_exit:
    (void)SPIFFS_close(&instance.fs, fd);
    return ret;
}

int FSpiffsOpsReadFile(const char *read_file_name)
{
    FASSERT((read_file_name) && (strlen(read_file_name) > 0));
    int ret = FSPIFFS_OPS_OK;
    int result = SPIFFS_OK;

    if (FALSE == instance.fs_ready)
    {
        FSPIFFS_ERROR("Please mount file system first.");
        return FSPIFFS_OPS_NOT_YET_MOUNT;
    }

    /* check file status */
    static spiffs_stat status;

    spiffs_flags open_flags = 0;

    /* open the file in read-only mode */
    open_flags = SPIFFS_RDWR;
    spiffs_file fd = SPIFFS_open(&instance.fs, read_file_name, open_flags, 0);
    if (0 > fd)
    {
        FSPIFFS_ERROR("Failed to open file %s, errno %d", read_file_name, SPIFFS_errno(&instance.fs));
        return FSPIFFS_OPS_OPEN_FILE_FAILED;
    }

    /* check file status */
    memset(&status, 0, sizeof(status));
    result = SPIFFS_fstat(&instance.fs, fd, &status);
    if (result < 0)
    {
        FSPIFFS_ERROR("Failed to get status of file %s, errno %d", read_file_name, SPIFFS_errno(&instance.fs));
        ret = FSPIFFS_OPS_OPEN_FILE_FAILED;
        goto err_exit;
    }

    s32_t offset = SPIFFS_lseek(&instance.fs, fd, 0, SPIFFS_SEEK_END);
    if ((s32_t)status.size != offset)
    {
        FSPIFFS_ERROR("File %s spiffs:%ld != fs:%ld", read_file_name, status.size, offset);
        ret = FSPIFFS_OPS_OPEN_FILE_FAILED;
        goto err_exit;
    }

    memset(fspiffs_rd_buf, 0, FSPIFFS_RW_BUF_SIZE);

    /* seek to offset and start read */
    if (0 > SPIFFS_lseek(&instance.fs, fd, 0, SPIFFS_SEEK_SET))
    {
        FSPIFFS_ERROR("Seek file failed.");
        ret = FSPIFFS_OPS_READ_FILE_FAILED;
        goto err_exit;
    }


    s32_t read_len = min((s32_t)FSPIFFS_RW_BUF_SIZE, (s32_t)status.size);
    s32_t read_bytes = SPIFFS_read(&instance.fs, fd, (void *)fspiffs_rd_buf, read_len);
    if (read_bytes < 0)
    {
        FSPIFFS_ERROR("Failed to read file %s, errno %d", read_file_name, SPIFFS_errno(&instance.fs));
        ret = FSPIFFS_OPS_READ_FILE_FAILED;
        goto err_exit;
    }

    FSPIFFS_INFO("Read %s success, str = %s\n", read_file_name, fspiffs_rd_buf);

err_exit :
    /* close file */
    (void)SPIFFS_close(&instance.fs, fd);

    return ret;
}


static FError QspiSpiffsInit(void)
{
    FError init_ret = FSPIFFS_OPS_INIT_FAILED;
    int result = 0;

    if (TRUE == spiffs_inited)
    {
        FSPIFFS_WARN("Spiffs is already initialized.");
        return FSPIFFS_OPS_ALREADY_INITED;
    }

    FIOMuxInit();
#if defined(CONFIG_E2000D_DEMO_BOARD)
    FIOPadSetQspiMux(FSPI0_ID, FQSPI_CS_0);
#endif

    memset(&config, 0, sizeof(config));
    config = *FSpiffsGetDefaultConfig();
    config.phys_addr = FSPIFFS_START_ADDR; /* may use part of flash */
    config.phys_size = FSPIFFS_USE_SIZE;

    memset(&instance, 0, sizeof(instance));
    instance.fs_addr = FSPIFFS_START_ADDR;
    instance.fs_size = FSPIFFS_USE_SIZE;

    result = FSpiffsInitialize(&instance, FSPIFFS_PORT_TO_FQSPI);
    if (FSPIFFS_PORT_OK != result)
    {
        FSPIFFS_ERROR("Initialize spiffs failed.");
        return FSPIFFS_OPS_INIT_FAILED;
    }

    FSpiffsOpsMount(FSPIFFS_IF_FORMAT);

    FSpiffsOpsCreateFile(file_name);

    spiffs_inited = TRUE;

    FSpiffsOpsListAll();

    FSPIFFS_INFO("Spiffs init successfully.");

    return FSPIFFS_OPS_OK;
}

static FError QspiSpiffsWriteRead(void)
{
    FError ret = FT_SUCCESS;
    char *string = "spiffs qspi write times";
    static int i = 0;
    memset(fspiffs_wr_buf, 0, FSPIFFS_RW_BUF_SIZE);
    
    /* Print out the name of this task. */
    FSPIFFS_INFO(xString);
    i++;
    sprintf(fspiffs_wr_buf, "%s-%d", string, i);
    FSPIFFS_INFO("Write to %s, str = %s\n", file_name, fspiffs_wr_buf);
    
    FSpiffsOpsWriteFile(file_name, fspiffs_wr_buf);
    FSpiffsOpsReadFile(file_name);

    if (0 != strcmp(fspiffs_wr_buf, fspiffs_rd_buf))
    {
        FSPIFFS_ERROR("Read and write data are not equal!!!!\nwrite data:%s ,read data:%s\n", fspiffs_wr_buf, fspiffs_rd_buf);
        return FSPIFFS_OPS_DATA_FAILED;
    }
    else
    {
        printf("Successfully, read and write data are equal.\n\n");
    }

    return FSPIFFS_OPS_OK;
}


static void FFreeRTOSQspiSpiffsWriteThenReadTask(void *pvParameters)
{
    FError ret = FT_SUCCESS;
    int task_res = FSPIFFS_OPS_OK;

    /* QspiSpiffs初始化 */
    ret = QspiSpiffsInit();
    if (ret != FT_SUCCESS)
    {
        task_res = FSPIFFS_OPS_INIT_FAILED;
        FSPIFFS_ERROR("QspiSpiffsInit failed.");
        goto task_exit;
    }
    /* QspiSpiffs写文件读文件 */
    ret = QspiSpiffsWriteRead();
    if (ret != FT_SUCCESS)
    {
        task_res = FSPIFFS_OPS_DATA_FAILED;
        FSPIFFS_ERROR("QspiSpiffsWriteRead failed.");
        goto task_exit;
    }
    /* 去初始化 */
    FFreeRTOSQspiSpiffsDelete();

task_exit:
    xQueueSend(xQueue, &task_res, 0);
    vTaskDelete(NULL);    
}

int FFreeRTOSQspiSpiffsCreate(void)
{
    BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为 pdPASS */
    int task_res = FSPIFFS_OPS_UNKNOWN_STATE;

    xQueue = xQueueCreate(1, sizeof(int)); /* 创建消息队列 */
    if (xQueue == NULL)
    {
        FSPIFFS_ERROR("xQueue create failed.");
        goto exit;
    }    

    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSQspiSpiffsWriteThenReadTask,  /* 任务入口函数 */
                          (const char *)"FFreeRTOSQspiSpiffsWriteThenReadTask",/* 任务名字 */
                          4096,  /* 任务栈大小 */
                          NULL,/* 任务入口函数参数 */
                          (UBaseType_t)QSPI_SPIFFS_WRITE_READ_TASK_PRIORITY, /* 任务的优先级 */
                          NULL); /* 任务控制 */
    if (xReturn == pdFAIL)
    {
        FSPIFFS_ERROR("xTaskCreate FFreeRTOSQspiSpiffsWriteThenReadTask failed.");
        goto exit;
    }

    xReturn = xQueueReceive(xQueue, &task_res, TIMER_OUT);
    if (xReturn == pdFAIL)
    {
        FSPIFFS_ERROR("xQueue receive timeout.");
        goto exit;
    }    

exit:
    if (xQueue != NULL)
    {
        vQueueDelete(xQueue);
    }

    if (task_res != FSPIFFS_OPS_OK)
    {
        printf("%s@%d: Qspi_spiffs write and read example [failure], task_res = %d\r\n", __func__, __LINE__, task_res);
        return task_res;
    }
    else
    {
        printf("%s@%d: Qspi_spiffs write and read example [success].\r\n", __func__, __LINE__);
        return task_res;
    }
}

static void FFreeRTOSQspiSpiffsDelete(void)
{
    /*deinit iomux */
    FIOMuxDeInit();
    FSpiffsDeInitialize(&instance);
    spiffs_inited = FALSE;
    return;
}



