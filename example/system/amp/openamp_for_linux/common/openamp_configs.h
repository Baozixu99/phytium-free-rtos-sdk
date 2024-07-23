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
 * FilePath: memory_attr.h
 * Created Date: 2024-05-06 19:20:51
 * Last Modified: 2024-07-22 17:02:32
 * Description:  This file is for
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 *  1.0     liusm   2024-05-17        first version
 */

#ifndef OPENAMP_CONFIGS_H
#define OPENAMP_CONFIGS_H

#include "fmmu.h"
#include "memory_layout.h"

#if defined __cplusplus
extern "C" {
#endif

/* 从核发送消息时，需要指定发送的cpu的核号，用来确定软件中断的发送到哪个核上 */

#define MASTER_CORE_MASK              255 /* 采用协商好的方式，给所有核心都发送中断，注意：openamp 的freeRTOS 接口层PhytiumProcNotify()做了区分 */

#if defined __cplusplus
}
#endif

#endif /* OPENAMP_CONFIGS_H */
