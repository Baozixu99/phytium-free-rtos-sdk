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
 * FilePath: lwip_port.h
 * Date: 2022-10-25 02:18:02
 * LastEditTime: 2025-01-06 02:18:03
 * Description:  This file is part of lwip port. This file comtains the functions to Initialize,input,stop,dhcp lwip stack. 
 * 
 * Modify History: 
 *  Ver     Who           Date                  Changes
 * -----   ------       --------     --------------------------------------
 *  1.0    huanghe     2022/10/20            first release
 *  1.1   liuzhihong   2022/11/7     function and variable naming adjustment
 *  1.2    huangjin    2025/01/06            add e1000e
 */

#ifndef __LWIP_PORT_H_
#define __LWIP_PORT_H_


#include "ftypes.h"
#include "lwipopts.h"

#include "lwip/netif.h"
#include "lwip/ip.h"

#ifdef __cplusplus
extern "C"
{
#endif


#define LWIP_PORT_CAPS(nr) (1ULL << (nr))
/* network interface device status */

enum lwip_port_link_status
{
    ETH_LINK_UNDEFINED = 0,
    ETH_LINK_UP,
    ETH_LINK_DOWN,
    ETH_LINK_NEGOTIATING
};

/* capability code */
#define LWIP_PORT_MODE_NAIVE 0
#define LWIP_PORT_MODE_JUMBO LWIP_PORT_CAPS(0)
#define LWIP_PORT_MODE_MULTICAST_ADDRESS_FILITER \
    LWIP_PORT_CAPS(1) /* Allow multicast address filtering  */
#define LWIP_PORT_MODE_COPY_ALL_FRAMES LWIP_PORT_CAPS(2) /* enable copy all frames */
#define LWIP_PORT_MODE_CLOSE_FCS_CHECK LWIP_PORT_CAPS(3) /* close fcs check */
#define LWIP_PORT_MODE_UNICAST_ADDRESS_FILITER \
    LWIP_PORT_CAPS(5) /* Allow unicast address filtering  */
/* driver type */
#define LWIP_PORT_TYPE_XMAC         0
#define LWIP_PORT_TYPE_GMAC         1
#define LWIP_PORT_TYPE_XMAC_V2      2
#define LWIP_PORT_TYPE_E1000E       3

/* Mii interface */
#define LWIP_PORT_INTERFACE_RGMII   0
#define LWIP_PORT_INTERFACE_SGMII   1
#define LWIP_PORT_INTERFACE_USX     2

/* Phy speed */
#define LWIP_PORT_SPEED_10M         10
#define LWIP_PORT_SPEED_100M        100
#define LWIP_PORT_SPEED_1000M       1000
#define LWIP_PORT_SPEED_10G         10000


/* Duplex */
#define LWIP_PORT_HALF_DUPLEX       0
#define LWIP_PORT_FULL_DUPLEX       1

#define LWIP_PORT_CONFIG_MAGIC_CODE 0x616b6200

typedef struct
{
    u32 magic_code;      /* LWIP_PORT_CONFIG_MAGIC_CODE */
    char name[2];        /* Used to name netif */
    u32 driver_type;     /* driver type */
    u32 mac_instance;    /* mac controler id */
    u32 mii_interface;   /* LWIP_PORT_INTERFACE_XXX */
    u32 autonegotiation; /* 1 is autonegotiation ,0 is manually set */
    u32 phy_speed;       /* LWIP_PORT_SPEED_XXX */
    u32 phy_duplex;      /* LWIP_PORT_XXX_DUPLEX */
    u32 capability;      /* LWIP_PORT_MODE_XXX */
} UserConfig;

#define LWIP_PORT_CONFIG_DEFAULT_INIT(config)             \
    do                                                    \
    {                                                     \
        config.magic_code = LWIP_PORT_CONFIG_MAGIC_CODE;  \
        config.driver_type = LWIP_PORT_TYPE_XMAC;         \
        config.mac_instance = 3;                          \
        config.mii_interface = LWIP_PORT_INTERFACE_RGMII; \
        config.autonegotiation = 1;                       \
        config.phy_speed = LWIP_PORT_SPEED_1000M;         \
        config.phy_duplex = LWIP_PORT_FULL_DUPLEX;        \
        config.capability = LWIP_PORT_MODE_NAIVE;         \
    } while (0)


typedef struct
{
    void (*eth_input)(struct netif *netif);                        /*LwipTestLoop call*/
    enum lwip_port_link_status (*eth_detect)(struct netif *netif); /*LwipPortInput call*/
    void (*eth_deinit)(struct netif *netif);                       /*LwipPortStop call*/
    void (*eth_start)(struct netif *netif);                        /*LwipPortAdd call*/
    void (*eth_debug)(struct netif *netif);
} LwipPortOps;


struct LwipPort
{
    void *state; /* mac controler */
#if !NO_SYS
    sys_sem_t sem_rx_data_available;
    sys_thread_t detect_thread_handle;
    sys_thread_t rx_thread_handle;
#endif
    LwipPortOps ops;
};

struct netif *LwipPortAdd(struct netif *netif, ip_addr_t *ipaddr, ip_addr_t *netmask, ip_addr_t *gw,
                          unsigned char *mac_ethernet_address, UserConfig *user_config);
void LwipPortInput(struct netif *netif);

#if !NO_SYS
void LwipPortInputThread(struct netif *netif);
#else
void LinkDetectLoop(struct netif *netif);
#endif

void LwipPortStop(struct netif *netif, u32 dhcp_en);
struct netif *LwipPortGetByName(const char *name);
void LwipPortDhcpSet(struct netif *netif, boolean is_enabled);


#if !NO_SYS
void LwipDhcpThread(void *p);
#else
void LwipPortDhcpLoop(u32 period_msec_cnt);
#endif

void LwipPortDebug(const char *name);

#ifdef __cplusplus
}
#endif

#endif
