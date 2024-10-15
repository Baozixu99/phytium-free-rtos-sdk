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
 * FilePath: lv_ui_screen.c
 * Date: 2024-01-20 14:22:40
 * LastEditTime: 2024-02-20 15:40:40
 * Description:  This file is for providing the ui screen
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------  -------- --------------------------------------
 *  1.0  Wangzq     2024/02/20  first add
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>


#include "finterrupt.h"
#include "fpl011_os.h"
#include "fdebug.h"
#include "fassert.h"
#include "fparameters.h"
#include "fpl011_hw.h"
#include "fearly_uart.h"

#include "lv_ui.h"
#include "lvgl.h"

LV_FONT_DECLARE(USER_FONT);
void lvgl_shell(void);
static void text_receive_event_cb(lv_event_t *e);
extern lv_indev_t *indev_keypad;
static lv_obj_t *tv;
lv_obj_t *ui_text_receive_message;
lv_obj_t *ui_image_logo;


static char data[64];
#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
#include "shell_port.h"
extern Shell shell_object;
/**
 * @brief LVGL用户shell写
 *
 * @param data 数据
 */
void LSLvglShellWrite(char data)
{
    lv_textarea_add_char(ui_text_receive_message, data);
    lv_obj_set_style_text_font(ui_text_receive_message,&USER_FONT,0);
}

signed char LSLvglShellRead(char *data)
{
    lv_textarea_add_text(ui_text_receive_message, data);
    return 0;
}

void lvgl_shell(void)
{
    FInitializePrintf(&LSLvglShellWrite); 
    shell_object.write = LSLvglShellWrite;
    shell_object.read = LSLvglShellRead;
    shell_object.echo = 0;
}


static void text_receive_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *ta = lv_event_get_target(e);
    lv_indev_t *indev = lv_indev_get_act();

    if (code == LV_EVENT_FOCUSED)
    {
        if (lv_indev_get_type(lv_indev_get_act()) != LV_INDEV_TYPE_KEYPAD)
        {
            lv_obj_update_layout(tv);   /*Be sure the sizes are recalculated*/
            lv_obj_scroll_to_view_recursive(ta, LV_ANIM_OFF);
        }
    }
    else if (code == LV_EVENT_DEFOCUSED)
    {
        lv_obj_set_height(tv, LV_VER_RES);
        lv_indev_reset(NULL, ta);
    }
    else if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL)
    {
        lv_obj_set_height(tv, LV_VER_RES);
        lv_obj_clear_state(ta, LV_STATE_FOCUSED);
        lv_indev_reset(NULL, ta);   /*To forget the last clicked object to make it focusable again*/
    }
    if (indev->proc.state == LV_INDEV_STATE_PRESSED && code == LV_EVENT_KEY)
    {
       shellHandler(&shell_object, indev->proc.types.keypad.last_key);
    }
}
#endif

void ui_screen_screen_init(lv_obj_t *parent)
{
    lvgl_shell();
    lv_group_t *group = lv_group_create();
    lv_indev_set_group(indev_keypad, group);

    tv = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, 50);
    ui_screen = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_screen, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_bg_color(ui_screen, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_screen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_text_receive_message = lv_textarea_create(ui_screen);
    lv_obj_set_width(ui_text_receive_message, 810);
    lv_obj_set_height(ui_text_receive_message, 610);
    lv_obj_set_x(ui_text_receive_message, 0);
    lv_obj_set_y(ui_text_receive_message, 0);
    lv_obj_set_align(ui_text_receive_message, LV_ALIGN_CENTER);
    lv_textarea_set_placeholder_text(ui_text_receive_message, "Placeholder...");
    lv_obj_add_state(ui_text_receive_message, LV_STATE_FOCUSED);       /// States
    lv_obj_set_style_bg_color(ui_text_receive_message, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_text_receive_message, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(ui_text_receive_message, text_receive_event_cb, LV_EVENT_ALL, NULL);
    lv_group_add_obj(group, ui_text_receive_message);

    ui_image_logo = lv_img_create(ui_screen);
    lv_img_set_src(ui_image_logo, &lv_logo);
    lv_obj_set_width(ui_image_logo, 300);
    lv_obj_set_height(ui_image_logo, LV_SIZE_CONTENT);    /// 30
    lv_obj_set_x(ui_image_logo, 244);
    lv_obj_set_y(ui_image_logo, -246);
    lv_obj_set_align(ui_image_logo, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_image_logo, LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
    lv_obj_clear_flag(ui_image_logo, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
}
