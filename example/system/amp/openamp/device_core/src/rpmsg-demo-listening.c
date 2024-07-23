/*
 * @Copyright : (C) 2022 Phytium Information Technology, Inc. 
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
 * @FilePath: rpmsg-demo-listening.c
 * @Date: 2023-04-23 16:42:27
 * @LastEditTime: 2023-04-23 16:42:27
 * @Description:  This file is for wait driver_core msg
 * 
 * @Modify History: 
 *  Ver   Who           Date        Changes
 * ----- ------         --------    --------------------------------------
 * 1.0  liushengming    2023/04/23  first release
 */

/***************************** Include Files *********************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <openamp/open_amp.h>
#include <metal/alloc.h>
#include "platform_info.h"
#include "rsc_table.h"
#include "helper.h"
#include "rpmsg_service.h"
#include "fcache.h"
#include "fdebug.h"
#include "ftypes.h"
#include "FreeRTOS.h"
#include "task.h"

#include "rpmsg-demo-listening.h"
#include "openamp_configs.h"
#include "libmetal_configs.h"

/************************** Constant Definitions *****************************/
/***************** Macros (Inline Functions) Definitions *********************/

#define SHUTDOWN_MSG                0xEF56A55A
#define RECV_MSG                    0xE5E5E5E5

#define     DEMO_LIST_SLAVE_DEBUG_TAG "DEMO_LIST_SLAVE"
#define     DEMO_LIST_SLAVE_DEBUG_I(format, ...) FT_DEBUG_PRINT_I( DEMO_LIST_SLAVE_DEBUG_TAG, format, ##__VA_ARGS__)
#define     DEMO_LIST_SLAVE_DEBUG_W(format, ...) FT_DEBUG_PRINT_W( DEMO_LIST_SLAVE_DEBUG_TAG, format, ##__VA_ARGS__)
#define     DEMO_LIST_SLAVE_DEBUG_E(format, ...) FT_DEBUG_PRINT_E( DEMO_LIST_SLAVE_DEBUG_TAG, format, ##__VA_ARGS__)

#define RPMSG_PING_DEMO     0x1
#define RPMSG_SAM_PING_DEMO 0X2
#define RPMSG_MAT_MULT_DEMO 0X3
#define RPMSG_NO_COPY_DEMO  0X4

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

struct metal_device kick_driver_00 = {
    .name = SLAVE_00_KICK_DEV_NAME,
	.bus = NULL,
    .irq_num = 1,/* Number of IRQs per device */
	.irq_info = (void *)SLAVE_00_SGI,
} ;


struct remoteproc_priv slave_00_priv = {
    .kick_dev_name =           SLAVE_00_KICK_DEV_NAME  ,
	.kick_dev_bus_name =        KICK_BUS_NAME ,
    .cpu_id        = MASTER_DRIVER_CORE ,

	.src_table_attribute = SLAVE00_SOURCE_TABLE_ATTRIBUTE ,
	
	/* |rx vring|tx vring|share buffer| */
	.share_mem_va = SLAVE00_SHARE_MEM_ADDR ,
	.share_mem_pa = SLAVE00_SHARE_MEM_ADDR ,
	.share_buffer_offset =  SLAVE00_VRING_SIZE ,
	.share_mem_size = SLAVE00_SHARE_MEM_SIZE ,
	.share_mem_attribute = SLAVE00_SHARE_BUFFER_ATTRIBUTE
} ;

struct remoteproc remoteproc_slave_00;
static struct rpmsg_device *rpdev_slave_00 = NULL;
struct rpmsg_endpoint ept_slave_00;

static volatile u32 flag_req = 0;
static volatile int demo_flag = 0;
/************************** Function Prototypes ******************************/
extern int rpmsg_ping(struct rpmsg_device *rdev, void *priv) ;
extern int rpmsg_sample_ping(struct rpmsg_device *rdev, void *priv) ;
extern int matrix_multiply(struct rpmsg_device *rdev, void *priv) ;
extern int rpmsg_nocopy_ping(struct rpmsg_device *rdev, void *priv) ;
/*-----------------------------------------------------------------------------*
 *  RPMSG endpoint callbacks
 *-----------------------------------------------------------------------------*/
static int rpmsg_endpoint_cb(struct rpmsg_endpoint *ept, void *data, size_t len, uint32_t src, void *priv)
{
    (void)priv;
    DEMO_LIST_SLAVE_DEBUG_E("src:0x%x",src);
    ept->dest_addr = src;
    /* On reception of a shutdown we signal the application to terminate */
    if ((*(unsigned int *)data) == SHUTDOWN_MSG)
    {
        DEMO_LIST_SLAVE_DEBUG_I("Shutdown message is received.\r\n");
        flag_req = SHUTDOWN_MSG;
        return RPMSG_SUCCESS;
    }

    demo_flag = (*(unsigned int *)data);
    /* Send data back to master */
    if (rpmsg_send(ept, data, len) < 0)
    {
        DEMO_LIST_SLAVE_DEBUG_E("rpmsg_send failed.\r\n");
    }
    flag_req = RECV_MSG;
    return RPMSG_SUCCESS;
}

static void rpmsg_service_unbind(struct rpmsg_endpoint *ept)
{
    (void)ept;
    rpmsg_destroy_ept(&ept_slave_00);
    DEMO_LIST_SLAVE_DEBUG_I("Unexpected remote endpoint destroy.\r\n");
    flag_req = SHUTDOWN_MSG;
}

/*-----------------------------------------------------------------------------*
 *  Application
 *-----------------------------------------------------------------------------*/
static int app(struct rpmsg_device *rdev, void *priv)
{
    int ret;
    /* Initialize RPMSG framework */
    ret = rpmsg_create_ept(&ept_slave_00, rdev, RPMSG_SERVICE_00_NAME, SLAVE_DEVICE_00_EPT_ADDR, MASTER_DRIVER_EPT_ADDR, rpmsg_endpoint_cb, rpmsg_service_unbind);
    if (ret)
    {
        DEMO_LIST_SLAVE_DEBUG_E("Failed to create endpoint. %d \r\n", ret);
        return -1;
    }
    DEMO_LIST_SLAVE_DEBUG_I("Successfully created rpmsg endpoint.\r\n");
    demo_flag = 0;
    flag_req = 0;
    while (1)
    {
        platform_poll(priv);
        /* we got a shutdown request, exit */
        if (demo_flag != 0 && flag_req == RECV_MSG)
        {
            if (demo_flag == RPMSG_PING_DEMO)
            {
                printf("\r\n\r\n");
                DEMO_LIST_SLAVE_DEBUG_I("*********************************************......\r\n");
                DEMO_LIST_SLAVE_DEBUG_I("***********Demo rpmsg_ping running***********......\r\n");
                DEMO_LIST_SLAVE_DEBUG_I("*********************************************......\r\n");
                ret = rpmsg_ping(rdev, priv);
                if (ret != 0)
                {
                    DEMO_LIST_SLAVE_DEBUG_E("rpmsg_echo running error,ecode:%d.", ret);
                    return 0;
                }
            }

            if (demo_flag == RPMSG_SAM_PING_DEMO)
            {
                printf("\r\n\r\n");
                DEMO_LIST_SLAVE_DEBUG_I("*********************************************......\r\n");
                DEMO_LIST_SLAVE_DEBUG_I("*******Demo rpmsg_sample_ping running********......\r\n");
                DEMO_LIST_SLAVE_DEBUG_I("*********************************************......\r\n");
                ret = rpmsg_sample_ping(rdev, priv);
                if (ret != 0)
                {
                    DEMO_LIST_SLAVE_DEBUG_E("rpmsg_sample_ping running error,ecode:%d.", ret);
                    return 0;
                }
            }

            if (demo_flag == RPMSG_MAT_MULT_DEMO)
            {
                printf("\r\n\r\n");
                DEMO_LIST_SLAVE_DEBUG_I("*********************************************......\r\n");
                DEMO_LIST_SLAVE_DEBUG_I("********Demo matrix_multiply running*********......\r\n");
                DEMO_LIST_SLAVE_DEBUG_I("*********************************************......\r\n");
                ret = matrix_multiply(rdev, priv);
                if (ret != 0)
                {
                    DEMO_LIST_SLAVE_DEBUG_E("matrix_multiply running error,ecode:%d.", ret);
                    return 0;
                }
            }

            if (demo_flag == RPMSG_NO_COPY_DEMO)
            {
                printf("\r\n\r\n");
                DEMO_LIST_SLAVE_DEBUG_I("*********************************************......\r\n");
                DEMO_LIST_SLAVE_DEBUG_I("*******Demo rpmsg_nocopy_ping running********......\r\n");
                DEMO_LIST_SLAVE_DEBUG_I("*********************************************......\r\n");
                ret = rpmsg_nocopy_ping(rdev, priv);
                if (ret != 0)
                {
                    DEMO_LIST_SLAVE_DEBUG_E("rpmsg_nocopy_ping running error,ecode:%d.", ret);
                    return 0;
                }
            }
            flag_req = 0;
            demo_flag = 0;
            DEMO_LIST_SLAVE_DEBUG_I(" Demo running over...");
        }
        if (flag_req == SHUTDOWN_MSG)
        {
            break;
        }
    }
    DEMO_LIST_SLAVE_DEBUG_I("Listening demo over.");
    return ret;
}

int FFreeRTOSOpenAMPSlaveInit(void)
{
    init_system();  /* Initialize the system resources and environment */
    
    if (!platform_create_proc(&remoteproc_slave_00, &slave_00_priv, &kick_driver_00)) 
    {
        DEMO_LIST_SLAVE_DEBUG_E("Failed to create remoteproc instance for slave 00\r\n");
        return -1;  /* Return with an error if creation fails */
    }
    /* Setup resource tables for the created remoteproc instances */
    remoteproc_slave_00.rsc_table = &resources;
    /* 在主核创建rpmsg之前，保证从核已经初始化资源表，否则会导致主核发送中断，从核进入中断服务函数后，获取异常指针资源表 */
    if (platform_setup_src_table(&remoteproc_slave_00,remoteproc_slave_00.rsc_table)) 
    {
        DEMO_LIST_SLAVE_DEBUG_E("Failed to setup src table for slave 00\r\n");
        return -1;  /* Return with an error if setup fails */ 
    }
    
    DEMO_LIST_SLAVE_DEBUG_I("Setup resource tables for the created remoteproc instances is over \r\n");

    if (platform_setup_share_mems(&remoteproc_slave_00)) 
    {
        DEMO_LIST_SLAVE_DEBUG_E("Failed to setup shared memory for slave 00\r\n");
        return -1;  /* Return with an error if setup fails */ 
    }

    DEMO_LIST_SLAVE_DEBUG_I("Setup shared memory regions for both remoteproc instances is over \r\n");

    rpdev_slave_00 = platform_create_rpmsg_vdev(&remoteproc_slave_00, 0, VIRTIO_DEV_SLAVE, NULL, NULL);
    if (!rpdev_slave_00) 
    {
        DEMO_LIST_SLAVE_DEBUG_E("Failed to create rpmsg vdev for slave 00\r\n");
        return -1;  /* Return with an error if creation fails */ 
    }

    return 0 ;   
}

void rpmsg_listening_task(void *args)
{
    int ret = 0;
    while (!ret)
    {
        DEMO_LIST_SLAVE_DEBUG_I("SLAVER Starting application...\r\n");
        if(!FFreeRTOSOpenAMPSlaveInit())
        {
            ret = app(rpdev_slave_00,&remoteproc_slave_00) ;
        }
        else
        {
            DEMO_LIST_SLAVE_DEBUG_E("Failed to init remoteproc.\r\n");
            break;
        }
        platform_release_rpmsg_vdev(rpdev_slave_00, &remoteproc_slave_00);
        DEMO_LIST_SLAVE_DEBUG_I("Stopping application...\r\n");
        platform_cleanup(&remoteproc_slave_00);
    }
    DEMO_LIST_SLAVE_DEBUG_E("SLAVER Exiting...\r\n");
    vTaskDelete(NULL);
}

int rpmsg_listening_func(void)
{
    BaseType_t ret; 
    
    taskENTER_CRITICAL(); /* no schedule when create task */
    ret = xTaskCreate((TaskFunction_t )rpmsg_listening_task, /* 任务入口函数 */
                        (const char* )"rpmsg_listening_task",/* 任务名字 */
                        (uint16_t )4096*2, /* 任务栈大小 */
                        (void* )NULL,/* 任务入口函数参数 */
                        (UBaseType_t )4, /* 任务的优先级 */
                        NULL); /* 任务控制块指针 */
    taskEXIT_CRITICAL(); /* allow schedule since task created */

    if(ret != pdPASS)
    {
        DEMO_LIST_SLAVE_DEBUG_E("Failed to create a rpmsg_listening_task task ");
        return -1;
    }
    return ret;
}