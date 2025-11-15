/*
 * Copyright (C) 2023, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: fpcie_caps.c
 * Created Date: 2023-07-12 17:32:19
 * Last Modified: 2025-08-11 15:42:44
 * Description:  This file is for
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 * 1.0     huanghe    2023/08/06        first release
 */

#include "fpcie_ecam.h"
#include "fpcie_ecam_common.h"
#include "fdrivers_port.h"

/***************** Macros (Inline Functions) Definitions *********************/

#define FPCIE_CAP_DEBUG_TAG "FPCIE_CAP"
#define FPCIE_CAP_DEBUG_D(format, ...) \
    FT_DEBUG_PRINT_D(FPCIE_CAP_DEBUG_TAG, format, ##__VA_ARGS__)
#define FPCIE_CAP_DEBUG_I(format, ...) \
    FT_DEBUG_PRINT_I(FPCIE_CAP_DEBUG_TAG, format, ##__VA_ARGS__)
#define FPCIE_CAP_DEBUG_W(format, ...) \
    FT_DEBUG_PRINT_W(FPCIE_CAP_DEBUG_TAG, format, ##__VA_ARGS__)
#define FPCIE_CAP_DEBUG_E(format, ...) \
    FT_DEBUG_PRINT_E(FPCIE_CAP_DEBUG_TAG, format, ##__VA_ARGS__)


u8 FPcieEcamGetBaseCapability(FPcieEcam *instance_p, u8 bus, u8 device, u8 function)
{
    u32 cap_value;

    if (FPcieEcamReadConfigSpace(instance_p, bus, device, function,
                                 FPCIE_CCR_CAPBILITIES_PONINTERS_REGS, &cap_value) != FT_SUCCESS)
    {
        FPCIE_CAP_DEBUG_E("%s:%d,Failed to read config space", __FUNCTION__, __LINE__);
        return 0;
    }

    return FPCIE_CCR_CAP_PONINTERS_MASK(cap_value);
}

/**
 * @name: FPcieEcamHasCapability
 * @msg: This function checks whether a specific capability is present in the PCIe device's capability list.
 * @param {FPcieEcam *} instance_p - Pointer to the FPcieEcam instance.
 * @param {u8} bus - PCIe bus number.
 * @param {u8} device - PCIe device number.
 * @param {u8} function - PCIe function number.
 * @param {u8} capid - ID of the capability to check for.
 * @return {u32} - Returns 1 if the capability is present, 0 otherwise.
 */
u32 FPcieEcamHasCapability(FPcieEcam *instance_p, u8 bus, u8 device, u8 function, u8 capid)
{
    u32 capability_valid = 0;
    u32 CapBilityBase = FPcieEcamGetBaseCapability(instance_p, bus, device, function);

    while (CapBilityBase != 0U)
    {
        if (FPcieEcamReadConfigSpace(instance_p, bus, device, function,
                                     FPCIE_CAP_ADDRESS_GET(CapBilityBase), &CapBilityBase) != FT_SUCCESS)
        {
            FPCIE_CAP_DEBUG_E("%s:%d,Failed to read config space", __FUNCTION__, __LINE__);
            break;
        }

        if (FPCIE_CAP_ID_GET(CapBilityBase) == capid)
        {
            capability_valid = 1;
            break;
        }

        CapBilityBase = FPCIE_CAP_NEXT_POINTER_GET(CapBilityBase);
    }

    return capability_valid;
}

/**
 * @name: FPcieEcamHasExtendCapability
 * @msg: This function checks whether a specific extended capability is present in the PCIe device's extended capability list.
 * @param {FPcieEcam *} instance_p - Pointer to the FPcieEcam instance.
 * @param {u8} bus - PCIe bus number.
 * @param {u8} device - PCIe device number.
 * @param {u8} function - PCIe function number.
 * @param {u16} capid - ID of the extended capability to check for.
 * @return {u32} - Returns 1 if the extended capability is present, 0 otherwise.
 */
u32 FPcieEcamHasExtendCapability(FPcieEcam *instance_p, u8 bus, u8 device, u8 function, u16 capid)
{
    u32 capability_valid = 0;
    u32 capability_offset, capability_value;

    capability_offset = FPCI_EXT_REGS;

    do
    {
        if (FPcieEcamReadConfigSpace(instance_p, bus, device, function,
                                     capability_offset, &capability_value) != FT_SUCCESS)
        {
            FPCIE_CAP_DEBUG_E("%s:%d,Failed to read config space", __FUNCTION__, __LINE__);
            break;
        }
        if (capability_value == 0)
        {
            break;
        }

        if (FPCIE_EXT_CAP_ID_GET(capability_value) == capid)
        {
            capability_valid = 1;
            break;
        }

        capability_offset = FPCI_EXT_CAP_PONINTERS_MASK(capability_value);
    } while (capability_offset);
    return capability_valid;
}


u32 FPcieEcamGetExtendCapability(FPcieEcam *instance_p, u8 bus, u8 device, u8 function, u16 capid)
{
    u32 capability_valid = 0;
    u32 capability_offset, capability_value;

    capability_offset = FPCI_EXT_REGS;

    do
    {
        if (FPcieEcamReadConfigSpace(instance_p, bus, device, function,
                                     capability_offset, &capability_value) != FT_SUCCESS)
        {
            FPCIE_CAP_DEBUG_E("%s:%d,Failed to read config space", __FUNCTION__, __LINE__);
            break;
        }
        if (capability_value == 0)
        {
            break;
        }

        if (FPCIE_EXT_CAP_ID_GET(capability_value) == capid)
        {
            capability_valid = 1;
            break;
        }

        capability_offset = FPCI_EXT_CAP_PONINTERS_MASK(capability_value);
    } while (capability_offset);

    return (capability_valid ? FPCI_EXT_CAP_PONINTERS_MASK(capability_value) : 0);
}


u32 FPcieEcamGetCapability(FPcieEcam *instance_p, u8 bus, u8 device, u8 function, u8 capid)
{
    u32 base = FPcieEcamGetBaseCapability(instance_p, bus, device, function);
    u32 data;
    u32 capability_valid = 0;

    while (base != 0U)
    {
        if (FPcieEcamReadConfigSpace(instance_p, bus, device, function,
                                     FPCIE_CAP_ADDRESS_GET(base), &data) != FT_SUCCESS)
        {
            FPCIE_CAP_DEBUG_E("%s:%d,Failed to read config space", __FUNCTION__, __LINE__);
            break;
        }

        if (FPCIE_CAP_ID_GET(data) == capid)
        {
            capability_valid = 1;
            break;
        }

        base = FPCIE_CAP_NEXT_POINTER_GET(data);
    }

    return (capability_valid ? FPCIE_CAP_ADDRESS_GET(base) : 0);
}
