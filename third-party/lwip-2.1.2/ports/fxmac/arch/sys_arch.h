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
 * FilePath: sys_arch.h
 * Date: 2022-07-18 13:25:02
 * LastEditTime: 2022-07-18 13:25:02
 * Description:  This file is for 
 * 
 * Modify History: 
 *  Ver   Who  Date   Changes
 * ----- ------  -------- --------------------------------------
 */

#ifndef __SYS_ARCH_H__
#define __SYS_ARCH_H__


#if !NO_SYS

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define SYS_MBOX_NULL (xQueueHandle)0
#define SYS_SEM_NULL (xSemaphoreHandle)0
#define SYS_DEFAULT_THREAD_STACK_DEPTH configMINIMAL_STACK_SIZE

typedef xSemaphoreHandle sys_sem_t;
typedef xSemaphoreHandle sys_mutex_t;
typedef xQueueHandle sys_mbox_t;
typedef xTaskHandle sys_thread_t;

typedef struct _sys_arch_state_t
{
    // Task creation data.
    char cTaskName[configMAX_TASK_NAME_LEN];
    unsigned short nStackDepth;
    unsigned short nTaskCount;
} sys_arch_state_t;

/* Message queue constants. */
#define archMAX_MESG_QUEUE_LENGTH (12)

#ifdef __cplusplus
}
#endif

#endif

#endif /* __SYS_ARCH_H__ */
