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
 * FilePath: openamp_configs.h
 * Created Date: 2024-05-06 19:20:51
 * Last Modified: 2024-12-02 19:51:28
 * Description:  This file is for
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 * 1.0     liusm   2024-05-17        first version
 */

#ifndef OPENAMP_CONFIGS_H
#define OPENAMP_CONFIGS_H

#include "fmmu.h"
#include "memory_layout.h"
#include "sdkconfig.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*记录当前系统中有多少个remoteproc设备*/
static u32 remoteproc_online_mask = 0 ;

#define MASTER_DRIVER_EPT_ADDR    (0x3 + 1024)
#define SLAVE_DEVICE_00_EPT_ADDR   (0x4 + 1024)

#define RPMSG_SERVICE_00_NAME          "rpmsg_service_name_00"

/* 从核发送消息时，需要指定发送的cpu的核号，用来确定软件中断的发送到哪个核上 */
#if defined(CONFIG_TARGET_E2000D) || defined(CONFIG_TARGET_D2000)
#define MASTER_DRIVER_CORE              0 /* 与amp_config.json中的管理（主）核配置保持一致 */
#else
#define MASTER_DRIVER_CORE              2 /* 与amp_config.json中的管理（主）核配置保持一致 */
#endif

/* 主核发送消息时，需要指定接收的cpu的核号，用来确定软件中断的发送到哪个核上，以及主核指定启动镜像在那个核心上运行 */
#if defined(CONFIG_TARGET_D2000)
#define SLAVE_DEVICE_CORE_00            4
#elif defined(CONFIG_TARGET_E2000D)
#define SLAVE_DEVICE_CORE_00            1
#else
#define SLAVE_DEVICE_CORE_00            0
#endif

#ifdef __cplusplus
}
#endif

#endif /* OPENAMP_CONFIGS_H */

