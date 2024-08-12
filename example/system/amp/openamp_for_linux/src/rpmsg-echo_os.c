/*
 * Copyright : (C) 2024 Phytium Information Technology, Inc.
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
 * FilePath: rpmsg-echo_os.c
 * Created Date: 2022-02-25 09:12:07
 * Last Modified: 2024-07-17 09:17:44
 * Description:  This file is for This file is for a sample demonstration application that showcases usage of rpmsg.
 *               This application is meant to run on the remote CPU running freertos code.
 *               This application echoes back data that was sent to it by the master core.
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 huanghe    2022/03/25  first commit  
 * 1.1 liusm	  2023/11/17  Adapter example for linux
 * 1.2 liusm 	  2024/03/04  update example
 */

/***************************** Include Files *********************************/

#include <stdio.h>
#include <openamp/open_amp.h>
#include <openamp/version.h>
#include <metal/alloc.h>
#include <metal/version.h>
#include "platform_info.h"
#include "rpmsg_service.h"
#include <metal/sleep.h>
#include "rsc_table.h"
#include "FreeRTOS.h"
#include "task.h"
#include "finterrupt.h"
#include "fpsci.h"
#include "fdebug.h"
#include "helper.h"
#include "openamp_configs.h"
#include "rsc_table.h"
#include "libmetal_configs.h"
#include "slaver_00_example.h"

/**************************** Type Definitions *******************************/

#define OPENAMP_SLAVE_DEBUG_TAG "OPENAMP_SLAVE"
#define OPENAMP_SLAVE_ERROR(format, ...) FT_DEBUG_PRINT_E(OPENAMP_SLAVE_DEBUG_TAG, format, ##__VA_ARGS__)
#define OPENAMP_SLAVE_WARN(format, ...)  FT_DEBUG_PRINT_W(OPENAMP_SLAVE_DEBUG_TAG, format, ##__VA_ARGS__)
#define OPENAMP_SLAVE_INFO(format, ...)  FT_DEBUG_PRINT_I(OPENAMP_SLAVE_DEBUG_TAG, format, ##__VA_ARGS__)
#define OPENAMP_SLAVE_DEBUG(format, ...) FT_DEBUG_PRINT_D(OPENAMP_SLAVE_DEBUG_TAG, format, ##__VA_ARGS__)

#define SHUTDOWN_MSG				0xEF56A55A

/************************** 资源表定义，与linux协商一致 **********/
static struct remote_resource_table __resource resources __attribute__((used)) = {
	/* Version */
	1,

	/* NUmber of table entries */
	NUM_TABLE_ENTRIES,
	/* reserved fields */
	{0, 0,},

	/* Offsets of rsc entries */
	{
	 offsetof(struct remote_resource_table, rpmsg_vdev),
	},

	/* Virtio device entry */
	{
	 RSC_VDEV, VIRTIO_ID_RPMSG_, VDEV_NOTIFYID, RPMSG_IPU_C0_FEATURES, 0, 0, 0,
	 NUM_VRINGS, {0, 0},
	},
    
	/* Vring rsc entry - part of vdev rsc entry */
	{SLAVE00_TX_VRING_ADDR, VRING_ALIGN, SLAVE00_VRING_NUM, 1, 0},
	{SLAVE00_RX_VRING_ADDR, VRING_ALIGN, SLAVE00_VRING_NUM, 2, 0},
};

/********** 共享内存定义，与linux协商一致 **********/
static metal_phys_addr_t poll_phys_addr = SLAVE00_KICK_IO_ADDR;
struct metal_device kick_driver_00 = {
    .name = SLAVE_00_KICK_DEV_NAME,
	.bus = NULL,
    .num_regions = 1,
	.regions = {
		{
			.virt = (void *)SLAVE00_KICK_IO_ADDR,
			.physmap = &poll_phys_addr,
			.size = 0x1000,
			.page_shift = -1UL,
			.page_mask = -1UL,
			.mem_flags = SLAVE00_SOURCE_TABLE_ATTRIBUTE,
			.ops = {NULL},
		}
	},
    .irq_num = 1,/* Number of IRQs per device */
	.irq_info = (void *)SLAVE_00_SGI,
} ;

struct remoteproc_priv slave_00_priv = {
    .kick_dev_name =           SLAVE_00_KICK_DEV_NAME  ,
	.kick_dev_bus_name =        KICK_BUS_NAME ,
    .cpu_id        =  MASTER_CORE_MASK,/* 给所有core发送中断 */

	.src_table_attribute = SLAVE00_SOURCE_TABLE_ATTRIBUTE ,
	
	/* |rx vring|tx vring|share buffer| */
	.share_mem_va = SLAVE00_SHARE_MEM_ADDR ,
	.share_mem_pa = SLAVE00_SHARE_MEM_ADDR ,
	.share_buffer_offset = SLAVE00_VRING_SIZE ,
	.share_mem_size = SLAVE00_SHARE_MEM_SIZE ,
	.share_mem_attribute = SLAVE00_SHARE_BUFFER_ATTRIBUTE
} ;

/*******************例程全局变量***********************************************/
struct remoteproc remoteproc_slave_00;
static struct rpmsg_device *rpdev_slave_00 = NULL;
static int shutdown_req;
static char temp_data[RPMSG_BUFFER_SIZE];
/************************** Function Prototypes ******************************/
/*-----------------------------------------------------------------------------*
 *  RPMSG endpoint callbacks
 *-----------------------------------------------------------------------------*/
static int rpmsg_endpoint_cb(struct rpmsg_endpoint *ept, void *data, size_t len, uint32_t src, void *priv)
{
	(void)priv;
	(void)src;
	/* On reception of a shutdown we signal the application to terminate */
	if ((*(unsigned int *)data) == SHUTDOWN_MSG) {
		OPENAMP_SLAVE_INFO("shutdown message is received.");
		shutdown_req = 1;
		return RPMSG_SUCCESS;
	}

    memset(temp_data,0,len) ;
    memcpy(temp_data,data,len) ;
	/* Send temp_data back to master */
    /* 请勿直接对data指针对应的内存进行写操作，操作vring中remoteproc发送通道分配的内存，引发错误的问题*/
	
	/* Send data back to master */
	if (rpmsg_send(ept, temp_data, len) < 0)
		OPENAMP_SLAVE_ERROR("rpmsg_send failed!!!\r\n");

	return RPMSG_SUCCESS;
}

static void rpmsg_service_unbind(struct rpmsg_endpoint *ept)
{
	(void)ept;
	OPENAMP_SLAVE_INFO("unexpected Remote endpoint destroy.");
	shutdown_req = 1;
}

/*-----------------------------------------------------------------------------*
 *  Application
 *-----------------------------------------------------------------------------*/
static int FRpmsgEchoApp(struct rpmsg_device *rdev, void *priv)
{
    int ret = 0;
    struct rpmsg_endpoint lept;
    shutdown_req = 0;
    /* Initialize RPMSG framework */
    OPENAMP_SLAVE_INFO("Try to create rpmsg endpoint.\r\n");

    ret = rpmsg_create_ept(&lept, rdev, RPMSG_SERVICE_NAME, 0, RPMSG_ADDR_ANY, rpmsg_endpoint_cb, rpmsg_service_unbind);
    if (ret)
    {
        OPENAMP_SLAVE_ERROR("Failed to create endpoint. %d \r\n", ret);
        return -1;
    }

    OPENAMP_SLAVE_INFO("Successfully created rpmsg endpoint.\r\n");

    while (1)
    {
        platform_poll(priv);
        /* we got a shutdown request, exit */
        if (shutdown_req)
        {
            break;
        }
    }

    rpmsg_destroy_ept(&lept);

    return ret;
}


/*-----------------------------------------------------------------------------*
 *  Application entry point
 *-----------------------------------------------------------------------------*/
int slave_init(void)
{
    init_system();  // Initialize the system resources and environment
    
    if (!platform_create_proc(&remoteproc_slave_00, &slave_00_priv, &kick_driver_00)) 
    {
        OPENAMP_SLAVE_ERROR("Failed to create remoteproc instance for slave 00\r\n");
        return -1;  // Return with an error if creation fails
    }
    
    remoteproc_slave_00.rsc_table = &resources;

    if (platform_setup_src_table(&remoteproc_slave_00,remoteproc_slave_00.rsc_table)) 
    {
        OPENAMP_SLAVE_ERROR("Failed to setup src table for slave 00\r\n");
        return -1;  // Return with an error if setup fails
    }
    
    OPENAMP_SLAVE_INFO("Setup resource tables for the created remoteproc instances is over \r\n");

    if (platform_setup_share_mems(&remoteproc_slave_00)) 
    {
        OPENAMP_SLAVE_ERROR("Failed to setup shared memory for slave 00\r\n");
        return -1;  // Return with an error if setup fails
    }

    OPENAMP_SLAVE_INFO("Setup shared memory regions for both remoteproc instances is over \r\n");

    rpdev_slave_00 = platform_create_rpmsg_vdev(&remoteproc_slave_00, 0, VIRTIO_DEV_SLAVE, NULL, NULL);
    if (!rpdev_slave_00) 
    {
        OPENAMP_SLAVE_ERROR("Failed to create rpmsg vdev for slave 00\r\n");
        return -1;  // Return with an error if creation fails
    }

    return 0 ;   
} 

int RpmsgEchoTask(void * args)
{
	int ret;
	printf("openamp lib version: %s (", openamp_version());
	printf("Major: %d, ", openamp_version_major());
	printf("Minor: %d, ", openamp_version_minor());
	printf("Patch: %d)\r\n", openamp_version_patch());

	printf("libmetal lib version: %s (", metal_ver());
	printf("Major: %d, ", metal_ver_major());
	printf("Minor: %d, ", metal_ver_minor());
	printf("Patch: %d)\r\n", metal_ver_patch());

	/* Initialize platform */
	OPENAMP_SLAVE_INFO("start application...");
	if(!slave_init())
    {
        FRpmsgEchoApp(rpdev_slave_00,&remoteproc_slave_00) ;
        if (ret)
        {
            OPENAMP_SLAVE_ERROR("Failed to running echoapp");
            return platform_cleanup(&remoteproc_slave_00);
        }
        platform_release_rpmsg_vdev(rpdev_slave_00, &remoteproc_slave_00);
        OPENAMP_SLAVE_INFO("Stopping application...");
        platform_cleanup(&remoteproc_slave_00);
        return ret;
    }
    else
    {
        platform_cleanup(&remoteproc_slave_00);
        OPENAMP_SLAVE_ERROR("Failed to init remoteproc.\r\n");
    }
    vTaskDelete(NULL);
}

int rpmsg_echo_task(void)
{
    BaseType_t ret; 

    ret = xTaskCreate((TaskFunction_t )RpmsgEchoTask, /* 任务入口函数 */
                        (const char* )"RpmsgEchoTask",/* 任务名字 */
                        (4096*2), /* 任务栈大小 */
                        (void* )NULL,/* 任务入口函数参数 */
                        (UBaseType_t )4, /* 任务的优先级 */
                        NULL); /* 任务控制块指针 */
    
    if(ret != pdPASS)
    {
        OPENAMP_SLAVE_ERROR("Failed to create a rpmsg_echo task. \r\n");
        return -1;
    }
    return 0;
}
