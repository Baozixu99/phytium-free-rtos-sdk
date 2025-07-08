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
 * FilePath: shell_port.h
 * Date: 2022-02-24 22:03:34
 * LastEditTime: 2022-02-24 22:03:34
 * Description:  This file is for the shell port related functions definition
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe    2022/3/25   first release
 */


#ifndef SHELL_PORT_H
#define SHELL_PORT_H

#include "shell.h"
#include "ftypes.h"
#include "FreeRTOS.h"
#include "task.h"

#ifdef __cplusplus
extern "C"
{
#endif

extern Shell shell;
BaseType_t LSUserShellTask(void);
void LSUserExec(const char *cmd);
int LSUserGetLastRet(void);
void LSUserSetResult(u32 result);
u32 LSUserGetResult(void);
void LSUserSlient(boolean slient);
boolean LSUserIsSlient(void);
void LSUShowVersion(void);
#define LSUserPrintf(format, ...)      \
    if (!LSUserIsSlient())             \
    {                                  \
        printf(format, ##__VA_ARGS__); \
    }

#ifdef __cplusplus
}
#endif

#endif
