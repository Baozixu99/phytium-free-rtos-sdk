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
 * FilePath: fmedia_os.h
 * Date: 2022-08-24 16:42:19
 * LastEditTime: 2022-08-26 17:59:12
 * Description:  This file is for defining the media config and function
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  Wangzq     2022/12/20  Modify the format and establish the version
 */

#ifndef FMEDIA_OS_H
#define FMEDIA_OS_H

#include <FreeRTOS.h>
#include <semphr.h>
#include "fdcdp.h"
#include "ftypes.h"
#include "fparameters.h"
#include "event_groups.h"

#ifdef __cplusplus
extern "C"
{
#endif


/* freertos media interrupt priority */
#define FREERTOS_MEDIA_IRQ_PRIORITY IRQ_PRIORITY_VALUE_14

typedef struct
{
    FDcDp dcdp_ctrl;
    EventGroupHandle_t media_event;
} FFreeRTOSMedia;

/*init the media and return the meidia instance*/
FFreeRTOSMedia *FFreeRTOSMediaHwInit(FFreeRTOSMedia *instance, u32 index, u32 width, u32 height);


#ifdef __cplusplus
}
#endif

#endif