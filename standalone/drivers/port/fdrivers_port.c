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
 * FilePath: fdrivers_port.c
 * Created Date: 2023-10-17 08:29:18
 * Last Modified: 2025-07-17 14:29:00
 * Description:  This file is for drive layer code decoupling
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 * 1.0     huanghe    2023/10/17    first release
 */
#include "fdrivers_port.h"

#include "fcache.h"
#include "fsleep.h"
#include "fgeneric_timer.h"
#include "fparameters.h"

/* cache */
void FDriverDCacheRangeFlush(uintptr_t adr, size_t len)
{
    FCacheDCacheFlushRange(adr, len);
}

void FDriverDCacheRangeInvalidate(uintptr_t adr, size_t len)
{
    FCacheDCacheInvalidateRange(adr, len);
}


void FDriverICacheRangeInvalidate(void)
{
    FCacheICacheInvalidate();
}


/* time delay */

void FDriverUdelay(u32 usec)
{
    fsleep_microsec(usec);
}

void FDriverMdelay(u32 msec)
{
    fsleep_millisec(msec);
}

void FDriverSdelay(u32 sec)
{
    fsleep_seconds(sec);
}

u64 FDriverGetTimerTick(void)
{
    return GenericTimerRead(GENERIC_TIMER_ID0);
}
