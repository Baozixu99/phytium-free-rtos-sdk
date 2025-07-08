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
 * FilePath: feature_resource.h
 * Date: 2022-06-17 10:42:40
 * LastEditTime: 2024-05-07 10:42:40
 * Description:  This file is for task function define
 *
 * Modify History:
 *  Ver   Who  Date   Changes
 * ----- ------  -------- --------------------------------------
 * 1.0 wangxiaodong 2022/08/09  first commit
 * 1.1 huangjin     2024/05/07  add no letter shell mode, adapt to auto-test system
 */

#ifndef FEATURE_RESOURCE_H
#define FEATURE_RESOURCE_H

#ifdef __cplusplus
extern "C"
{
#endif

/* mutex task */
int CreateResourceTasks(void);
/* gatekeeper task */
int CreateGatekeeperTasks(void);

#ifdef __cplusplus
}
#endif

#endif // !