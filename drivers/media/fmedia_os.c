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
 * FilePath: fmedia_os.c
 * Date: 2022-09-15 14:20:19
 * LastEditTime: 2022-09-21 16:59:51
 * Description:  This file is for providing the media driver
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  Wangzq     2022/12/20  Modify the format and establish the version
 */
#include <stdio.h>
#include <string.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include "ftypes.h"
#include "fdebug.h"
#include "fparameters_comm.h"
#include "fmedia_os.h"
#include "fdcdp.h"
#include "fdp_hw.h"
#include "fdp.h"

/***************** Macros (Inline Functions) Definitions *********************/

#define FMEDIA_DEBUG_TAG "FFreeRTOSMEDIA"
#define FMEDIA_ERROR(format, ...) FT_DEBUG_PRINT_E(FMEDIA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FMEDIA_INFO(format, ...) FT_DEBUG_PRINT_I(FMEDIA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FMEDIA_DEBUG(format, ...) FT_DEBUG_PRINT_D(FMEDIA_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/

/**
 * @name: FFreeRTOSMediaHwInit
 * @msg:  init the media,dc and dp
 * @param  {FFreeRTOSMedia*} instance is the driver instance
 * @param  {u32} index is the dp index
 * @param  {u32} width is the width
 * @param  {u32} height is the height
 * @return err code information, 0 indicates success，others indicates failed
 */
FFreeRTOSMedia *FFreeRTOSMediaHwInit(FFreeRTOSMedia *instance, u32 index, u32 width, u32 height)
{
    FError ret = FT_SUCCESS;

    ret = FDcDpInitial(&instance->dcdp_ctrl, index, width, height);
    if(FT_SUCCESS != ret)
    {
        FMEDIA_ERROR("FDcDpInitial failed, ret = %d", ret);
        return NULL;
    }

    if((instance->dcdp_ctrl.is_initialized[index]) == FDCDP_IS_INITIALIZED)
    {
        ret = FDP_SUCCESS;
    }
    return (FT_SUCCESS == ret) ? instance : NULL; /* exit with NULL if failed */
}

