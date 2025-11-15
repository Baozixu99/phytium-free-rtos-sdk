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
 * FilePath: fdp_v2_hw.h
 * Created Date: 2025-02-06 11:14:24
 * Last Modified: 2025-07-08 16:28:45
 * Description:  This file is for
 *
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 * 1.0      wzq         2025/07/08         first version
 */

#ifndef FDP_V2_HW_H
#define FDP_V2_HW_H

#ifdef __cplusplus
extern "C"
{
#endif
/***************************** Include Files *********************************/

#include "ftypes.h"
#include "fio.h"
#include "ferror_code.h"

/***************************** Macro Definitions **********/
#define FDP_READ_REG32(addr, reg_offset) FtIn32((addr) + (u32)(reg_offset))

#define FDP_WRITE_REG32(addr, reg_value) FtOut32(addr, (u32)(reg_value))

#define FDP_SETBIT(base_addr, reg_offset, data) \
    FtSetBit32((base_addr) + (u32)(reg_offset), (u32)(data))

#define FDP_CLEARBIT(base_addr, reg_offset, data) \
    FtClearBit32((base_addr) + (u32)(reg_offset), (u32)(data))

/***************************** function Definitions **********/
/***************************** Macro Definitions **********/
/* write the data to the channel of Dc */
void FDpV2ChannelRegWrite(uintptr addr, uintptr offset, u32 data);

/* read the data from the channel of Dc */
u32 FDpV2ChannelRegRead(uintptr addr, uintptr offset);

/* write the data to the channel of phy */
void FDpV2PhyRegWrite(uintptr addr, uintptr offset, u32 data);

/* read the data of the channel of phy */
u32 FDpV2PhyRegRead(uintptr addr, uintptr offset);
#ifdef __cplusplus
}
#endif

#endif /* FDC_V2_HW_H */
