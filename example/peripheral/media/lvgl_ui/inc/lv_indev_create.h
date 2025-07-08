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
 * FilePath: lv_indev_create.h
 * Date: 2023-02-05 18:27:47
 * LastEditTime: 2023-07-07 11:02:47
 * Description:  This file is for providing the lvgl indev task
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------  -------- --------------------------------------
 *  1.0  Wangzq     2023/03/20  Modify the format and establish the version
 *  1.1  Wangzq     2023/07/07  change the third-party and driver relation 
 */


#ifndef LV_INDEV_CREATE_H
#define LV_INDEV_CREATE_H

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
BaseType_t FFreeRTOSDemoCreate(void);

/*init the keyboard*/
BaseType_t FFreeRTOSInitKbCreate(u32 id);

/*init the mouse*/
BaseType_t FFreeRTOSInitMsCreate(u32 id);

/*list the usb device*/
BaseType_t FFreeRTOSListUsbDev(int argc, char *argv[]);

/*init the media*/
BaseType_t FFreeRTOSMediaInitCreate(void);

/*set the lvgl init task*/
BaseType_t FFreeRTOSlVGLConfigCreate(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif