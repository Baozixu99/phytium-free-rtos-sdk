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
 * See the Phytium Public License for more details.
 *
 *
 * FilePath: net_pcie_common.c
 * Date: 2025-01-13 14:53:42
 * LastEditTime: 2025-01-13 17:46:03
 * Description:  This file is for net pcie common functions 
 *
 * Modify History:
 *  Ver      Who         Date                Changes
 * -----  ----------   --------   ---------------------------------
 *  1.0   huangjin     2025/01/13          first release
 */

/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
#warning "Please include sdkconfig.h"
#endif
#include "finterrupt.h"
#include "fpcie_ecam.h"
#include "fpcie_ecam_common.h"
#include "e1000e.h"
#include "e1000e_hw.h"
#include "fcpu_info.h"
#include "ferror_code.h"
#include "fpcie_ecam_msi.h"
#include "net_pcie_common.h"

#include "lwip/ip4_addr.h"
#include "lwip/inet.h"
#include "lwip/netif.h"

#include "lwiperf.h"


/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

extern FE1000EConfig FE1000E_CONFIG_TBL[FE1000E_NUM];

/*max support 16 ahci controllers*/
static FPcieEcam pcie_device;
static boolean e1000e_up_flag = TRUE;

static BoardMacConfig pcie_mac_config = {
    CONFIG_DEFAULT_INIT(lwip_mac_config, LWIP_PORT_TYPE_E1000E, 0, LWIP_PORT_INTERFACE_SGMII)
        .dhcp_en = 0,
    .ipaddr = "192.168.4.12",
    .gw = "192.168.4.1",
    .netmask = "255.255.255.0",
    .mac_address = {0x6c, 0xb3, 0x11, 0x0f, 0x9a, 0x44},
};
/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Function *****************************************/

void SetIP(ip_addr_t *ipaddr, ip_addr_t *gw, ip_addr_t *netmask, BoardMacConfig mac_config)
{
    if (inet_aton(mac_config.ipaddr, ipaddr) == 0)
    {
        printf("The addr of ipaddr is wrong\r\n");
    }
    if (inet_aton(mac_config.gw, gw) == 0)
    {
        printf("The addr of gw is wrong\r\n");
    }
    if (inet_aton(mac_config.netmask, netmask) == 0)
    {
        printf("The addr of netmask is wrong\r\n");
    }
}

static FError FPcieInit(FPcieEcam *pcie_device)
{
    FError ret = FE1000E_SUCCESS;

    ret = FPcieEcamCfgInitialize(pcie_device, FPcieEcamLookupConfig(FPCIE_ECAM_INSTANCE0), NULL);
    if (ret != FT_SUCCESS)
    {
        return ret;
    }
    FNET_DEBUG("\n");
    FNET_DEBUG("	PCI:\n");
    FNET_DEBUG("	B:D:F			VID:PID			parent_BDF			class_code\n");
    ret = FPcieEcamEnumerateBus(pcie_device, 0);
    if (ret != FT_SUCCESS)
    {
        return ret;
    }

    return ret;
}

FError FNetPcieInit(void)
{
    FError ret = FE1000E_SUCCESS;
    s32 host;
    u32 pci_class = 0;
    const u32 class_code = PCI_CLASS_STORAGE_NET_AHCI;
    uintptr bar_addr = 0;
    u8 bus = 0;
    u8 device = 0;
    u8 function = 0;
    u32 config_data;
    u32 cmdstat;

    struct netif *netif_p = NULL;
    ip_addr_t ipaddr, netmask, gw;

    /* pcie init */
    ret = FPcieInit(&pcie_device);
    if (ret != FE1000E_SUCCESS)
    {
        FNET_ERROR("FPcieInit failed.");
        return FE1000E_ERR_FAILED;
    }

    /* find xhci host from pcie instance */
    for (host = 0; host < pcie_device.scans_bdf_count; host++)
    {
        bus = pcie_device.scans_bdf[host].bus;
        device = pcie_device.scans_bdf[host].device;
        function = pcie_device.scans_bdf[host].function;

        FPcieEcamReadConfigSpace(&pcie_device, bus, device, function,
                                 FPCIE_CCR_REV_CLASSID_REGS, &config_data);
        FNET_DEBUG("FPCIE_CCR_REV_CLASSID_REGS = %x\n", config_data);
        pci_class = config_data >> 8;

        if (pci_class == class_code)
        {
            FPcieEcamReadConfigSpace(&pcie_device, bus, device, function,
                                     FPCIE_CCR_ID_REG, &config_data);

            FNET_DEBUG("AHCI-PCI HOST found !!!, b.d.f = %x.%x.%x\n", bus, device, function);

            FPcieEcamReadConfigSpace(&pcie_device, bus, device, function,
                                     FPCIE_CCR_BAR_ADDR0_REGS, (u32 *)&bar_addr);
            FNET_DEBUG("FNetPcieIntrInstall BarAddress %p", bar_addr);
            FE1000E_CONFIG_TBL[FE1000E0_ID].base_addr = bar_addr;

            if (0x0 == bar_addr)
            {
                FNET_ERROR("Bar address: 0x%lx", bar_addr);
                return FE1000E_ERR_FAILED;
            }

            break;
        }
    }
    if (pci_class != class_code)
    {
        FNET_ERROR("class_code: 0x%06x is not pcie net card!", pci_class);
        return FE1000E_ERR_FAILED;
    }

    /* 设置cmd reg */
    FPcieEcamReadConfigSpace(&pcie_device, bus, device, function, FPCIE_CCR_CMD_STATUS_REGS, &cmdstat);
    cmdstat |= (FPCIE_CCR_CMD_MEMORY_ACCESS_ENABLED | FPCIE_CCR_CMD_BUS_MASTER_ENABLED);
    FPcieEcamWriteConfigSpace(&pcie_device, bus, device, function, FPCIE_CCR_CMD_STATUS_REGS, cmdstat);

    /* pcie net init */
    pcie_mac_config.lwip_mac_config.name[0] = PCIE_ETH_NAME_PREFIX;
    itoa(pcie_mac_config.lwip_mac_config.mac_instance,
         &(pcie_mac_config.lwip_mac_config.name[1]), 10);
    SetIP(&ipaddr, &gw, &netmask, pcie_mac_config);
    netif_p = &pcie_mac_config.netif;
    /* Add network interface to the netif_list, and set it as default */
    if (!LwipPortAdd(netif_p, &ipaddr, &netmask, &gw, pcie_mac_config.mac_address,
                     (UserConfig *)&pcie_mac_config))
    {
        printf("Error adding N/W interface %d.\n\r", pcie_mac_config.lwip_mac_config.mac_instance);
        ret = ERR_GENERAL;
        return FE1000E_ERR_FAILED;
    }
    printf("LwipPortAdd pcie_mac_instance %d is over.\n\r",
           pcie_mac_config.lwip_mac_config.mac_instance);

    netif_set_default(netif_p);

    if (netif_is_link_up(netif_p))
    {
        /* 当netif完全配置好时，必须调用该函数 */
        netif_set_up(netif_p);
        if (pcie_mac_config.dhcp_en == 1)
        {
            LwipPortDhcpSet(netif_p, TRUE);
        }
    }
    else
    {
        /* 当netif链接关闭时，必须调用该函数 */
        netif_set_down(netif_p);
    }

    return ret;
}

FError FNetPcieMsiInit(void)
{
    FError ret = FE1000E_SUCCESS;
    struct netif *netif_p = NULL;
    ip_addr_t ipaddr, netmask, gw;

    /* net init */
    pcie_mac_config.lwip_mac_config.name[0] = PCIE_ETH_NAME_PREFIX;
    itoa(pcie_mac_config.lwip_mac_config.mac_instance,
         &(pcie_mac_config.lwip_mac_config.name[1]), 10);
    SetIP(&ipaddr, &gw, &netmask, pcie_mac_config);
    netif_p = &pcie_mac_config.netif;
    /* Add network interface to the netif_list, and set it as default */
    if (!LwipPortAdd(netif_p, &ipaddr, &netmask, &gw, pcie_mac_config.mac_address,
                     (UserConfig *)&pcie_mac_config))
    {
        printf("Error adding N/W interface %d.\n\r", pcie_mac_config.lwip_mac_config.mac_instance);
        ret = ERR_GENERAL;
        e1000e_up_flag = FALSE;
        return ret;
    }
    printf("LwipPortAdd pcie_mac_instance %d is over.\n\r",
           pcie_mac_config.lwip_mac_config.mac_instance);

    netif_set_default(netif_p);

    if (netif_is_link_up(netif_p))
    {
        /* 当netif完全配置好时，必须调用该函数 */
        netif_set_up(netif_p);
        if (pcie_mac_config.dhcp_en == 1)
        {
            LwipPortDhcpSet(netif_p, TRUE);
        }
    }
    else
    {
        /* 当netif链接关闭时，必须调用该函数 */
        netif_set_down(netif_p);
    }

    return ret;
}

void LwipIperfE1000EClientDeinit(void)
{
    if (e1000e_up_flag == TRUE)
    {
        printf("Now reset E1000E all active iperf session. \r\n");
        lwiperf_reset();
        printf("Reset E1000E all active iperf session complete! \r\n");

        struct netif *netif_p = NULL;
        netif_p = &pcie_mac_config.netif;
        LwipPortStop(netif_p, pcie_mac_config.dhcp_en);
    }
}

void LwipIperfE1000EServerDeinit(void)
{
    if (e1000e_up_flag == TRUE)
    {
        printf("Now reset E1000E all active iperf session. \r\n");
        lwiperf_reset();
        printf("Reset E1000E all active iperf session complete! \r\n");

        struct netif *netif_p = NULL;
        netif_p = &pcie_mac_config.netif;
        LwipPortStop(netif_p, pcie_mac_config.dhcp_en);
    }
}
