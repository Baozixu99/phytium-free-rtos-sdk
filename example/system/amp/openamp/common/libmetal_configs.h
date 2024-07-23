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
 * FilePath: libmetal_configs.h
 * Created Date: 2024-05-17 10:18:19
 * Last Modified: 2024-07-15 18:44:39
 * Description:  This file is for
 * 
 * Modify History:
 *   Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 *  1.0     liusm   2024-05-17        first version
 */

#ifndef LIBMETAL_CONFIGS_H
#define LIBMETAL_CONFIGS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <metal/sys.h>
#include <metal/device.h>
#include <metal/irq.h>

#define KICK_BUS_NAME         "generic"
#define KICK_SGI_NUM_1            1
#define KICK_SGI_NUM_2            2
#define KICK_SGI_NUM_3            3
#define KICK_SGI_NUM_4            4
/* device name for slave 00 */
#define SLAVE_00_KICK_DEV_NAME "slave_00_kick"
#define SLAVE_00_SGI            KICK_SGI_NUM_1

#ifdef __cplusplus
}
#endif

#endif /* LIBMETAL_CONFIGS_H */
