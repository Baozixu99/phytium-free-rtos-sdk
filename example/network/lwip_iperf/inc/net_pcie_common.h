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
 * FilePath: net_pcie_common.h
 * Date: 2025-01-13 14:53:42
 * LastEditTime: 2025-01-13 17:46:03
 * Description:  This file is for net pcie example common definition
 *
 * Modify History:
 *  Ver      Who         Date                Changes
 * -----  ----------   --------   ---------------------------------
 *  1.0   huangjin     2025/01/13          first release
 */
#ifndef NET_PCIE_COMMON_H
#define NET_PCIE_COMMON_H

#include "fdebug.h"
#include "e1000e.h"
#include "fpcie_ecam.h"
#include "lwip_port.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define FNET_DEBUG_TAG             "NET_PCIE_TEST"
#define FNET_ERROR(format, ...)    FT_DEBUG_PRINT_E(FNET_DEBUG_TAG, format, ##__VA_ARGS__)
#define FNET_WARN(format, ...)     FT_DEBUG_PRINT_W(FNET_DEBUG_TAG, format, ##__VA_ARGS__)
#define FNET_INFO(format, ...)     FT_DEBUG_PRINT_I(FNET_DEBUG_TAG, format, ##__VA_ARGS__)
#define FNET_DEBUG(format, ...)    FT_DEBUG_PRINT_D(FNET_DEBUG_TAG, format, ##__VA_ARGS__)

#define PCI_CLASS_STORAGE_NET_AHCI 0x020000

#define ETH_NAME_PREFIX            'e'
#define PCIE_ETH_NAME_PREFIX       'p'

#define CONFIG_DEFAULT_INIT(config, driver_config, instance_id, interface_type)            \
    .config.magic_code = LWIP_PORT_CONFIG_MAGIC_CODE, .config.driver_type = driver_config, \
    .config.mac_instance = instance_id, .config.mii_interface = interface_type,            \
    .config.autonegotiation = 1, .config.phy_speed = LWIP_PORT_SPEED_1000M,                \
    .config.phy_duplex = LWIP_PORT_FULL_DUPLEX, .config.capability = LWIP_PORT_MODE_NAIVE,

typedef struct
{
    UserConfig lwip_mac_config;
    u32 dhcp_en;
    char *ipaddr;
    char *netmask;
    char *gw;
    unsigned char mac_address[6];
    struct netif netif;
} BoardMacConfig;

/* 转化地址格式 */
void SetIP(ip_addr_t *ipaddr, ip_addr_t *gw, ip_addr_t *netmask, BoardMacConfig mac_config);

/* PCIE and net init function */
FError FNetPcieInit(void);

/* PCIE and net MSI init function */
FError FNetPcieMsiInit(void);

void LwipIperfE1000EClientDeinit(void);

void LwipIperfE1000EServerDeinit(void);

#ifdef __cplusplus
}
#endif

#endif
