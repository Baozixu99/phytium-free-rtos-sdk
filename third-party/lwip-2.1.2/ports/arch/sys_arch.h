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
 * FilePath: sys_arch.h
 * Date: 2022-07-18 13:25:02
 * LastEditTime: 2022-07-18 13:25:02
 * Description:  This file is for the lwIP TCP/IP stack.
 *
 * Modify History:
 *  Ver   Who         Date       Changes
 * ----- ------      --------    --------------------------------------
 * 1.0   liuzhihong  2022/5/26  first release
 */

#ifndef SYS_ARCH_H
#define SYS_ARCH_H

#include "sdkconfig.h"
#include "lwipopts.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "cc.h"


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
typedef s8_t err_t;
typedef struct _sys_arch_state_t
{
    // Task creation data.
    char cTaskName[configMAX_TASK_NAME_LEN];
    unsigned short nStackDepth;
    unsigned short nTaskCount;
} sys_arch_state_t;

/* Message queue constants. */
void sys_thread_delete(sys_thread_t handle);
void sys_arch_delay(const unsigned int msec);

err_t sys_countingsem_create(sys_sem_t *sem,u32 semaphore_maxcount,u32 semaphore_initialcount);

sys_prot_t sys_arch_protect(void);
void sys_arch_unprotect(sys_prot_t pval);

#ifdef __cplusplus
}
#endif

#endif /* __SYS_ARCH_H__ */
