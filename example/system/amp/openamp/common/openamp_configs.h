/*
 * Copyright (C) 2024, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: openamp_configs.h
 * Created Date: 2024-05-06 19:20:51
 * Last Modified: 2025-06-19 11:15:38
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
#if defined(CONFIG_TARGET_PE2204)
#define MASTER_DRIVER_CORE              2 /* 与amp_config.json中的管理（主）核配置保持一致 */
#else
#define MASTER_DRIVER_CORE              0 /* 与amp_config.json中的管理（主）核配置保持一致 */
#endif

/* 主核发送消息时，需要指定接收的cpu的核号，用来确定软件中断的发送到哪个核上，以及主核指定启动镜像在那个核心上运行 */
#if defined(CONFIG_TARGET_PD2008)
#define SLAVE_DEVICE_CORE_00            4
#elif defined(CONFIG_TARGET_PE2204)
#define SLAVE_DEVICE_CORE_00            0
#else
#define SLAVE_DEVICE_CORE_00            1
#endif

#ifdef __cplusplus
}
#endif

#endif /* OPENAMP_CONFIGS_H */

