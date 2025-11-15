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
 * FilePath: fdc_hw.h
 * Date: 2024-09-17 14:53:42
 * LastEditTime: 2024-09-19 08:29:10
 * Description:  This file is for providing some hardware register and function definition
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  Wangzq     2024/09/19   Modify the format and establish the version
 */

#ifndef FDC_HW_H
#define FDC_HW_H

/***************************** Include Files *********************************/

#include "ftypes.h"
#include "ferror_code.h"
#include "fdrivers_port.h"
#include "fio.h"

#ifdef __cplusplus
extern "C"
{
#endif
/************************** Constant Definitions *****************************/

#define FDC_READ_REG32(addr, reg_offset) FtIn32((addr) + (u32)(reg_offset))
#define FDC_WRITE_REG32(addr, reg_value) FtOut32(addr, (u32)(reg_value))
#define FDC_PHY_ALIGN(data, Offset)      ((data + Offset - 1) & ~(Offset - 1))

/***************************** Macro Definitions **********/


/* write the data to the channel of DcDp */
void FDcChannelRegWrite(uintptr addr, uintptr offset, u32 data);

/* read the data from the channel of DcDp */
u32 FDcChannelRegRead(uintptr addr, uintptr offset);

#ifdef __cplusplus
}
#endif

#endif /* FDC_HW_H */