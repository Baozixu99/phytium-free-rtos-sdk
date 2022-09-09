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
 * FilePath: qspi_example.c
 * Date: 2022-07-11 11:32:48
 * LastEditTime: 2022-07-11 11:32:48
 * Description:  This file is for 
 * 
 * Modify History: 
 *  Ver Who      Date        Changes
 * ----- ------     --------    --------------------------------------
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "fpinctrl.h"
#include "fsata.h"
#include "fsata_hw.h"
#include "timers.h"
#include "sata_fatfs_example.h"
#include "strto.h"
#include "ft_assert.h"
#include "ft_debug.h"
#include "parameters.h"

/* file system */
#include "ff.h"

#define FSATA_DEBUG_TAG "FATFS-SATA"
#define FSATA_ERROR(format, ...)   FT_DEBUG_PRINT_E(FSATA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSATA_WARN(format, ...)    FT_DEBUG_PRINT_W(FSATA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSATA_INFO(format, ...)    FT_DEBUG_PRINT_I(FSATA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSATA_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FSATA_DEBUG_TAG, format, ##__VA_ARGS__)

/* The periods assigned to the one-shot timers. */
#define ONE_SHOT_TIMER_PERIOD		( pdMS_TO_TICKS( 500000UL ) )

/* write and read task delay in milliseconds */
#define TASK_DELAY_MS 	3000UL

static xTaskHandle read_handle;
static xTaskHandle write_handle;
static TimerHandle_t xOneShotTimer;

static void FFreeRTOSSataFatfsDelete(void);

static xSemaphoreHandle xCountingSemaphore;

#define FSATA_FATFS_FILE_SIZE       128

#define FSATA_READ_LEN 32

static FATFS fatfs;
static boolean fatfs_ok = FALSE;
static BYTE work[FF_MAX_SS] __attribute__((aligned(32))) = {0};
static BYTE buff[FSATA_FATFS_FILE_SIZE] __attribute__((aligned(32))) = {0};
static char mount_path[256] = "";
const char *file_name = "test.txt";

int FatfsSataFormat(const char *mount_point)
{
    FRESULT res;

    printf("no file system, formatting ...\r\n");

    res = f_mkfs(mount_point, FM_EXFAT, 0, work, sizeof(work));
    if (res == FR_OK)
    {
        printf("format ok\r\n");
        res = f_mount(NULL, mount_point, 1);
        res = f_mount(&fatfs, mount_point, 1);
        if (FR_OK != res)
        {
            FSATA_ERROR("mount failed %d", res);
        }
    }
    else
    {
        FSATA_ERROR("format fail %d", res);
        return -1;
    }

    return 0;
}

FRESULT FatfsSataListFile(char *path)
{
	FRESULT res; 
	DIR dir;
	UINT i;
	static FILINFO fno;
	res = f_opendir(&dir,path); /* open dir */

	if(res == FR_OK) 
	{
		for(;;) /* traversal dir */
		{
			res = f_readdir(&dir, &fno); /* read dir */
			if(res != FR_OK || fno.fname[0] == 0) 
                break;
			if(fno.fattrib & AM_DIR) /* if it is dir */
			{
				i = strlen(path); /* get length of dir */
				sprintf(&path[i],"/%s",fno.fname); /* append dir */
				printf("is dir::%s\r\n",path);
				res = FatfsSataListFile(path);
				if(res != FR_OK) break;
				path[i] = 0; 
			}
            else
			{
				printf("is file:%s/%s\r\n",path,fno.fname); /* if it is file */
			}
		}
	}
	else
	{
		printf("failed - %s\r\n",&res);
	}

	f_closedir(&dir);
	return res;    
}

static int FatfsSataSetup(const char *mount_point)
{
    FASSERT(mount_point);
    FRESULT res;
    UINT fnum;
    FIL file_handler;

    if (TRUE == fatfs_ok)
    {
        FSATA_WARN("fatfs initialization already done !!!");
        return 0;
    }

    fatfs.pdrv = DEV_MMC;

    res = f_mount(&fatfs, mount_point, 1);
    printf("mount fatfs at %s, ret = %d\r\n", mount_point, res);
    if (res == FR_NO_FILESYSTEM)
    {
        if (0 != FatfsSataFormat(mount_point))
        {
            return -1;
        }
        else
        {
            res = FR_OK;
        }
    }
    else if (res != FR_OK)
    {
        FSATA_ERROR("file system mount fail %d", res);
        return -2;
    }
    else
	{
		printf("file system mount ok\r\n");
	}

    if (FR_OK == res)
    {
        fatfs_ok = TRUE;
        (void)FatfsSataListFile(mount_path);
    }
    
    return 0;
}

int FatfsSataWriteFile(const char *file_name, const char *input_str, boolean create, size_t off)
{
    FASSERT(file_name);
	FRESULT res;
	UINT fnum;
    FIL file_handler;
    BYTE temp;
    int ret = 0;

    if (FALSE == fatfs_ok)
    {
        printf("please setup fatfs first !!!\n");
        return -1;
    }

    if (create)
        res = f_open(&file_handler, file_name, FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
    else
        res = f_open(&file_handler, file_name, FA_WRITE | FA_READ);

    if (res == FR_OK)
    {        
        f_lseek(&file_handler, off);

        if (NULL == input_str)
        {
            memset(buff, 0x0, sizeof(buff));
            res = f_write(&file_handler, buff, sizeof(buff), &fnum); 
        }
        else
        {
            res = f_write(&file_handler, input_str, strlen(input_str) + 1, &fnum);
        }

        if (res != FR_OK)
        {
            printf("write %s fail\n", file_name);
            ret = -3;
            goto err_handle;            
        }             
    }
    else
    {
        printf("f_open %s fail %d\n", file_name, res);
        return -4; /* no file handler, directly return */
    }

err_handle:
    getchar();
    res = f_close(&file_handler);
    if (FR_OK != res)
    {
        printf("close file %s failed !!!\n", file_name);
        ret = -6;
    }

    return ret;
}

int FatfsSataReadFile(const char *file_name, size_t len)
{
    FASSERT(file_name);
    FRESULT res;
    UINT fnum;
    FIL file_handler;

    if (FALSE == fatfs_ok)
    {
        printf("please setup fatfs first !!!\n");
        return -1;
    }

    if (len > sizeof(buff))
    {
        printf("length %d is not support\n", len);
        return -2;
    }

    res = f_open(&file_handler, file_name, FA_READ);
    if (res != FR_OK)
    {
        printf("open file %s failed !!!\n", file_name);
        return -3;
    }

    res = f_lseek(&file_handler, 0);
    if (res != FR_OK)
    {
        printf("seek to file begin failed !!!\n");
        return -4;
    }    

    memset(buff, 0, sizeof(buff));
    res = f_read(&file_handler, buff, len, &fnum);
    if (res != FR_OK)
    {
        printf("read file %s failed !!!\n", file_name);
        return -5;
    }
    else
    {
        printf("read %s success, str = %s\n\n", file_name, buff);
	}

    return 0;    
}

static void FFreeRTOSSataFatfsReadTask(void *pvParameters)
{
	const char *pcTaskName = "FFreeRTOSSataFatfsReadTask is running\r\n";
	const TickType_t xDelay = pdMS_TO_TICKS(TASK_DELAY_MS);
	FError ret = FT_SUCCESS;

    xSemaphoreTake(xCountingSemaphore, portMAX_DELAY);

	/* As per most tasks, this task is implemented in an infinite loop. */
	for( ;; )
	{
		/* Print out the name of this task. */
		// printf( pcTaskName );

		FatfsSataReadFile(file_name, FSATA_READ_LEN);

		/* Delay for a period.  This time a call to vTaskDelay() is used which
		places the task into the Blocked state until the delay period has
		expired.  The parameter takes a time specified in 'ticks', and the
		pdMS_TO_TICKS() macro is used (where the xDelay constant is
		declared) to convert TASK_DELAY_MS milliseconds into an equivalent time in
		ticks. */
		vTaskDelay(xDelay);
	}
}

static void FFreeRTOSSataFatfsWriteTask(void *pvParameters)
{
	const char *pcTaskName = "FFreeRTOSSataFatfsWriteTask is running\r\n";
	const TickType_t xDelay = pdMS_TO_TICKS(TASK_DELAY_MS);
	FError ret = FT_SUCCESS;
	const char *string = "write to test.txt.";
	char string_out[FSATA_READ_LEN] = {0};
	static int i = 0;

    xSemaphoreTake(xCountingSemaphore, portMAX_DELAY);
    
    /* As per most tasks, this task is implemented in an infinite loop. */
    for (;;)
    {
		/* Print out the name of this task. */
		// printf( pcTaskName );
		i++;
		sprintf(string_out, "%s-%d", string, i);
		printf( "write to %s, str = %s\n", file_name, string_out);
		FatfsSataWriteFile(file_name, string_out, FALSE, 0);

		/* Delay for a period.  This time a call to vTaskDelay() is used which
		places the task into the Blocked state until the delay period has
		expired.  The parameter takes a time specified in 'ticks', and the
		pdMS_TO_TICKS() macro is used (where the xDelay constant is
		declared) to convert TASK_DELAY_MS milliseconds into an equivalent time in
		ticks. */
		vTaskDelay(xDelay);
	}
}

static void prvOneShotTimerCallback( TimerHandle_t xTimer )
{
	/* Output a string to show the time at which the callback was executed. */
	vPrintf( "One-shot timer callback executing, delete SataFatfs ReadTask and WriteTask.\r\n" );

	FFreeRTOSSataFatfsDelete();
}

static void FFreeRTOSSataFatfsInitTask(void *pvParameters)
{
	const char *pcTaskName = "FFreeRTOSSataFatfsInitTask is running\r\n";
	const TickType_t xDelay = pdMS_TO_TICKS(TASK_DELAY_MS);
	FError ret = FT_SUCCESS;

    const char *string = NULL;
    const char *mount_point = "0:/";
    
    /* setup sata */
	FatfsSataSetup(mount_point);

	/* create a file*/
	FatfsSataWriteFile(file_name, NULL, TRUE, 0);

    xSemaphoreGive(xCountingSemaphore);
    xSemaphoreGive(xCountingSemaphore);

    vTaskDelete(NULL);

}

BaseType_t FFreeRTOSSataFatfsCreate(void)
{
    BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为 pdPASS */
	BaseType_t xTimerStarted = pdPASS;
    
    xCountingSemaphore = xSemaphoreCreateCounting(2, 0);
    if (xCountingSemaphore == NULL)
	{
		printf("FFreeRTOSSataFatfsCreate xCountingSemaphore create failed.\r\n" );
        return pdFAIL;
    }

	taskENTER_CRITICAL(); /*进入临界区*/
	
    xReturn = xTaskCreate((TaskFunction_t )FFreeRTOSSataFatfsInitTask, /* 任务入口函数 */
                            (const char* )"FFreeRTOSSataFatfsInitTask",/* 任务名字 */
                            (uint16_t )4096, /* 任务栈大小 */
                            (void* )NULL,/* 任务入口函数参数 */
                            (UBaseType_t )2, /* 任务的优先级 */
                            NULL); /* 任务控制 */

	xReturn = xTaskCreate((TaskFunction_t )FFreeRTOSSataFatfsReadTask, /* 任务入口函数 */
                            (const char* )"FFreeRTOSSataFatfsReadTask",/* 任务名字 */
                            (uint16_t )4096, /* 任务栈大小 */
                            (void* )NULL,/* 任务入口函数参数 */
                            (UBaseType_t )configMAX_PRIORITIES-1, /* 任务的优先级 */
                            (TaskHandle_t* )&read_handle); /* 任务控制 */

    xReturn = xTaskCreate((TaskFunction_t )FFreeRTOSSataFatfsWriteTask, /* 任务入口函数 */
                            (const char* )"FFreeRTOSSataFatfsWriteTask",/* 任务名字 */
                            (uint16_t )4096, /* 任务栈大小 */
                            (void* )NULL,/* 任务入口函数参数 */
                            (UBaseType_t )configMAX_PRIORITIES-1, /* 任务的优先级 */
                            (TaskHandle_t* )&write_handle); /* 任务控制 */

	/* Create the one shot software timer, storing the handle to the created
	software timer in xOneShotTimer. */
	xOneShotTimer = xTimerCreate( "OneShot Software Timer",		/* Text name for the software timer - not used by FreeRTOS. */
								  ONE_SHOT_TIMER_PERIOD,		/* The software timer's period in ticks. */
								  pdFALSE,						/* Setting uxAutoRealod to pdFALSE creates a one-shot software timer. */
								  0,							/* This example does not use the timer id. */
								  prvOneShotTimerCallback );	/* The callback function to be used by the software timer being created. */

	/* Check the timers were created. */
	if( xOneShotTimer != NULL )
	{
		/* Start the software timers, using a block time of 0 (no block time).
		The scheduler has not been started yet so any block time specified here
		would be ignored anyway. */
		xTimerStarted = xTimerStart( xOneShotTimer, 0 );
		
		/* The implementation of xTimerStart() uses the timer command queue, and
		xTimerStart() will fail if the timer command queue gets full.  The timer
		service task does not get created until the scheduler is started, so all
		commands sent to the command queue will stay in the queue until after
		the scheduler has been started.  Check both calls to xTimerStart()
		passed. */
		if( xTimerStarted != pdPASS)
		{
			vPrintf("CreateSoftwareTimerTasks xTimerStart failed \r\n");
		}
	}
	else
	{
		vPrintf("CreateSoftwareTimerTasks xTimerCreate failed \r\n");
	}

	taskEXIT_CRITICAL(); 	
                            
    return xReturn;
}

static void FFreeRTOSSataFatfsDelete(void)
{
	BaseType_t xReturn = pdPASS;
    
	if(read_handle)
    {
        vTaskDelete(read_handle);
        vPrintf("Delete FFreeRTOSSataFatfsReadTask success\r\n");
    }

    if(write_handle)
    {
        vTaskDelete(write_handle);
        vPrintf("Delete FFreeRTOSSataFatfsWriteTask success\r\n");
    }

    /* delete count sem */
	vSemaphoreDelete(xCountingSemaphore);
	
	/* delete timer */
	xReturn = xTimerDelete(xOneShotTimer, 0);
	if(xReturn != pdPASS)
	{
		vPrintf("OneShot Software Timer Delete failed.\r\n");
	}
	else
	{
		vPrintf("OneShot Software Timer Delete success.\r\n");
	}

}



