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