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
 * FilePath: phy_yt.c
 * Date: 2022-04-06 14:46:52
 * LastEditTime: 2022-04-06 14:46:58
 * Description:  This file is for  yt PHYs chip
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe    2022/06/16    first release
 */

#include "phy_yt.h"
#include "fdrivers_port.h"

#define PHY_YT_DEBUG_TAG "PHY_YT"
#define PHY_YT_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(PHY_YT_DEBUG_TAG, format, ##__VA_ARGS__)
#define PHY_YT_INFO(format, ...) \
    FT_DEBUG_PRINT_I(PHY_YT_DEBUG_TAG, format, ##__VA_ARGS__)
#define PHY_YT_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(PHY_YT_DEBUG_TAG, format, ##__VA_ARGS__)
#define PHY_YT_WARN(format, ...) \
    FT_DEBUG_PRINT_W(PHY_YT_DEBUG_TAG, format, ##__VA_ARGS__)

FError PhyYtCheckConnectStatus(void *instance_p, u32 phy_addr, EthPhyWrite write_p, EthPhyRead read_p)
{
    u16 phy_reg0 = 0;
    FError status;
    /*  It's the address offset of the extended register
    that will be Write or Read  */
    status = write_p(instance_p, phy_addr, 0x1e, 0xa001);
    if (status != FT_SUCCESS)
    {
        PHY_YT_ERROR("Write 0xa001 to 0x1e failed.");
        return FETH_PHY_ERR_READ;
    }

    status = read_p(instance_p, phy_addr, 0x1f, &phy_reg0);
    if (status != FT_SUCCESS)
    {
        PHY_YT_ERROR("Read 0x1f reg failed.");
        return FETH_PHY_ERR_READ;
    }

    PHY_YT_INFO("Phy reg0 status is 0x%x", phy_reg0);

    return FT_SUCCESS;
}

FError PhyYtSetLoopBack(void *instance_p, u32 phy_addr, EthPhyWrite write_p, EthPhyRead read_p)
{
    FError status;
    u16 phy_reg0 = 0;

    status = read_p(instance_p, phy_addr, 0, &phy_reg0);
    if (status != FT_SUCCESS)
    {
        PHY_YT_ERROR("Error setup phy loopback.");
        return FETH_PHY_ERR_READ;
    }
    PHY_YT_INFO("Phy reg 0 is 0x%x", phy_reg0);
    /*
     * Enable loopback
     */
    phy_reg0 |= PHY_CONTROL_LOOPBACK_MASK;
    status = write_p(instance_p, phy_addr, 0, phy_reg0);

    status = read_p(instance_p, phy_addr, 0, &phy_reg0);
    if (status != FT_SUCCESS)
    {
        PHY_YT_ERROR("Error setup phy loopback.");
        return FETH_PHY_ERR_READ;
    }

    status = read_p(instance_p, phy_addr, 0, &phy_reg0);
    if (status != FT_SUCCESS)
    {
        PHY_YT_ERROR("Error setup phy loopback.");
        return FETH_PHY_ERR_READ;
    }
    PHY_YT_INFO("Phy reg 0 is 0x%x", phy_reg0);

    return FT_SUCCESS;
}


FError PhyChangeModeToSgmii(void *instance_p, u32 phy_addr, EthPhyWrite write_p, EthPhyRead read_p)
{
    FError status;
    u16 phy_reg0 = 0;

    /* read default mode */
    status = write_p(instance_p, phy_addr, 0x1e, 0xa001);
    if (status != FT_SUCCESS)
    {
        PHY_YT_ERROR("Write 0xa001 to reg 0x1e failed.");
        return FETH_PHY_ERR_READ;
    }


    status = read_p(instance_p, phy_addr, 0x1f, &phy_reg0);
    if (status != FT_SUCCESS)
    {
        PHY_YT_ERROR("Read reg 0x1f failed.");
        return FETH_PHY_ERR_READ;
    }

    PHY_YT_INFO("Default 0x1f status is 0x%x", phy_reg0);

    /* change mode to sds */
    status = write_p(instance_p, phy_addr, 0x1e, 0xa001);
    if (status != FT_SUCCESS)
    {
        PHY_YT_ERROR("Write 0xa001 to reg 0x1e failed.");
        return FETH_PHY_ERR_READ;
    }

    status = write_p(instance_p, phy_addr, 0x1f, 0x8063);
    if (status != FT_SUCCESS)
    {
        PHY_YT_ERROR("Write 0x8063 to reg 0x1f failed.");
        return FETH_PHY_ERR_READ;
    }

    /* read changged mode */
    status = write_p(instance_p, phy_addr, 0x1e, 0xa001);
    if (status != FT_SUCCESS)
    {
        PHY_YT_ERROR("Write 0xa001 to reg 0x1e failed.");
        return FETH_PHY_ERR_READ;
    }


    status = read_p(instance_p, phy_addr, 0x1f, &phy_reg0);
    if (status != FT_SUCCESS)
    {
        PHY_YT_ERROR("Read reg 0x1f failed.");
        return FETH_PHY_ERR_READ;
    }

    PHY_YT_INFO("Read reg 0x1f status is 0x%x", phy_reg0);

    return FT_SUCCESS;
}

FError PhyChangeModeToSds(void *instance_p, u32 phy_addr, EthPhyWrite write_p, EthPhyRead read_p)
{
    FError status;
    u16 phy_reg0 = 0;

    /* read default mode */
    status = write_p(instance_p, phy_addr, 0x1e, 0xa000);
    if (status != FT_SUCCESS)
    {
        PHY_YT_ERROR("Write 0xa000 to reg 0x1e failed.");
        return FETH_PHY_ERR_READ;
    }

    status = read_p(instance_p, phy_addr, 0x1f, &phy_reg0);
    if (status != FT_SUCCESS)
    {
        PHY_YT_ERROR("Read reg 0x1f failed.");
        return FETH_PHY_ERR_READ;
    }

    PHY_YT_INFO("Read reg 0x1f status is 0x%x \r\n", phy_reg0);

    /* change mode to sds */
    status = write_p(instance_p, phy_addr, 0x1e, 0xa000);
    if (status != FT_SUCCESS)
    {
        PHY_YT_ERROR("Write 0xa000 to reg 0x1e failed.");
        return FETH_PHY_ERR_READ;
    }

    status = write_p(instance_p, phy_addr, 0x1f, 0x2);
    if (status != FT_SUCCESS)
    {
        PHY_YT_ERROR("Write 0x02 to reg 0x1f failed.");
        return FETH_PHY_ERR_READ;
    }

    /* read changged mode */
    status = write_p(instance_p, phy_addr, 0x1e, 0xa000);
    if (status != FT_SUCCESS)
    {
        PHY_YT_ERROR("Write 0xa000 to reg 0x1e failed.");
        return FETH_PHY_ERR_READ;
    }

    status = read_p(instance_p, phy_addr, 0x1f, &phy_reg0);
    if (status != FT_SUCCESS)
    {
        PHY_YT_ERROR("Read reg 0x1f failed.");
        return FETH_PHY_ERR_READ;
    }

    PHY_YT_INFO("Read reg 0x1f status is 0x%x \r\n", phy_reg0);

    return FT_SUCCESS;
}
