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
 * FilePath: fdp.c
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

#include <string.h>
#include "fio.h"
#include "ftypes.h"
#include "fdrivers_port.h"
#include "fassert.h"

#include "fdc_hw.h"
#include "fdp.h"
#include "fdp_hw.h"
#include "fdcdp_reg.h"
#include "fdcdp_aux.h"

/***************************** Macro Definitions **********/
#define FDP_DEBUG_TAG          "FDP"
#define FDP_ERROR(format, ...) FT_DEBUG_PRINT_E(FDP_DEBUG_TAG, format, ##__VA_ARGS__)
#define FDP_INFO(format, ...)  FT_DEBUG_PRINT_I(FDP_DEBUG_TAG, format, ##__VA_ARGS__)
#define FDP_DEBUG(format, ...) FT_DEBUG_PRINT_D(FDP_DEBUG_TAG, format, ##__VA_ARGS__)

/***************************** Type Definitions **********/
static u32 MgnfsTable[4][4] = {{0x0026, 0x0013, 0x0012, 0x0000},
                               {0x0013, 0x0006, 0x0000, 0x0000},
                               {0x0006, 0x0000, 0x0000, 0x0000},
                               {0x0000, 0x0000, 0x0000, 0x0000}};

static u32 CpostTable[4][4] = {{0x0000, 0x0014, 0x0020, 0x002A},
                               {0x0000, 0x0012, 0x001F, 0x0000},
                               {0x0000, 0x0013, 0x0000, 0x0000},
                               {0x0000, 0x0000, 0x0000, 0x0000}};

/* [reg][ling_rate 1.62->8.1] */
static int vco_val[12][4] = {
    {0x0509, 0x0509, 0x0509, 0x0509}, /* CP_PADJ		 */
    {0x0f00, 0x0f00, 0x0f00, 0x0f00}, /* CP_IADJ		 */
    {0x0F08, 0x0F08, 0x0F08, 0x0F08}, /* FILT_PADJ		 */
    {0x0061, 0x006C, 0x006C, 0x0051}, /* INTDIV		 */
    {0x3333, 0x0000, 0x0000, 0x0000}, /* FRACDIVL		 */
    {0x0000, 0x0000, 0x0000, 0x0000}, /* FRACDIVH		 */
    {0x0042, 0x0048, 0x0048, 0x0036}, /* HIGH_THR		 */
    {0x0002, 0x0002, 0x0002, 0x0002}, /* PDIAG_CTRL		 */
    {0x0c5e, 0x0c5e, 0x0c5e, 0x0c5e}, /* VCOCAL_PLLCNT_START */
    {0x00c7, 0x00c7, 0x00c7, 0x00c7}, /* LOCK_PEFCNT	 */
    {0x00c7, 0x00c7, 0x00c7, 0x00c7}, /* LOCK_PLLCNT_START	 */
    {0x0005, 0x0005, 0x0005, 0x0005}, /* LOCK_PLLCNT_THR	 */
};
/***************************** Function Prototypes **********/
/**
 * @name: FDpLinkPhyInit
 * @msg:  get the lane count of the dp link
 * @param {FDpCtrl *} instance is the instance of the display
 * @param {u32 *} link_rate is the pointer to store the link rate
 * @return 
 */
void FDpLinkPhyInit(FDpCtrl *instance_p, u32 link_rate)
{
    FASSERT(instance_p != NULL);
    uintptr address = instance_p->config.dp_phy_base_addr;
    /*
    reset,reg 40250
    */
    FDpChannelRegWrite(address, REG_PHY_RESET_DEASSERT, 0x0);
    /*
    dp mode, reg 40034
    */
    FDpChannelRegWrite(address, REG_MODE, 0x3);
    /*
    Pll Enable, reg 40214
    */
    FDpChannelRegWrite(address, REG_PLLCLK_EN, 0x1);
    /*
      link reset, reg 40258
    */
    FDpChannelRegWrite(address, REG_PHY_RESET_LINK_DEASSERT, 0x1);
    /*
      Data width 20 bits, reg 4021c
    */
    FDpChannelRegWrite(address, REG_PMA_DATA_WIDTH, 0x5);
    FDpChannelRegWrite(address, REG_SGMII_DPSEL_INIT, 0x1);
    /*
     link reset, reg 40258
    */
    FDpChannelRegWrite(address, REG_PHY_RESET_LINK_DEASSERT, 0x1);
    /*
      APB Reset, reg 40250
    */
    FDpChannelRegWrite(address, REG_APB_RESET_DEASSERT, 0x1);
    /*
     Phy interal register
    */
    /*
     Config single link
    */
    FDpChannelRegWrite(address, PHY_PLL_CFG, 0);
    /*
      Reference PLL frequence
    */
    switch (link_rate)
    {
        case 162:
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_CLK_SEL_M0, 0xf01);
            FDpChannelRegWrite(address, XCVR_DIAG_HSCLK_SEL, 0x0); /*Select Analog High Speed Clock*/
            FDpChannelRegWrite(address, XCVR_DIAG_HSCLK_DIV, 0x2); /*Select div of High Speed Clock*/
            FDpChannelRegWrite(address, XCVR_DIAG_PLLDRC_CTRL, 0x1); /*Select Digital PLL and Data Speed */
            /*
        Config PLL for all VCO
        */
            FDpChannelRegWrite(address, CMN_PLL0_DSM_DIAG_M0, 0x4);
            FDpChannelRegWrite(address, CMN_PLL0_VCOCAL_REFTIM_START, 0xc5e);
            FDpChannelRegWrite(address, CMN_PLL0_VCOCAL_TCTRL, 0x3);
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_CP_PADJ_M0, 0x509);
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_CP_IADJ_M0, 0xf00);
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_FILT_PADJ_M0, 0xf08);
            FDpChannelRegWrite(address, CMN_PLL0_INTDIV_M0, 0x061);
            FDpChannelRegWrite(address, CMN_PLL0_FRACDIVL_M0, 0x3333);
            FDpChannelRegWrite(address, CMN_PLL0_FRACDIVH_M0, 0x0);
            FDpChannelRegWrite(address, CMN_PLL0_HIGH_THR_M0, 0x42);
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_CTRL_M0, 0x02);
            FDpChannelRegWrite(address, CMN_PLL0_VCOCAL_PLLCNT_START, 0xc5e);
            FDpChannelRegWrite(address, CMN_PLL0_LOCK_REFCNT_START, 0xc7);
            FDpChannelRegWrite(address, CMN_PLL0_LOCK_PLLCNT_START, 0xc7);
            FDpChannelRegWrite(address, CMN_PLL0_LOCK_PLLCNT_THR, 0x05);
            break;
        case 270:
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_CLK_SEL_M0, 0x701);
            FDpChannelRegWrite(address, XCVR_DIAG_HSCLK_SEL, 0x0);
            FDpChannelRegWrite(address, XCVR_DIAG_HSCLK_DIV, 0x2);
            FDpChannelRegWrite(address, XCVR_DIAG_PLLDRC_CTRL, 0x1);
            FDpChannelRegWrite(address, CMN_PLL0_DSM_DIAG_M0, 0x4);
            FDpChannelRegWrite(address, CMN_PLL0_VCOCAL_REFTIM_START, 0xc5e);
            FDpChannelRegWrite(address, CMN_PLL0_VCOCAL_TCTRL, 0x3);
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_CP_PADJ_M0, 0x509);
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_CP_IADJ_M0, 0xf00);
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_FILT_PADJ_M0, 0xf08);
            FDpChannelRegWrite(address, CMN_PLL0_INTDIV_M0, 0x06c);
            FDpChannelRegWrite(address, CMN_PLL0_FRACDIVL_M0, 0x0);
            FDpChannelRegWrite(address, CMN_PLL0_FRACDIVH_M0, 0x0);
            FDpChannelRegWrite(address, CMN_PLL0_HIGH_THR_M0, 0x48);
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_CTRL_M0, 0x02);
            FDpChannelRegWrite(address, CMN_PLL0_VCOCAL_PLLCNT_START, 0xc5e);
            FDpChannelRegWrite(address, CMN_PLL0_LOCK_REFCNT_START, 0xc7);
            FDpChannelRegWrite(address, CMN_PLL0_LOCK_PLLCNT_START, 0xc7);
            FDpChannelRegWrite(address, CMN_PLL0_LOCK_PLLCNT_THR, 0x05);
            break;
        case 540:
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_CLK_SEL_M0, 0x301);
            FDpChannelRegWrite(address, XCVR_DIAG_HSCLK_SEL, 0x0);
            FDpChannelRegWrite(address, XCVR_DIAG_HSCLK_DIV, 0x2);
            FDpChannelRegWrite(address, XCVR_DIAG_PLLDRC_CTRL, 0x1);
            FDpChannelRegWrite(address, CMN_PLL0_DSM_DIAG_M0, 0x4);
            FDpChannelRegWrite(address, CMN_PLL0_VCOCAL_REFTIM_START, 0xc5e);
            FDpChannelRegWrite(address, CMN_PLL0_VCOCAL_TCTRL, 0x3);
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_CP_PADJ_M0, 0x509);
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_CP_IADJ_M0, 0xf00);
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_FILT_PADJ_M0, 0xf08);
            FDpChannelRegWrite(address, CMN_PLL0_INTDIV_M0, 0x06c);
            FDpChannelRegWrite(address, CMN_PLL0_FRACDIVL_M0, 0x0);
            FDpChannelRegWrite(address, CMN_PLL0_FRACDIVH_M0, 0x0);
            FDpChannelRegWrite(address, CMN_PLL0_HIGH_THR_M0, 0x48);
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_CTRL_M0, 0x02);
            FDpChannelRegWrite(address, CMN_PLL0_VCOCAL_PLLCNT_START, 0xc5e);
            FDpChannelRegWrite(address, CMN_PLL0_LOCK_REFCNT_START, 0xc7);
            FDpChannelRegWrite(address, CMN_PLL0_LOCK_PLLCNT_START, 0xc7);
            FDpChannelRegWrite(address, CMN_PLL0_LOCK_PLLCNT_THR, 0x05);
            break;
        case 810:
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_CLK_SEL_M0, 0x200);
            FDpChannelRegWrite(address, XCVR_DIAG_HSCLK_SEL, 0x0);
            FDpChannelRegWrite(address, XCVR_DIAG_HSCLK_DIV, 0x2);
            FDpChannelRegWrite(address, XCVR_DIAG_PLLDRC_CTRL, 0x1);
            FDpChannelRegWrite(address, CMN_PLL0_DSM_DIAG_M0, 0x4);
            FDpChannelRegWrite(address, CMN_PLL0_VCOCAL_REFTIM_START, 0xc5e);
            FDpChannelRegWrite(address, CMN_PLL0_VCOCAL_TCTRL, 0x3);
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_CP_PADJ_M0, 0x509);
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_CP_IADJ_M0, 0xf00);
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_FILT_PADJ_M0, 0xf08);
            FDpChannelRegWrite(address, CMN_PLL0_INTDIV_M0, 0x051);
            FDpChannelRegWrite(address, CMN_PLL0_FRACDIVL_M0, 0x0);
            FDpChannelRegWrite(address, CMN_PLL0_FRACDIVH_M0, 0x036);
            FDpChannelRegWrite(address, CMN_PLL0_HIGH_THR_M0, 0x42);
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_CTRL_M0, 0x02);
            FDpChannelRegWrite(address, CMN_PLL0_VCOCAL_PLLCNT_START, 0xc5e);
            FDpChannelRegWrite(address, CMN_PLL0_LOCK_REFCNT_START, 0xc7);
            FDpChannelRegWrite(address, CMN_PLL0_LOCK_PLLCNT_START, 0xc7);
            FDpChannelRegWrite(address, CMN_PLL0_LOCK_PLLCNT_THR, 0x05);
            break;
        default:
            FDP_DEBUG("the current is invalid.use the default link-rate 162.");
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_CLK_SEL_M0, 0xf01);
            FDpChannelRegWrite(address, XCVR_DIAG_HSCLK_SEL, 0x0);
            FDpChannelRegWrite(address, XCVR_DIAG_HSCLK_DIV, 0x2);
            FDpChannelRegWrite(address, XCVR_DIAG_PLLDRC_CTRL, 0x1);
            FDpChannelRegWrite(address, CMN_PLL0_DSM_DIAG_M0, 0x4);
            FDpChannelRegWrite(address, CMN_PLL0_VCOCAL_REFTIM_START, 0xc5e);
            FDpChannelRegWrite(address, CMN_PLL0_VCOCAL_TCTRL, 0x3);
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_CP_PADJ_M0, 0x509);
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_CP_IADJ_M0, 0xf00);
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_FILT_PADJ_M0, 0xf08);
            FDpChannelRegWrite(address, CMN_PLL0_INTDIV_M0, 0x061);
            FDpChannelRegWrite(address, CMN_PLL0_FRACDIVL_M0, 0x3333);
            FDpChannelRegWrite(address, CMN_PLL0_FRACDIVH_M0, 0x0);
            FDpChannelRegWrite(address, CMN_PLL0_HIGH_THR_M0, 0x42);
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_CTRL_M0, 0x02);
            FDpChannelRegWrite(address, CMN_PLL0_VCOCAL_PLLCNT_START, 0xc5e);
            FDpChannelRegWrite(address, CMN_PLL0_LOCK_REFCNT_START, 0xc7);
            FDpChannelRegWrite(address, CMN_PLL0_LOCK_PLLCNT_START, 0xc7);
            FDpChannelRegWrite(address, CMN_PLL0_LOCK_PLLCNT_THR, 0x05);
            break;
    }
    /*
     Power Control
    */
    FDpChannelRegWrite(address, TX_PSC_A0, 0xfb);
    FDpChannelRegWrite(address, TX_PSC_A2, 0x4aa);
    FDpChannelRegWrite(address, TX_PSC_A3, 0x4aa);
    FDpChannelRegWrite(address, RX_PSC_A0, 0x0);
    FDpChannelRegWrite(address, RX_PSC_A2, 0x0);
    FDpChannelRegWrite(address, RX_PSC_A3, 0x0);
    FDpChannelRegWrite(address, RX_PSC_CAL, 0x0);
    /*
      Transceiver control and diagnosis
    */
    FDpChannelRegWrite(address, XCVR_DIAG_BIDI_CTRL, 0xf);
    /*
      Receiver equalization engine
    */
    FDpChannelRegWrite(address, RX_REE_GCSM1_CTRL, 0x0);
    FDpChannelRegWrite(address, RX_REE_GCSM2_CTRL, 0x0);
    FDpChannelRegWrite(address, RX_REE_PERGCSM_CTRL, 0x0);
    /*
      Voltage swing and pre-emphasis
    */
    FDpChannelRegWrite(address, TX_DIAG_ACYA, 0x1);
    FDpChannelRegWrite(address, TX_TXCC_CTRL, 0x08a4);
    FDpChannelRegWrite(address, DRV_DIAG_TX_DRV, 0x0003);
    /*FDpChannelRegWrite (address, TX_TXCC_MGNFS_MULT_000, 0x0000);
     FDpChannelRegWrite (address, TX_TXCC_CPOST_MULT_00, 0x001c);*/
    FDpChannelRegWrite(address, TX_TXCC_MGNFS_MULT_000, 0x002a);
    FDpChannelRegWrite(address, TX_TXCC_CPOST_MULT_00, 0x0000);
    FDpChannelRegWrite(address, TX_DIAG_ACYA, 0x0);
    /*
      Release reset signal
    */
    FDpChannelRegWrite(address, REG_PHY_RESET_DEASSERT, 0x1);
    FDP_DEBUG("FDpLinkPhyInit end");
    return;
}

/**
 * @name: FDptxPhyGetLaneCount
 * @msg:  get the lane count of the dp link
 * @param {FDpCtrl *} instance is the instance of the display
 * @param {u32 *} lane_count is the pointer to store the link count
 * @return {FError} FDP_SUCCESS if success, otherwise FDP_FAILED.
 */
FError FDptxPhyGetLaneCount(FDpCtrl *instance, u8 *lane_count)
{
    FASSERT(lane_count != NULL);
    FError status = FDP_FAILED;

    status = FDpSinkDpcdRead(instance, MAX_LANE_COUNT, lane_count);
    if (status == FDP_SUCCESS)
    {
        *lane_count = (*lane_count) & 0x1F;
        FDP_DEBUG("Phy max lanecount :%d", *lane_count);
    }
    return status;
}

/**
 * @name: FDpTxPhyGetLinkRate
 * @msg:  get the lane rate of the dp link
 * @param {FDpCtrl *} instance is the instance of the display
 * @param {u32 *} link_rate is the pointer to store the link rate
 * @return {FError} FDP_SUCCESS if success, otherwise FDP_FAILED.
 */
FError FDpTxPhyGetLinkRate(FDpCtrl *instance, u32 *link_rate)
{
    FASSERT(instance != NULL);
    FASSERT(link_rate != NULL);
    u8 value;
    FError status = FDP_FAILED;

    status = FDpSinkDpcdRead(instance, MAX_LINK_RATE, &value);
    if (status == FDP_SUCCESS)
    {
        switch (value)
        {
            case 0x06:
                *link_rate = 162;
                break;
            case 0x0A:
                *link_rate = 270;
                break;
            case 0x14:
                *link_rate = 540;
                break;
            case 0x1E:
                *link_rate = 810;
                break;
            default:
                *link_rate = 270;
                break;
        }
        FDP_DEBUG("Phy max link rate : 0x%02x", value);
    }
    return status;
}

/**
 * @name: FDpHwSetLaneRate
 * @msg:  set the lane rate of the dp link
 * @param {FDpCtrl *} instance is the instance of the display
 * @return {FError} FDP_SUCCESS if success, otherwise FDP_FAILED.
 */
FError FDpHwSetLaneRate(FDpCtrl *instance)
{
    int i = 0, data, tmp, tmp1, index = 0, mask = 0;
    int timeout = 500;
    u32 link_rate = instance->trans_config.link_rate;
    FError ret = FDP_SUCCESS;
    uintptr phyaddr = instance->config.dp_phy_base_addr;


    /* set pma powerdown */
    data = 0;
    for (i = 0; i < instance->trans_config.link_lane_count; i++)
    {
        data |= (A3_POWERDOWN3 << (i * A3_POWERDOWN3_SHIFT));
        FDpChannelRegWrite(phyaddr, REG_XCVR_POWER_STATE_REQ, data);
    }

    /* lane pll disable */
    data = 0;
    for (i = 0; i < instance->trans_config.link_lane_count; i++)
    {
        data |= (PLL_EN << (i * PLL_EN_SHIFT));
        mask |= (((1 << PLL_EN_SHIFT) - 1) << (i * PLL_EN_SHIFT));
    }
    data &= ~mask;
    FDpChannelRegWrite(phyaddr, REG_PLLCLK_EN, data);

    /* pma pll disable */
    data = CONTROL_ENABLE & (~CONTROL_ENABLE_MASK);
    FDpChannelRegWrite(phyaddr, PHY_PMA_PLL_RAW_CTRL, data);

    /* read pma pll disable state */
    FDriverUdelay(2);
    FDpChannelRegRead(phyaddr, PHY_PMA_CMN_CTRL2);

    /* config link rate */
    switch (link_rate)
    {
        case 810000:
            tmp = PLL_LINK_RATE_810000;
            tmp1 = HSCLK_LINK_RATE_810000;
            index = 3;
            break;
        case 540000:
            tmp = PLL_LINK_RATE_540000;
            tmp1 = HSCLK_LINK_RATE_540000;
            index = 2;
            break;
        case 270000:
            tmp = PLL_LINK_RATE_270000;
            tmp1 = HSCLK_LINK_RATE_270000;
            index = 1;
            break;
        case 162000:
            tmp = PLL_LINK_RATE_162000;
            tmp1 = HSCLK_LINK_RATE_162000;
            index = 0;
            break;
        default:
            FDP_ERROR("phytium dp rate(%d) not support\n", link_rate);
            tmp = PLL_LINK_RATE_162000;
            tmp1 = HSCLK_LINK_RATE_162000;
            index = 0;
            break;
    }

    /* config analog pll for link0 */
    FDpChannelRegWrite(phyaddr, CMN_PDIAG_PLL0_CLK_SEL_M0, tmp);
    FDpChannelRegWrite(phyaddr, PHYTIUM_PHY_HSCLK0_SEL, HSCLK_LINK_0);
    FDpChannelRegWrite(phyaddr, PHYTIUM_PHY_HSCLK0_DIV, tmp1);

    /* config digital pll for link0 */
    FDpChannelRegWrite(phyaddr, PHYTIUM_PHY_PLLDRC0_CTRL, PLLDRC_LINK0);

    /* common for all rate */
    FDpChannelRegWrite(phyaddr, CMN_PLL0_DSM_DIAG_M0, PLL0_DSM_M0);
    FDpChannelRegWrite(phyaddr, CMN_PLL0_VCOCAL_REFTIM_START, PLL0_VCOCAL_START);
    FDpChannelRegWrite(phyaddr, CMN_PLL0_VCOCAL_TCTRL, PLL0_VCOCAL_CTRL);

    /* different for all rate */
    FDpChannelRegWrite(phyaddr, CMN_PDIAG_PLL0_CP_PADJ_M0, vco_val[0][index]);
    FDpChannelRegWrite(phyaddr, CMN_PDIAG_PLL0_CP_IADJ_M0, vco_val[1][index]);
    FDpChannelRegWrite(phyaddr, CMN_PDIAG_PLL0_FILT_PADJ_M0, vco_val[2][index]);
    FDpChannelRegWrite(phyaddr, CMN_PLL0_INTDIV_M0, vco_val[3][index]);
    FDpChannelRegWrite(phyaddr, CMN_PLL0_FRACDIVL_M0, vco_val[4][index]);
    FDpChannelRegWrite(phyaddr, CMN_PLL0_FRACDIVH_M0, vco_val[5][index]);
    FDpChannelRegWrite(phyaddr, CMN_PLL0_HIGH_THR_M0, vco_val[6][index]);
    FDpChannelRegWrite(phyaddr, CMN_PDIAG_PLL0_CTRL_M0, vco_val[7][index]);
    FDpChannelRegWrite(phyaddr, CMN_PLL0_VCOCAL_PLLCNT_START, vco_val[8][index]);
    FDpChannelRegWrite(phyaddr, CMN_PLL0_LOCK_REFCNT_START, vco_val[9][index]);
    FDpChannelRegWrite(phyaddr, CMN_PLL0_LOCK_PLLCNT_START, vco_val[10][index]);
    FDpChannelRegWrite(phyaddr, CMN_PLL0_LOCK_PLLCNT_THR, vco_val[11][index]);

    FDpChannelRegWrite(phyaddr, PHYTIUM_PHY_PLL0_TX_PSC_A0, PLL0_TX_PSC_A0);
    FDpChannelRegWrite(phyaddr, PHYTIUM_PHY_PLL0_TX_PSC_A2, PLL0_TX_PSC_A2);
    FDpChannelRegWrite(phyaddr, PHYTIUM_PHY_PLL0_TX_PSC_A3, PLL0_TX_PSC_A3);
    FDpChannelRegWrite(phyaddr, PHYTIUM_PHY_PLL0_RX_PSC_A0, PLL0_RX_PSC_A0);
    FDpChannelRegWrite(phyaddr, PHYTIUM_PHY_PLL0_RX_PSC_A2, PLL0_RX_PSC_A2);
    FDpChannelRegWrite(phyaddr, PHYTIUM_PHY_PLL0_RX_PSC_A3, PLL0_RX_PSC_A3);
    FDpChannelRegWrite(phyaddr, PHYTIUM_PHY_PLL0_RX_PSC_CAL, PLL0_RX_PSC_CAL);

    FDpChannelRegWrite(phyaddr, PHYTIUM_PHY_PLL0_XCVR_CTRL, PLL0_XCVR_CTRL);
    FDpChannelRegWrite(phyaddr, PHYTIUM_PHY_PLL0_RX_GCSM1_CTRL, PLL0_RX_GCSM1_CTRL);
    FDpChannelRegWrite(phyaddr, PHYTIUM_PHY_PLL0_RX_GCSM2_CTRL, PLL0_RX_GCSM2_CTRL);
    FDpChannelRegWrite(phyaddr, PHYTIUM_PHY_PLL0_RX_PERGCSM_CTRL, PLL0_RX_PERGCSM_CTRL);

    /* pma pll enable */
    data = CONTROL_ENABLE;
    FDpChannelRegWrite(phyaddr, PHY_PMA_PLL_RAW_CTRL, data);

    /* lane pll enable */
    data = 0;
    for (i = 0; i < instance->trans_config.link_lane_count; i++)
    {
        data |= (PLL_EN << (i * PLL_EN_SHIFT));
    }
    FDpChannelRegWrite(phyaddr, REG_PLLCLK_EN, data);

    /* set pma power active */
    data = 0;
    for (i = 0; i < instance->trans_config.link_lane_count; i++)
    {
        data |= (A0_ACTIVE << (i * A0_ACTIVE_SHIFT));
    }
    FDpChannelRegWrite(phyaddr, REG_XCVR_POWER_STATE_REQ, data);

    mask = PLL0_LOCK_DONE;
    do
    {
        FDriverUdelay(1);
        timeout--;
        tmp = FDpChannelRegRead(phyaddr, PHY_PMA_CMN_CTRL2);
    } while ((!(tmp & mask)) && timeout);

    if (timeout == 0)
    {
        FDP_ERROR("dp(%d) phy pll lock failed\n");
        ret = FDP_FAILED;
    }
    FDriverUdelay(1);

    return ret;
}

/**
 * @name: FDpInitAux
 * @msg:  init the aux of the dp link
 * @param {FDpCtrl *} instance is the instance of the display
 */
void FDpInitAux(FDpCtrl *instance_p)
{
    FASSERT(instance_p != NULL);
    uintptr address = instance_p->config.dp_channe_base_addr;
    /* disable dptx */
    FDpChannelRegWrite(address, PHYTIUM_DP_TRANSMITTER_OUTPUT_ENABLE, TRANSMITTER_OUTPUT_DISABLE);
    /* dptx clk devide  100MHz / 100*/
    FDpChannelRegWrite(address, PHYTIUM_DP_AUX_CLK_DIVIDER, AUX_CLK_DIVIDER_100);
    /* enable dptx */
    FDpChannelRegWrite(address, PHYTIUM_DP_TRANSMITTER_OUTPUT_ENABLE, TRANSMITTER_OUTPUT_ENABLE);
    /* set dptx power mode to D0 */
    FDpSinkDpcdWrite(instance_p, SET_POWER_SET_DP_PWR_VALTAGE, FDP_PWR_MODE_D0);

    return;
}

/**
 * @name: FDpHwDisableOutput
 * @msg:  disable the output of the dp link
 * @param {FDpCtrl *} instance is the instance of the display
 */
void FDpHwDisableOutput(FDpCtrl *instance)
{
    uintptr addr = instance->config.dp_channe_base_addr;
    FDcChannelRegWrite(addr, PHYTIUM_DP_TRANSMITTER_OUTPUT_ENABLE, TRANSMITTER_OUTPUT_DISABLE);
    FDcChannelRegWrite(addr, PHYTIUM_DP_SOFT_RESET, LINK_SOFT_RESET);

    FDcChannelRegWrite(addr, PHYTIUM_INPUT_SOURCE_ENABLE,
                       (~VIRTUAL_SOURCE_0_ENABLE) & VIRTUAL_SOURCE_0_ENABLE_MASK);
    FDcChannelRegWrite(addr, PHYTIUM_DP_VIDEO_STREAM_ENABLE, SST_MST_SOURCE_0_DISABLE);
}

/**
 * @name: FDpHwEnableOutput
 * @msg:  enable the output of the dp link
 * @param {FDpCtrl *} instance is the instance of the display
 */
void FDpHwEnableOutput(FDpCtrl *instance)
{
    uintptr addr = instance->config.dp_channe_base_addr;
    FDcChannelRegWrite(addr, PHYTIUM_INPUT_SOURCE_ENABLE, VIRTUAL_SOURCE_0_ENABLE);
    FDcChannelRegWrite(addr, PHYTIUM_DP_USER_SYNC_POLARITY, 0xc);

    FDcChannelRegWrite(addr, PHYTIUM_DP_TRANSMITTER_OUTPUT_ENABLE, TRANSMITTER_OUTPUT_ENABLE);
    FDpChannelRegWrite(addr, PHYTIUM_DP_SECONDARY_STREAM_ENABLE, 1);
    FDpChannelRegWrite(addr, PHYTIUM_DP_VIDEO_STREAM_ENABLE, 1);
    FDcChannelRegWrite(addr, PHYTIUM_DP_SOFT_RESET, LINK_SOFT_RESET);
}

void FDpHwSetDpcdTrainPattern(FDpCtrl *instance, u8 pattern)
{
    uint8_t value;

    /* Scrambling is disabled for TPS1/2/3 and enabled for TPS4 */
    if ((pattern == TRAINING_PATTERN_4) || (pattern == TEST_PATTERN_NONE))
    {
        value = pattern;
    }
    else
    {
        value = (pattern | PHYTIUM_DP_SCRAMBLING_DISABLE);
    }
    FDpSinkDpcdWrite(instance, PHYTIUM_DP_TRAINING_PATTERN_SET, value);
}

/**
 * @name: FDpSetTestPattern
 * @msg:  set the training of the dp link
 * @param {FDpCtrl *} instance is the instance of the display
 * @param {u8} lane_count is the lane count of the dp link
 * @param {u8} test_pattern is the test pattern of the dp link
 * @param {u8*} custom_pattern is the custom pattern of the dp link
 * @param {u32} custom_pattern_size is the custom pattern size of the dp link
 */
void FDpSetTestPattern(FDpCtrl *instance, uint8_t lane_count, uint8_t test_pattern,
                       uint8_t *custom_pattern, uint32_t custom_pattern_size)
{
    uintptr addr = instance->config.dp_channe_base_addr;
    int val = 0, tmp = 0, i;
    if ((test_pattern == PHYTIUM_PHY_TP_80BIT_CUSTOM) && custom_pattern && (custom_pattern_size > 0))
    {
        val = *(int *)custom_pattern;
        FDcChannelRegWrite(addr, PHYTIUM_DP_CUSTOM_80BIT_PATTERN_0, 0);
        val = *(int *)(custom_pattern + 4);
        FDcChannelRegWrite(addr, PHYTIUM_DP_CUSTOM_80BIT_PATTERN_1, 0);
        val = *(short int *)(custom_pattern + 8);
        FDcChannelRegWrite(addr, PHYTIUM_DP_CUSTOM_80BIT_PATTERN_2, 0);
    }
    if (test_pattern == PHYTIUM_PHY_TP_D10_2 || test_pattern == PHYTIUM_PHY_TP_PRBS7 ||
        test_pattern == PHYTIUM_PHY_TP_80BIT_CUSTOM)
    {
        FDcChannelRegWrite(addr, PHYTIUM_DP_SCRAMBLING_DISABLE, SCRAMBLING_DISABLE);
    }
    else
    {
        FDcChannelRegWrite(addr, PHYTIUM_DP_SCRAMBLING_DISABLE, SCRAMBLING_ENABLE);
    }

    tmp = test_pattern - PHYTIUM_PHY_TP_NONE + TEST_PATTERN_NONE;
    val = 0;
    for (i = 0; i < lane_count; i++)
    {
        val |= (tmp << (TEST_PATTERN_LANE_SHIFT * i));
    }
    FDcChannelRegWrite(addr, PHYTIUM_DP_LINK_QUAL_PATTERN_SET, val);
}

/**
 * @name: FDpHwSetLink
 * @msg:  set the training  of the dp link
 * @param {FDpCtrl *} instance is the instance of the display
 * @param {u8} lane_count is the lane count of the dp link
 * @param {u8} link_rate is the link rate of the dp link
 * @return {FError} FDP_SUCCESS is success,otherwise is failed
 */
FError FDpHwSetLink(FDpCtrl *instance, u8 lane_count, u8 link_rate)
{
    FError ret = FDP_SUCCESS;
    uintptr addr = instance->config.dp_channe_base_addr;
    u8 value = 0;
    FDpSinkDpcdRead(instance, MAX_LANE_COUNT, &value);
    value = (value >> 7) & 0x01;
    FDcChannelRegWrite(addr, PHYTIUM_DP_LANE_COUNT_SET, lane_count);
    FDcChannelRegWrite(addr, LINK_BW_SET, link_rate / 27); ///27还是27000

    FDcChannelRegWrite(addr, PHYTIUM_DP_ENHANCED_FRAME_EN, value);

    ret = FDpHwSetLaneRate(instance);
    if (ret != FDP_SUCCESS)
    {
        FDP_ERROR("FDpHwSetLaneRate failed");
    }
    return ret;
}

/**
 * @name: FDpDpcdSetLink
 * @msg:  set the training dpcd of the dp link
 * @param {FDpCtrl *} instance is the instance of the display
 * @param {u8} lane_count is the lane count of the dp link
 * @param {u8} link_rate is the link rate of the dp link
 * @return {FError} FDP_SUCCESS is success,otherwise is failed
 */
FError FDpDpcdSetLink(FDpCtrl *instance, uint8_t lane_count, uint8_t link_rate)
{
    FError ret = FDP_SUCCESS;
    u8 link_config[2];
    link_config[0] = link_rate / 27000;
    link_config[1] = lane_count;

    u8 value = 0;
    FDpSinkDpcdRead(instance, MAX_LANE_COUNT, &value);
    value = (value >> 7) & 0x01;
    if (value)
    {
        link_config[1] = 1 << 7;
    }
    ret = FDpSinkDpcdWrite(instance, PHYTIUM_DP_LINK_BW_SET, 0xa);
    if (ret != FDP_SUCCESS)
    {
        FDP_ERROR("FDpSinkDpcdWrite PHYTIUM_DP_LINK_BW_SET failed\n");
        return ret;
    }
    link_config[0] = 0;
    link_config[1] = 1 << 0;
    ret = FDpSinkDpcdWrite(instance, DOWNSPREAD_CTRL, link_config[1]);
    if (ret != FDP_SUCCESS)
    {
        FDP_ERROR("FDpSinkDpcdWrite PHYTIUM_DP_LINK_BW_SET failed\n");
        return ret;
    }

    return ret;
}

void FDpLinkPhyChangeVsWing(FDpCtrl *instance, u32 vswing, u32 pre_emphasis)
{
    FASSERT(instance != NULL);
    uintptr addr = instance->config.dp_phy_base_addr;
    FDpChannelRegWrite(addr, TX_DIAG_ACYA, 0x1);
    FDpChannelRegWrite(addr, TX_TXCC_CTRL, 0x08a4);
    FDpChannelRegWrite(addr, DRV_DIAG_TX_DRV, 0x0003);
    FDpChannelRegWrite(addr, TX_TXCC_MGNFS_MULT_000, MgnfsTable[vswing][pre_emphasis]);
    FDpChannelRegWrite(addr, TX_TXCC_CPOST_MULT_00, CpostTable[vswing][pre_emphasis]);
    FDpChannelRegWrite(addr, TX_DIAG_ACYA, 0x0);
}

static void FDpTxSetTrainingPar(FDpCtrl *instance, u8 *swing, u8 *pre_emphasis, u8 lane_count)
{
    FASSERT(instance != NULL);
    u8 value = 0;
    FDpLinkPhyChangeVsWing(instance, swing[0], pre_emphasis[0]);
    switch (lane_count)
    {
        case 1:
            value = 0;
            value |= swing[0] & 0x03;
            value |= (pre_emphasis[0] & 0x03) << 3;
            FDpSinkDpcdWrite(instance, TRAINING_LANE0_SET, value);
            break;
        case 2:
            value = 0;
            value |= swing[0] & 0x03;
            value |= (pre_emphasis[0] & 0x03) << 3;
            FDpSinkDpcdWrite(instance, TRAINING_LANE0_SET, value);
            value = 0;
            value |= swing[1] & 0x03;
            value |= (pre_emphasis[1] & 0x03) << 3;
            FDpSinkDpcdWrite(instance, TRAINING_LANE1_SET, value);
            break;
        case 4:
            value = 0;
            value |= swing[0] & 0x03;
            value |= (pre_emphasis[0] & 0x03) << 3;
            FDpSinkDpcdWrite(instance, TRAINING_LANE0_SET, value);
            value = 0;
            value |= swing[1] & 0x03;
            value |= (pre_emphasis[1] & 0x03) << 3;
            FDpSinkDpcdWrite(instance, TRAINING_LANE1_SET, value);
            value = 0;
            value |= swing[2] & 0x03;
            value |= (pre_emphasis[2] & 0x03) << 3;
            FDpSinkDpcdWrite(instance, TRAINING_LANE2_SET, value);
            value = 0;
            value |= swing[3] & 0x03;
            value |= (pre_emphasis[3] & 0x03) << 3;
            FDpSinkDpcdWrite(instance, TRAINING_LANE3_SET, value);
            break;
        default:
            FDP_DEBUG("the current lane count max is 4 ,please check it");
            break;
    }
    return;
}
/**
 * @name: FDpTxGetTrainingStatus
 * @msg:  get the training status of the dp link
 * @param {FDpCtrl *} instance is the instance of the display
 * @param {u8} *train_status is the training status of the dp link
 */
void FDpTxGetTrainingStatus(FDpCtrl *instance, u8 *train_status)
{
    FASSERT(instance != NULL);
    u8 value_lane = 0;
    u8 value_sink = 0;
    /*read the lane value,and update the train status*/
    FDpSinkDpcdRead(instance, DP_LANE0_1_STATUS, &value_lane);
    FDpSinkDpcdRead(instance, DP_LANE2_3_STATUS, &value_sink);
    train_status[0] = (value_lane & 0x07);

    train_status[1] = ((value_lane >> 4) & 0x07);

    train_status[2] = (value_sink & 0x07);

    train_status[3] = ((value_sink >> 4) & 0x07);

    /*read update,read the sink and lane status,and update it*/
    FDpSinkDpcdRead(instance, LANE_ALIGN_STATUS_UPDATED, &value_lane);
    FDpSinkDpcdRead(instance, SINK_STATUS, &value_sink);

    return;
}

/**
 * @name: FDpCheckTrainingStatus
 * @msg:  check the training status of the dp link
 * @param {FDpCtrl *} instance is the instance of the display
 * @param {u8} lane_count is the lane count of the dp link
 * @param {u8} tpsn is the training pattern of the dp link
 * @param {u8} *value is the training status of the dp link
 * @return {FError} status is the return status of the function
 */
FError FDpCheckTrainingStatus(FDpCtrl *instance, u8 lane_count, u8 tpsn, u8 *value)
{
    FASSERT(instance != NULL);
    u8 index;
    FError status = FDP_SUCCESS;
    switch (tpsn)
    {
        case LINK_TRAINING_TPS1:
            for (index = 0; index < lane_count; index++)
            {
                if ((value[index] & 0x01) == 0)
                {
                    status = FDP_FAILED;
                    break;
                }
                else
                {
                    status = FDP_SUCCESS;
                }
            }
            break;
        case LINK_TRAINING_TPS2:
        case LINK_TRAINING_TPS3:
        case LINK_TRAINING_TPS4:
            {
                for (index = 0; index < lane_count; index++)
                {
                    if (((value[index] >> 1) & 0x03) != 0x03)
                    {
                        status = FDP_FAILED;
                        break;
                    }
                    else
                    {
                        status = FDP_SUCCESS;
                    }
                }
            }
            break;
        default:
            FDP_DEBUG("The current TPS type is invalid ,please check it ");
            break;
    }
    return status;
}

u8 FDpTxSourceVswingForValue(FDpCtrl *instance, u8 lane_num)
{
    FASSERT(instance != NULL);
    u8 value = 0;
    u8 vswing;
    if (lane_num < 2)
    {
        FDpSinkDpcdRead(instance, ADJUST_REQUEST_LANE0_1, &value);
    }
    else
    {
        FDpSinkDpcdRead(instance, ADJUST_REQUEST_LANE2_3, &value);
    }
    vswing = value >> (4 * (lane_num % 2));
    vswing &= 0x03;
    return vswing;
}

u8 FDpTxSourcePreemphasisForValue(FDpCtrl *instance, u8 lane_num)
{
    u8 value = 0;
    u8 preem;
    if (lane_num < 2)
    {
        FDpSinkDpcdRead(instance, ADJUST_REQUEST_LANE0_1, &value);
    }
    else
    {
        FDpSinkDpcdRead(instance, ADJUST_REQUEST_LANE2_3, &value);
    }
    preem = value >> (4 * (lane_num % 2) + 2);
    preem &= 0x03;
    return preem;
}

/**
 * @name: FDpLinkPhyChangeRate
 * @msg:  train the dp link
 * @param {FDpCtrl *} instance is the instance of the display
 * @param {u32} link_rate is the link rate of the dp link
 */
void FDpLinkPhyChangeRate(FDpCtrl *instance, u32 link_rate)
{
    FASSERT(instance != NULL);
    u32 value;

    uintptr address = instance->config.dp_phy_base_addr;
    /*
    Set power state to A3
    */
    FDpChannelRegWrite(address, REG_XCVR_POWER_STATE_REQ, 0x8);
    /*
    Disable PLL clock
    */
    FDpChannelRegWrite(address, REG_PLLCLK_EN, 0x0);
    /*
    Disable PLL, pll_raw_ctrl
    */
    FDpChannelRegWrite(address, PHY_PMA_PLL_RAW_CTRL, 0);
    /*
    Wait disable PLL success
    */
    while (1)
    {
        value = FDpChannelRegRead(address, PHY_PMA_CMN_CTRL2);
        if (((value >> 2) & 0x00000001) == 0x01)
        {
            break;
        }
    }
    /*
    Reference PLL frequence
    */
    switch (link_rate)
    {
        case 162:
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_CLK_SEL_M0, 0xf01);
            /*
        Select Analog High Speed Clock
        */
            FDpChannelRegWrite(address, XCVR_DIAG_HSCLK_SEL, 0x0);
            FDpChannelRegWrite(address, XCVR_DIAG_HSCLK_DIV, 0x2);
            FDpChannelRegWrite(address, XCVR_DIAG_PLLDRC_CTRL, 0x1);
            /*
        Config PLL for all VCO
        */
            FDpChannelRegWrite(address, CMN_PLL0_DSM_DIAG_M0, 0x4);
            FDpChannelRegWrite(address, CMN_PLL0_VCOCAL_REFTIM_START, 0xc5e);
            FDpChannelRegWrite(address, CMN_PLL0_VCOCAL_TCTRL, 0x3);
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_CP_PADJ_M0, 0x509);
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_CP_IADJ_M0, 0xf00);
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_FILT_PADJ_M0, 0xf08);
            FDpChannelRegWrite(address, CMN_PLL0_INTDIV_M0, 0x061);
            FDpChannelRegWrite(address, CMN_PLL0_FRACDIVL_M0, 0x3333);
            FDpChannelRegWrite(address, CMN_PLL0_FRACDIVH_M0, 0x0);
            FDpChannelRegWrite(address, CMN_PLL0_HIGH_THR_M0, 0x42);
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_CTRL_M0, 0x02);
            FDpChannelRegWrite(address, CMN_PLL0_VCOCAL_PLLCNT_START, 0xc5e);
            FDpChannelRegWrite(address, CMN_PLL0_LOCK_REFCNT_START, 0xc7);
            FDpChannelRegWrite(address, CMN_PLL0_LOCK_PLLCNT_START, 0xc7);
            FDpChannelRegWrite(address, CMN_PLL0_LOCK_PLLCNT_THR, 0x05);
            break;
        case 270:
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_CLK_SEL_M0, 0x701);
            FDpChannelRegWrite(address, XCVR_DIAG_HSCLK_SEL, 0x0);
            FDpChannelRegWrite(address, XCVR_DIAG_HSCLK_DIV, 0x1);
            FDpChannelRegWrite(address, XCVR_DIAG_PLLDRC_CTRL, 0x1);
            FDpChannelRegWrite(address, CMN_PLL0_DSM_DIAG_M0, 0x4);
            FDpChannelRegWrite(address, CMN_PLL0_VCOCAL_REFTIM_START, 0xc5e);
            FDpChannelRegWrite(address, CMN_PLL0_VCOCAL_TCTRL, 0x3);
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_CP_PADJ_M0, 0x509);
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_CP_IADJ_M0, 0xf00);
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_FILT_PADJ_M0, 0xf08);
            FDpChannelRegWrite(address, CMN_PLL0_INTDIV_M0, 0x06c);
            FDpChannelRegWrite(address, CMN_PLL0_FRACDIVL_M0, 0x0);
            FDpChannelRegWrite(address, CMN_PLL0_FRACDIVH_M0, 0x0);
            FDpChannelRegWrite(address, CMN_PLL0_HIGH_THR_M0, 0x48);
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_CTRL_M0, 0x02);
            FDpChannelRegWrite(address, CMN_PLL0_VCOCAL_PLLCNT_START, 0xc5e);
            FDpChannelRegWrite(address, CMN_PLL0_LOCK_REFCNT_START, 0xc7);
            FDpChannelRegWrite(address, CMN_PLL0_LOCK_PLLCNT_START, 0xc7);
            FDpChannelRegWrite(address, CMN_PLL0_LOCK_PLLCNT_THR, 0x05);
            break;
        case 540:
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_CLK_SEL_M0, 0x301);
            FDpChannelRegWrite(address, XCVR_DIAG_HSCLK_SEL, 0x0);
            FDpChannelRegWrite(address, XCVR_DIAG_HSCLK_DIV, 0x0);
            FDpChannelRegWrite(address, XCVR_DIAG_PLLDRC_CTRL, 0x1);
            FDpChannelRegWrite(address, CMN_PLL0_DSM_DIAG_M0, 0x4);
            FDpChannelRegWrite(address, CMN_PLL0_VCOCAL_REFTIM_START, 0xc5e);
            FDpChannelRegWrite(address, CMN_PLL0_VCOCAL_TCTRL, 0x3);
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_CP_PADJ_M0, 0x509);
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_CP_IADJ_M0, 0xf00);
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_FILT_PADJ_M0, 0xf08);
            FDpChannelRegWrite(address, CMN_PLL0_INTDIV_M0, 0x06c);
            FDpChannelRegWrite(address, CMN_PLL0_FRACDIVL_M0, 0x0);
            FDpChannelRegWrite(address, CMN_PLL0_FRACDIVH_M0, 0x0);
            FDpChannelRegWrite(address, CMN_PLL0_HIGH_THR_M0, 0x48);
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_CTRL_M0, 0x02);
            FDpChannelRegWrite(address, CMN_PLL0_VCOCAL_PLLCNT_START, 0xc5e);
            FDpChannelRegWrite(address, CMN_PLL0_LOCK_REFCNT_START, 0xc7);
            FDpChannelRegWrite(address, CMN_PLL0_LOCK_PLLCNT_START, 0xc7);
            FDpChannelRegWrite(address, CMN_PLL0_LOCK_PLLCNT_THR, 0x05);
            break;
        case 810:
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_CLK_SEL_M0, 0x200);
            FDpChannelRegWrite(address, XCVR_DIAG_HSCLK_SEL, 0x0);
            FDpChannelRegWrite(address, XCVR_DIAG_HSCLK_DIV, 0x0);
            FDpChannelRegWrite(address, XCVR_DIAG_PLLDRC_CTRL, 0x1);
            FDpChannelRegWrite(address, CMN_PLL0_DSM_DIAG_M0, 0x4);
            FDpChannelRegWrite(address, CMN_PLL0_VCOCAL_REFTIM_START, 0xc5e);
            FDpChannelRegWrite(address, CMN_PLL0_VCOCAL_TCTRL, 0x3);
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_CP_PADJ_M0, 0x509);
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_CP_IADJ_M0, 0xf00);
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_FILT_PADJ_M0, 0xf08);
            FDpChannelRegWrite(address, CMN_PLL0_INTDIV_M0, 0x051);
            FDpChannelRegWrite(address, CMN_PLL0_FRACDIVL_M0, 0x0);
            FDpChannelRegWrite(address, CMN_PLL0_FRACDIVH_M0, 0x036);
            FDpChannelRegWrite(address, CMN_PLL0_HIGH_THR_M0, 0x42);
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_CTRL_M0, 0x02);
            FDpChannelRegWrite(address, CMN_PLL0_VCOCAL_PLLCNT_START, 0xc5e);
            FDpChannelRegWrite(address, CMN_PLL0_LOCK_REFCNT_START, 0xc7);
            FDpChannelRegWrite(address, CMN_PLL0_LOCK_PLLCNT_START, 0xc7);
            FDpChannelRegWrite(address, CMN_PLL0_LOCK_PLLCNT_THR, 0x05);
            break;
        default:
            FDP_DEBUG("The input link_rate is invalid, use the default link_rate 162");
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_CLK_SEL_M0, 0xf01);
            FDpChannelRegWrite(address, XCVR_DIAG_HSCLK_SEL, 0x0);
            FDpChannelRegWrite(address, XCVR_DIAG_HSCLK_DIV, 0x2);
            FDpChannelRegWrite(address, XCVR_DIAG_PLLDRC_CTRL, 0x1);
            FDpChannelRegWrite(address, CMN_PLL0_DSM_DIAG_M0, 0x4);
            FDpChannelRegWrite(address, CMN_PLL0_VCOCAL_REFTIM_START, 0xc5e);
            FDpChannelRegWrite(address, CMN_PLL0_VCOCAL_TCTRL, 0x3);
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_CP_PADJ_M0, 0x509);
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_CP_IADJ_M0, 0xf00);
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_FILT_PADJ_M0, 0xf08);
            FDpChannelRegWrite(address, CMN_PLL0_INTDIV_M0, 0x061);
            FDpChannelRegWrite(address, CMN_PLL0_FRACDIVL_M0, 0x3333);
            FDpChannelRegWrite(address, CMN_PLL0_FRACDIVH_M0, 0x0);
            FDpChannelRegWrite(address, CMN_PLL0_HIGH_THR_M0, 0x42);
            FDpChannelRegWrite(address, CMN_PDIAG_PLL0_CTRL_M0, 0x02);
            FDpChannelRegWrite(address, CMN_PLL0_VCOCAL_PLLCNT_START, 0xc5e);
            FDpChannelRegWrite(address, CMN_PLL0_LOCK_REFCNT_START, 0xc7);
            FDpChannelRegWrite(address, CMN_PLL0_LOCK_PLLCNT_START, 0xc7);
            FDpChannelRegWrite(address, CMN_PLL0_LOCK_PLLCNT_THR, 0x05);
            break;
    }
    /*
        Enable pll , pll_raw_ctrl
      */
    FDpChannelRegWrite(address, PHY_PMA_PLL_RAW_CTRL, 1);
    /*
    Enable pll clock and Set power state to A1
    */
    FDpChannelRegWrite(address, REG_PLLCLK_EN, 0x1);
    FDpChannelRegWrite(address, REG_XCVR_POWER_STATE_REQ, 0x1);
}

static void FDpLinkTrainPattern(FDpCtrl *instance)
{
    uintptr addr = instance->config.dp_channe_base_addr;

    FDpChannelRegWrite(addr, PHYTIUM_DP_TRAINING_PATTERN_SET, TRAINING_PATTERN_1);

    FDpChannelRegWrite(addr, PHYTIUM_DP_SCRAMBLING_DISABLE, SCRAMBLING_ENABLE);
    return;
}

static void FDpLinkTrainReset(FDpCtrl *instance)
{
    uintptr addr = instance->config.dp_channe_base_addr;

    FDpChannelRegWrite(addr, PHYTIUM_DP_TRAINING_PATTERN_SET, FDP_TRAINING_OFF);

    FDpSinkDpcdWrite(instance, TRAINING_PATTERN_SET, FDP_TRAINING_OFF);
    return;
}

static void FDpConfigDptx(FDpCtrl *instance, u32 link_rate, u32 lane_count)
{
    FASSERT(instance != NULL);
    uintptr addr = instance->config.dp_channe_base_addr;

    FDP_DEBUG("FDpConfigDptx link_rate = %d, lane_count = %d\r\n", link_rate, lane_count);

    FDpChannelRegWrite(addr, PHYTIUM_DP_TRANSMITTER_OUTPUT_ENABLE, TRANSMITTER_OUTPUT_DISABLE);

    FDpChannelRegWrite(addr, PHYTIUM_DP_LINK_BW_SET, link_rate / 27);

    FDpChannelRegWrite(addr, PHYTIUM_DP_LANE_COUNT_SET, lane_count);

    FDpChannelRegWrite(addr, PHYTIUM_DP_SOFT_RESET, LINK_SOFT_RESET);

    FDpChannelRegWrite(addr, PHYTIUM_DP_TRANSMITTER_OUTPUT_ENABLE, TRANSMITTER_OUTPUT_ENABLE);
    return;
}

static void FDpConfigSink(FDpCtrl *instance, u32 link_rate, u8 lane_count)
{
    FASSERT(instance != NULL);

    FDpSinkDpcdWrite(instance, LINK_BW_SET, link_rate / 27);

    FDpSinkDpcdWrite(instance, LANE_COUNT_SET, 0x80 + lane_count);

    FDpSinkDpcdWrite(instance, DOWNSPREAD_CTRL, DOWNSPREAD_CTRL_DISABLE);

    FDpSinkDpcdWrite(instance, MAIN_LINK_CHANNEL_CODING_SET, MAIN_LINK_CHANNEL_CTRL_ENABLE);
    return;
}

/**
 * @name: FDpLinkTrainingClockRecovery
 * @msg:  train the dp link
 * @param {FDpCtrl *} instance is the instance of the display
 * @param {u8 *} lane_count is the lane count of the display
 * @param {u32 *} link_rate is the link rate of the display
 * @param {u8 *} vswing is the voltage swing of the display
 * @param {u8 *} pre_emphasis is the pre_emphasis of the display
 * @return  return the training status of the dp link
 */
FError FDpLinkTrainingClockRecovery(FDpCtrl *instance, u8 *lane_count, u32 *link_rate,
                                    u8 *vswing, u8 *pre_emphasis)
{
    FError ret = FDP_SUCCESS;
    u32 index, tpsl_tries, voltage_tries, max_vswing_tries;
    u8 train_status;
    u8 voltage[2];
    u8 value[4];
    /*reset link training*/
    FDpLinkTrainReset(instance);
    /*config pll*/
    FDpLinkPhyChangeRate(instance, *link_rate);
    /*config dptx*/
    FDpConfigDptx(instance, *link_rate, *lane_count);
    /*config sink lane count and link rate*/
    FDpConfigSink(instance, *link_rate, *lane_count);

    memset(vswing, 0, sizeof(*vswing));
    memset(pre_emphasis, 0, sizeof(*pre_emphasis));
    /*config pattern*/
    FDpLinkTrainPattern(instance);
    /*
    Start link training
    */
    FDpTxSetTrainingPar(instance, vswing, pre_emphasis, *lane_count);
    FDpSinkDpcdWrite(instance, TRAINING_PATTERN_SET, 0x20 | LINK_TRAINING_TPS1);
    voltage_tries = 1;
    tpsl_tries = 0;
    max_vswing_tries = 0;
    while (1)
    {
        FDpTxGetTrainingStatus(instance, value);
        train_status = FDpCheckTrainingStatus(instance, *lane_count, LINK_TRAINING_TPS1, value);
        if (train_status == FDP_SUCCESS)
        {
            FDP_DEBUG("Training success = %d\r\n", train_status);
            return FDP_SUCCESS;
        }
        if (voltage_tries >= 5)
        {
            FDP_ERROR("Same voltage has been tried 5 times ");
            return FDP_FAILED;
        }
        if (tpsl_tries >= 10)
        {
            return FDP_FAILED;
        }
        if (max_vswing_tries == 1)
        {
            FDP_ERROR("Max voltage swing reached");
            return FDP_FAILED;
        }
        voltage[0] = vswing[0];
        voltage[1] = pre_emphasis[0];

        for (index = 0; index < *lane_count; index++)
        {
            vswing[index] = FDpTxSourceVswingForValue(instance, index);
            pre_emphasis[index] = FDpTxSourcePreemphasisForValue(instance, index);
        }

        FDpTxSetTrainingPar(instance, vswing, pre_emphasis, *lane_count);
        tpsl_tries++;
        if (vswing[0] == voltage[0])
        {
            voltage_tries++;
        }
        if (voltage[0] >= 3)
        {
            max_vswing_tries++;
        }
    }

    return ret;
}

/**
 * @name: FDpConfigTraingPattern
 * @msg:  config the training pattern of the dp link
 * @param {FDpCtrl *} instance is the instance of the display
 * @return  return the training pattern of the dp link
 */
static u8 FDpConfigTraingPattern(FDpCtrl *instance)
{
    FASSERT(instance != NULL);
    u8 value = 0;
    FDpSinkDpcdRead(instance, MAX_LANE_COUNT, &value);
    value = (value >> 5) & 0x01;
    if (value)
    {
        return LINK_TRAINING_TPS4;
    }
    FDpSinkDpcdRead(instance, MAX_LANE_COUNT, &value);
    value = (value >> 6) & 0x01;
    if (value)
    {
        return LINK_TRAINING_TPS3;
    }

    return LINK_TRAINING_TPS2;
}

/**
 * @name: FDpTxPhyGetAdjustRequest
 * @msg:  train the dp link
 * @param {FDpCtrl *} instance is the instance of the display
 * @param {u8 *} swing is the voltage swing of the display
 * @param {u8 *} pre_emphasis is the pre_emphasis of the display
 */
void FDpTxPhyGetAdjustRequest(FDpCtrl *instance, u8 *swing, u8 *pre_emphasis)
{
    u8 value[2];
    FDpSinkDpcdRead(instance, ADJUST_REQUEST_LANE0_1, &value[0]);
    FDpSinkDpcdRead(instance, ADJUST_REQUEST_LANE2_3, &value[1]);
    swing[0] = value[0] & 0x03;
    swing[1] = (value[0] >> 4) & 0x03;
    swing[2] = value[1] & 0x03;
    swing[3] = (value[1] >> 4) & 0x03;
    pre_emphasis[0] = (value[0] >> 2) & 0x03;
    pre_emphasis[1] = (value[0] >> 6) & 0x03;
    pre_emphasis[2] = (value[1] >> 2) & 0x03;
    pre_emphasis[3] = (value[1] >> 6) & 0x03;
    return;
}

/**
 * @name: FDpLinkTrainingChannelEqual
 * @msg:  train the dp link
 * @param {FDpCtrl *} instance is the instance of the display
 * @param {u8 *} lane_count is the lane count of the display
 * @param {u32 *} link_rate is the link rate of the display
 * @param {u8 *} vswing is the voltage swing of the display
 * @param {u8 *} pre_emphasis is the pre_emphasis of the display
 * @return  return FDP_SUCCESS if success,otherwise return FDP_FAILED
 */
static FError FDpLinkTrainingChannelEqual(FDpCtrl *instance, u8 *lane_count,
                                          u32 *link_rate, u8 *vswing, u8 *pre_emphaisis)
{
    FASSERT(instance != NULL);
    u32 train_status;
    u8 train_pattern;
    u32 tries;
    u8 value[4];
    FError ret = FDP_SUCCESS;
    uintptr addr = instance->config.dp_channe_base_addr;
    FDpTxSetTrainingPar(instance, vswing, pre_emphaisis, *lane_count);
    train_pattern = FDpConfigTraingPattern(instance);

    switch (train_pattern)
    {
        case LINK_TRAINING_TPS4:
            FDpChannelRegWrite(addr, PHYTIUM_DP_SCRAMBLING_DISABLE, 0);
            FDpChannelRegWrite(addr, PHYTIUM_DP_TRAINING_PATTERN_SET, 4);
            FDpSinkDpcdWrite(instance, TRAINING_PATTERN_SET, LINK_TRAINING_TPS4);
            break;
        case LINK_TRAINING_TPS3:
            FDpChannelRegWrite(addr, PHYTIUM_DP_SCRAMBLING_DISABLE, 1);
            FDpChannelRegWrite(addr, PHYTIUM_DP_TRAINING_PATTERN_SET, 3);
            FDpSinkDpcdWrite(instance, TRAINING_PATTERN_SET, 0x20 | LINK_TRAINING_TPS3);
            break;
        case LINK_TRAINING_TPS2:
            FDpChannelRegWrite(addr, PHYTIUM_DP_SCRAMBLING_DISABLE, 1);
            FDpChannelRegWrite(addr, PHYTIUM_DP_TRAINING_PATTERN_SET, 2);
            FDpSinkDpcdWrite(instance, TRAINING_PATTERN_SET, 0x20 | LINK_TRAINING_TPS2);
            break;
        default:
            FDP_DEBUG("The train_pattern is invalid ");
            break;
    }

    for (tries = 0; tries < 5; tries++)
    {
        FDpTxGetTrainingStatus(instance, value);

        train_status = FDpCheckTrainingStatus(instance, *lane_count, LINK_TRAINING_TPS1, value);
        if (train_status == FDP_FAILED)
        {
            FDP_ERROR("Training failed");
            return FDP_FAILED;
        }
        train_status = FDpCheckTrainingStatus(instance, *lane_count, train_pattern, value);
        if (train_status == FDP_SUCCESS)
        {
            return FDP_SUCCESS;
        }
        FDpTxPhyGetAdjustRequest(instance, vswing, pre_emphaisis);
        FDpTxSetTrainingPar(instance, vswing, pre_emphaisis, *lane_count);
    }
    return ret;
}

/**
 * @name: FDpStartLinkTraining
 * @msg:  train the dp link
 * @param {FDpCtrl *} instance is the instance of the display
 * @return 
 */
FError FDpStartLinkTraining(FDpCtrl *instance)
{
    FError ret = FDP_SUCCESS;
    u8 lane_count = 0;
    u32 link_rate;
    u8 value;
    u8 vswing[4];
    u8 pre_emphasis[4];

    ret = FDptxPhyGetLaneCount(instance, &lane_count);
    if (ret == FDP_FAILED)
    {
        FDP_ERROR("FDptxPhyGetLaneCount is timeodp_phy_base_addrut");
        return FDP_FAILED;
    }
    if (lane_count > 1)
    {
        lane_count = 1;
    }

    ret = FDpTxPhyGetLinkRate(instance, &link_rate);
    if (ret == FDP_FAILED)
    {
        FDP_ERROR("FDpTxPhyGetLinkRate is timeout");
        return FDP_FAILED;
    }
    for (int index = 0; index < 16; index++)
    {
        FDpSinkDpcdRead(instance, index, &value);
    }
    instance->trans_config.link_lane_count = lane_count;
    instance->trans_config.link_rate = link_rate * 1000;
    FDpSinkDpcdRead(instance, SINK_COUNT, &value);
    ret = FDpLinkTrainingClockRecovery(instance, &lane_count, &link_rate, vswing, pre_emphasis);
    if (ret != FDP_SUCCESS)
    {
        FDP_ERROR("FDpLinkTrainingClockRecovery failed\n");
        return ret;
    }
    ret = FDpLinkTrainingChannelEqual(instance, &lane_count, &link_rate, vswing, pre_emphasis);
    if (ret != FDP_SUCCESS)
    {
        FDP_ERROR("FDpLinkTrainingChannelEqual failed\n");
        return ret;
    }
    FDpSinkDpcdWrite(instance, TRAINING_PATTERN_SET, FDP_TRAINING_OFF);
    return ret;
}

/**
 * @name: FDpHwScramblerReset
 * @msg:  reset the dp Scrambler 
 * @param {FDpCtrl *} instance is the instance of the display
 * @return 
 */
void FDpHwScramblerReset(FDpCtrl *instance)
{
    uintptr addr = instance->config.dp_channe_base_addr;
    FDpChannelRegWrite(addr, PHYTIUM_DP_TRAINING_PATTERN_SET, FDP_TRAINING_OFF);
    FDpChannelRegWrite(addr, PHYTIUM_DP_SCRAMBLING_DISABLE, SCRAMBLING_DISABLE);
    FDpChannelRegWrite(addr, PHYTIUM_DP_FORCE_SCRAMBLER_RESET, SCRAMBLING_ENABLE);
    FDP_DEBUG("Resetting scrambling");
}

/**
 * @name: FDpHwReset
 * @msg:  reset  the dp channel
 * @param {FDpCtrl *} instance is the instance of the display
 * @return 
 */
void FDpHwReset(FDpCtrl *instance)
{
    uintptr dp_addr = instance->config.dp_channe_base_addr;
    FDpChannelRegWrite(dp_addr, PHYTIUM_DP_CONTROLLER_RESET, DP_RESET);
    FDriverUdelay(500);
    FDpChannelRegWrite(dp_addr, PHYTIUM_DP_AUX_CLK_DIVIDER, AUX_CLK_DIVIDER_100);
    FDpChannelRegWrite(dp_addr, PHYTIUM_EDP_CRC_ENABLE, SUPPORT_EDP_1_4);

    FDP_DEBUG("Resetting dp channel");
    return;
}