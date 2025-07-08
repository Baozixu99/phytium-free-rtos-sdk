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
 * FilePath: lv_ui.c
 * Date: 2024-01-20 14:22:40
 * LastEditTime: 2024-02-20 15:40:40
 * Description:  This file is for providing the ui init
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------  -------- --------------------------------------
 *  1.0  Wangzq     2024/02/20  first add
 */
#include "lv_ui.h"
#include "lv_ui_helpers.h"

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
#include "shell_port.h"
#endif

extern void ui_screen_screen_init(lv_obj_t *parent);
lv_obj_t *ui_screen;
lv_obj_t *ui____initial_actions0;

#if LV_COLOR_DEPTH != 32
    #error "LV_COLOR_DEPTH should be 32bit to match SquareLine Studio's settings"
#endif
#if LV_COLOR_16_SWAP !=0
    #error "LV_COLOR_16_SWAP should be 0 to match SquareLine Studio's settings"
#endif

static lv_obj_t *tv;


void ui_init(void)
{
    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED),
                        false, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);

    ui_screen = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_screen, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
#ifdef CONFIG_USE_LETTER_SHELL
    ui_screen_screen_init(ui_screen);
#endif
    lv_disp_load_scr(ui_screen);
    ui____initial_actions0 = lv_obj_create(NULL);

}
