/*
 * Copyright (C) 2021, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: fiopad_hw.c
 * Date: 2021-04-29 10:21:53
 * LastEditTime: 2022-02-18 08:29:20
 * Description:  This files is for the iopad register related functions
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhangyan   2023/7/3    first release
 */

/***************************** Include Files *********************************/
#include "fparameters.h"
#include "fiopad_hw.h"
#include "fdrivers_port.h"

#define FIOPAD_DEBUG_TAG "FIOPAD_HW"
#define FIOPAD_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FIOPAD_DEBUG_TAG, format, ##__VA_ARGS__)
#define FIOPAD_WARN(format, ...) \
    FT_DEBUG_PRINT_W(FIOPAD_DEBUG_TAG, format, ##__VA_ARGS__)
#define FIOPAD_INFO(format, ...) \
    FT_DEBUG_PRINT_I(FIOPAD_DEBUG_TAG, format, ##__VA_ARGS__)
#define FIOPAD_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FIOPAD_DEBUG_TAG, format, ##__VA_ARGS__)

/**
 * @name: FIOPadDump
 * @msg: print information of all iopad
 * @return {*}
 */
void FIOPadDump(uintptr base_addr)
{
    uintptr beg_off = 0x0000U;
    uintptr end_off = 0x024CU;
    uintptr off;

    FIOPAD_DEBUG("Pad Func Info...");
    for (off = beg_off; off <= end_off; off += 4U)
    {
        FIOPAD_DEBUG("  [0x%x] func: %d, drive: %d, res: %d ", off,
                     FIOPAD_REG0_FUNC_GET(FIOPAD_READ_REG32(base_addr, off)),
                     FIOPAD_REG0_DRIVE_GET(FIOPAD_READ_REG32(base_addr, off)),
                     FIOPAD_REG0_PULL_GET(FIOPAD_READ_REG32(base_addr, off)));
    }

    return;
}