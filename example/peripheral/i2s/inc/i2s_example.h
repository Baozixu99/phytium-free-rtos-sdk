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
 * FilePath: i2s_example.h
 * Date: 2024-02-29 13:22:40
 * LastEditTime: 2024-02-29 15:40:40
 * Description:  This file is for task create function define
 *
 * Modify History:
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   Wangzq       2024/02/29  first commit
 */


#ifndef I2S_EXAMPLE_H
#define I2S_EXAMPLE_H

#include "fi2s_os.h"
#ifdef __cplusplus
extern "C"
{
#endif
/*init i2s task*/
BaseType_t FFreeRTOSI2sInitCreate(void);

/*deinit i2s task*/
BaseType_t FFreeRTOSI2sDeInitCreate(void);

#ifdef __cplusplus
}
#endif
#endif // !