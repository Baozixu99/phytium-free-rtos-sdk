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
 * FilePath: fmio_hw.c
 * Date: 2022-06-20 21:05:23
 * LastEditTime: 2022-06-20 21:05:23
 * Description:  This file is for mio hardware operation
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  liushengming 2022/06/20 first commit
 */

/***************************** Include Files *********************************/

#include "fmio_hw.h"
#include "ftypes.h"
#include "fassert.h"
#include "fmio.h"
/***************** Macros (Inline Functions) Definitions *********************/

#define FMIO_DEBUG_TAG          "MIO-HW"
#define FMIO_ERROR(format, ...) FT_DEBUG_PRINT_E(FMIO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FMIO_INFO(format, ...)  FT_DEBUG_PRINT_I(FMIO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FMIO_DEBUG(format, ...) FT_DEBUG_PRINT_D(FMIO_DEBUG_TAG, format, ##__VA_ARGS__)
/************************** Function Prototypes ******************************/

/**
 * @name: FMioSelectFunc
 * @msg: 设置Mio功能
 * @return {*}
 * @param {uintptr} addr
 * @param {u32} mio_type
 */
FError FMioSelectFunc(uintptr addr, u32 mio_type)
{
    FASSERT(mio_type < FMIO_NUM_OF_MIO_FUNC);
    FASSERT(addr);

    u32 reg_val;

    reg_val = FMioReadStatus(addr);

    if (mio_type == reg_val)
    {
        return FMIO_SUCCESS;
    }

    FMioWriteFunc(addr, mio_type);

    return FMIO_SUCCESS;
}

/**
 * @name: FMioGetMioFunc
 * @msg: 获取Mio状态
 * @return {*}
 * @param {uintptr} addr
 */
u32 FMioGetFunc(uintptr addr)
{
    FASSERT(addr);

    return FMioReadStatus(addr);
}

/**
 * @name: FMioGetVersion
 * @msg: 获取版本信息，默认32'h1
 * @return {*}
 * @param {uintptr} addr
 */
u32 FMioGetVersion(uintptr addr)
{
    FASSERT(addr);

    return FMioReadVersion(addr);
}
