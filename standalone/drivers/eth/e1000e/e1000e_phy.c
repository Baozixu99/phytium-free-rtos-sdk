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
 * FilePath: e1000e_phy.c
 * Date: 2025-01-13 14:46:52
 * LastEditTime: 2025-01-13 14:46:58
 * Description:  This file is for phy types.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huangjin    2025/01/13    first release
 */

#include "fdrivers_port.h"
#include "e1000e.h"
#include "e1000e_phy.h"
#include "e1000e_hw.h"
#include "faarch.h"

#define FE1000E_DEBUG_TAG "FE1000E_PHY"
#define FE1000E_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FE1000E_DEBUG_TAG, format, ##__VA_ARGS__)
#define FE1000E_INFO(format, ...) \
    FT_DEBUG_PRINT_I(FE1000E_DEBUG_TAG, format, ##__VA_ARGS__)
#define FE1000E_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FE1000E_DEBUG_TAG, format, ##__VA_ARGS__)
#define FE1000E_WARN(format, ...) \
    FT_DEBUG_PRINT_W(FE1000E_DEBUG_TAG, format, ##__VA_ARGS__)

/**
 * Write data to the specified PHY register. The Ethernet driver does not
 * require the device to be stopped before writing to the PHY.  Although it is
 * probably a good idea to stop the device, it is the responsibility of the
 * application to deem this necessary. The MAC provides the driver with the
 * ability to talk to a PHY that adheres to the Media Independent Interface
 * (MII) as defined in the IEEE 802.3 standard.
 *
 * @param base_addr Base address of the Ethernet controller's register space.
 * @param reg The register number (0-31) of the PHY to which data will be written.
 * @param val The 16-bit value to write to the specified PHY register.
 *
 * @return
 *
 * - FT_SUCCESS if the PHY was written to successfully. Since there is no error
 *   status from the MAC on a write, the user should read the PHY to verify the
 *   write was successful.
 * - FE1000E_ERR_PHY_BUSY if there is another PHY operation in progress
 *
 * @note
 *
 * This function is not thread-safe. If multiple threads can access this 
 * function, the user must ensure mutual exclusion.
 *
 * If the hardware malfunctions (e.g., the `MDIC_READY` status bit is not set), 
 * this function could enter an infinite loop. Users should consider adding a 
 * timeout mechanism for error recovery.
 *
 * During the execution of this function, all host interface reads and writes 
 * to the specified Ethernet controller instance are blocked.
 *
 ******************************************************************************/
FError FE1000EPhyWrite(uintptr base_addr, unsigned int reg, uint16_t val)
{
    FE1000E_WRITEREG32(base_addr, E1000_MDIC,
                       val | (reg << MDIC_REGADD_SHFT) | MDIC_PHYADD | MDIC_OP_WRITE);
    while (!(FE1000E_READREG32(base_addr, E1000_MDIC) & MDIC_READY))
    {
        BARRIER();
    }
    return FT_SUCCESS;
}

/**
 * Read the current value of the PHY register indicated by the phy_address and
 * the register_num parameters. The MAC provides the driver with the ability to
 * talk to a PHY that adheres to the Media Independent Interface (MII) as
 * defined in the IEEE 802.3 standard.
 *
 *
 * @param base_addr Base address of the Ethernet controller's register space.
 * @param reg The register number (0-31) of the PHY to read.
 *
 * @return
 *
 * - The 16-bit value read from the specified PHY register.
 * - If the read operation is not successful due to hardware issues, the 
 *   function may not return (see note below).
 *
 * @note
 *
 * This function is not thread-safe. Users must ensure mutual exclusion 
 * when multiple threads access this function.
 * 
 * If the hardware malfunctions (e.g., the `MDIC_READY` bit is not set), 
 * the function may enter an infinite loop. Users are advised to implement 
 * a timeout mechanism for error recovery if needed.
 * 
 * During the execution of this function, all host interface reads and writes 
 * to the specified Ethernet controller instance are blocked.
 *
 ******************************************************************************/
uint16_t FE1000EPhyRead(uintptr base_addr, unsigned int reg)
{
    uint32_t val;
    FE1000E_WRITEREG32(base_addr, E1000_MDIC, (reg << MDIC_REGADD_SHFT) | MDIC_PHYADD | MDIC_OP_READ);
    val = FE1000E_READREG32(base_addr, E1000_MDIC);
    do
    {
        val = FE1000E_READREG32(base_addr, E1000_MDIC);
        BARRIER();
    } while (!(val & MDIC_READY));

    return (uint16_t)val;
}
