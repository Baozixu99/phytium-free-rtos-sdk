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
 * FilePath: lv_port_disp.h
 * Date: 2023-02-05 18:27:47
 * LastEditTime: 2023-02-10 11:02:47
 * Description:  This file is for providing the lvgl config
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------  -------- --------------------------------------
 *  1.0  Wangzq     2023/02/10  Modify the format and establish the version
 */

/**
 * @file lv_port_disp.h
 *
 */

#ifndef LV_PORT_DISP_H
#define LV_PORT_DISP_H

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
#ifdef CONFIG_USE_FMEDIA_V1
#include "fdcdp.h"
#endif

#ifdef CONFIG_USE_FMEDIA_V2
#include "fdcdp_v2.h"
#endif


/* Initialize low level display driver */
void FMediaLvgldispInit();
/* Enable updating the screen (the flushing process) when disp_flush() is called by LVGL*/

void FMediaDispFramebuffer(FDcDp *instance);

void disp_enable_update(void);

/* Disable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_disable_update(void);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_PORT_DISP_TEMPL_H*/
