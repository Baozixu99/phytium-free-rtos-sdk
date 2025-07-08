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
 * FilePath: lv_port_disp.h
 * Date: 2022-09-05 17:38:05
 * LastEditTime: 2023-07-07  12:11:05
 * Description:  This file is for providing the interface of lvgl test
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------  -------- --------------------------------------
 *  1.0  Wangzq     2023/03/20  Modify the format and establish the version
 *  1.1  Wangzq     2023/07/07  change the third-party and driver relation 
 * 
 */
/**
 * @file lv_port_disp.h
 *
 */

/*Copy this file as "lv_port_disp.h" and set this value to "1" to enable content*/

#ifndef LV_PORT_DISP_H
#define LV_PORT_DISP_H

#define LV_HOR_RES_MAX (800)
#define LV_VER_RES_MAX (600)

#include "ftypes.h"
#include "fparameters.h"
#include "fdcdp.h"
#ifdef __cplusplus
extern "C"
{
#endif

/*********************
 *      INCLUDES
 *********************/
#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
#include "lvgl.h"
#else
#include "lvgl-8.3/lvgl.h"
#endif


/**********************
 * GLOBAL PROTOTYPES
 **********************/
/* Enable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void FFreeRTOSDispdEnableUpdate(void);

/* Disable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void FFreeRTOSDispdDisableUpdate(void);

/* Framebuffer config*/
void FMediaDispFramebuffer(FDcDp *instance);

/*init the lv config and set the instance*/
void FFreeRTOSPortInit(void);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_PORT_DISP_TEMPL_H*/

