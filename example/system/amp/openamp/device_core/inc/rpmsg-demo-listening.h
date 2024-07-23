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
 * FilePath: rpmsg-demo-listening.h
 * Created Date: 2024-07-19 10:26:11
 * Last Modified: 2024-07-19 10:31:27
 * Description:  This file is for
 * 
 * Modify History:
 *   Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 */

#ifndef RPMSG_DEMO_LISTENING_H_
#define RPMSG_DEMO_LISTENING_H_

#include "FreeRTOS.h"
#include "ftypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

int FFreeRTOSOpenAMPSlaveInit(void);
int rpmsg_listening_func(void);

#ifdef __cplusplus
}
#endif

#endif /* RPMSG_DEMO_LISTENING_H_ */
