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
 * FilePath: fcache.h
 * Date: 2022-02-10 14:53:41
 * LastEditTime: 2022-02-17 17:29:29
 * Description:  This file is for This file is for the arm cache functionality.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe     2021/7/3     first release
 */


#ifndef ARCH_AARCH32_CACHE_H
#define ARCH_AARCH32_CACHE_H

#include "ftypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*  Dcache */
void FCacheDCacheEnable(void);
void FCacheDCacheDisable(void);
void FCacheDCacheInvalidate(void);
void FCacheDCacheInvalidateLine(intptr adr);
void FCacheDCacheInvalidateRange(intptr adr, intptr len);
void FCacheDCacheFlush(void);
void FCacheDCacheFlushLine(intptr adr);
void FCacheDCacheFlushRange(intptr adr, intptr len);

/*  Icache */
void FCacheICacheEnable(void);
void FCacheICacheDisable(void);
void FCacheICacheInvalidate(void);
void FCacheICacheInvalidateLine(u32 adr);
void FCacheICacheInvalidateRange(intptr adr, u32 len);
void FCacheICacheFlush(void);

#ifdef __cplusplus
}
#endif

#endif