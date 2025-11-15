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
 * FilePath: fdc_v2_hw.h
 * Created Date: 2025-02-11 17:22:12
 * Last Modified: 2025-07-08 16:28:02
 * Description:  This file is for
 *
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 * 1.0      wzq         2025/07/08         first version
 */
#ifndef FDC_V2_HW_H
#define FDC_V2_HW_H

/***************************** Include Files *********************************/

#include "ftypes.h"
#include "ferror_code.h"
#include "fio.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define FDC_READ_REG32(addr, reg_offset) FtIn32((addr) + (u32)(reg_offset))
#define FDC_WRITE_REG32(addr, reg_value) FtOut32(addr, (u32)(reg_value))
#define FDC_PHY_ALIGN(data, Offset)      ((data + Offset - 1) & ~(Offset - 1))

void FDcV2ChannelRegWrite(uintptr addr, uintptr offset, u32 data);

u32 FDcV2ChannelRegRead(uintptr addr, uintptr offset);

#ifdef __cplusplus
}
#endif

#endif /* FDC_V2_HW_H */
