/*
 * Copyright (C) 2022, Phytium Technology Co., Ltd.   All Rights Reserved.
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