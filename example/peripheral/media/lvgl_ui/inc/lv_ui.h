/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc.
 * All Rights Reserved.
 *
 * This program is OPEN SOURCE software: you can redistribute it and/or modify it
 * under the terms of the Phytium Public License as published by the Phytium Technology Co.,Ltd,
 * either version 1.0 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the Phytium Public License for more details.
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
