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
 * FilePath: lwip_test.c
 * Date: 2022-09-15 10:24:38
 * LastEditTime: 2022-09-15 10:24:38
 * Description:  This file is for 
 * 
 * Modify History: 
 *  Ver   Who  Date   Changes
 * ----- ------  -------- --------------------------------------
 */


#include <string.h>
#include <stdio.h>
#include "strto.h"
#include "sdkconfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "ft_types.h"
#include "fpinctrl.h"
#ifndef SDK_CONFIG_H__
	#error "Please include sdkconfig.h first"
#endif

#include "lwipopts.h"
#include "lwip_port.h"
#include "lwip/ip4_addr.h"
#include "lwip/init.h"
#include "netif/ethernet.h"
#include "ethernetif.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "../src/shell.h"


#if defined(CONFIG_TARGET_E2000)
#define PHY_INTERRUPTFACE_RGMII 0
#define PHY_INTERRUPTFACE_SGMII 1 
#endif


#if LWIP_IPV6
#include "lwip/ip.h"
#include "lwip/ip6_addr.h"
#else
#if LWIP_DHCP
#include "lwip/dhcp.h"
#endif
#endif



user_config lwip_mac_config = {0};


#if !LWIP_IPV6
#if LWIP_DHCP
static TaskHandle_t appTaskCreateHandle = NULL;
void LwipDhcpTest(struct netif *echo_netif)
{
    int mscnt = 0;
    dhcp_start(echo_netif);
    printf("LwipDhcpTest is start \r\n");
    while (1)
    {
		vTaskDelay(DHCP_FINE_TIMER_MSECS / portTICK_RATE_MS);
		dhcp_fine_tmr();
		mscnt += DHCP_FINE_TIMER_MSECS;
		if (mscnt >= DHCP_COARSE_TIMER_SECS*1000) 
        {
			dhcp_coarse_tmr();
			mscnt = 0;
		}
	}
}
#endif
#endif

void LwipTestCreate(void * args)
{
    struct netif *echo_netif;
    static boolean init_flag = FALSE;
    BaseType_t ret = pdPASS;
    /* the mac address of the board. this should be unique per board */
	unsigned char mac_ethernet_address[] =
	{0x98, 0x0e, 0x24, 0x00, 0x11, 0x22};

    echo_netif = pvPortMalloc(sizeof(struct netif));
    if(echo_netif == NULL)
    {
        printf("malloc netif is error \r\n");
        goto exit;
    }

#if !LWIP_IPV6
	ip_addr_t ipaddr, netmask, gw;
#if LWIP_DHCP
    ipaddr.addr = 0;
	gw.addr = 0;
	netmask.addr = 0;
#else
	/* initialize IP addresses to be used */
	IP4_ADDR(&ipaddr,  192, 168,   4, 10);
	IP4_ADDR(&netmask, 255, 255, 255,  0);
	IP4_ADDR(&gw,      192, 168,   4,  1);
#endif
#endif

    /* 初始化LwIP堆 */
    if(init_flag == FALSE)
    {
        tcpip_init(NULL, NULL);
        init_flag = TRUE;
    }

#if !LWIP_IPV6
	/* Add network interface to the netif_list, and set it as default */
	if (!lwip_port_add(echo_netif, &ipaddr, &netmask,
						&gw, mac_ethernet_address,
						(user_config *)args))
	{
		printf("Error adding N/W interface\n\r");
		return ;
	}
	printf("lwip_port_add is over \n\r");
#else
	/* Add network interface to the netif_list, and set it as default */
	if (!lwip_port_add(echo_netif, NULL, NULL, NULL, mac_ethernet_address, (user_config *)args)) 
	{
		printf("Error adding N/W interface\n\r");
		return ;
	}
	echo_netif->ip6_autoconfig_enabled = 1;

	netif_create_ip6_linklocal_address(echo_netif, 1);
	netif_ip6_addr_set_state(echo_netif, 0, IP6_ADDR_VALID);

	printf("Board IPv6 address %x:%x:%x:%x:%x:%x:%x:%x\n\r",
			IP6_ADDR_BLOCK1(&echo_netif->ip6_addr[0].u_addr.ip6),
			IP6_ADDR_BLOCK2(&echo_netif->ip6_addr[0].u_addr.ip6),
			IP6_ADDR_BLOCK3(&echo_netif->ip6_addr[0].u_addr.ip6),
			IP6_ADDR_BLOCK4(&echo_netif->ip6_addr[0].u_addr.ip6),
			IP6_ADDR_BLOCK5(&echo_netif->ip6_addr[0].u_addr.ip6),
			IP6_ADDR_BLOCK6(&echo_netif->ip6_addr[0].u_addr.ip6),
			IP6_ADDR_BLOCK7(&echo_netif->ip6_addr[0].u_addr.ip6),
			IP6_ADDR_BLOCK8(&echo_netif->ip6_addr[0].u_addr.ip6));

#endif

	netif_set_default(echo_netif);

    if (netif_is_link_up(echo_netif))
    {
        /* 当netif完全配置好时，必须调用该函数 */
        netif_set_up(echo_netif);
    }
    else
    {
        /* 当netif链接关闭时，必须调用该函数 */
        netif_set_down(echo_netif);
    }

	printf("neftwork setup complete\n");
    
    if (xTaskCreate((TaskFunction_t )lwip_port_input_thread,
                    "recv_echo",
                    8192,
                    echo_netif, 
                    4,
                    NULL) != pdPASS)
    {
      printf("xTaskCreate is Error %s\r\n", "recv_echo");
      FASSERT(0);
	}

#if LWIP_DHCP && LWIP_IPV4
	/* Create a new DHCP client for this interface.
	 * Note: you must call dhcp_fine_tmr() and dhcp_coarse_tmr() at
	 * the predefined regular intervals after starting the client.
	 */
    printf("dhcp_start...\r\n");
    
    ret = xTaskCreate((TaskFunction_t )LwipDhcpTest, /* 任务入口函数 */
                            (const char* )"LwipDhcpTest",/* 任务名字 */
                            (uint16_t )4096, /* 任务栈大小 */
                            (void* )(echo_netif),/* 任务入口函数参数 */
                            (UBaseType_t )configMAX_PRIORITIES-1, /* 任务的优先级 */
                            (TaskHandle_t* )&appTaskCreateHandle); /* 任务控制 */
                            
    if (pdPASS == ret)
    {
        printf("create lwip dhcp task success!\r\n");  
    }   
	
#endif

exit:
    vTaskDelete(NULL);
}

void LwipTest(void *args)
{
    BaseType_t ret;
    ret = xTaskCreate((TaskFunction_t)LwipTestCreate, /* 任务入口函数 */
                      (const char *)"LwipTestCreate", /* 任务名字 */
                      (uint16_t)2048,                 /* 任务栈大小 */
                      (void *)args,                   /* 任务入口函数参数 */
                      (UBaseType_t)configMAX_PRIORITIES-1,/* 任务的优先级 */
                      NULL);                          /* 任务控制块指针 */

    FASSERT_MSG(ret == pdPASS,"LwipTestCreate Task create is failed");

}



int FXmacPhyGpioInit(u32 instance_id,u32 interface_type)
{
#if defined(CONFIG_TARGET_E2000Q)
#if defined(CONFIG_BOARD_TYPE_B)
    if(instance_id == 3)
    {
        if(interface_type == PHY_INTERRUPTFACE_RGMII)
        {
            FPinSetConfig(FIOPAD_J37,FPIN_FUNC1,FPIN_PULL_NONE,FPIN_DRV4); /* gsd_gmu_rgmii_txd1_0
 */
            FPinSetConfig(FIOPAD_J39,FPIN_FUNC1,FPIN_PULL_NONE,FPIN_DRV4); /* gsd_gmu_rgmii_txd1_1
 */
            FPinSetConfig(FIOPAD_G41,FPIN_FUNC1,FPIN_PULL_NONE,FPIN_DRV4); /* gsd_gmu_rgmii_rxd1_0
 */
            FPinSetConfig(FIOPAD_E43,FPIN_FUNC1,FPIN_PULL_NONE,FPIN_DRV4); /* gsd_gmu_rgmii_rxd1_1
 */
            FPinSetConfig(FIOPAD_L43,FPIN_FUNC1,FPIN_PULL_NONE,FPIN_DRV4); /* gsd_gmu_rgmii_tx_ctl1 */
            FPinSetConfig(FIOPAD_C43,FPIN_FUNC1,FPIN_PULL_NONE,FPIN_DRV4); /* gsd_gmu_rgmii_rxd1_2 */
            FPinSetConfig(FIOPAD_E41,FPIN_FUNC1,FPIN_PULL_NONE,FPIN_DRV4); /* gsd_gmu_rgmii_rxd1_3 */
            FPinSetConfig(FIOPAD_L45,FPIN_FUNC1,FPIN_PULL_NONE,FPIN_DRV4); /* gsd_gmu_rgmii_rx_clk1 */
            FPinSetConfig(FIOPAD_J43,FPIN_FUNC1,FPIN_PULL_NONE,FPIN_DRV4); /* gsd_gmu_rgmii_rx_ctl1 */
            FPinSetConfig(FIOPAD_J41,FPIN_FUNC1,FPIN_PULL_NONE,FPIN_DRV4); /* gsd_gmu_rgmii_tx_clk1 */
            FPinSetDelay(FIOPAD_J41_DELAY,FPIN_OUTPUT_DELAY,FPIN_DELAY_FINE_TUNING,FPIN_DELAY_7);
            FPinSetDelay(FIOPAD_J41_DELAY,FPIN_OUTPUT_DELAY,FPIN_DELAY_COARSE_TUNING,FPIN_DELAY_5);
            FPinSetDelayEn(FIOPAD_J41_DELAY,FPIN_OUTPUT_DELAY,1);

            FPinSetConfig(FIOPAD_L39,FPIN_FUNC1,FPIN_PULL_NONE,FPIN_DRV4); /* gsd_gmu_rgmii_txd1_2 */
            FPinSetConfig(FIOPAD_E37,FPIN_FUNC1,FPIN_PULL_NONE,FPIN_DRV4); /* gsd_gmu_rgmii_txd1_3 */
            FPinSetConfig(FIOPAD_E35,FPIN_FUNC1,FPIN_PULL_NONE,FPIN_DRV4); /* gsd_gmu_mdc_mac3 */
            FPinSetConfig(FIOPAD_G35,FPIN_FUNC1,FPIN_PULL_NONE,FPIN_DRV4); /* gsd_gmu_mdio_mac3 */
        }
        else if(interface_type == PHY_INTERRUPTFACE_SGMII)
        {
            FPinSetConfig(FIOPAD_E35,FPIN_FUNC1,FPIN_PULL_NONE,FPIN_DRV4); /* gsd_gmu_mdc_mac3 */
            FPinSetConfig(FIOPAD_G35,FPIN_FUNC1,FPIN_PULL_NONE,FPIN_DRV4); /* gsd_gmu_mdio_mac3 */
        }
        else
        {
            printf("interface_type 0x%x is not support  \r\n");
            return -1;
        }
    }
#elif defined(CONFIG_BOARD_TYPE_C)
    if(instance_id == 1)
    {
        FPinSetConfig(FIOPAD_AJ53,FPIN_FUNC3,FPIN_PULL_NONE,FPIN_DRV4); /* gsd_gmu_mdc_mac1 */
        FPinSetConfig(FIOPAD_AL49,FPIN_FUNC3,FPIN_PULL_NONE,FPIN_DRV4); /* gsd_gmu_mdio_mac1 */
    }
    else if(instance_id == 2)
    {
        FPinSetConfig(FIOPAD_E29,FPIN_FUNC1,FPIN_PULL_NONE,FPIN_DRV4); /* gsd_gmu_mdc_mac2 */
        FPinSetConfig(FIOPAD_G29,FPIN_FUNC1,FPIN_PULL_NONE,FPIN_DRV4); /* gsd_gmu_mdio_mac2 */
    }
    else if(instance_id == 3)
    {
        FPinSetConfig(FIOPAD_E35,FPIN_FUNC1,FPIN_PULL_NONE,FPIN_DRV4); /* gsd_gmu_mdc_mac3  */
        FPinSetConfig(FIOPAD_G35,FPIN_FUNC1,FPIN_PULL_NONE,FPIN_DRV4); /* gsd_gmu_mdio_mac3 */
    }
    else
    {
        printf("interface_type 0x%x is not support  \r\n");
        return -1;
    }
#endif
#elif defined(CONFIG_TARGET_E2000D) || defined(CONFIG_TARGET_E2000S)

#if defined(CONFIG_BOARD_TYPE_B)
    if(instance_id == 3)
    {
        if(interface_type == PHY_INTERRUPTFACE_RGMII)
        {
            FPinSetConfig(FIOPAD_J33,FPIN_FUNC1,FPIN_PULL_NONE,FPIN_DRV4); /* gsd_gmu_rgmii_txd1_0
 */
            FPinSetConfig(FIOPAD_J35,FPIN_FUNC1,FPIN_PULL_NONE,FPIN_DRV4); /* gsd_gmu_rgmii_txd1_1
 */
            FPinSetConfig(FIOPAD_G37,FPIN_FUNC1,FPIN_PULL_NONE,FPIN_DRV4); /* gsd_gmu_rgmii_rxd1_0
 */
            FPinSetConfig(FIOPAD_E39,FPIN_FUNC1,FPIN_PULL_NONE,FPIN_DRV4); /* gsd_gmu_rgmii_rxd1_1
 */
            FPinSetConfig(FIOPAD_L39,FPIN_FUNC1,FPIN_PULL_NONE,FPIN_DRV4); /* gsd_gmu_rgmii_tx_ctl1 */
            FPinSetConfig(FIOPAD_C39,FPIN_FUNC1,FPIN_PULL_NONE,FPIN_DRV4); /* gsd_gmu_rgmii_rxd1_2 */
            FPinSetConfig(FIOPAD_E37,FPIN_FUNC1,FPIN_PULL_NONE,FPIN_DRV4); /* gsd_gmu_rgmii_rxd1_3 */
            FPinSetConfig(FIOPAD_L41,FPIN_FUNC1,FPIN_PULL_NONE,FPIN_DRV4); /* gsd_gmu_rgmii_rx_clk1 */
            FPinSetConfig(FIOPAD_J39,FPIN_FUNC1,FPIN_PULL_NONE,FPIN_DRV4); /* gsd_gmu_rgmii_rx_ctl1 */
            FPinSetConfig(FIOPAD_J37,FPIN_FUNC1,FPIN_PULL_NONE,FPIN_DRV4); /* gsd_gmu_rgmii_tx_clk1 */
            FPinSetDelay(FIOPAD_J37_DELAY,FPIN_OUTPUT_DELAY,FPIN_DELAY_COARSE_TUNING,FPIN_DELAY_5);
            FPinSetDelay(FIOPAD_J37_DELAY,FPIN_OUTPUT_DELAY,FPIN_DELAY_FINE_TUNING,FPIN_DELAY_7);
            FPinSetDelayEn(FIOPAD_J37_DELAY,FPIN_OUTPUT_DELAY,1);
            FPinSetConfig(FIOPAD_L35,FPIN_FUNC1,FPIN_PULL_NONE,FPIN_DRV4); /* gsd_gmu_rgmii_txd1_2 */
            FPinSetConfig(FIOPAD_E33,FPIN_FUNC1,FPIN_PULL_NONE,FPIN_DRV4); /* gsd_gmu_rgmii_txd1_3 */
            FPinSetConfig(FIOPAD_E31,FPIN_FUNC1,FPIN_PULL_NONE,FPIN_DRV4); /* gsd_gmu_mdc_mac3 */
            FPinSetConfig(FIOPAD_G31,FPIN_FUNC1,FPIN_PULL_NONE,FPIN_DRV4); /* gsd_gmu_mdio_mac3 */
        }
        else if(interface_type == PHY_INTERRUPTFACE_SGMII)
        {
            FPinSetConfig(FIOPAD_E31,FPIN_FUNC1,FPIN_PULL_NONE,FPIN_DRV4); /* gsd_gmu_mdc_mac3 */
            FPinSetConfig(FIOPAD_G31,FPIN_FUNC1,FPIN_PULL_NONE,FPIN_DRV4); /* gsd_gmu_mdio_mac3 */
        }
        else
        {
            printf("interface_type 0x%x is not support  \r\n");
            return -1;
        }
    }
#elif defined(CONFIG_BOARD_TYPE_C)
    if(instance_id == 1)
    {
        FPinSetConfig(FIOPAD_AJ49,FPIN_FUNC3,FPIN_PULL_NONE,FPIN_DRV4); /* gsd_gmu_mdc_mac1 */
        FPinSetConfig(FIOPAD_AL45,FPIN_FUNC3,FPIN_PULL_NONE,FPIN_DRV4); /* gsd_gmu_mdio_mac1 */
    }
    else if(instance_id == 2)
    {
        FPinSetConfig(FIOPAD_E25,FPIN_FUNC1,FPIN_PULL_NONE,FPIN_DRV4); /* gsd_gmu_mdc_mac2 */
        FPinSetConfig(FIOPAD_G25,FPIN_FUNC1,FPIN_PULL_NONE,FPIN_DRV4); /* gsd_gmu_mdio_mac2 */
    }
    else if(instance_id == 3)
    {
        FPinSetConfig(FIOPAD_E31,FPIN_FUNC1,FPIN_PULL_NONE,FPIN_DRV4); /* gsd_gmu_mdc_mac3  */
        FPinSetConfig(FIOPAD_G31,FPIN_FUNC1,FPIN_PULL_NONE,FPIN_DRV4); /* gsd_gmu_mdio_mac3 */
    }
    else
    {
        printf("interface_type 0x%x is not support  \r\n");
        return -1;
    }

#endif

#endif

}


static int LwipDeviceSet(int argc, char *argv[])
{
    u32 id = 0,type = 0;

    static int probe_flg = 0;
    LWIP_PORT_CONFIG_DEFAULT_INIT(lwip_mac_config);

    if (!strcmp(argv[1], "probe"))
    {
        if(probe_flg == 1)
        {
            printf("The initialization of the instance is complete. Do not repeat this process \r\n") ;
            return -1;
        }

        switch(argc)
        {
            case 4:
                type = (u32)simple_strtoul(argv[3], NULL, 10);
                id = (u32)simple_strtoul(argv[2], NULL, 10);
                break;
            case 3:
                id = (u32)simple_strtoul(argv[2], NULL, 10);
                break;
            default:
                break;
        }
        printf("types   %d\n", type);
        printf("id   %d\n", id);
        FXmacPhyGpioInit(id,type);
        lwip_mac_config.mac_instance = id;
        if(type == 0)
        {
            lwip_mac_config.mii_interface = LWIP_PORT_INTERFACE_RGMII;
        }
        else
        {
            lwip_mac_config.mii_interface = LWIP_PORT_INTERFACE_SGMII;
        }

        LwipTest(&lwip_mac_config); 
        probe_flg = 1;
    }
    else
    {
        printf("Please enter xmac probe <device id> <interface id > \r\n") ;
        printf("        -- device id is mac instance number \r\n");
        printf("        -- interface id is media independent interface  , 0 is rgmii ,1 is sgmii \r\n");
    }

    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), xmac, LwipDeviceSet, Setup LWIP device test);

