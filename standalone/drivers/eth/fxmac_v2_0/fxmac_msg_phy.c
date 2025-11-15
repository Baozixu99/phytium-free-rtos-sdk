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
 * FilePath: fxmac_msg_phy.c
 * Date: 2024-10-28 14:46:52
 * LastEditTime: 2024-10-28 14:46:52
 * Description:  This file is for phy types.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huangjin   2024/10/28  first release
 */

#include "fdrivers_port.h"
#include "fxmac_msg_phy.h"
#include "fxmac_msg_common.h"
#include "fxmac_msg_hw.h"

#include "eth_ieee_reg.h"

#define FXMAC_MSG_DEBUG_TAG "FXMAC_MSG_PHY"
#define FXMAC_MSG_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FXMAC_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
#define FXMAC_MSG_INFO(format, ...) \
    FT_DEBUG_PRINT_I(FXMAC_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
#define FXMAC_MSG_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FXMAC_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
#define FXMAC_MSG_WARN(format, ...) \
    FT_DEBUG_PRINT_W(FXMAC_MSG_DEBUG_TAG, format, ##__VA_ARGS__)

static FXmacMsgCtrl *instance_b;
static u32 phy_addr_b;

void FXmacMsgInterfaceConfig(FXmacMsgCtrl *pdata, unsigned int mode)
{
    FXmacMsgInterfaceInfo para;
    u16 cmd_id, cmd_subid;
    u8 autoneg = 1;

    if (pdata->config.interface == FXMAC_MSG_PHY_INTERFACE_MODE_SGMII)
    {
        if (pdata->config.speed == FXMAC_MSG_SPEED_2500 || mode == MLO_AN_FIXED)
        {
            autoneg = 0;
        }
    }
    else if (pdata->config.interface == FXMAC_MSG_PHY_INTERFACE_MODE_2500BASEX)
    {
        autoneg = 0;
    }
    else
    {
        autoneg = 1;
    }

    memset(&para, 0, sizeof(para));
    cmd_id = FXMAC_MSG_CMD_SET;
    cmd_subid = FXMAC_MSG_CMD_SET_INIT_MAC_CONFIG;
    para.interface = pdata->config.interface;
    para.autoneg = autoneg;
    para.speed = pdata->config.speed;
    para.duplex = pdata->config.duplex;
    pdata->autoneg = para.autoneg;
    FXmacMsgSendMessage(pdata, cmd_id, cmd_subid, (void *)(&para), sizeof(para), 1);
}

int FXmacMsgInterfaceLinkup(FXmacMsgCtrl *pdata, FXmacMsgPhyInterface interface, int speed, int duplex)
{
    FXmacMsgInterfaceInfo para;
    u16 cmd_id, cmd_subid;

    memset(&para, 0, sizeof(para));
    cmd_id = FXMAC_MSG_CMD_SET;
    cmd_subid = FXMAC_MSG_CMD_SET_MAC_LINK_CONFIG;
    para.interface = interface;
    para.duplex = duplex;
    para.speed = speed;
    para.autoneg = pdata->autoneg;
    FXmacMsgSendMessage(pdata, cmd_id, cmd_subid, (void *)(&para), sizeof(para), 1);

    return 0;
}

int FXmacMsgInterfaceLinkdown(FXmacMsgCtrl *pdata)
{
    return 0;
}

int FXmacMsgPcsLinkup(FXmacMsgCtrl *pdata, FXmacMsgPhyInterface interface, int speed, int duplex)
{
    FXmacMsgInterfaceInfo para;
    u16 cmd_id, cmd_subid;

    if (interface == FXMAC_MSG_PHY_INTERFACE_MODE_USXGMII || interface == FXMAC_MSG_PHY_INTERFACE_MODE_10GBASER)
    {
        memset(&para, 0, sizeof(para));
        cmd_id = FXMAC_MSG_CMD_SET;
        cmd_subid = FXMAC_MSG_CMD_SET_PCS_LINK_UP;
        para.interface = interface;
        para.duplex = duplex;
        para.speed = speed;
        para.autoneg = 0;
        FXmacMsgSendMessage(pdata, cmd_id, cmd_subid, (void *)(&para), sizeof(para), 1);
    }

    return 0;
}

int FXmacMsgPcsLinkdown(FXmacMsgCtrl *pdata)
{
    return 0;
}

unsigned int FXmacMsgPcsGetLink(FXmacMsgCtrl *pdata, FXmacMsgPhyInterface interface)
{
    if (interface == FXMAC_MSG_PHY_INTERFACE_MODE_SGMII || interface == FXMAC_MSG_PHY_INTERFACE_MODE_2500BASEX)
    {
        return FXMAC_MSG_READ_BITS(pdata, FXMAC_MSG_NETWORK_STATUS, LINK);
    }
    else if (interface == FXMAC_MSG_PHY_INTERFACE_MODE_USXGMII || interface == FXMAC_MSG_PHY_INTERFACE_MODE_10GBASER)
    {
        return FXMAC_MSG_READ_BITS(pdata, FXMAC_MSG_USX_LINK_STATUS, USX_LINK);
    }

    return 0;
}

static FError FXmacMsgDetect(FXmacMsgCtrl *instance_p, u32 *phy_addr_p)
{
    u32 phy_addr = 0;
    u16 phy_reg = 0, phy_id1_reg, phy_id2_reg;
    instance_b = instance_p;

    for (phy_addr = 0; phy_addr < FXMAC_MSG_PHY_MAX_NUM; phy_addr++)
    {
        phy_reg = FXmacMsgMdioDataReadC22(instance_p, phy_addr, PHY_STATUS_REG_OFFSET);
        FXMAC_MSG_INFO("Phy status reg is %x", phy_reg);
        if (phy_reg != 0xffff)
        {
            phy_id1_reg = FXmacMsgMdioDataReadC22(instance_p, phy_addr, PHY_IDENTIFIER_1_REG);
            phy_id2_reg = FXmacMsgMdioDataReadC22(instance_p, phy_addr, PHY_IDENTIFIER_2_REG);
            FXMAC_MSG_INFO("Phy id1 reg is 0x%x, Phy id2 reg is 0x%x", phy_id1_reg, phy_id2_reg);
            if ((phy_id2_reg != 0) && (phy_id2_reg != 0xffff) && (phy_id1_reg != 0xffff))
            {
                *phy_addr_p = phy_addr;
                phy_addr_b = phy_addr;
                FXMAC_MSG_INFO("Phy addr is 0x%x", phy_addr);
                return FT_SUCCESS;
            }
        }
    }

    return FXMAC_MSG_PHY_IS_NOT_FOUND;
}

/**
 * @name: FXmacMsgPhyReset
 * @msg: Perform phy software reset
 * @param {FXmacMsgCtrl} *instance_p, instance of FXmac controller
 * @param {u32} phy_addr, phy address connect to fxmac
 * @return err code information, FT_SUCCESS indicates success，others indicates failed
 */
static FError FXmacMsgPhyReset(FXmacMsgCtrl *instance_p, u32 phy_addr)
{
    u16 control;
    FError ret = FT_SUCCESS;

    control = FXmacMsgMdioDataReadC22(instance_p, phy_addr, PHY_CONTROL_REG_OFFSET);
    control |= PHY_CONTROL_RESET_MASK;

    ret = FXmacMsgMdioDataWriteC22(instance_p, phy_addr, PHY_CONTROL_REG_OFFSET, control);
    if (ret != FT_SUCCESS)
    {
        FXMAC_MSG_ERROR("%s:%d,write PHY_CONTROL_REG_OFFSET is error", __func__, __LINE__);
        return ret;
    }

    do
    {
        control = FXmacMsgMdioDataReadC22(instance_p, phy_addr, PHY_CONTROL_REG_OFFSET);
    } while ((control & PHY_CONTROL_RESET_MASK) != 0);

    FXMAC_MSG_INFO(" Phy reset end.");
    return ret;
}

static FError FXmacMsgGetIeeePhySpeed(FXmacMsgCtrl *instance_p, u32 phy_addr)
{
    u16 temp, temp2;
    u16 control;
    u16 status;
    u32 negotitation_timeout_cnt = 0;
    FError ret;
    FUNUSED(temp2);
    FXMAC_MSG_INFO("Start phy auto negotiation.");

    control = FXmacMsgMdioDataReadC22(instance_p, phy_addr, PHY_CONTROL_REG_OFFSET);
    control |= PHY_CONTROL_AUTONEGOTIATE_ENABLE;
    control |= PHY_CONTROL_AUTONEGOTIATE_RESTART;
    ret = FXmacMsgMdioDataWriteC22(instance_p, phy_addr, PHY_CONTROL_REG_OFFSET, control);
    if (ret != FT_SUCCESS)
    {
        FXMAC_MSG_ERROR("%s:%d,write PHY_CONTROL_REG_OFFSET is error", __func__, __LINE__);
        return ret;
    }

    FXMAC_MSG_INFO("Waiting for phy to complete auto negotiation.");

    do
    {
        FDriverMdelay(50);
        status = FXmacMsgMdioDataReadC22(instance_p, phy_addr, PHY_STATUS_REG_OFFSET);

        if (negotitation_timeout_cnt++ >= 0xff)
        {
            FXMAC_MSG_ERROR("Auto negotiation is error.");
            return FXMAC_MSG_PHY_AUTO_AUTONEGOTIATION_FAILED;
        }
    } while (!(status & PHY_STATUS_AUTONEGOTIATE_COMPLETE));

    FXMAC_MSG_INFO("Auto negotiation complete.");

    temp = FXmacMsgMdioDataReadC22(instance_p, phy_addr, PHY_SPECIFIC_STATUS_REG);
    FXMAC_MSG_INFO("Temp is 0x%x", temp);
    temp2 = FXmacMsgMdioDataReadC22(instance_p, phy_addr, PHY_STATUS_REG_OFFSET);
    FXMAC_MSG_INFO("Temp2 is 0x%x", temp2);

    if (temp & (1 << 13))
    {
        FXMAC_MSG_INFO("Duplex is full.");
        instance_p->config.duplex = 1;
    }
    else
    {
        FXMAC_MSG_INFO("Duplex is half.");
        instance_p->config.duplex = 0;
    }

    if ((temp & 0xC000) == PHY_SPECIFIC_STATUS_SPEED_1000M)
    {
        FXMAC_MSG_INFO("Speed is 1000M.");
        instance_p->config.speed = 1000;
    }
    else if ((temp & 0xC000) == PHY_SPECIFIC_STATUS_SPEED_100M)
    {
        FXMAC_MSG_INFO("Speed is 100M.");
        instance_p->config.speed = 100;
    }
    else
    {
        FXMAC_MSG_INFO("Speed is 10M.");
        instance_p->config.speed = 10;
    }

    return FT_SUCCESS;
}

static FError FXmacMsgConfigureIeeePhySpeed(FXmacMsgCtrl *instance_p, u32 phy_addr,
                                            u32 speed, u32 duplex_mode)
{
    u16 control;
    u16 autonereg;
    FError ret;
    u16 specific_reg = 0;

    FXMAC_MSG_INFO("Manual setting ,phy_addr is %d,speed %d, duplex_mode is %d.",
                   phy_addr, speed, duplex_mode);

    autonereg = FXmacMsgMdioDataReadC22(instance_p, phy_addr, PHY_AUTONEGO_ADVERTISE_REG);
    autonereg |= PHY_AUTOADVERTISE_ASYMMETRIC_PAUSE_MASK;
    autonereg |= PHY_AUTOADVERTISE_PAUSE_MASK;
    ret = FXmacMsgMdioDataWriteC22(instance_p, phy_addr, PHY_AUTONEGO_ADVERTISE_REG, autonereg);
    if (ret != FT_SUCCESS)
    {
        FXMAC_MSG_ERROR("%s:%d,write PHY_AUTONEGO_ADVERTISE_REG is error.", __func__, __LINE__);
        return ret;
    }


    control = FXmacMsgMdioDataReadC22(instance_p, phy_addr, PHY_CONTROL_REG_OFFSET);
    FXMAC_MSG_INFO("PHY_CONTROL_REG_OFFSET is 0x%x.", control);
    control &= ~PHY_CONTROL_LINKSPEED_1000M;
    control &= ~PHY_CONTROL_LINKSPEED_100M;
    control &= ~PHY_CONTROL_LINKSPEED_10M;

    if (speed == 100)
    {
        control |= PHY_CONTROL_LINKSPEED_100M;
    }
    else if (speed == 10)
    {
        control |= PHY_CONTROL_LINKSPEED_10M;
    }

    if (duplex_mode == 1)
    {
        control |= PHY_CONTROL_FULL_DUPLEX_MASK;
    }
    else
    {
        control &= ~PHY_CONTROL_FULL_DUPLEX_MASK;
    }

    /* disable auto-negotiation */
    control &= ~(PHY_CONTROL_AUTONEGOTIATE_ENABLE);
    control &= ~(PHY_CONTROL_AUTONEGOTIATE_RESTART);

    /* Technology Ability Field */
    ret = FXmacMsgMdioDataWriteC22(instance_p, phy_addr, PHY_CONTROL_REG_OFFSET, control);
    if (ret != FT_SUCCESS)
    {
        FXMAC_MSG_ERROR("%s:%d,write PHY_AUTONEGO_ADVERTISE_REG is error.", __func__, __LINE__);
        return ret;
    }

    FDriverMdelay(1500);

    FXMAC_MSG_INFO("Manual selection completed.");

    specific_reg = FXmacMsgMdioDataReadC22(instance_p, phy_addr, PHY_SPECIFIC_STATUS_REG);
    FXMAC_MSG_INFO("Specific reg is 0x%x.", specific_reg);

    if (specific_reg & (1 << 13))
    {
        FXMAC_MSG_INFO("Duplex is full.");
        instance_p->config.duplex = 1;
    }
    else
    {
        FXMAC_MSG_INFO("Duplex is half.");
        instance_p->config.duplex = 0;
    }

    if ((specific_reg & 0xC000) == PHY_SPECIFIC_STATUS_SPEED_100M)
    {
        FXMAC_MSG_INFO("Speed is 100M.");
        instance_p->config.speed = 100;
    }
    else
    {
        FXMAC_MSG_INFO("Speed is 10M.");
        instance_p->config.speed = 10;
    }

    return FT_SUCCESS;
}

/**
 * @name: FXmacMsgPhyInit
 * @msg:  setup the PHYs for proper speed setting.
 * @param {FXmacMsgCtrl} *instance_p is a pointer to the instance to be worked on.
 * @param {u32} speed is phy operating speed
 * @param {u32} phy_addr is the address of the PHY to be read (supports multiple PHYs)
 * @param {u32} duplex_mode is The duplex mode can be selected via either the Auto-Negotiation process or manual duplex selection.
 * @param {u32} autonegotiation_en is an auto-negotiated flag . 1 is enable auto ,0 is manual
 * @param {u32} reset_flag is a flag which indicates whether to reset xmac phy.
 * @return {FError}
 */
FError FXmacMsgPhyInit(FXmacMsgCtrl *instance_p, u32 speed, u32 duplex_mode,
                       u32 autonegotiation_en, u32 reset_flag)
{
    FError ret;
    u32 phy_addr;

    if (FXmacMsgDetect(instance_p, &phy_addr) != FT_SUCCESS)
    {
        FXMAC_MSG_ERROR("Phy is not found.");
        return FXMAC_MSG_PHY_IS_NOT_FOUND;
    }

    FXMAC_MSG_INFO("Setting phy addr is %d.", phy_addr);
    instance_p->phy_address = phy_addr;
    if (reset_flag)
    {
        FXmacMsgPhyReset(instance_p, phy_addr);
    }

    if (autonegotiation_en)
    {
        ret = FXmacMsgGetIeeePhySpeed(instance_p, phy_addr);
        if (ret != FT_SUCCESS)
        {
            return ret;
        }
    }
    else
    {
        FXMAC_MSG_INFO("Set the communication speed manually.");
        FASSERT_MSG(speed != FXMAC_MSG_SPEED_1000, "The speed must be 100M or 10M!");
        ret = FXmacMsgConfigureIeeePhySpeed(instance_p, phy_addr, speed, duplex_mode);
        if (ret != FT_SUCCESS)
        {
            FXMAC_MSG_ERROR("Failed to manually set the phy.");
            return ret;
        }
    }

    instance_p->link_status = FXMAC_MSG_LINKUP;
    return FT_SUCCESS;
}
