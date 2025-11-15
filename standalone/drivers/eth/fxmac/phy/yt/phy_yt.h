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
 * FilePath: phy_yt.h
 * Date: 2022-04-06 14:46:52
 * LastEditTime: 2022-04-06 14:46:58
 * Description:  This file is for yt PHYs chip.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe    2022/06/16    first release
 */

#ifndef PHY_YT_H
#define PHY_YT_H

#include "ferror_code.h"
#include "eth_ieee_reg.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define FETH_PHY_ERR_READ FT_MAKE_ERRCODE(ErrModBsp, ErrEthPhy, 0x1u)

FError PhyYtSetLoopBack(void *instance_p, u32 phy_addr, EthPhyWrite write_p, EthPhyRead read_p);
FError PhyYtCheckConnectStatus(void *instance_p, u32 phy_addr, EthPhyWrite write_p, EthPhyRead read_p);
FError PhyChangeModeToSds(void *instance_p, u32 phy_addr, EthPhyWrite write_p, EthPhyRead read_p);
FError PhyChangeModeToSgmii(void *instance_p, u32 phy_addr, EthPhyWrite write_p, EthPhyRead read_p);

#ifdef __cplusplus
}
#endif

#endif // !