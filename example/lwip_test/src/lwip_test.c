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
 * Date: 2022-06-06 22:57:08
 * LastEditTime: 2022-06-06 22:57:08
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
#include "parameters.h"

#ifndef SDK_CONFIG_H__
	#error "Please include sdkconfig.h first"
#endif



#include "lwip_port.h"
#include "lwip/ip4_addr.h"
#include "lwip/init.h"
#include "netif/ethernet.h"
#include "ethernetif.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "../src/shell.h"



#if LWIP_IPV6
#include "lwip/ip.h"
#include "lwip/ip6_addr.h"
#else
#if LWIP_DHCP
#include "lwip/dhcp.h"
#endif
#endif

#if defined(CONFIG_ENABLE_FGMAC)
#include "ft_os_gmac.h"

static FtOsGmac os_gmac[GMAC_INSTANCE_NUM] = {0};
static u32 gmac_id = FT_OS_GMAC0_ID;

/* the mac address of the board. this should be unique per board */
    unsigned char mac_ethernet_address[GMAC_INSTANCE_NUM][NETIF_MAX_HWADDR_LEN] = {
        {0x0, 0x0, 0x1, 0x11, 0x1, 0x21},
        {0x0, 0x0, 0x2, 0x22, 0x2, 0x23}
    };

#if !LWIP_IPV6
ip4_addr_t ipaddr[GMAC_INSTANCE_NUM], netmask[GMAC_INSTANCE_NUM], gw[GMAC_INSTANCE_NUM];
#if LWIP_DHCP
static TaskHandle_t appTaskCreateHandle = NULL;
void LwipDhcpTest(FtOsGmac *os_gmac)
{
    int mscnt = 0;
    dhcp_start(&(os_gmac->netif_object));
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

void LwipRawInit(FtOsGmac *os_gmac)
{
    BaseType_t ret = pdPASS;/* 定义一个创建信息返回值，默认为 pdPASS */
    static boolean init_flag = FALSE;

#if !LWIP_IPV6

    memset(&ipaddr[gmac_id], 0, sizeof(ip4_addr_t));
    memset(&netmask[gmac_id], 0, sizeof(ip4_addr_t));
    memset(&gw[gmac_id], 0, sizeof(ip4_addr_t));

#if LWIP_DHCP
    ipaddr[gmac_id].addr = 0;
	gw[gmac_id].addr = 0;
	netmask[gmac_id].addr = 0;
#else
	/* initialize IP addresses to be used */
    IP4_ADDR(&ipaddr[gmac_id], 
            os_gmac->config.address.ip_address[0], 
            os_gmac->config.address.ip_address[1], 
            os_gmac->config.address.ip_address[2], 
            os_gmac->config.address.ip_address[3]);
    IP4_ADDR(&netmask[gmac_id], 
            os_gmac->config.address.netmask_address[0], 
            os_gmac->config.address.netmask_address[1], 
            os_gmac->config.address.netmask_address[2], 
            os_gmac->config.address.netmask_address[3]);
    IP4_ADDR(&gw[gmac_id], 
            os_gmac->config.address.netmask_address[0], 
            os_gmac->config.address.netmask_address[1], 
            os_gmac->config.address.netmask_address[2], 
            os_gmac->config.address.netmask_address[3]);
#endif

#endif
   
    /* 初始化LwIP堆 */
    if(init_flag == FALSE)
    {
        tcpip_init(NULL, NULL);
        init_flag = TRUE;
    }
    
    /* 添加网络接口 (IPv4/IPv6)  */
    lwip_port_add(&os_gmac->netif_object, &ipaddr[gmac_id], &netmask[gmac_id], &gw[gmac_id], 
                    mac_ethernet_address[gmac_id], gmac_id);
    
    /* 注册默认网络接口 */
    netif_set_default(&os_gmac->netif_object);
    
    if (netif_is_link_up(&os_gmac->netif_object))
    {
        /* 当netif完全配置好时，必须调用该函数 */
        netif_set_up(&os_gmac->netif_object);
    }
    else
    {
        /* 当netif链接关闭时，必须调用该函数 */
        netif_set_down(&os_gmac->netif_object);
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
                            (void* )(os_gmac),/* 任务入口函数参数 */
                            (UBaseType_t )configMAX_PRIORITIES-1, /* 任务的优先级 */
                            (TaskHandle_t* )&appTaskCreateHandle); /* 任务控制 */
    if (pdPASS == ret)
    {
        printf("create lwip dhcp task success!\r\n");  
    }   
	
#endif

}


void LwipTestCreate(void * args)
{
    FtOsGmacConfig os_config[GMAC_INSTANCE_NUM] = 
    {
        {
            .gmac_instance = 0,
            .isr_priority = 0, /* irq Priority */
            .address = 
            {
                {192, 168, 4, 10},
                {255, 255, 255, 0},
                {192, 168, 4, 1}
            },
            .mac_input_thread = 
            {
                .thread_name = "gmac0",
                .stack_depth = 4096, /* The number of words the stack */
                .priority = configMAX_PRIORITIES-1, /* Defines the priority at which the task will execute. */
                .thread_handle = NULL,
            }, /* Gmac input thread */
        },
        {
            .gmac_instance = 1,
            .isr_priority = 0, /* irq Priority */
            .address = 
            {
                {192, 168, 4, 20},
                {255, 255, 255, 0},
                {192, 168, 4, 1}
            },
            .mac_input_thread = 
            {
                .thread_name = "gmac1",
                .stack_depth = 4096, /* The number of words the stack */
                .priority = configMAX_PRIORITIES-1, /* Defines the priority at which the task will execute. */
                .thread_handle = NULL,
            }, /* Gmac input thread */
        },
    };
    
    /* !!! make sure eth in-place before init gmac */
    FtOsGmacObjectInit(&os_gmac[gmac_id], &os_config[gmac_id]);

    LwipRawInit(&os_gmac[gmac_id]);

    vTaskDelete(NULL);
}

void LwipTest(void)
{
    BaseType_t ret;
    ret = xTaskCreate((TaskFunction_t)LwipTestCreate, /* 任务入口函数 */
                      (const char *)"LwipTestCreate", /* 任务名字 */
                      (uint16_t)2048,                 /* 任务栈大小 */
                      (void *)NULL,                   /* 任务入口函数参数 */
                      (UBaseType_t)configMAX_PRIORITIES-1,/* 任务的优先级 */
                      NULL);                          /* 任务控制块指针 */

    FASSERT_MSG(ret == pdPASS,"LwipTestCreate Task create is failed");

}

static int GmacIdSet(int argc, char *argv[])
{

    if (!strcmp(argv[1], "probe"))
    {
        if (argc >= 3)
        {
            gmac_id = (u32)simple_strtoul(argv[2], NULL, 10);            
        }
        else
        {
            gmac_id = FT_OS_GMAC0_ID;
        }
        LwipTest(); 
    }

    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), gmac, GmacIdSet, set the gmac id);
#endif