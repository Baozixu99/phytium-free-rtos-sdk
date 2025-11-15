/*
 * Copyright (C) 2023, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * @FilePath: fboard_init.h
 * @Date: 2023-07-24 09:51:48
 * @LastEditTime: 2023-07-24 09:51:48
 * @Description:  This file is for io and status init function definition
 * 
 * @Modify History: 
 *  Ver   Who       Date        Changes
 * ----- ------  --------       --------------------------------------
 * 1.0   liusm   2023/07/24     first release
 */

#ifndef FBOARD_INIT_H
#define FBOARD_INIT_H

#include "fboard_port.h"

#ifdef __cplusplus
extern "C"
{
#endif

int FBoardStatusInit(void *para);

#ifdef __cplusplus
}
#endif

#endif
