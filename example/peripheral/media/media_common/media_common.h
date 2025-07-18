/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: media_common.h
 * Date: 2022-08-25 16:22:40
 * LastEditTime: 2022-07-07 15:40:40
 * Description:  This file is for defining the config and  functions
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------  -------- --------------------------------------
 * 1.0  Wangzq     2022/12/20  Modify the format and establish the version
 */

#ifndef MEDIA_COMMON_H
#define MEDIA_COMMON_H

#include "ftypes.h"
#include "fdcdp.h"
#include "ferror_code.h"
#include "fmedia_os.h"

#ifdef __cplusplus
extern "C"
{
#endif
/*hpd task*/
void FFreeRTOSMediaHpdTask(FFreeRTOSMedia *os_media);
/*init the media*/
FError FMediaInitTask(FFreeRTOSMedia *os_media);
/*deinit the media*/
void FFreeRTOSMediaChannelDeinit(FFreeRTOSMedia *os_media);

#ifdef __cplusplus
}
#endif

#endif // !