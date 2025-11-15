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
 * FilePath: e1000e.c
 * Date: 2024-12-23 14:46:52
 * LastEditTime: 2024-12-23 14:46:58
 * Description:  This file is for e1000e driver .Functions in this file are the minimum required functions
 * for this driver.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huangjin   2024/12/23    first release
 */

#include "e1000e.h"
#include "e1000e_hw.h"
#include "ftypes.h"
#include "stdio.h"
#include "fdrivers_port.h"

#define E1000E_DEBUG_TAG "E1000E"
#define E1000E_PRINT_E(format, ...) \
    FT_DEBUG_PRINT_E(E1000E_DEBUG_TAG, format, ##__VA_ARGS__)
#define E1000E_PRINT_I(format, ...) \
    FT_DEBUG_PRINT_I(E1000E_DEBUG_TAG, format, ##__VA_ARGS__)
#define E1000E_PRINT_D(format, ...) \
    FT_DEBUG_PRINT_D(E1000E_DEBUG_TAG, format, ##__VA_ARGS__)
#define E1000E_PRINT_W(format, ...) \
    FT_DEBUG_PRINT_W(E1000E_DEBUG_TAG, format, ##__VA_ARGS__)

/**
 * @name: FE1000ECfgInitialize
 * @msg: Initialize e1000e ctrl
 * @param {FE1000ECtrl} *instance_p, pointer to the FE1000ECtrl instance.
 * @param {FE1000EConfig} *input_config_p, Default configuration parameters of FE1000e
 * @return {FError} return FE1000E_SUCCESS if successful, return others if failed
 */
FError FE1000ECfgInitialize(FE1000ECtrl *instance_p, FE1000EConfig *input_config_p)
{
    FError ret = FE1000E_SUCCESS;
    FASSERT(instance_p != NULL);
    FASSERT(input_config_p != NULL);

    instance_p->config = *input_config_p;

    instance_p->link_status = FE1000E_LINKDOWN;

    instance_p->is_ready = FT_COMPONENT_IS_READY;

    /* initialize the e1000e controller */
    ret = FE1000EReset(instance_p);
    if (FE1000E_SUCCESS != ret)
    {
        /*permit failed*/
        E1000E_PRINT_E("E1000E reset failed.");
        return FE1000E_ERR_FAILED;
    }

    return FE1000E_SUCCESS;
}

/**
 * @name: FE1000ECfgDeInitialize
 * @msg: DeInitialization function for the device instance
 * @param {FE1000ECtrl} *instance_p, pointer to the FE1000ECtrl instance.
 * @return {*}
 */
void FE1000ECfgDeInitialize(FE1000ECtrl *pctrl)
{
    FASSERT(pctrl);

    pctrl->is_ready = 0;
    memset(pctrl, 0, sizeof(*pctrl));

    return;
}

/**
 * @name: FE1000EStop
 * @msg: Deinitializes the specified FE1000E device instance by disabling its
 *       interrupts, receiver, and transmitter, and marking the instance as stopped.
 * @param {FE1000ECtrl} *instance_p, pointer to the FE1000ECtrl instance.
 * @return {*}
 */
void FE1000EStop(FE1000ECtrl *instance_p)
{
    u32 reg_val;

    FASSERT(instance_p != NULL);
    FASSERT(instance_p->is_ready == (u32)FT_COMPONENT_IS_READY);

    /* Disable all interrupts */
    FE1000E_WRITEREG32(instance_p->config.base_addr, E1000_IMS, ~IMS_ALL_MASK);

    /* Disable the receiver */
    reg_val = FE1000E_READREG32(instance_p->config.base_addr, E1000_RCTL);
    reg_val &= (u32)(~RCTL_EN);
    FE1000E_WRITEREG32(instance_p->config.base_addr, E1000_RCTL, reg_val);

    /* Disable the transmitter */
    reg_val = FE1000E_READREG32(instance_p->config.base_addr, E1000_TCTL);
    reg_val &= (u32)(~TCTL_EN);
    FE1000E_WRITEREG32(instance_p->config.base_addr, E1000_TCTL, reg_val);

    /* Clear the descriptor flags */
    memset(&instance_p->rx_ring, 0, sizeof(FE1000ERingDescData));
    memset(&instance_p->tx_ring, 0, sizeof(FE1000ERingDescData));

    /* Mark as stopped */
    instance_p->is_ready = 0U;

    return;
}

/**
 * @name: FE1000EStart
 * @msg: Initializes and starts the specified FE1000E device instance by enabling
 *       the transmitter and receiver and clearing any existing interrupt statuses.
 * @param {FE1000ECtrl} *instance_p, pointer to the FE1000ECtrl instance.
 * @return: None
 */
void FE1000EStart(FE1000ECtrl *instance_p)
{
    u32 reg_val;

    /* Assert bad arguments and conditions */
    FASSERT(instance_p != NULL);
    FASSERT(instance_p->is_ready == (u32)FT_COMPONENT_IS_READY);

    /* clear any existed int status */
    FE1000E_WRITEREG32(instance_p->config.base_addr, E1000_IMS, ~IMS_ALL_MASK);

    /* Enable transmitter if not already enabled */
    reg_val = FE1000E_READREG32(instance_p->config.base_addr, E1000_TCTL);
    reg_val |= (u32)(TCTL_EN);
    FE1000E_WRITEREG32(instance_p->config.base_addr, E1000_TCTL, reg_val);

    /* Enable receiver if not already enabled */
    reg_val = FE1000E_READREG32(instance_p->config.base_addr, E1000_RCTL);
    reg_val |= (u32)(RCTL_EN);
    FE1000E_WRITEREG32(instance_p->config.base_addr, E1000_RCTL, reg_val);

    return;
}

/**
 * @name: FE1000EGetMacAddr
 * @msg: 获取FE1000E控制器配置的MAC地址
 * @return {*}
 * @param {FE1000ECtrl*} instance_p 控制器
 */
void FE1000EGetMacAddr(FE1000ECtrl *instance_p)
{
    uintptr base_addr = instance_p->config.base_addr;
    uint32_t ral, rah;
    uint32_t eerd;
    unsigned int n;

    /* 读取mac 地址 */
    if (FE1000E_READREG32(base_addr, E1000_RAH) & RAH_AV)
    {
        E1000E_PRINT_D("Address Valid!");
        ral = FE1000E_READREG32(base_addr, E1000_RAL);
        rah = FE1000E_READREG32(base_addr, E1000_RAH);
        memcpy(instance_p->mac, &ral, 4);
        memcpy(instance_p->mac + 4, &rah, 2);
    }
    else
    {
        E1000E_PRINT_D("Address Not Valid!");
        for (n = 0; n < 3; n++)
        {
            FE1000E_WRITEREG32(base_addr, E1000_EERD, EERD_START | (n << EERD_ADDR_SHIFT));
            do
            {
                eerd = FE1000E_READREG32(base_addr, E1000_EERD);
                BARRIER();
            } while (!(eerd & EERD_DONE));
            instance_p->mac[n * 2] = (uint8_t)(eerd >> EERD_DATA_SHIFT);
            instance_p->mac[n * 2 + 1] = (uint8_t)(eerd >> (EERD_DATA_SHIFT + 8));
        }
    }

    return;
}

/**
 * @name: FE1000ESetMacAddr
 * @msg: 设置FE1000E控制器的MAC地址
 * @return {*}
 * @param {FE1000ECtrl*} instance_p 控制器寄存器基地址
 */
void FE1000ESetMacAddr(FE1000ECtrl *instance_p)
{
    uintptr base_addr = instance_p->config.base_addr;

    /* 写入mac 地址 */
    FE1000E_WRITEREG32(base_addr, E1000_RAL, *(uint32_t *)instance_p->mac);
    FE1000E_WRITEREG32(base_addr, E1000_RAH, *(uint16_t *)&instance_p->mac[4] | RAH_AV);

    return;
}

/**
 * @name: FE1000EReset
 * @msg: reset FE1000E controller
 * @param {FE1000ECtrl} *instance_p, instance of FE1000E controller
 * @return err code information, FE1000E_SUCCESS indicates success，others indicates failed
 */
FError FE1000EReset(FE1000ECtrl *instance_p)
{
    FASSERT(instance_p);
    FError ret = FE1000E_SUCCESS;

    /* do software reset per init */
    ret = FE1000ESoftwareReset(instance_p->config.base_addr);

    E1000E_PRINT_I("MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", instance_p->mac[0],
                   instance_p->mac[1], instance_p->mac[2], instance_p->mac[3],
                   instance_p->mac[4], instance_p->mac[5]);

    /* recover mac address after softwate reset */
    FE1000ESetMacAddr(instance_p);

    instance_p->link_status = FE1000E_LINKUP;

    return ret;
}
