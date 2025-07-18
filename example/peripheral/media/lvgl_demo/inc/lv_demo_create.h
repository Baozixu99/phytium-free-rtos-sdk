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
 * FilePath: lv_demo_create.h
 * Date: 2023-02-05 18:27:47
 * LastEditTime: 2023-07-07 11:02:47
 * Description:  This file is for providing the lvgl demo config
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------  -------- --------------------------------------
 *  1.0  Wangzq     2023/03/20  Modify the format and establish the version
 *  1.1  Wangzq     2023/07/07  change the third-party and driver relation 
 */


#ifndef LV_DEMO_CREATE_H
#define LV_DEMO_CREATE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
#include "lvgl.h"
#else
#include "lvgl-8.3/lvgl.h"
#endif

/*create the media demo init task*/
BaseType_t FFreeRTOSlVGLDemoCreate(void);

/*create the media init task*/
BaseType_t FFreeRTOSMediaInitCreate(void );

BaseType_t FFreeRTOSMediaDeinit(void);

/*create the lvgl config task*/
BaseType_t FFreeRTOSlVGLConfigCreate(void);

#if LV_USE_DEMO_BENCHMARK
/*the benchmark demo of lvgl*/
void benchmark(void);
#endif

#if LV_USE_DEMO_WIDGETS
/*the widgets demo*/
void widgets(void);
#endif

#if LV_USE_DEMO_STRESS
/*the stress demo*/
void stress(void);
#endif

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_PORT_DISP_TEMPL_H*/