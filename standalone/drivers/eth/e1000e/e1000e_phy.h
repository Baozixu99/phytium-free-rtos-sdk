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
 * FilePath: e1000e_phy.h
 * Date: 2025-01-13 14:46:52
 * LastEditTime: 2025-01-13 14:46:58
 * Description:  This file is for phy configuration.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huangjin   2025/01/13    first release
 */

#ifndef FE1000E_PHY_H
#define FE1000E_PHY_H

#include "ftypes.h"
#include "ferror_code.h"
#include "e1000e.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define E1000E_PHY_RESET_ENABLE  1
#define E1000E_PHY_RESET_DISABLE 0

enum
{
    FE1000E_PHY_AUTONEGOTIATION_DISABLE = 0,
    FE1000E_PHY_AUTONEGOTIATION_ENABLE
};

enum
{
    FE1000E_PHY_MODE_HALFDUPLEX = 0,
    FE1000E_PHY_MODE_FULLDUPLEX
};

/* phy interface */
FError FE1000EPhyWrite(uintptr base_addr, unsigned int reg, uint16_t val);
uint16_t FE1000EPhyRead(uintptr base_addr, unsigned int reg);

#ifdef __cplusplus
}
#endif

#endif // !