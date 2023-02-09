/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc.
 * All Rights Reserved.
 *
 * This program is OPEN SOURCE software: you can redistribute it and/or modify it
 * under the terms of the Phytium Public License as published by the Phytium Technology Co.,Ltd,
 * either version 1.0 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the Phytium Public License for more details.
 *
 *
 * FilePath: e2000_board.c
 * Date: 2022-11-07 09:04:21
 * LastEditTime: 2022-11-07 09:04:21
 * Description:  This file is for lwip e2000 board pin define 
 *
 * Modify History:
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 wangxiaodong 2022/10/21  init
 * 1.1 liuzhihong   2023/01/12  driver and application restructure
 */

#include "ftypes.h"
#include "fpinctrl.h"
#include "fparameters.h"

int FXmacPhyGpioInit(u32 instance_id, u32 interface_type)
{
#if defined(CONFIG_TARGET_E2000Q)
#if defined(CONFIG_BOARD_TYPE_B)
    if (instance_id == 3)
    {
        if (interface_type == PHY_INTERRUPTFACE_RGMII)
        {
            FPinSetConfig(FIOPAD_J37, FPIN_FUNC1, FPIN_PULL_NONE, FPIN_DRV4); /* gsd_gmu_rgmii_txd1_0
 */
            FPinSetConfig(FIOPAD_J39, FPIN_FUNC1, FPIN_PULL_NONE, FPIN_DRV4); /* gsd_gmu_rgmii_txd1_1
 */
            FPinSetConfig(FIOPAD_G41, FPIN_FUNC1, FPIN_PULL_NONE, FPIN_DRV4); /* gsd_gmu_rgmii_rxd1_0
 */
            FPinSetConfig(FIOPAD_E43, FPIN_FUNC1, FPIN_PULL_NONE, FPIN_DRV4); /* gsd_gmu_rgmii_rxd1_1
 */
            FPinSetConfig(FIOPAD_L43, FPIN_FUNC1, FPIN_PULL_NONE, FPIN_DRV4); /* gsd_gmu_rgmii_tx_ctl1 */
            FPinSetConfig(FIOPAD_C43, FPIN_FUNC1, FPIN_PULL_NONE, FPIN_DRV4); /* gsd_gmu_rgmii_rxd1_2 */
            FPinSetConfig(FIOPAD_E41, FPIN_FUNC1, FPIN_PULL_NONE, FPIN_DRV4); /* gsd_gmu_rgmii_rxd1_3 */
            FPinSetConfig(FIOPAD_L45, FPIN_FUNC1, FPIN_PULL_NONE, FPIN_DRV4); /* gsd_gmu_rgmii_rx_clk1 */
            FPinSetConfig(FIOPAD_J43, FPIN_FUNC1, FPIN_PULL_NONE, FPIN_DRV4); /* gsd_gmu_rgmii_rx_ctl1 */
            FPinSetConfig(FIOPAD_J41, FPIN_FUNC1, FPIN_PULL_NONE, FPIN_DRV4); /* gsd_gmu_rgmii_tx_clk1 */
            FPinSetDelay(FIOPAD_J41_DELAY, FPIN_OUTPUT_DELAY, FPIN_DELAY_FINE_TUNING, FPIN_DELAY_7);
            FPinSetDelay(FIOPAD_J41_DELAY, FPIN_OUTPUT_DELAY, FPIN_DELAY_COARSE_TUNING, FPIN_DELAY_5);
            FPinSetDelayEn(FIOPAD_J41_DELAY, FPIN_OUTPUT_DELAY, 1);

            FPinSetConfig(FIOPAD_L39, FPIN_FUNC1, FPIN_PULL_NONE, FPIN_DRV4); /* gsd_gmu_rgmii_txd1_2 */
            FPinSetConfig(FIOPAD_E37, FPIN_FUNC1, FPIN_PULL_NONE, FPIN_DRV4); /* gsd_gmu_rgmii_txd1_3 */
            FPinSetConfig(FIOPAD_E35, FPIN_FUNC1, FPIN_PULL_NONE, FPIN_DRV4); /* gsd_gmu_mdc_mac3 */
            FPinSetConfig(FIOPAD_G35, FPIN_FUNC1, FPIN_PULL_NONE, FPIN_DRV4); /* gsd_gmu_mdio_mac3 */
        }
        else if (interface_type == PHY_INTERRUPTFACE_SGMII)
        {
            FPinSetConfig(FIOPAD_E35, FPIN_FUNC1, FPIN_PULL_NONE, FPIN_DRV4); /* gsd_gmu_mdc_mac3 */
            FPinSetConfig(FIOPAD_G35, FPIN_FUNC1, FPIN_PULL_NONE, FPIN_DRV4); /* gsd_gmu_mdio_mac3 */
        }
        else
        {
            printf("Interface_type 0x%x is not support.\r\n");
            return -1;
        }
    }
#elif defined(CONFIG_BOARD_TYPE_C)
    if (instance_id == 1)
    {
        FPinSetConfig(FIOPAD_AJ53, FPIN_FUNC3, FPIN_PULL_NONE, FPIN_DRV4); /* gsd_gmu_mdc_mac1 */
        FPinSetConfig(FIOPAD_AL49, FPIN_FUNC3, FPIN_PULL_NONE, FPIN_DRV4); /* gsd_gmu_mdio_mac1 */
    }
    else if (instance_id == 2)
    {
        FPinSetConfig(FIOPAD_E29, FPIN_FUNC1, FPIN_PULL_NONE, FPIN_DRV4); /* gsd_gmu_mdc_mac2 */
        FPinSetConfig(FIOPAD_G29, FPIN_FUNC1, FPIN_PULL_NONE, FPIN_DRV4); /* gsd_gmu_mdio_mac2 */
    }
    else if (instance_id == 3)
    {
        FPinSetConfig(FIOPAD_E35, FPIN_FUNC1, FPIN_PULL_NONE, FPIN_DRV4); /* gsd_gmu_mdc_mac3  */
        FPinSetConfig(FIOPAD_G35, FPIN_FUNC1, FPIN_PULL_NONE, FPIN_DRV4); /* gsd_gmu_mdio_mac3 */
    }
    else
    {
        printf("Interface_type 0x%x is not support.\r\n");
        return -1;
    }
#endif
#elif defined(CONFIG_TARGET_E2000D) || defined(CONFIG_TARGET_E2000S)

#if defined(CONFIG_BOARD_TYPE_B)
    if (instance_id == 3)
    {
        if (interface_type == PHY_INTERRUPTFACE_RGMII)
        {
            FPinSetConfig(FIOPAD_J33, FPIN_FUNC1, FPIN_PULL_NONE, FPIN_DRV4); /* gsd_gmu_rgmii_txd1_0
 */
            FPinSetConfig(FIOPAD_J35, FPIN_FUNC1, FPIN_PULL_NONE, FPIN_DRV4); /* gsd_gmu_rgmii_txd1_1
 */
            FPinSetConfig(FIOPAD_G37, FPIN_FUNC1, FPIN_PULL_NONE, FPIN_DRV4); /* gsd_gmu_rgmii_rxd1_0
 */
            FPinSetConfig(FIOPAD_E39, FPIN_FUNC1, FPIN_PULL_NONE, FPIN_DRV4); /* gsd_gmu_rgmii_rxd1_1
 */
            FPinSetConfig(FIOPAD_L39, FPIN_FUNC1, FPIN_PULL_NONE, FPIN_DRV4); /* gsd_gmu_rgmii_tx_ctl1 */
            FPinSetConfig(FIOPAD_C39, FPIN_FUNC1, FPIN_PULL_NONE, FPIN_DRV4); /* gsd_gmu_rgmii_rxd1_2 */
            FPinSetConfig(FIOPAD_E37, FPIN_FUNC1, FPIN_PULL_NONE, FPIN_DRV4); /* gsd_gmu_rgmii_rxd1_3 */
            FPinSetConfig(FIOPAD_L41, FPIN_FUNC1, FPIN_PULL_NONE, FPIN_DRV4); /* gsd_gmu_rgmii_rx_clk1 */
            FPinSetConfig(FIOPAD_J39, FPIN_FUNC1, FPIN_PULL_NONE, FPIN_DRV4); /* gsd_gmu_rgmii_rx_ctl1 */
            FPinSetConfig(FIOPAD_J37, FPIN_FUNC1, FPIN_PULL_NONE, FPIN_DRV4); /* gsd_gmu_rgmii_tx_clk1 */
            FPinSetDelay(FIOPAD_J37_DELAY, FPIN_OUTPUT_DELAY, FPIN_DELAY_COARSE_TUNING, FPIN_DELAY_5);
            FPinSetDelay(FIOPAD_J37_DELAY, FPIN_OUTPUT_DELAY, FPIN_DELAY_FINE_TUNING, FPIN_DELAY_7);
            FPinSetDelayEn(FIOPAD_J37_DELAY, FPIN_OUTPUT_DELAY, 1);
            FPinSetConfig(FIOPAD_L35, FPIN_FUNC1, FPIN_PULL_NONE, FPIN_DRV4); /* gsd_gmu_rgmii_txd1_2 */
            FPinSetConfig(FIOPAD_E33, FPIN_FUNC1, FPIN_PULL_NONE, FPIN_DRV4); /* gsd_gmu_rgmii_txd1_3 */
            FPinSetConfig(FIOPAD_E31, FPIN_FUNC1, FPIN_PULL_NONE, FPIN_DRV4); /* gsd_gmu_mdc_mac3 */
            FPinSetConfig(FIOPAD_G31, FPIN_FUNC1, FPIN_PULL_NONE, FPIN_DRV4); /* gsd_gmu_mdio_mac3 */
        }
        else if (interface_type == PHY_INTERRUPTFACE_SGMII)
        {
            FPinSetConfig(FIOPAD_E31, FPIN_FUNC1, FPIN_PULL_NONE, FPIN_DRV4); /* gsd_gmu_mdc_mac3 */
            FPinSetConfig(FIOPAD_G31, FPIN_FUNC1, FPIN_PULL_NONE, FPIN_DRV4); /* gsd_gmu_mdio_mac3 */
        }
        else
        {
            printf("Interface_type 0x%x is not support.\r\n");
            return -1;
        }
    }
#elif defined(CONFIG_BOARD_TYPE_C)
    if (instance_id == 1)
    {
        FPinSetConfig(FIOPAD_AJ49, FPIN_FUNC3, FPIN_PULL_NONE, FPIN_DRV4); /* gsd_gmu_mdc_mac1 */
        FPinSetConfig(FIOPAD_AL45, FPIN_FUNC3, FPIN_PULL_NONE, FPIN_DRV4); /* gsd_gmu_mdio_mac1 */
    }
    else if (instance_id == 2)
    {
        FPinSetConfig(FIOPAD_E25, FPIN_FUNC1, FPIN_PULL_NONE, FPIN_DRV4); /* gsd_gmu_mdc_mac2 */
        FPinSetConfig(FIOPAD_G25, FPIN_FUNC1, FPIN_PULL_NONE, FPIN_DRV4); /* gsd_gmu_mdio_mac2 */
    }
    else if (instance_id == 3)
    {
        FPinSetConfig(FIOPAD_E31, FPIN_FUNC1, FPIN_PULL_NONE, FPIN_DRV4); /* gsd_gmu_mdc_mac3  */
        FPinSetConfig(FIOPAD_G31, FPIN_FUNC1, FPIN_PULL_NONE, FPIN_DRV4); /* gsd_gmu_mdio_mac3 */
    }
    else
    {
        printf("Interface_type 0x%x is not support.\r\n");
        return -1;
    }
#endif

#endif


}