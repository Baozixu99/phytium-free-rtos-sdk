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
 * FilePath: media_example.h
 * Date: 2022-08-25 16:22:40
 * LastEditTime: 2022-07-07 15:40:40
 * Description:  This file is for defining the config and  functions
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------  -------- --------------------------------------
 * 1.0  Wangzq     2022/12/20  Modify the format and establish the version
 * 1.1  Wangzq     2023/07/07  change the third-party and driver relation 
 */

#ifndef MEDIA_EXAMPLE_H
#define MEDIA_EXAMPLE_H

#include "ftypes.h"
#include "fdcdp.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
    u8 Blue;
    u8 Green;
    u8 Red;
    u8 reserve;
} GraphicsTest;

/*create the media demo*/
FError FMediaDisplayDemo(void);

/*create the media init task*/
BaseType_t FFreeRTOSMediaCreate(void);

/*deinit the media*/
void FFreeRTOSMediaChannelDeinit(u32 id);

#ifdef __cplusplus
}
#endif

#endif // !