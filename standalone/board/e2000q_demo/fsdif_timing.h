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
 * @FilePath: fsdif_timing.c
 * @Date: 2023-07-25 11:29:44
 * @LastEditTime: 2023-07-25 11:29:44
 * @Description:  This file is for sdif timing function definition
 * 
 * @Modify History: 
 *  Ver   Who       Date       Changes
 * ----- ------  --------      --------------------------------------
 * 1.0   zhugengyu  2023/9/25   init commit
 * 
 */

#ifndef FSDIF_TIMING_H
#define FSDIF_TIMING_H

#include "fboard_port.h"
#include "ftypes.h"
#include "fsdif.h"

#ifdef __cplusplus
extern "C"
{
#endif

void FSdifTimingInit(void);
void FSdifTimingDeinit(void);

/* register this function in FSDIF driver to set timing */
FError FSdifGetTimingSetting(FSdifClkSpeed clock_freq, boolean non_removable, FSdifTiming *tuning);

#ifdef __cplusplus
}
#endif

#endif
