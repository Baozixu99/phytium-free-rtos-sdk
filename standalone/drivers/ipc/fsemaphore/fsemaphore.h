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
 * FilePath: fsemaphore.h
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 08:25:35
 * Description:  This file is for semaphore user api definition
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/5/23    init commit
 */


#ifndef FSEMAPHORE_H
#define FSEMAPHORE_H

/***************************** Include Files *********************************/

#include "ftypes.h"
#include "ferror_code.h"

#ifdef __cplusplus
extern "C"
{
#endif

/************************** Constant Definitions *****************************/
#define FSEMA_NUM_OF_LOCKER           32U
#define FSEMA_OWNER_NONE              0U

#define FSEMA_SUCCESS                 FT_SUCCESS
#define FSEMA_ERR_NOT_INIT            FT_MAKE_ERRCODE(ErrModBsp, ErrSema, 0U)
#define FSEMA_ERR_NO_AVAILABLE_LOCKER FT_MAKE_ERRCODE(ErrModBsp, ErrSema, 1U)
#define FSEMA_ERR_LOCK_TIMEOUT        FT_MAKE_ERRCODE(ErrModBsp, ErrSema, 2U)
#define FSEMA_ERR_NO_PERMISSION       FT_MAKE_ERRCODE(ErrModBsp, ErrSema, 3U)
/**************************** Type Definitions *******************************/
typedef struct
{
    u32 id;            /* Semaphore控制器id */
    uintptr base_addr; /* Semaphore控制器基地址 */
} FSemaConfig;         /* Semaphore控制器配置 */

typedef struct _FSema FSema;

typedef struct
{
    u32 index; /* Semaphore锁id */
#define FSEMA_LOCKER_NAME_LEN 32U
    char name[FSEMA_LOCKER_NAME_LEN]; /* Semaphore锁的名字 */
    u32 owner; /* Semaphore锁的拥有者, 当前持有锁的人, 如果没有上锁就标记FSEMA_OWNER_NONE */
    FSema *sema; /* Semaphore控制器实例 */
} FSemaLocker;   /* Semaphore锁实例 */

typedef struct _FSema
{
    FSemaConfig config; /* Semaphore控制器配置 */
    u32 is_ready;       /* Semaphore控制器初始化是否完成 */
    FSemaLocker *locker[FSEMA_NUM_OF_LOCKER]; /* Semaphore锁实例，locker[i] == NULL 表示锁尚未分配 */
} FSema;                                      /* Semaphore控制器实例 */

typedef void (*FSemaRelaxHandler)(FSema *const instance); /* 等待下一次上锁的relax函数 */
/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
/* 获取Semaphore的默认配置 */
const FSemaConfig *FSemaLoopkupConfig(u32 instance_id);

/* 初始化Semaphore控制器 */
FError FSemaCfgInitialize(FSema *const instance, const FSemaConfig *config);

/* 去初始化Semaphore控制器 */
void FSemaDeInitialize(FSema *const instance);

/* 分配和创建Semaphore锁 */
FError FSemaCreateLocker(FSema *const instance, FSemaLocker *const locker);

/* 强制解除Semaphore锁并删除锁实例 */
FError FSemaDeleteLocker(FSemaLocker *const locker);

/* 尝试获取指定Semaphore锁 */
FError FSemaTryLock(FSemaLocker *const locker, u32 owner, u32 try_times, FSemaRelaxHandler relax_handler);

/* 尝试释放指定Semaphore锁 */
FError FSemaUnlock(FSemaLocker *const locker, u32 owner);

/* 强制解除所有Semaphore锁 */
FError FSemaUnlockAll(FSema *const instance);

/* 检查指定Semaphore锁是否处于锁定状态 */
boolean FSemaIsLocked(FSemaLocker *locker);

#ifdef __cplusplus
}
#endif

#endif
