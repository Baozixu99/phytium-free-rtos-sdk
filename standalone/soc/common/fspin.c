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
 * FilePath: fspin.c
 * Date: 2024-08-08 14:53:42
 * LastEditTime: 2024-08-08 17:58:14
 * Description:  This files is for a way to provide spinlocks for multicore operations
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.1   carl      2022-12-30   add init function
 * 1.2   carl      2023-02-28   Use GCC built-in functions to implement spinlock 
 * 1.3   wangxiaodong 2024-08-08   Use mcs lock to implement spinlock
 */
#include <stdint.h>
#include <stdatomic.h>
#include "fspin.h"
#include "sdkconfig.h"
#include "ftypes.h"
#include "fatomic.h"

mcs_lock_t *mcs_lock_instance = NULL;

/* init mcs lock */
void FMcsLockInit(mcs_lock_t *lock)
{
    if (mcs_lock_instance != NULL)
    {
        return;
    }
    mcs_lock_instance = lock;
    if (atomic_load(&mcs_lock_instance->is_ready) == 1)
    {
        return;
    }
    atomic_store(&mcs_lock_instance->tail, NULL);
    atomic_store(&mcs_lock_instance->is_ready, 1);
}

/* mcs lock */
void FMcsLock(mcs_lock_t *lock, mcs_node_t *node)
{
    if (lock == NULL)
    {
        return;
    }

    if (atomic_load(&lock->is_ready) == 0)
    {
        return;
    }

    volatile mcs_node_t *prev;
    node->next = NULL;              /*初始化节点的next指针*/
    atomic_store(&node->locked, 1); /*将节点的locked标志设置为1，表示锁定*/

    prev = atomic_exchange(&lock->tail, node); /*将当前节点设置为新的尾部节点，并返回之前的尾部节点*/
    if (prev != NULL)
    {
        atomic_store(&prev->next, node); /*将之前的尾部节点的next指针指向当前节点*/
        while (atomic_load(&node->locked)) /*自旋等待，直到当前节点的locked标志被前驱节点清除*/
        {
            /*busy wait*/
        }
    }
}

/* mcs unlock */
void FMcsUnlock(mcs_lock_t *lock, mcs_node_t *node)
{
    if (lock == NULL)
    {
        return;
    }

    if (atomic_load(&lock->is_ready) == 0)
    {
        return;
    }

    mcs_node_t *next = atomic_load(&node->next);
    if (next == NULL) /*如果没有后继节点，尝试将tail设置为NULL*/
    {
        /*尝试将tail指针设置为NULL，表示锁已空闲*/
        if (atomic_compare_exchange_strong(&lock->tail, &node, NULL))
        {
            return;
        }
        /*等待后继节点被设置*/
        while ((next = atomic_load(&node->next)) == NULL)
        {
            /*busy wait*/
        }
    }

    /*清除下一个节点的locked标志，释放锁*/
    atomic_store(&next->locked, 0);
}
