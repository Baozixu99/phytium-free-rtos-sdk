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
 * FilePath: fxmac_phy.h
 * Date: 2022-04-06 14:46:52
 * LastEditTime: 2022-04-06 14:46:58
 * Description:  This file is for phy configuration.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe    2022/06/16    first release
 */

#ifndef FXMAC_PHY_H
#define FXMAC_PHY_H

#include "ftypes.h"
#include "ferror_code.h"
#include "fxmac.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define XMAC_PHY_RESET_ENABLE  1
#define XMAC_PHY_RESET_DISABLE 0

enum
{
    FXMAC_PHY_AUTONEGOTIATION_DISABLE = 0,
    FXMAC_PHY_AUTONEGOTIATION_ENABLE
};

enum
{
    FXMAC_PHY_MODE_HALFDUPLEX = 0,
    FXMAC_PHY_MODE_FULLDUPLEX
};

/* phy interface */
FError FXmacPhyWrite(FXmac *instance_p, u32 phy_address, u32 register_num, u16 phy_data);
FError FXmacPhyRead(FXmac *instance_p, u32 phy_address, u32 register_num, u16 *phydat_aptr);
FError FXmacPhyInit(FXmac *instance_p, u32 speed, u32 duplex_mode,
                    u32 autonegotiation_en, u32 reset_flag);

#ifdef __cplusplus
}
#endif

#endif // !