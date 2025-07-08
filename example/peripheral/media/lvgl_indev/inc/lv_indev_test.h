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
 * FilePath: lv_indev_test.h
 * Date: 2023-02-05 18:27:47
 * LastEditTime: 2023-07-06 11:02:47
 * Description:  This file is for providing the lvgl test config
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------  -------- --------------------------------------
 *  1.0  Wangzq     2023//07/06  Modify the format and establish the version
 *  1.1  Wangzq     2023/07/07  change the third-party and driver relation 
 */

#ifndef LV_INDEV_TEST_H
#define LV_INDEV_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "ftypes.h"


/*the lvgl demo*/
void lv_demo_indev(void);

/*deinit the media */
void FFreeRTOSMediaChannelDeinit(u32 id);

/*handle the hpd event*/
void FFreeRTOSMediaHpdHandle(void);

/*init the media */
void FFreeRTOSMediaDeviceInit(void);

/*lvgl config*/
void FFreeRTOSLVGLConfigTask(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif
#endif