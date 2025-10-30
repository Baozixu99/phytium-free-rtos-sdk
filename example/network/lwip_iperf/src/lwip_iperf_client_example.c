/*
 * Copyright (C) 2023, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: lwip_iperf_client_example.c
 * Created Date: 2023-10-18 14:30:04
 * Last Modified: 2024-04-29 15:21:41
 * Description:  This file is for lwip iperf client example function implementation.
 * 
 * Modify History:
 *  Ver      Who         Date               Changes
 * -----  ----------   --------  ---------------------------------
 *  1.0   liuzhihong  2023/10/18          first release
 *  2.0   liuzhihong  2024/04/28   add no letter shell mode, adapt to auto-test system
 */

#include <string.h>
#include <stdio.h>
#include "strto.h"
#include "sdkconfig.h"
#include "ftypes.h"
#include "fassert.h"
#include "fparameters.h"
#include "eth_board.h"
#ifndef SDK_CONFIG_H__
    #error "Please include sdkconfig.h first"
#endif


#include "lwip_port.h"
#include "lwip/ip4_addr.h"
#include "lwip/init.h"
#include "netif/ethernet.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "lwip/inet.h"
#include "lwiperf.h"

#include "net_pcie_common.h"

#define LWIPERF_TCP_PORT_REMOTE 5001
const ip_addr_t remote= IPADDR4_INIT_BYTES(192,168,4,50);

#define ETH_NAME_PREFIX 'e'

#define CONFIG_DEFAULT_INIT(config,driver_config,instance_id,interface_type)           \
		.config.magic_code = LWIP_PORT_CONFIG_MAGIC_CODE,  \
		.config.driver_type = driver_config,		  \
		.config.mac_instance = instance_id,                          \
		.config.mii_interface = interface_type, \
		.config.autonegotiation = 1,                       \
		.config.phy_speed = LWIP_PORT_SPEED_1000M,         \
		.config.phy_duplex = LWIP_PORT_FULL_DUPLEX, \
		.config.capability = LWIP_PORT_MODE_NAIVE, 

#define TIMER_OUT (pdMS_TO_TICKS(5000UL))
enum
{
    LWIP_IPERF_CLIENT_EXAMPLE_SUCCESS = 0,
    LWIP_IPERF_CLIENT_EXAMPLE_UNKNOWN_STATE = 1,
    LWIP_IPERF_CLIENT_EXAMPLE_INIT_FAILURE = 2,
};
static QueueHandle_t xQueue = NULL;

static BoardMacConfig board_mac_config[MAC_NUM] = 
 {
    #if defined(MAC_NUM0)
            {
                CONFIG_DEFAULT_INIT(lwip_mac_config,MAC_NUM0_LWIP_PORT_TYPE,MAC_NUM0_CONTROLLER,MAC_NUM0_MII_INTERFACE)
                .dhcp_en=0,
                .ipaddr="192.168.4.10",
                .gw="192.168.4.1",
                .netmask="255.255.255.0",
                .mac_address={0x98, 0x0e, 0x24, 0x00, 0x11, 0x0},
            },
    #endif
    #if defined(MAC_NUM1) 
            {
                CONFIG_DEFAULT_INIT(lwip_mac_config,MAC_NUM1_LWIP_PORT_TYPE,MAC_NUM1_CONTROLLER,MAC_NUM1_MII_INTERFACE)
                .dhcp_en=0,
                .ipaddr="192.168.4.11",
                .gw="192.168.4.1",
                .netmask="255.255.255.0",
                .mac_address={0x98, 0x0e, 0x24, 0x00, 0x11, 0x1},
            },
    #endif
};

void LwipIperfClientTestTask(void)
{
    int task_res = LWIP_IPERF_CLIENT_EXAMPLE_SUCCESS;
    /* mac init */
    for (int i = 0; i < MAC_NUM; i++)
    {
        struct netif *netif_p = NULL;
        ip_addr_t ipaddr,netmask, gw;
        board_mac_config[i].lwip_mac_config.name[0] = ETH_NAME_PREFIX;
        itoa(board_mac_config[i].lwip_mac_config.mac_instance, &(board_mac_config[i].lwip_mac_config.name[1]), 10);

        /* mac ip addr set: char* -> ip_addr_t */
        SetIP(&ipaddr,&gw,&netmask,board_mac_config[i]);

        /*********************************************************************/
        netif_p= &board_mac_config[i].netif;
        /* Add network interface to the netif_list, and set it as default */
        if (!LwipPortAdd(netif_p, &ipaddr, &netmask, &gw, board_mac_config[i].mac_address, (UserConfig *)&board_mac_config[i]))
        {
            printf("Error adding N/W interface %d.\n\r",board_mac_config[i].lwip_mac_config.mac_instance);
            task_res = LWIP_IPERF_CLIENT_EXAMPLE_INIT_FAILURE;
            goto task_exit;
        }
        printf("LwipPortAdd mac_instance %d is over.\n\r",board_mac_config[i].lwip_mac_config.mac_instance);

        netif_set_default(netif_p);

        if (netif_is_link_up(netif_p))
        {
            /* 当netif完全配置好时，必须调用该函数 */
            netif_set_up(netif_p);
            if (board_mac_config[i].dhcp_en == 1)
            {
                LwipPortDhcpSet(netif_p, TRUE);
            }
        }
        else
        {
            /* 当netif链接关闭时，必须调用该函数 */
            netif_set_down(netif_p);
        }
    }

    /* init pcie & msi net */
    if (FNetPcieMsiInit() != FT_SUCCESS)
    {
        FNET_ERROR("Net Pcie msi init failed, please check if the e1000e is "
                   "successfully connected");
    }

    printf("Network setup complete.\n");

    printf("Now start iperf client test\r\n");
    if(lwiperf_start_tcp_client(&remote, LWIPERF_TCP_PORT_REMOTE, LWIPERF_CLIENT,
                                  NULL, NULL))
    {
        printf("Start iperf client test success !!! \r\n");
        task_res = LWIP_IPERF_CLIENT_EXAMPLE_SUCCESS;
    }
    else
    {   
        printf("Start iperf client test failed !!! \r\n");
        task_res = LWIP_IPERF_CLIENT_EXAMPLE_INIT_FAILURE;
        goto task_exit;
    }     

task_exit:
    xQueueSend(xQueue, &task_res, 0);

    vTaskDelete(NULL);
}


int FFreeRTOSLwipIperfClientTaskCreate(void)
{
    BaseType_t xReturn = pdPASS; /* 定义一个创建信息返回值，默认为 pdPASS */
    int task_res = LWIP_IPERF_CLIENT_EXAMPLE_UNKNOWN_STATE;

    xQueue = xQueueCreate(1, sizeof(int)); /* 创建消息队列 */
    if (xQueue == NULL)
    {
        printf("xQueue create failed.\r\n");
        goto exit;
    }
    
    xReturn = xTaskCreate((TaskFunction_t)LwipIperfClientTestTask, /* 任务入口函数 */
                      (const char *)"LwipIperfClientTestTask", /* 任务名字 */
                      4096,                 /* 任务栈大小 */
                        NULL,                   /* 任务入口函数参数 */
                      (UBaseType_t)configMAX_PRIORITIES - 1, /* 任务的优先级 */
                      NULL);                          /* 任务控制块指针 */
    if (xReturn == pdFAIL)
    {
        printf("xTaskCreate LwipIpv4InitTask failed.\r\n");
        goto exit;
    }

    xReturn = xQueueReceive(xQueue, &task_res, TIMER_OUT);
    if (xReturn == pdFAIL)
    {
        printf("xQueue receive timeout.\r\n");
        goto exit;
    }

exit:
    if (xQueue != NULL)
    {
        vQueueDelete(xQueue);
    }

    if (task_res != LWIP_IPERF_CLIENT_EXAMPLE_SUCCESS)
    {
        printf("%s@%d: Lwip iperf client example [failure], task_res = %d\r\n", __func__, __LINE__, task_res);
        return task_res;
    }
    else
    {
        printf("%s@%d: Lwip iperf client example [success].\r\n", __func__, __LINE__);
        return task_res;
    }
}

void LwipIperfClientDeinit(void)
{    
    printf("Now reset all active iperf session. \r\n");
    lwiperf_reset();
    printf("Reset all active iperf session complete! \r\n");
    for (int i = 0; i < MAC_NUM; i++)
    {
        struct netif *netif_p = NULL;
        netif_p=&board_mac_config[i].netif;
        LwipPortStop(netif_p,board_mac_config[i].dhcp_en); 
    }

}