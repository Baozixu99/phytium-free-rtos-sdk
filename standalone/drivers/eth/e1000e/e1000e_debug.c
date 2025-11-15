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
 * FilePath: e1000e_debug.c
 * Date: 2025-01-03 15:33:03
 * LastEditTime: 2025-01-03 15:33:04
 * Description:  This file is for user to debug e1000e information.
 * 
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huangjin   2025/01/03    first release
 */

#include "e1000e_hw.h"
#include "e1000e.h"
#include "fio.h"
#include "fdrivers_port.h"

#define FE1000E_DEBUG_TAG "FE1000E_DEBUG"
#define FE1000E_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FE1000E_DEBUG_TAG, format, ##__VA_ARGS__)
#define FE1000E_INFO(format, ...) \
    FT_DEBUG_PRINT_I(FE1000E_DEBUG_TAG, format, ##__VA_ARGS__)
#define FE1000E_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FE1000E_DEBUG_TAG, format, ##__VA_ARGS__)
#define FE1000E_WARN(format, ...) \
    FT_DEBUG_PRINT_W(FE1000E_DEBUG_TAG, format, ##__VA_ARGS__)

/**
 * @name: FE1000EDebugPrint
 * @msg: Prints debug information for the FE1000E device instance by reading
 *       and logging various hardware registers.
 * @param {FE1000ECtrl *}instance_p 驱动控制数据
 * @return: None
 */
void FE1000EDebugPrint(FE1000ECtrl *instance_p)
{
    FE1000EConfig *config_p;
    config_p = &instance_p->config;
    FUNUSED(config_p);
    FE1000E_INFO("E1000_CTRL 0x%08x", FtIn32(config_p->base_addr + E1000_CTRL));
    FE1000E_INFO("E1000_MDIC 0x%08x", FtIn32(config_p->base_addr + E1000_MDIC));
    FE1000E_INFO("E1000_STATUS 0x%08x", FtIn32(config_p->base_addr + E1000_STATUS));
    FE1000E_INFO("E1000_RAL 0x%08x", FtIn32(config_p->base_addr + E1000_RAL));
    FE1000E_INFO("E1000_RAH 0x%08x", FtIn32(config_p->base_addr + E1000_RAH));
    FE1000E_INFO("E1000_RDBAL 0x%08x", FtIn32(config_p->base_addr + E1000_RDBAL));
    FE1000E_INFO("E1000_RDBAH 0x%08x", FtIn32(config_p->base_addr + E1000_RDBAH));
    FE1000E_INFO("E1000_RDLEN 0x%08x", FtIn32(config_p->base_addr + E1000_RDLEN));
    FE1000E_INFO("E1000_RDH 0x%08x", FtIn32(config_p->base_addr + E1000_RDH));
    FE1000E_INFO("E1000_RDT 0x%08x", FtIn32(config_p->base_addr + E1000_RDT));
    FE1000E_INFO("E1000_RXDCTL 0x%08x", FtIn32(config_p->base_addr + E1000_RXDCTL));
    FE1000E_INFO("E1000_RCTL 0x%08x", FtIn32(config_p->base_addr + E1000_RCTL));
    FE1000E_INFO("E1000_TDBAL 0x%08x", FtIn32(config_p->base_addr + E1000_TDBAL));
    FE1000E_INFO("E1000_TDBAH 0x%08x", FtIn32(config_p->base_addr + E1000_TDBAH));
    FE1000E_INFO("E1000_TDLEN 0x%08x", FtIn32(config_p->base_addr + E1000_TDLEN));
    FE1000E_INFO("E1000_TDH 0x%08x", FtIn32(config_p->base_addr + E1000_TDH));
    FE1000E_INFO("E1000_TDT 0x%08x", FtIn32(config_p->base_addr + E1000_TDT));
    FE1000E_INFO("E1000_TXDCTL 0x%08x", FtIn32(config_p->base_addr + E1000_TXDCTL));
    FE1000E_INFO("E1000_TCTL 0x%08x", FtIn32(config_p->base_addr + E1000_TCTL));
    FE1000E_INFO("E1000_TIPG 0x%08x", FtIn32(config_p->base_addr + E1000_TIPG));
    FE1000E_INFO("E1000_ICR 0x%08x", FtIn32(config_p->base_addr + E1000_ICR));
    return;
}
