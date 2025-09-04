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
/* device name for device 00 */
#define DEVICE_00_KICK_DEV_NAME "device_00_kick"
#define DEVICE_00_SGI            KICK_SGI_NUM_1

#ifdef __cplusplus
}
#endif

#endif /* LIBMETAL_CONFIGS_H */
