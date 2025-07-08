/*
 * Copyright (C) 2024, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: lv_ui.h
 * Date: 2024-01-20 14:22:40
 * LastEditTime: 2024-02-20 15:40:40
 * Description:  This file is for providing the ui config
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------  -------- --------------------------------------
 *  1.0  Wangzq     2024/02/20  first add 
 */

#ifndef LV_UI_H
#define LV_UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

#include "lv_ui_helpers.h"
#include "lv_ui_events.h"
// SCREEN: ui_screen
void ui_screen_screen_init(lv_obj_t *parent);
signed char LSLvglShellRead(char *data);
void LSLvglShellWrite(char data);

extern lv_obj_t * ui_screen;
extern lv_obj_t * ui____initial_actions0;

LV_IMG_DECLARE(lv_logo);

void ui_init(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
