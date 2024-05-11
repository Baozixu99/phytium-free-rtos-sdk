/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc.
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
 * FilePath: canfd_example.h
 * Date: 2022-08-25 16:22:40
 * LastEditTime: 2022-08-26 15:40:40
 * Description:  This file is for task create function define
 *
 * Modify History:
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 huangjin     2024/04/25  first commit
 */


#ifndef CAN_EXAMPLE_H
#define CAN_EXAMPLE_H

#include "portmacro.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* canfd test */
BaseType_t FFreeRTOSCreateCanfdIntrTestTask(void);
BaseType_t FFreeRTOSCreateCanfdPolledTestTask(void);
BaseType_t FFreeRTOSCanfdCreateFilterTestTask(void);

#ifdef __cplusplus
}
#endif

#endif // !