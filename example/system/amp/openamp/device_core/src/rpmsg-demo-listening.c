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

#define     DEMO_LIST_DEVICE_DEBUG_TAG "DEMO_LIST_DEVICE"
#define     DEMO_LIST_DEVICE_DEBUG_I(format, ...) FT_DEBUG_PRINT_I( DEMO_LIST_DEVICE_DEBUG_TAG, format, ##__VA_ARGS__)
#define     DEMO_LIST_DEVICE_DEBUG_W(format, ...) FT_DEBUG_PRINT_W( DEMO_LIST_DEVICE_DEBUG_TAG, format, ##__VA_ARGS__)
#define     DEMO_LIST_DEVICE_DEBUG_E(format, ...) FT_DEBUG_PRINT_E( DEMO_LIST_DEVICE_DEBUG_TAG, format, ##__VA_ARGS__)

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
	{DEVICE00_TX_VRING_ADDR, VRING_ALIGN, DEVICE00_VRING_NUM, 1, 0},
	{DEVICE00_RX_VRING_ADDR, VRING_ALIGN, DEVICE00_VRING_NUM, 2, 0},
};

struct metal_device kick_driver_00 = {
    .name = DEVICE_00_KICK_DEV_NAME,
	.bus = NULL,
    .irq_num = 1,/* Number of IRQs per device */
	.irq_info = (void *)DEVICE_00_SGI,
} ;


struct remoteproc_priv device_00_priv = {
    .kick_dev_name =           DEVICE_00_KICK_DEV_NAME  ,
	.kick_dev_bus_name =        KICK_BUS_NAME ,
    .cpu_id        = DRIVER_CORE ,

	.src_table_attribute = DEVICE00_SOURCE_TABLE_ATTRIBUTE ,
	
	/* |rx vring|tx vring|share buffer| */
	.share_mem_va = DEVICE00_SHARE_MEM_ADDR ,
	.share_mem_pa = DEVICE00_SHARE_MEM_ADDR ,
	.share_buffer_offset =  DEVICE00_VRING_SIZE ,
	.share_mem_size = DEVICE00_SHARE_MEM_SIZE ,
	.share_mem_attribute = DEVICE00_SHARE_BUFFER_ATTRIBUTE
} ;

struct remoteproc remoteproc_device_00;
static struct rpmsg_device *rpdev_device_00 = NULL;
struct rpmsg_endpoint ept_device_00;

static volatile u32 flag_req = 0;
static volatile int demo_flag = 0;
/************************** Function Prototypes ******************************/
/* External functions */
extern int init_system();

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
    DEMO_LIST_DEVICE_DEBUG_E("src:0x%x",src);
    ept->dest_addr = src;
    /* On reception of a shutdown we signal the application to terminate */
    if ((*(unsigned int *)data) == SHUTDOWN_MSG)
    {
        DEMO_LIST_DEVICE_DEBUG_I("Shutdown message is received.\r\n");
        flag_req = SHUTDOWN_MSG;
        return RPMSG_SUCCESS;
    }

    demo_flag = (*(unsigned int *)data);
    /* Send data back to driver */
    if (rpmsg_send(ept, data, len) < 0)
    {
        DEMO_LIST_DEVICE_DEBUG_E("rpmsg_send failed.\r\n");
    }
    flag_req = RECV_MSG;
    return RPMSG_SUCCESS;
}

static void rpmsg_service_unbind(struct rpmsg_endpoint *ept)
{
    (void)ept;
    rpmsg_destroy_ept(&ept_device_00);
    DEMO_LIST_DEVICE_DEBUG_I("Unexpected remote endpoint destroy.\r\n");
    flag_req = SHUTDOWN_MSG;
}

/*-----------------------------------------------------------------------------*
 *  Application
 *-----------------------------------------------------------------------------*/
static int app(struct rpmsg_device *rdev, void *priv)
{
    int ret;
    /* Initialize RPMSG framework */
    ret = rpmsg_create_ept(&ept_device_00, rdev, RPMSG_SERVICE_00_NAME, DEVICE_00_EPT_ADDR, DRIVER_EPT_ADDR, rpmsg_endpoint_cb, rpmsg_service_unbind);
    if (ret)
    {
        DEMO_LIST_DEVICE_DEBUG_E("Failed to create endpoint. %d \r\n", ret);
        return -1;
    }
    DEMO_LIST_DEVICE_DEBUG_I("Successfully created rpmsg endpoint.\r\n");
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
                DEMO_LIST_DEVICE_DEBUG_I("*********************************************......\r\n");
                DEMO_LIST_DEVICE_DEBUG_I("***********Demo rpmsg_ping running***********......\r\n");
                DEMO_LIST_DEVICE_DEBUG_I("*********************************************......\r\n");
                ret = rpmsg_ping(rdev, priv);
                if (ret != 0)
                {
                    DEMO_LIST_DEVICE_DEBUG_E("rpmsg_echo running error,ecode:%d.", ret);
                    return 0;
                }
            }

            if (demo_flag == RPMSG_SAM_PING_DEMO)
            {
                printf("\r\n\r\n");
                DEMO_LIST_DEVICE_DEBUG_I("*********************************************......\r\n");
                DEMO_LIST_DEVICE_DEBUG_I("*******Demo rpmsg_sample_ping running********......\r\n");
                DEMO_LIST_DEVICE_DEBUG_I("*********************************************......\r\n");
                ret = rpmsg_sample_ping(rdev, priv);
                if (ret != 0)
                {
                    DEMO_LIST_DEVICE_DEBUG_E("rpmsg_sample_ping running error,ecode:%d.", ret);
                    return 0;
                }
            }

            if (demo_flag == RPMSG_MAT_MULT_DEMO)
            {
                printf("\r\n\r\n");
                DEMO_LIST_DEVICE_DEBUG_I("*********************************************......\r\n");
                DEMO_LIST_DEVICE_DEBUG_I("********Demo matrix_multiply running*********......\r\n");
                DEMO_LIST_DEVICE_DEBUG_I("*********************************************......\r\n");
                ret = matrix_multiply(rdev, priv);
                if (ret != 0)
                {
                    DEMO_LIST_DEVICE_DEBUG_E("matrix_multiply running error,ecode:%d.", ret);
                    return 0;
                }
            }

            if (demo_flag == RPMSG_NO_COPY_DEMO)
            {
                printf("\r\n\r\n");
                DEMO_LIST_DEVICE_DEBUG_I("*********************************************......\r\n");
                DEMO_LIST_DEVICE_DEBUG_I("*******Demo rpmsg_nocopy_ping running********......\r\n");
                DEMO_LIST_DEVICE_DEBUG_I("*********************************************......\r\n");
                ret = rpmsg_nocopy_ping(rdev, priv);
                if (ret != 0)
                {
                    DEMO_LIST_DEVICE_DEBUG_E("rpmsg_nocopy_ping running error,ecode:%d.", ret);
                    return 0;
                }
            }
            flag_req = 0;
            demo_flag = 0;
            DEMO_LIST_DEVICE_DEBUG_I(" Demo running over...");
        }
        if (flag_req == SHUTDOWN_MSG)
        {
            break;
        }
    }
    DEMO_LIST_DEVICE_DEBUG_I("Listening demo over.");
    return ret;
}

int FFreeRTOSOpenAMPSlaveInit(void)
{
    init_system();  /* Initialize the system resources and environment */
    
    if (!platform_create_proc(&remoteproc_device_00, &device_00_priv, &kick_driver_00)) 
    {
        DEMO_LIST_DEVICE_DEBUG_E("Failed to create remoteproc instance for device 00\r\n");
        return -1;  /* Return with an error if creation fails */
    }
    /* Setup resource tables for the created remoteproc instances */
    remoteproc_device_00.rsc_table = &resources;
    /* 在主核创建rpmsg之前，保证从核已经初始化资源表，否则会导致主核发送中断，从核进入中断服务函数后，获取异常指针资源表 */
    if (platform_setup_src_table(&remoteproc_device_00,remoteproc_device_00.rsc_table)) 
    {
        DEMO_LIST_DEVICE_DEBUG_E("Failed to setup src table for device 00\r\n");
        return -1;  /* Return with an error if setup fails */ 
    }
    
    DEMO_LIST_DEVICE_DEBUG_I("Setup resource tables for the created remoteproc instances is over \r\n");

    if (platform_setup_share_mems(&remoteproc_device_00)) 
    {
        DEMO_LIST_DEVICE_DEBUG_E("Failed to setup shared memory for device 00\r\n");
        return -1;  /* Return with an error if setup fails */ 
    }

    DEMO_LIST_DEVICE_DEBUG_I("Setup shared memory regions for both remoteproc instances is over \r\n");

    rpdev_device_00 = platform_create_rpmsg_vdev(&remoteproc_device_00, 0, VIRTIO_DEV_DEVICE, NULL, NULL);
    if (!rpdev_device_00) 
    {
        DEMO_LIST_DEVICE_DEBUG_E("Failed to create rpmsg vdev for device 00\r\n");
        return -1;  /* Return with an error if creation fails */ 
    }

    return 0 ;   
}

void rpmsg_listening_task(void *args)
{
    int ret = 0;
    while (!ret)
    {
        DEMO_LIST_DEVICE_DEBUG_I("DEVICE Starting application...\r\n");
        if(!FFreeRTOSOpenAMPSlaveInit())
        {
            ret = app(rpdev_device_00,&remoteproc_device_00) ;
        }
        else
        {
            DEMO_LIST_DEVICE_DEBUG_E("Failed to init remoteproc.\r\n");
            break;
        }
        platform_release_rpmsg_vdev(rpdev_device_00, &remoteproc_device_00);
        DEMO_LIST_DEVICE_DEBUG_I("Stopping application...\r\n");
        platform_cleanup(&remoteproc_device_00);
    }
    DEMO_LIST_DEVICE_DEBUG_E("DEVICE Exiting...\r\n");
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
        DEMO_LIST_DEVICE_DEBUG_E("Failed to create a rpmsg_listening_task task ");
        return -1;
    }
    return ret;
}