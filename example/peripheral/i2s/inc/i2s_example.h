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

#include "FreeRTOS.h"

#ifdef __cplusplus
extern "C"
{
#endif
/*init i2s task*/
BaseType_t FFreeRTOSRunI2sExample(void);

#ifdef __cplusplus
}
#endif
#endif // !