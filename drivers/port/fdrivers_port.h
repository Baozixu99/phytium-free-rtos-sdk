/*
 * Copyright (C) 2023, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: fdrivers_port.h
 * Created Date: 2023-10-16 17:02:35
 * Last Modified: 2023-11-21 17:03:55
 * Description:  This file is for drive layer code decoupling
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 *  1.0     huanghe    2023/10/17    first release
 */

#ifndef FDRIVERS_PORT_H
#define FDRIVERS_PORT_H

#include "ftypes.h"
#include "faarch.h"
#include "fio.h"
#include "fassert.h"
#include "fkernel.h"
#include "fdebug.h"
#include "fgeneric_timer.h"
#include "sdkconfig.h"

/***************************** Include Files *********************************/
#ifdef __cplusplus
extern "C"
{
#endif

/* cache */
void FDriverDCacheRangeFlush(uintptr_t adr,size_t len);

void FDriverDCacheRangeInvalidate(uintptr_t adr,size_t len);

void FDriverICacheRangeInvalidate(void);


/* memory barrier */

#define FDRIVER_DSB() DSB()

#define FDRIVER_DMB() DMB()

#define FDRIVER_ISB() ISB()

/* time delay */

void FDriverUdelay(u32 usec);

void FDriverMdelay(u32 msec);

void FDriverSdelay(u32 sec);

#ifndef FT_DEBUG_PRINT_I
#define FT_DEBUG_PRINT_I(TAG, format, ...)
#endif

#ifndef FT_DEBUG_PRINT_I
#define FT_DEBUG_PRINT_E(TAG, format, ...)
#endif

#ifndef FT_DEBUG_PRINT_I
#define FT_DEBUG_PRINT_D(TAG, format, ...)
#endif

#ifndef FT_DEBUG_PRINT_W
#define FT_DEBUG_PRINT_W(TAG, format, ...)
#endif

#ifndef FT_DEBUG_PRINT_V
#define FT_DEBUG_PRINT_V(TAG, format, ...)
#endif

u64 FDriverGetTimerTick(void);


/**
 * @name: TICKS_TO_SECONDS
 * @msg:  将tick数转换为秒（整数除法，向下取整）
 * @param {u64} tick, tick计数值
 * @return {u64} 对应的秒数
 */
#define TICKS_TO_SECONDS() ((u64)(GenericTimerRead(0)) / GenericTimerFrequecy())

/**
 * @name: TICKS_TO_MILLISECONDS
 * @msg:  将tick数转换为毫秒（避免乘法溢出的安全计算）
 * @param {u64} tick, tick计数值
 * @return {u64} 对应的毫秒数
 */
#define TICKS_TO_MILLISECONDS()                           \
    ({                                                    \
        u64 __freq = GenericTimerFrequecy();              \
        u64 __quot = (u64)(GenericTimerRead(0)) / __freq; \
        u64 __rem = (u64)(GenericTimerRead(0)) % __freq;  \
        (__quot * 1000) + (__rem * 1000) / __freq;        \
    })

/**
 * @name: MSEC_TO_TICKS
 * @msg:  将毫秒转换为定时器 tick 值（向下取整）
 * @param {u64} ms, 毫秒时间值
 * @return {u64} 对应的定时器 tick 值
 */
#define MSEC_TO_TICKS(ms) ((u64)(((ms)*GenericTimerFrequecy()) / 1000))

/**
 * @name: SEC_TO_TICKS
 * @msg:  将秒转换为定时器 tick 值
 * @param {u64} s, 秒时间值
 * @return {u64} 对应的定时器 tick 值
 */
#define SEC_TO_TICKS(s)   ((u64)((s)*GenericTimerFrequecy()))

/**
 * @name: USEC_TO_TICKS
 * @msg:  将微秒转换为定时器 tick 值（向下取整）
 * @param {u64} us, 微秒时间值
 * @return {u64} 对应的定时器 tick 值
 */
#define USEC_TO_TICKS(us) ((u64)(((us)*GenericTimerFrequecy()) / 1000000))



#ifdef __cplusplus
}
#endif


#endif
