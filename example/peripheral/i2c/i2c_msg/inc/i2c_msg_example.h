/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: i2c_msg_example.h
 * Date: 2025-04-21  14:41:23
 * LastEditTime: 2025-04-21  14:41:23
 * Description:  This file is for i2c msg test example function declarations.
 *
 * Modify History:
 *  Ver       Who            Date              Changes
 * -----    ------         --------          --------------------------------------
 *  1.0    zhangyan        2025/04/21         init commit
 */
#ifndef I2C_MSG_EXAMPLE_H
#define I2C_MSG_EXAMPLE_H

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
BaseType_t FFreeRTOSI2cMsgWriteThenReadCreate(void);

#ifdef __cplusplus
}
#endif

#endif