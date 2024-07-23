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
 * @FilePath: rpmsg-demo-manager_cmd.c
 * @Date: 2023-04-23 16:02:34
 * @LastEditTime: 2024-03-04 19:45:19
 * @Description:  This file is for manager rpmsg demos
 * 
 * @Modify History: 
 *  Ver   Who           Date        Changes
 * ----- ------         --------    --------------------------------------
 * 1.0  liushengming    2023/04/23  first release
 * 1.1  liushengming    2023/05/31  add loadelf
 */

#include <stdio.h>
#include <openamp/version.h>
#include <openamp/open_amp.h>
#include <metal/version.h>
#include <metal/alloc.h>
#include <string.h>
#include "strto.h"

#include "platform_info.h"
#include "rpmsg_service.h"
#include "rsc_table.h"
#include "helper.h"
#include "load_fw.h"
#include "fparameters.h"
#include "fdebug.h"
#include "felf.h"
#include "sdkconfig.h"
#include "libmetal_configs.h"
#include "openamp_configs.h"
#include "fsleep.h"

#include "FreeRTOS.h"
#include "task.h"
#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
#endif
/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define     DEMO_MANG_MASTER_DEBUG_TAG "DEMO_MANG_MASTER"
#define     DEMO_MANG_MASTER_DEBUG_I(format, ...) FT_DEBUG_PRINT_I( DEMO_MANG_MASTER_DEBUG_TAG, format, ##__VA_ARGS__)
#define     DEMO_MANG_MASTER_DEBUG_W(format, ...) FT_DEBUG_PRINT_W( DEMO_MANG_MASTER_DEBUG_TAG, format, ##__VA_ARGS__)
#define     DEMO_MANG_MASTER_DEBUG_E(format, ...) FT_DEBUG_PRINT_E( DEMO_MANG_MASTER_DEBUG_TAG, format, ##__VA_ARGS__)

/**************************** Type Definitions *******************************/

#define MANAGE_READ 1
#define MANAGE_WAIT 0

#define RPMSG_PING_DEMO     0x1
#define RPMSG_SAM_PING_DEMO 0X2
#define RPMSG_MAT_MULT_DEMO 0X3
#define RPMSG_NO_COPY_DEMO  0X4
#define RPMSG_DEMO_MAX      0x5

/************************** Variable Definitions *****************************/
struct metal_device kick_device_00 = {
    .name = SLAVE_00_KICK_DEV_NAME,
	.bus = NULL,
    .irq_num = 1,
	.irq_info = (void *)SLAVE_00_SGI,
} ;

struct remoteproc_priv slave_00_priv = {
    .kick_dev_name =           SLAVE_00_KICK_DEV_NAME  ,
	.kick_dev_bus_name =        KICK_BUS_NAME ,
    .cpu_id        = SLAVE_DEVICE_CORE_00 ,/* 设置CPU ID */

	.src_table_attribute = SLAVE00_SOURCE_TABLE_ATTRIBUTE ,
	
	/* |rx vring|tx vring|share buffer| */
	.share_mem_va = SLAVE00_SHARE_MEM_ADDR ,
	.share_mem_pa = SLAVE00_SHARE_MEM_ADDR ,
	.share_buffer_offset =  SLAVE00_VRING_SIZE ,
	.share_mem_size = SLAVE00_SHARE_MEM_SIZE ,
	.share_mem_attribute = SLAVE00_SHARE_BUFFER_ATTRIBUTE ,
} ;
/*OpenAMP 相关定义*/
struct remoteproc remoteproc_slave_00 ;
static struct rpmsg_device *rpdev_slave_00 = NULL;
static struct rpmsg_endpoint ept_master_slave_00;
/* 标志相关定义 */
static int volatile cmd_ok = MANAGE_WAIT;
static u32 demo_flag = RPMSG_PING_DEMO;
static u32 elf_boot_flag = 0;
/* 镜像相关定义 */
extern const struct image_store_ops mem_image_store_ops;
static struct mem_file image[FCORE_NUM] = {(void *)0};
extern  void * amp_img_start;
extern  void * amp_img_end;
/************************** Function Prototypes ******************************/
extern int rpmsg_echo(struct rpmsg_device *rdev, void *priv);
extern int rpmsg_sample_echo(struct rpmsg_device *rdev, void *priv) ;
extern int matrix_multiplyd(struct rpmsg_device *rdev, void *priv) ;
extern int rpmsg_nocopy_echo(struct rpmsg_device *rdev, void *priv) ;
static int FOpenampClose(void *platform);

/*-----------------------------------------------------------------------------*
 *  RPMSG endpoint callbacks
 *-----------------------------------------------------------------------------*/
static int rpmsg_endpoint_cb(struct rpmsg_endpoint *ept, void *data, size_t len,
                             uint32_t src, void *priv)
{
    u32 i = *(u32 *)data;

    (void)ept;
    (void)src;
    (void)priv;
    DEMO_MANG_MASTER_DEBUG_E("src:0x%x",src);
    /* 通过'src'可以筛选自己想要的数据来源 */

    if (i != demo_flag)
    {
        DEMO_MANG_MASTER_DEBUG_I("Data corruption at index %d.\r\n", i);
        DEMO_MANG_MASTER_DEBUG_I("Want data is %d.\r\n", demo_flag);
        return RPMSG_ERROR_BASE;
    }
    cmd_ok = MANAGE_READ;
    return RPMSG_SUCCESS;
}

static void rpmsg_service_unbind(struct rpmsg_endpoint *ept)
{
    (void)ept;
    rpmsg_destroy_ept(&ept_master_slave_00);
    DEMO_MANG_MASTER_DEBUG_E("Echo test: service is destroyed.\r\n");
}

static void rpmsg_name_service_bind_cb(struct rpmsg_device *rdev,
                                       const char *name, uint32_t dest)
{
    DEMO_MANG_MASTER_DEBUG_I("New endpoint notification is received.\r\n");
    if (strcmp(name, RPMSG_SERVICE_00_NAME))
    {
        DEMO_MANG_MASTER_DEBUG_E("Unexpected name service %s.\r\n", name);
    }
    else
        (void)rpmsg_create_ept(&ept_master_slave_00, rdev, RPMSG_SERVICE_00_NAME,
                               MASTER_DRIVER_EPT_ADDR, dest,
                               rpmsg_endpoint_cb,
                               rpmsg_service_unbind);
}

/*-----------------------------------------------------------------------------*
 *  Application
 *-----------------------------------------------------------------------------*/
static int FManageEptCreat(struct rpmsg_device *rdev, void *priv)
{
    int ret = 0;

    /* Create RPMsg endpoint */
    ret = rpmsg_create_ept(&ept_master_slave_00, rdev, RPMSG_SERVICE_00_NAME,
                            MASTER_DRIVER_EPT_ADDR, SLAVE_DEVICE_00_EPT_ADDR,
                            rpmsg_endpoint_cb, rpmsg_service_unbind);
    if (ret)
    {
        DEMO_MANG_MASTER_DEBUG_E("Failed to create endpoint. %d \r\n", ret);
        return -1;
    }
    while (!is_rpmsg_ept_ready(&ept_master_slave_00))
    {
        DEMO_MANG_MASTER_DEBUG_I("start to wait platform_poll \r\n");
        platform_poll(priv);
    }
    DEMO_MANG_MASTER_DEBUG_I("Manage rpmsg_ept_ready!!!");
    return ret;
}

int FRunningApp(struct rpmsg_device *rdev, void *priv)
{
    int ret = 0;

    ret = rpmsg_send(&ept_master_slave_00, &demo_flag, sizeof(u32));
    if (ret < 0)
    {
        DEMO_MANG_MASTER_DEBUG_E("Failed to send data,ret:%d...\r\n",ret);
        return ret;
    }
    cmd_ok = MANAGE_WAIT;
    do
    {
        platform_poll(priv);
    }
    while ( cmd_ok == MANAGE_WAIT );

    if (demo_flag != 0 && cmd_ok == MANAGE_READ)
    {
        if (demo_flag == RPMSG_PING_DEMO)
        {
            DEMO_MANG_MASTER_DEBUG_I("*********************************************......\r\n") ;
            DEMO_MANG_MASTER_DEBUG_I("***********Demo rpmsg_echo running***********......\r\n") ;
            DEMO_MANG_MASTER_DEBUG_I("*********************************************......\r\n") ;
            ret = rpmsg_echo(rdev, priv);
            if (ret != 0)
            {
                DEMO_MANG_MASTER_DEBUG_E("rpmsg_echo running error,ecode:%d.",ret);
                return ret;
            }
        }

        if (demo_flag == RPMSG_SAM_PING_DEMO)
        {
            printf("\r\n\r\n");
            DEMO_MANG_MASTER_DEBUG_I("*********************************************......\r\n");
            DEMO_MANG_MASTER_DEBUG_I("*******Demo rpmsg_sample_echo running********......\r\n") ;
            DEMO_MANG_MASTER_DEBUG_I("*********************************************......\r\n") ;
            ret = rpmsg_sample_echo(rdev, priv);
            if (ret != 0)
            {
                DEMO_MANG_MASTER_DEBUG_E("rpmsg_sample_echo running error,ecode:%d.",ret);
                return ret;
            }
        }

        if (demo_flag == RPMSG_MAT_MULT_DEMO)
        {
            printf("\r\n\r\n");
            DEMO_MANG_MASTER_DEBUG_I("*********************************************......\r\n") ;
            DEMO_MANG_MASTER_DEBUG_I("*******Demo matrix_multiplyd running********......\r\n") ;
            DEMO_MANG_MASTER_DEBUG_I("*********************************************......\r\n") ;
            ret = matrix_multiplyd(rdev, priv);
            if (ret != 0)
            {
                DEMO_MANG_MASTER_DEBUG_E("matrix_multiplyd running error,ecode:%d.",ret);
                return ret;
            }
        }
        
        if (demo_flag == RPMSG_NO_COPY_DEMO)
        {
            printf("\r\n\r\n");
            DEMO_MANG_MASTER_DEBUG_I("*********************************************......\r\n") ;
            DEMO_MANG_MASTER_DEBUG_I("*******Demo rpmsg_nocopy_echo running********......\r\n") ;
            DEMO_MANG_MASTER_DEBUG_I("*********************************************......\r\n") ;
            ret = rpmsg_nocopy_echo(rdev, priv);
            if (ret != 0)
            {
                DEMO_MANG_MASTER_DEBUG_E("rpmsg_nocopy_echo running error,ecode:%d.",ret);
                return ret;
            }
        }
        cmd_ok = MANAGE_WAIT;
        DEMO_MANG_MASTER_DEBUG_I(" Demo %d running over...",demo_flag);
        demo_flag++;
        return ret;
    }
}

/*-----------------------------------------------------------------------------*
 *  Application entry point
 *-----------------------------------------------------------------------------*/
static int FOpenampClose(void *platform)
{
    int ret = 0;
    struct remoteproc *rproc = platform;
    if (rproc == NULL)
        return -1;
    if (rpdev_slave_00 == NULL)
        return -1;
    
    rpmsg_destroy_ept(&ept_master_slave_00);
    
    platform_release_rpmsg_vdev(rpdev_slave_00, platform);

    ret = remoteproc_shutdown(rproc);
    if (ret != 0)
    {
        DEMO_MANG_MASTER_DEBUG_E("Can't shutdown remoteproc,error code:0x%x.",ret);
    }

    ret = platform_cleanup(platform);
    if (ret != 0)
    {
        DEMO_MANG_MASTER_DEBUG_E("Can't remove platform,error code:0x%x.",ret);
    }
    
    return ret;
}

static void * CheckElfStartAddress(void * address)
{
    if(ElfIsImageValid((unsigned long)address))
    {
        return address;
    }
    return NULL;
}


int FOpenampExample(void)
{
    int ret = 0;
    demo_flag = RPMSG_PING_DEMO;
    printf("amp_img_start is 0x%x \r\n",&amp_img_start);
    printf("amp_img_end is 0x%x \r\n",&amp_img_end);
    if (elf_boot_flag == 0)
    {
        void *temp_address = &amp_img_start;
        u32 i = 0;
        while (temp_address < (void *)&amp_img_end)
        {
            void * boot_elf_address = CheckElfStartAddress((void *)temp_address);
            if(boot_elf_address != NULL)
            {
                image[i].base = temp_address;
                i++ ;
            }
            temp_address++;
        }
        for (u32 j = 0;  j< i; j++)
        {
            printf("boot_elf_address is 0x%x. \r\n",image[j].base);
        }
        
        init_system();  /* Initialize the system resources and environment */ 
        /* Initialize remoteproc */
        if (!platform_create_proc(&remoteproc_slave_00, &slave_00_priv, &kick_device_00)) 
        {
            DEMO_MANG_MASTER_DEBUG_E("Failed to create remoteproc instance for slave 00\r\n");
            platform_cleanup(&remoteproc_slave_00);
            return -1;
        }
        /*加载镜像，并启动，镜像的序号对应为 amp_config.json中 一个配置项 的索引位置*/
        if(load_exectuable_block(&remoteproc_slave_00, &mem_image_store_ops, &image[SLAVE00_IMAGE_NUM].base, NULL))
        {
            return -1;
        }
        /* Setup resource tables for the created remoteproc instances*/
        if (platform_setup_src_table(&remoteproc_slave_00, remoteproc_slave_00.rsc_table)) 
        {
            DEMO_MANG_MASTER_DEBUG_E("Failed to setup src table for slave 00\r\n");
            return -1;
        }
        DEMO_MANG_MASTER_DEBUG_I("Setup resource tables for the created remoteproc instances is over \r\n");
        /* Setup shared memory regions for both remoteproc instances */ 
        if (platform_setup_share_mems(&remoteproc_slave_00)) 
        {
            DEMO_MANG_MASTER_DEBUG_E("Failed to setup shared memory for slave 00\r\n");
            return -1;
        }
        DEMO_MANG_MASTER_DEBUG_I("Setup shared memory regions for both remoteproc instances is over \r\n");
        /* Create rpmsg virtual devices for communication */ 
        rpdev_slave_00 = platform_create_rpmsg_vdev(&remoteproc_slave_00, 0, VIRTIO_DEV_MASTER, NULL, rpmsg_name_service_bind_cb);
        if (!rpdev_slave_00) 
        {
            DEMO_MANG_MASTER_DEBUG_E("Failed to create rpmsg vdev for slave 00\r\n");
            platform_cleanup(&remoteproc_slave_00);
            return -1;
        }
        DEMO_MANG_MASTER_DEBUG_I("Create rpmsg virtual devices for communication \r\n");
        ret = FManageEptCreat(rpdev_slave_00, &remoteproc_slave_00);
        if (ret)
        {
            return FOpenampClose(&remoteproc_slave_00);
        }
        elf_boot_flag = 1;
        fsleep_millisec(1000);/*等待slave 00 启动完成，并完成初始化任务调度*/
    }
    if (elf_boot_flag == 1)
    {
        while (demo_flag < RPMSG_DEMO_MAX)
        {
            ret = FRunningApp(rpdev_slave_00, &remoteproc_slave_00);
            if (ret)
            {
                return FOpenampClose(&remoteproc_slave_00);
            }
        }
    }
    vTaskDelay(pdMS_TO_TICKS(100));
    DEMO_MANG_MASTER_DEBUG_I("Stopping application...\r\n");
    if (ret != 0 && elf_boot_flag == 1 && demo_flag == (RPMSG_DEMO_MAX-1))
    {
        printf("%s@%d: openamp example [failure] !!! \r\n", __func__, __LINE__);
    }
    else
    {
        printf("%s@%d: openamp example [success] !!! \r\n", __func__, __LINE__);
    }
    return ret;
}

/*-----------------------------------------------------------------------------*
 *  Application entry point
 *-----------------------------------------------------------------------------*/
void RpmsgEchoTask( void * args )
{
	int ret;

    DEMO_MANG_MASTER_DEBUG_I("complier %s ,%s \r\n", __DATE__, __TIME__);
	DEMO_MANG_MASTER_DEBUG_I("openamp lib version: %s (", openamp_version());
	DEMO_MANG_MASTER_DEBUG_I("Major: %d, ", openamp_version_major());
	DEMO_MANG_MASTER_DEBUG_I("Minor: %d, ", openamp_version_minor());
	DEMO_MANG_MASTER_DEBUG_I("Patch: %d)\r\n", openamp_version_patch());

	DEMO_MANG_MASTER_DEBUG_I("libmetal lib version: %s (", metal_ver());
	DEMO_MANG_MASTER_DEBUG_I("Major: %d, ", metal_ver_major());
	DEMO_MANG_MASTER_DEBUG_I("Minor: %d, ", metal_ver_minor());
	DEMO_MANG_MASTER_DEBUG_I("Patch: %d)\r\n", metal_ver_patch());

	/* Initialize platform */
	DEMO_MANG_MASTER_DEBUG_I("start application");
	ret = FOpenampExample();
	if (ret)
	{
		DEMO_MANG_MASTER_DEBUG_E("Failed to running example.\r\n");
	}
    vTaskDelete(NULL);
}

BaseType_t FFreeRTOSOpenampExample(void)
{
    BaseType_t xReturn = pdPASS;

    xReturn = xTaskCreate((TaskFunction_t )RpmsgEchoTask, /* 任务入口函数 */
                        (const char* )"RpmsgEchoTask",/* 任务名字 */
                        (uint16_t )(4096*2), /* 任务栈大小 */
                        (void* )NULL,/* 任务入口函数参数 */
                        (UBaseType_t )4, /* 任务的优先级 */
                        NULL); /* 任务控制块指针 */
    
    if(xReturn != pdPASS)
    {
        DEMO_MANG_MASTER_DEBUG_E("Failed to create a RpmsgEchoTask task ");
        return -1;
    }
    return xReturn;
}

#ifdef CONFIG_USE_LETTER_SHELL
static void FOpenampCmdUsage()
{
    printf("Usage:\r\n");
    printf("openamp auto \r\n");
    printf("-- Auto running.\r\n");
    printf("-- [1] This application echoes back data that was sent to it by the master core.\r\n");
    printf("-- [2] This application simulate sample rpmsg driver. For this it echo 100 time message sent by the rpmsg sample client available in distribution.\r\n");
    printf("-- [3] This application receives two matrices from the master, multiplies them and returns the result to the master core.\r\n");
    printf("-- [4] This application echoes back data that was sent to it by the master core.\r\n");
}

BaseType_t FOpenampCmdEntry(int argc, char *argv[])
{
    BaseType_t ret = 0;

    if (!strcmp(argv[1], "auto"))
    {
        ret = FFreeRTOSOpenampExample();
    }
    else
    {
        FOpenampCmdUsage();
        return -1;
    }

    return ret;
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), openamp, FOpenampCmdEntry, test freertos openamp);
#endif
