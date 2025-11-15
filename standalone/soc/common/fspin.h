/*
 * Copyright (C) 2024, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: fspin.h
 * Date: 2024-08-08 14:53:42
 * LastEditTime: 2024-08-08 17:58:18
 * Description:  This file is for spinlock function
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0   carl       2023-02-28   Use GCC built-in functions to implement spinlock
 *  1.1   wangxiaodong 2024-08-08   Use mcs lock to implement spinlock
 */


#ifndef COMMON_FSPIN_H
#define COMMON_FSPIN_H

#include "ftypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* mcs node struct */
typedef struct mcs_node
{
    struct mcs_node *next;
    volatile boolean locked;
} mcs_node_t;

/* mcs lock struct*/
typedef struct
{
    volatile mcs_node_t *tail;
    int is_ready;
} mcs_lock_t;

extern mcs_lock_t *mcs_lock_instance;

void FMcsLockInit(mcs_lock_t *lock);
void FMcsLock(mcs_lock_t *lock, mcs_node_t *node);
void FMcsUnlock(mcs_lock_t *lock, mcs_node_t *node);

#ifdef __cplusplus
}
#endif


#endif // COMMON_FSPIN_H