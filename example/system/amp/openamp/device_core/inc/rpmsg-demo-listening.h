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
