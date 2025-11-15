/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: fi3c_hw.c
 * Date: 2023-9-11 14:53:42
 * LastEditTime: 2023-9-11 08:36:22
 * Description:  This file is for I3C register read/write operation
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  zhangyan    2023/9/11  first commit
 */
#include <string.h>
#include "fsleep.h"
#include "fdebug.h"
#include "ferror_code.h"
#include "fassert.h"
#include "fi3c_hw.h"
#include "fi3c.h"

#define FI3C_DEBUG_TAG          "I3C_HW"
#define FI3C_ERROR(format, ...) FT_DEBUG_PRINT_E(FI3C_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI3C_INFO(format, ...)  FT_DEBUG_PRINT_I(FI3C_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI3C_DEBUG(format, ...) FT_DEBUG_PRINT_D(FI3C_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/

/**
 * @name: FI3cSetEnable
 * @msg: 设置I3C控制器的使能状态
 * @return {*}
 * @param {uintptr} addr， I3c控制器基地址
 * @param {boolean} enable， TRUE: 使能，FALSE: 去使能
 */
FError FI3cSetEnable(uintptr addr, boolean enable)
{
    if (enable == TRUE)
    {
        FI3C_SETBIT(addr, FI3C_CTRL_OFFSET, FI3C_CTRL_DEV_EN);
    }
    else
    {
        FI3C_CLEARBIT(addr, FI3C_CTRL_OFFSET, FI3C_CTRL_DEV_EN);
    }

    return FI3C_SUCCESS;
}
