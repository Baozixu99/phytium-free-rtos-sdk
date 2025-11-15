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
 * FilePath: fnand_ecc.h
 * Date: 2022-05-12 11:17:42
 * LastEditTime: 2022-05-12 13:56:27
 * Description:  This file is for ecc validation related api
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe    2022/05/10    first release
 */

#ifndef FNAND_ECC_H
#define FNAND_ECC_H

#include "ftypes.h"
#include "fnand_hw.h"
#include "stdio.h"

#ifdef __cplusplus
extern "C"
{
#endif

static inline void FNandEnableHwEcc(uintptr_t base_address)
{
    FNAND_SETBIT(base_address, FNAND_CTRL0_OFFSET, FNAND_CTRL0_ECC_EN_MASK);
}


static inline void FNandDisableHwEcc(uintptr_t base_address)
{
    FNAND_CLEARBIT(base_address, FNAND_CTRL0_OFFSET, FNAND_CTRL0_ECC_EN_MASK);
}

u32 FNandGetEccTotalLength(u32 bytes_per_page, u32 ecc_strength);
s32 FNandCorrectEcc(uintptr_t base_address, u32 ecc_step_size, u32 hw_ecc_steps, u8 *buf, u32 length);

#ifdef __cplusplus
}
#endif

#endif
