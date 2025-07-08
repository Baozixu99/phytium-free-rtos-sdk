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
 * FilePath: can_example.h
 * Date: 2022-08-25 16:22:40
 * LastEditTime: 2022-08-26 15:40:40
 * Description:  This file is for task create function define
 *
 * Modify History:
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 wangxiaodong 2022/09/23  first commit
 * 2.0 huangjin     2023/10/17  add function
 */


#ifndef CAN_EXAMPLE_H
#define CAN_EXAMPLE_H

#include "portmacro.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* can test */
BaseType_t FFreeRTOSCreateCanIntrTestTask(void);
BaseType_t FFreeRTOSCreateCanPolledTestTask(void);
BaseType_t FFreeRTOSCanCreateFilterTestTask(void);

#ifdef __cplusplus
}
#endif

#endif // !