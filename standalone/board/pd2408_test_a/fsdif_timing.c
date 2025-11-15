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
* FilePath: fsdif_timing.c
* Date: 2023-07-25 14:53:42
* LastEditTime: 2023-07-25 08:25:29
* Description:  This file is for sdif timing function
*
* Modify History:
*  Ver   Who        Date       Changes
* ----- ------     --------    --------------------------------------
* 1.0   zhugengyu  2023/9/25   init commit
*/

#include "fparameters.h"
#include "fio_mux.h"
#include "fsdif_timing.h"

void FSdifTimingInit(void)
{
    FIOMuxInit();
    FIOPadSetSdifMuxDrvStrength();
}

void FSdifTimingDeinit(void)
{
    FIOMuxDeInit();
}