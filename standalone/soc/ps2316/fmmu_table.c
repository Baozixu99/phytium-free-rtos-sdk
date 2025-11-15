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
 * FilePath: fmmu_table.c
 * Date: 2023-11-6 10:33:28
 * LastEditTime: 2023-11-6 10:33:28
 * Description:  This file is for
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhangyan    2023/11/6       init commit
 * 1.1   huanghe     2024/05/21      Unified aarch32 and aarch64 mmu tables
 */

#include "fmmu.h"
#include "fparameters.h"

const struct ArmMmuRegion mmu_regions[] = {
    MMU_REGION_FLAT_ENTRY("DEVICE_REGION", 0x00, 0x40000000, MT_DEVICE_NGNRNE | MT_P_RW_U_NA | MT_NS),


    MMU_REGION_FLAT_ENTRY("PCIE_CONFIG_REGION", 0x40000000, 0x10000000,
                          MT_DEVICE_NGNRNE | MT_P_RW_U_NA | MT_NS),

    MMU_REGION_FLAT_ENTRY("PCIE_CONFIG_REGION", 0x50000000, 0x30000000,
                          MT_DEVICE_NGNRNE | MT_P_RW_U_NA | MT_NS),
    /* # TODO mmu 需要升级特性 */
    MMU_REGION_FLAT_ENTRY("PCIE_MEM64_REGION", 0x40000000000, 0x30000000,
                          MT_DEVICE_NGNRNE | MT_P_RW_U_NA | MT_NS),
};

const struct ArmMmuConfig mmu_config = {
    .num_regions = ARRAY_SIZE(mmu_regions),
    .mmu_regions = mmu_regions,
};