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
 * FilePath: fdp_hw.c
 * Date: 2024-09-17 22:53:24
 * LastEditTime: 2024/09/19  12:5:25
 * Description:  This file is for This file is for Handling the hardware register and
 *               providing some function interface
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  Wangzq     2024/09/19  Modify the format and establish the version
 */


/***************************** Include Files *********************************/

#include "ftypes.h"
#include "fdrivers_port.h"
#include "fio.h"
#include "fdp_hw.h"

/***************************** Macro Definitions **********/
#define FDP_READ_REG32(addr, reg_offset) FtIn32((addr) + (u32)(reg_offset))

#define FDP_WRITE_REG32(addr, reg_value) FtOut32(addr, (u32)(reg_value))

#define FDC_PHY_ALIGN(data, Offset)      ((data + Offset - 1) & ~(Offset - 1))

#define FDP_SETBIT(base_addr, reg_offset, data) \
    FtSetBit32((base_addr) + (u32)(reg_offset), (u32)(data))

#define FDP_CLEARBIT(base_addr, reg_offset, data) \
    FtClearBit32((base_addr) + (u32)(reg_offset), (u32)(data))

/***************************** function Definitions **********/
/**
 * @name: FDpChannelRegRead
 * @msg:  read the data of dp channel register
 * @param {uintptr} addr is the address of the dc baseaddr
 * @param {uintptr} offset is the register address offset
 * @return data, is the data you want to read from the dp channel addr
 */
u32 FDpChannelRegRead(uintptr addr, uintptr offset)
{
    u32 data;
    data = FDP_READ_REG32(addr, offset);
    return data;
}

/**
 * @name: FDpChannelRegWrite
 * @msg:  write the data to the channel of dp
 * @param {uintptr} addr is the address of the dp baseaddr
 * @param {uintptr} offset is the register address offset
 * @param {u32} data is the The value that you want to write to the
 * register in a specified group
 * @return Null
 */
void FDpChannelRegWrite(uintptr addr, uintptr offset, u32 data)
{
    FDP_WRITE_REG32(addr + offset, data);
}
