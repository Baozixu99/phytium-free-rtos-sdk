/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc.
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
 * FilePath: i2c_example.h
 * Date: 2022-07-18 14:41:23
 * LastEditTime: 2022-07-18 14:41:23
 * Description:  This file is for i2c test example function declarations.
 *
 * Modify History:
 *  Ver       Who            Date                 Changes
 * -----    ------         --------     --------------------------------------
 *  1.0    liushengming   2022/11/25             init commit
 *  1.1   zhangyan       2024/4/18     add no letter shell mode, adapt to auto-test system
 */
#ifndef I2C_EXAMPLE_H
#define I2C_EXAMPLE_H

#include "FreeRTOS.h"

#ifdef __cplusplus
extern "C"
{
#endif
/***************************** Include Files *********************************/

/************************** Definitions *****************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
/* i2c write and read test */
BaseType_t FFreeRTOSI2cRtcCreate(void);
BaseType_t FFreeRTOSI2cLoopbackCreate(void);

#ifdef __cplusplus
}
#endif

#endif