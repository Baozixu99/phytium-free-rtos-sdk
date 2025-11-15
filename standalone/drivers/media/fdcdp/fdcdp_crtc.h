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
 * FilePath: fdcdp_crtc.h
 * Date: 2024-09-17 14:53:42
 * LastEditTime: 2024-09-19 08:29:10
 * Description:  This file is for providing some hardware register and function definition
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  Wangzq     2024/09/19   Modify the format and establish the version
 */

#ifndef FDC_CRTC_H
#define FDC_CRTC_H

/***************************** Include Files *********************************/

#include "ftypes.h"
#include "ferror_code.h"
#include "fdrivers_port.h"
#include "fdc.h"
#include "fdp.h"

/************************** Function Prototypes ******************************/
FError FDcSetCrtcEnable(FDcCtrl *instance);
FError FDpSetCrtcEnable(FDpCtrl *instance);
int FDcWidthToStride(u32 width, u32 color_depth, u32 multi_mode);
#endif /* FDC_CRTC_H */
