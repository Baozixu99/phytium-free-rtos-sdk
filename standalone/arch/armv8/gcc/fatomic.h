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
 * FilePath: fatomic.h
 * Date: 2022-03-08 21:56:42
 * LastEditTime: 2022-03-15 11:14:45
 * Description:  This file is for l3 cache-related operations
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   wangxiaodong    2023/6/6       first release
 */

#ifndef FATOMIC_H
#define FATOMIC_H

#ifdef __cplusplus
extern "C"
{
#endif

/************************** Function Prototypes ******************************/

/* data atomic add val, return initial data */
#define FATOMIC_ADD(data, val)  __sync_fetch_and_add(&(data), (val))
/* data atomic add 1, return initial data */
#define FATOMIC_INC(data)       FATOMIC_ADD(data, 1)

/* data atomic subtract val, return initial data */
#define FATOMIC_SUB(data, val)  __sync_fetch_and_sub(&(data), (val))
/* data atomic subtract 1, return initial data */
#define FATOMIC_DEC(data)       FATOMIC_SUB(data, 1)

/* data atomic or val, return initial data */
#define FATOMIC_OR(data, val)   __sync_fetch_and_or(&(data), (val))
/* data atomic xor val, return initial data */
#define FATOMIC_XOR(data, val)  __sync_fetch_and_xor(&(data), (val))

/* data atomic and val, return initial data */
#define FATOMIC_AND(data, val)  __sync_fetch_and_and(&(data), (val))
/* data atomic nand val, return initial data */
#define FATOMIC_NAND(data, val) __sync_fetch_and_nand(&(data), (val))

/* 
atomic compare data and cmpval
if not equal, return false
if equal, data = newval, return true 
*/
#define FATOMIC_CAS_BOOL(data, cmpval, newval) \
    __sync_bool_compare_and_swap(&(data), (cmpval), (newval))

/* 
atomic compare data and cmpval
if not equal, return data
if equal, data = newval, return initial data
*/
#define FATOMIC_CAS_VAL(data, cmpval, newval) \
    __sync_val_compare_and_swap(&(data), (cmpval), (newval))

/* full memory barrier */
#define FATOMIC_MEM_BARRIER(data) __sync_synchronize()

/* set data = val, and lock data */
#define FATOMIC_LOCK(data, val)   __sync_lock_test_and_set(&(data), val)

/* release data, set data = 0 */
#define FATOMIC_UNLOCK(data)      __sync_lock_release(&(data))

#ifdef __cplusplus
}
#endif

#endif
