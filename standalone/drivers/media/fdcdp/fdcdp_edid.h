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
 * FilePath: fdcdp_edid.h
 * Date: 2024-09-19 14:53:42
 * LastEditTime: 2024-09-19 08:29:10
 * Description:  This file is for providing some hardware register and function definition
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  Wangzq     2024/09/19   Modify the format and establish the version
 */

#ifndef FDCDP_EDID_H
#define FDCDP_EDID_H

#include "ftypes.h"
#include "ferror_code.h"
#include "fdrivers_port.h"


typedef struct
{
    u32 pixel_clock;    /* pixel clock frequence in megahertz unit */
    u32 hor_pixel;      /* Horizontal total  lines. */
    u32 ver_pixel;      /* Vertical total lines. */
    u32 hor_blanking;   /* Horizontal Blanking in pixels minus*/
    u32 ver_blanking;   /* Vertical Blanking in pixels minus*/
    u32 hor_sync_front; /*Horizontal Front Proch in pixels.  */
    u32 ver_sync_front; /*vertical Front Proch in pixels. */
    u32 hor_sync_width; /* Horizontal Sync Pulse Width in pixels. */
    u32 ver_sync_width; /* Vertical Sync Pulse Width in lines. */
    u8 hor_polarity;    /*0 - active high , 1 - active low  */
    u8 ver_polarity;    /*0 - active high , 1 - active low   */
} Auxtable;

FError FDpGetEdid(FDpCtrl *instance_p, u8 *buffer);

#endif /* FDCDP_EDID_H */
