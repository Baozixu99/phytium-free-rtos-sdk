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
 * FilePath: spim_spiffs_example.c
 * Date: 2022-07-11 11:32:48
 * LastEditTime: 2024-04-25 11:32:48
 * Description:  This file is for the spim_spiffs example functions.
 * 
 * Modify History: 
 *  Ver     Who      Date         Changes
 * -----   ------  --------   --------------------------------------
 * 1.0  liqiaozhong 2022/11/2  first commit
 * 2.0  liyilun     2024/04/25 add no letter shell mode, adapt to auto test system
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "fkernel.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "spim_spiffs_example.h"
#include "strto.h"
#include "fassert.h"
#include "fdebug.h"
#include "fparameters.h"
#include "spiffs_port.h"
#include "sdkconfig.h"
#if defined(CONFIG_SPIFFS_ON_FSPIM_SFUD) || defined(CONFIG_SPIFFS_ON_FSPIM_V2_SFUD)
#include "fspim_spiffs_port.h"
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
};

#define FSPIFFS_DEBUG_TAG "SPIFFS-SPIM"
#define FSPIFFS_ERROR(format, ...)   FT_DEBUG_PRINT_E(FSPIFFS_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSPIFFS_WARN(format, ...)    FT_DEBUG_PRINT_W(FSPIFFS_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSPIFFS_INFO(format, ...)    FT_DEBUG_PRINT_I(FSPIFFS_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSPIFFS_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FSPIFFS_DEBUG_TAG, format, ##__VA_ARGS__)

/* spiffs start address and size */ 
#define FSPIFFS_START_ADDR		0x0
#define FSPIFFS_USE_SIZE		SZ_1M	

#define FSPIFFS_RW_BUF_SIZE     64
#define WR_LOOP_TIMES           2

/* if format flash, TRUE is need format, it tasks some time  */
#define FSPIFFS_IF_FORMAT		TRUE

/* 一个页大小两倍的一个RAM缓冲区, 用来加载和维护SPIFFS的逻辑页 */
static volatile u8 fspiffs_work_buf[FSPIFFS_LOG_PAGE_SIZE * 2] = {0};
static volatile u8 fspiffs_fds_buf[32 * 4] = {0};
static volatile u8 fspiffs_cache_buf[(FSPIFFS_LOG_PAGE_SIZE + 32) * 4] = {0};
static QueueHandle_t xQueue = NULL;
static FSpiffs instance;

/* The periods assigned to write and read. */
#define WR_TIMER_PERIOD		( pdMS_TO_TICKS( 50000UL ) )

static void FFreeRTOSSpimSpiffsDelete(void);

static int FSpiffsOpsMount(boolean do_format, spiffs_config config)
{
    int result = 0;

    if (do_format)
    {
        result = SPIFFS_mount(&instance.fs,/*挂载spiffs系统*/
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
            FSPIFFS_ERROR("mount spiffs failed: %d", result);
            return FSPIFFS_OPS_MOUNT_FAILED;            
        }
        /* must be unmounted prior to formatting */
        SPIFFS_unmount(&instance.fs);

        FSPIFFS_DEBUG("format spiffs in progress ...\r\n");
        result = SPIFFS_format(&instance.fs);
        if (SPIFFS_OK != result)
        {
            FSPIFFS_ERROR("format spiffs failed: %d", result);
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
        FSPIFFS_ERROR("remount spiffs failed: %d, you may format the medium first", result);
        return FSPIFFS_OPS_MOUNT_FAILED;
    }
    else
    {
        vPrintf("mount spiffs success !!! \r\n");
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
        FSPIFFS_ERROR("please mount file system first !!!");
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

int FSpiffsOpsCreateFile(const char *file_name)
{
    FASSERT((file_name) && (strlen(file_name) > 0));
    if (FALSE == instance.fs_ready)
    {
        FSPIFFS_ERROR("please mount file system first !!!");
        return FSPIFFS_OPS_NOT_YET_MOUNT;
    }

    int ret = FSPIFFS_OPS_OK;

    /* create file */
    s32_t result = SPIFFS_creat(&instance.fs, file_name, 0);
    if (result < 0)
    {
        FSPIFFS_ERROR("failed to create file %s", file_name);
        return FSPIFFS_OPS_OPEN_FILE_FAILED;
    }

    /* open file */
    spiffs_file fd = SPIFFS_open(&instance.fs, file_name, SPIFFS_RDONLY, 0);
    if (0 > fd)
    {
        FSPIFFS_ERROR("failed to open file %s errno %d line %d", file_name, SPIFFS_errno(&instance.fs), __LINE__);
        return FSPIFFS_OPS_OPEN_FILE_FAILED;
    }

    /* check file status */
    static spiffs_stat status;
    memset(&status, 0, sizeof(status));
    result = SPIFFS_fstat(&instance.fs, fd, &status);
    if (result < 0)
    {
        FSPIFFS_ERROR("failed to get status of file %s errno %d", file_name, SPIFFS_errno(&instance.fs));
        ret = FSPIFFS_OPS_OPEN_FILE_FAILED;
        goto err_exit;
    }    

    if (0 != strcmp(status.name, file_name))
    {
        FSPIFFS_ERROR("created file name %s != %s", status.name, file_name);
        ret = FSPIFFS_OPS_OPEN_FILE_FAILED;
        goto err_exit;
    }

    if (0 != status.size)
    {
        FSPIFFS_ERROR("invalid file size %d", status.size);
        ret = FSPIFFS_OPS_OPEN_FILE_FAILED;
        goto err_exit;
    }

    vPrintf("create file %s success !!!\r\n", file_name);

err_exit:
    (void)SPIFFS_close(&instance.fs, fd);
    return ret;
}

int FSpiffsOpsWriteFile(const char *file_name, const char *str)
{
    FASSERT((file_name) && (strlen(file_name) > 0));
    FASSERT(str);
    int ret = FSPIFFS_OPS_OK;
    const u32 wr_len = strlen(str) + 1;

    spiffs_file fd = SPIFFS_open(&instance.fs, file_name, SPIFFS_RDWR | SPIFFS_TRUNC, 0);
    if (0 > fd)
    {
        FSPIFFS_ERROR("failed to open file %s errno %d line %d", file_name, SPIFFS_errno(&instance.fs), __LINE__);
        return FSPIFFS_OPS_OPEN_FILE_FAILED;
    }

    int result = SPIFFS_write(&instance.fs, fd, (void *)str, wr_len);
    if (result < 0)
    {
        FSPIFFS_ERROR("failed to write file %s errno %d", file_name, SPIFFS_errno(&instance.fs));
        ret = FSPIFFS_OPS_WRITE_FILE_FAILED;
        goto err_exit;
    }

    /* check file status */
    static spiffs_stat status;
    memset(&status, 0, sizeof(status));
    result = SPIFFS_fstat(&instance.fs, fd, &status);
    if (result < 0)
    {
        FSPIFFS_ERROR("failed to get status of file %s errno %d", file_name, SPIFFS_errno(&instance.fs));
        ret = FSPIFFS_OPS_WRITE_FILE_FAILED;
        goto err_exit;
    }

    if (status.size != wr_len)
    {
        FSPIFFS_ERROR("file write size %ld != %ld", status.size, wr_len);
        ret = FSPIFFS_OPS_WRITE_FILE_FAILED;
        goto err_exit;
    }

    /* flush all pending writes from cache to flash */
    (void)SPIFFS_fflush(&instance.fs, fd);
    FSPIFFS_INFO("write file %s with %d bytes success !!!\r\n", file_name, wr_len);
err_exit:
    (void)SPIFFS_close(&instance.fs, fd);
    return ret; 
}

int FSpiffsOpsReadFile(const char *file_name, char *fspiffs_read_buf)
{
    FASSERT((file_name) && (strlen(file_name) > 0));
    int ret = FSPIFFS_OPS_OK;
    int result = SPIFFS_OK;

    if (FALSE == instance.fs_ready)
    {
        FSPIFFS_ERROR("please mount file system first !!!");
        return FSPIFFS_OPS_NOT_YET_MOUNT;
    }

    /* check file status */
    static spiffs_stat status;

    spiffs_flags open_flags = 0;

    /* open the file in read-only mode */
    open_flags = SPIFFS_RDWR;
    spiffs_file fd = SPIFFS_open(&instance.fs, file_name, open_flags, 0);
    if (0 > fd)
    {
        FSpiffsOpsListAll();
        FSPIFFS_ERROR("failed to open file %s errno %d  LINE %d", file_name, SPIFFS_errno(&instance.fs), __LINE__);
        return FSPIFFS_OPS_OPEN_FILE_FAILED;
    }

    /* check file status */
    memset(&status, 0, sizeof(status));
    result = SPIFFS_fstat(&instance.fs, fd, &status);
    if (result < 0)
    {
        FSPIFFS_ERROR("failed to get status of file %s errno %d", file_name, SPIFFS_errno(&instance.fs));
        ret = FSPIFFS_OPS_OPEN_FILE_FAILED;
        goto err_exit;
    }

    s32_t offset = SPIFFS_lseek(&instance.fs, fd, 0, SPIFFS_SEEK_END);
    if ((s32_t)status.size != offset)
    {
        FSPIFFS_ERROR("file %s spiffs:%ld! = fs:%ld", file_name, status.size, offset);
        ret = FSPIFFS_OPS_OPEN_FILE_FAILED;
        goto err_exit;
    }

    /* seek to offset and start read */
    if (0 > SPIFFS_lseek(&instance.fs, fd, 0, SPIFFS_SEEK_SET))
    {
        FSPIFFS_ERROR("seek file failed !!!");
        ret = FSPIFFS_OPS_READ_FILE_FAILED;
        goto err_exit;
    }

    /*vPrintf("read %s from position %ld\n", file_name, SPIFFS_tell(&instance.fs, fd));*/

    s32_t read_len = min((s32_t)FSPIFFS_RW_BUF_SIZE, (s32_t)status.size);
    s32_t read_bytes = SPIFFS_read(&instance.fs, fd, (void *)fspiffs_read_buf, read_len);
    if (read_bytes < 0)
    {
        FSPIFFS_ERROR("failed to read file %s errno %d", file_name, SPIFFS_errno(&instance.fs));
        ret = FSPIFFS_OPS_READ_FILE_FAILED;
        goto err_exit;
    }

    FSPIFFS_INFO("read %s success, str = %s\n", file_name, fspiffs_read_buf);        

err_exit : 
    /* close file */
    (void)SPIFFS_close(&instance.fs, fd);
    return ret;
}

static int FFreeRTOSSpimSpiffsInit(const char *file_name)
{
	int result = FSPIFFS_OPS_OK;
    static spiffs_config config;
    memset(&config, 0, sizeof(config));
    config = *FSpiffsGetDefaultConfig();
    config.phys_addr = FSPIFFS_START_ADDR; /* may use part of flash */
    config.phys_size = FSPIFFS_USE_SIZE;

    memset(&instance, 0, sizeof(instance));
    instance.fs_addr = FSPIFFS_START_ADDR;
    instance.fs_size = FSPIFFS_USE_SIZE;

    result = FSpiffsInitialize(&instance, FSPIFFS_PORT_TO_FSPIM);
    if (FSPIFFS_OPS_OK != result)
    {
        return FSPIFFS_OPS_INIT_FAILED;
    }

    result = FSpiffsOpsMount(FSPIFFS_IF_FORMAT, config);
    if(result != FSPIFFS_OPS_OK)
    {
        return result;

    }

    FSpiffsOpsCreateFile(file_name);
    if(result != FSPIFFS_OPS_OK)
    {
        return result;
    }

    FSpiffsOpsListAll();

    FSPIFFS_INFO("spiffs init success !!!\r\n");
    return result;

}

static void FFreeRTOSSpimSpiffsWRTask()
{
    const char *file_name = "test.txt";
    int ret = FSPIFFS_OPS_OK;
    u8 fspiffs_write_buf[FSPIFFS_RW_BUF_SIZE] = {0};
    u8 fspiffs_read_buf[FSPIFFS_RW_BUF_SIZE] = {0};
    char *string = "hello,freertos. spiffs spim write and read test";
    int i = 0;
    ret = FFreeRTOSSpimSpiffsInit(file_name);
    if(ret != FSPIFFS_OPS_OK)
    {
        FSPIFFS_ERROR("Spiffs init fail.");
        goto task_ret;
    }
    /* write and read */
    for(i = 0; i < WR_LOOP_TIMES; i++)
    {
        memset(fspiffs_read_buf, 0, FSPIFFS_RW_BUF_SIZE);
        memset(fspiffs_write_buf, 0, FSPIFFS_RW_BUF_SIZE);

        sprintf(fspiffs_write_buf, "%s-%d", string, i);
	    FSPIFFS_INFO( "write to %s, str = %s\n", file_name, fspiffs_write_buf);

        ret = FSpiffsOpsWriteFile(file_name, fspiffs_write_buf);
        if(ret != FSPIFFS_OPS_OK)
        {
            FSPIFFS_ERROR("Write file fail.");
            goto task_ret;
        }
        /*read after write*/
        ret = FSpiffsOpsReadFile(file_name, fspiffs_read_buf);
        if(ret != FSPIFFS_OPS_OK)
        {
            FSPIFFS_ERROR("Read file fail.");
            goto task_ret;
        }
        /*compare write data and read data*/
        if (0 != memcmp(fspiffs_read_buf, fspiffs_write_buf, FSPIFFS_RW_BUF_SIZE))
        {
            FSPIFFS_ERROR("times %d: spim spiffs write and read failed.\r\n\n",i);
            ret = FSPIFFS_OPS_READ_FILE_FAILED;
            goto task_ret;
        }
        else
        {
            printf("times %d: spim spiffs write and read pass.\r\n\n",i);
        }
    }
    ret = FSPIFFS_OPS_OK;
task_ret:
    FSpiffsDeInitialize(&instance);
    xQueueSend(xQueue,&ret,0);
    vTaskDelete(NULL);

}
int FFreeRTOSSpimSpiffsRunWR(void)
{
    BaseType_t xReturn = pdPASS;
    int task_ret;
    xQueue = xQueueCreate(1,sizeof(int));
    xReturn = xTaskCreate((TaskFunction_t )FFreeRTOSSpimSpiffsWRTask, /* 任务入口函数 */
                        (const char* )"FFreeRTOSSpimSpiffsWRTask",/* 任务名字 */
                        (uint16_t )4096, /* 任务栈大小 */
                        (void* )NULL,/* 任务入口函数参数 */
                        (UBaseType_t )configMAX_PRIORITIES-1, /* 任务的优先级 */
                        NULL); /* 任务控制 */

    xReturn = xQueueReceive(xQueue, &task_ret, WR_TIMER_PERIOD);
    FASSERT_MSG(pdPASS == xReturn, "xQueue Receive failed.\r\n");
    vQueueDelete(xQueue);
    if(task_ret != FSPIFFS_OPS_OK)
    {
        printf("%s@%d: Spim_spiffs wr example [failure].\r\n", __func__, __LINE__);
    }
    else
    {
        printf("%s@%d: Spim_spiffs wr example [success].\r\n", __func__, __LINE__);
    }
    return task_ret;

}