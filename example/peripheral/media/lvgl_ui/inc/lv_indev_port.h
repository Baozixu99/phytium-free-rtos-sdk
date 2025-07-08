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
 * FilePath: lv_indev_port.h
 * Date: 2022-04-20 14:22:40
 * LastEditTime: 2023-07-06 15:40:40
 * Description:  This file is for providing the indev config
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------  -------- --------------------------------------
 *  1.0  Wangzq     2023/07/06  add the device
 *  1.1  Wangzq     2023/07/07  change the third-party and driver relation 
 */

#ifndef LV_INDEV_PORT_H
#define LV_INDEV_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "ftypes.h"


/**********************
 * GLOBAL PROTOTYPES
 **********************/
void lv_port_kb_init(u32 id);

void lv_port_ms_init(u32 id);
/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif


#endif /*Disable/Enable content*/
