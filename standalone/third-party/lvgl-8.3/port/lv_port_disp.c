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
 * FilePath: lv_port_disp.c
 * Date: 2023-02-05 18:27:47
 * LastEditTime: 2023-02-10 11:02:47
 * Description:  This file is for providing the port of lvgl config and display
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------  -------- --------------------------------------
 *  1.0  Wangzq     2023/02/10  Modify the format and establish the version
 */

/**
 * @file lv_port_disp.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "ftypes.h"
#include "fparameters.h"
#include "ferror_code.h"
#include "fassert.h"
#include "finterrupt.h"
#include "fcpu_info.h"

#include "lv_port_disp.h"
#include "lv_conf.h"
#include "../lvgl.h"

#ifdef CONFIG_USE_FMEDIA
#include "fdcdp.h"
#endif

#ifdef CONFIG_USE_FMEDIA_V2
#include "fdcdp_v2.h"
#endif

/*********************
 *      DEFINES
 *********************/
static lv_color_int_t *rtt_fbp[FDP_INSTANCE_NUM];
static u32 multi_mode;

#define LV_HOR_RES_MAX (640) /* default value 320 */
#define LV_VER_RES_MAX (480) /* default value 240*/

static void FMediaDispFlush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);

/************************** Function Prototypes ******************************/
/**
 * @name: FMediaDispFramebuffer
 * @msg:  set the lvgl framebuffer addr and ensure the connected dp have the correct addr
 * @return null
 */
void FMediaDispFramebuffer(FDcDp *instance)
{
    FASSERT(instance != NULL);

    if ((rtt_fbp[0] == NULL))
    {
        rtt_fbp[0] = (lv_color_int_t *)instance->dc_instance_p[0].fb_virtual;
        multi_mode = instance->multi_mode;
    }
    else
    {
        rtt_fbp[1] = (lv_color_int_t *)instance->dc_instance_p[1].fb_virtual;
    }
    return;
}


void FMediaLvgldispInit(void)
{
    static lv_disp_draw_buf_t draw_buf_dsc_1;
    static lv_color_t buf_1[LV_HOR_RES_MAX * 10]; /*A buffer for 10 rows*/
    lv_disp_draw_buf_init(&draw_buf_dsc_1, buf_1, NULL, LV_HOR_RES_MAX * 10); /*Initialize the display buffer*/
    static lv_disp_drv_t disp_drv; /*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv);   /*Basic initialization*/
    /*Set the resolution of the display*/
    disp_drv.hor_res = LV_HOR_RES_MAX;
    disp_drv.ver_res = LV_VER_RES_MAX;
    /*Used to copy the buffer's content to the display*/
    disp_drv.flush_cb = FMediaDispFlush;
    /*Set a display buffer*/
    disp_drv.draw_buf = &draw_buf_dsc_1;
    /*Finally register the driver*/
    lv_disp_drv_register(&disp_drv);

    return;
}

volatile bool disp_flush_enabled = true;

/**
 * @name: disp_enable_update
 * @msg:  Enable updating the screen (the flushing process) when FMediaDispFlush() is called by LVGL
 * @return null
 */
void disp_enable_update(void)
{
    disp_flush_enabled = true;
}

/**
 * @name: disp_disable_update
 * @msg:  Disable updating the screen (the flushing process) when FMediaDispFlush() is called by LVGL
 * @return null
 */
void disp_disable_update(void)
{
    disp_flush_enabled = false;
}


/**
 * @name: FMediaDispFlush
 * @msg:  flush the framebuffer
 * @param {lv_disp_drv_t *} disp_drv is the Display Driver structure to be registered by HAL
 * @param {const lv_area_t *} area is the specific area on the display you want to flush
 * @param {lv_color_t *} color_p is the image pixel of
 * @return null
 */
static void FMediaDispFlush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    long int location = 0;
    int32_t x;
    int32_t y;
    if (disp_flush_enabled)
    {
        if (multi_mode == 0)
        {

            for (y = area->y1; y <= area->y2; y++)
            {
                for (x = area->x1; x <= area->x2; x++)
                {
                    location = (x) + (y)*LV_HOR_RES_MAX;
                    rtt_fbp[0][location] = color_p->full;
                    color_p++;
                }
            }
        }
        else if (multi_mode == 1)
        {
            for (y = area->y1; y <= area->y2; y++)
            {
                for (x = area->x1; x <= (area->x2); x++)
                {
                    if (x < area->x2 / 2)
                    {

                        location = (x) + (y) * (LV_HOR_RES_MAX);
                        rtt_fbp[0][location * 2] = (color_p->full);
                        rtt_fbp[0][location * 2 + 1] = (color_p->full);
                    }
                    else
                    {
                        location = (x) + (y) * (LV_HOR_RES_MAX);
                        rtt_fbp[1][(location - area->x2 / 2) * 2] = (color_p->full);
                        rtt_fbp[1][(location - area->x2 / 2) * 2 + 1] = (color_p->full);
                    }
                    color_p++;
                }
            }
        }
        else
        {
            for (y = area->y1; y <= area->y2; y++)
            {
                for (x = area->x1; x <= (area->x2); x++)
                {
                    if (y < LV_VER_RES_MAX / 2)
                    {
                        location = (x) + (y) * (LV_HOR_RES_MAX)*2;
                        rtt_fbp[0][location] = (color_p->full);
                        rtt_fbp[0][(location) + LV_HOR_RES_MAX] = (color_p->full);
                    }
                    else
                    {
                        location = (x) + (y - LV_VER_RES_MAX / 2) * (LV_HOR_RES_MAX)*2;
                        rtt_fbp[1][location] = (color_p->full);
                        rtt_fbp[1][(location) + LV_HOR_RES_MAX] = (color_p->full);
                    }
                    color_p++;
                }
            }
        }
    }
    lv_disp_flush_ready(disp_drv);
}
