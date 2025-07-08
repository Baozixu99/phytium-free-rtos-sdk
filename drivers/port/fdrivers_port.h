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

#ifdef __cplusplus
}
#endif


#endif
