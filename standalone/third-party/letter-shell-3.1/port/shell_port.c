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
 * FilePath: shell_port.c
 * Date: 2022-02-10 14:53:43
 * LastEditTime: 2022-02-25 11:47:29
 * Description:  This files is for letter shell port
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2021/9/6    init commit
 * 1.1   huanghe    2022/1/8    support shell interrupt
 */


#include "../src/shell.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "fassert.h"
#include "fparameters.h"
#include "finterrupt.h"
#include "fpl011.h"
#include "fpl011_hw.h"
#include "shell_port.h"

Shell shell_object;
u32 shell_last_result = 0;
char shell_buffer[4096];
extern void LSUserShellWrite(char data);
extern signed char LSUserShellRead(char *data);
extern void LSSerialConfig();
extern void LSSerialWaitLoop();
extern void LSSerialBNoWaitLoop();

/**
 * @brief 用户shell初始化
 *
 * Note: Call this function will lead to infinite loop
 */
void LSUserShellLoop(void)
{
    LSSerialConfig();
    shell_object.echo = 1;
    shell_object.write = LSUserShellWrite;
    shell_object.read = LSUserShellRead;
    shellInit(&shell_object, shell_buffer, sizeof(shell_buffer));

    LSSerialWaitLoop();
}

void LSUserShellInit(void)
{
    shell_object.echo = 1;
    LSSerialConfig();

    shell_object.write = LSUserShellWrite;
    shell_object.read = LSUserShellRead;
    shellInit(&shell_object, shell_buffer, sizeof(shell_buffer));
}

void LSuserShellNoWaitLoop(void)
{
    LSSerialBNoWaitLoop();
}

/**
 * @name: LSUserGetLastRet
 * @msg: 获取上一条命令的执行返回值
 * @return {*}
 */
int LSUserGetLastRet(void)
{
    return shell_object.lastRet;
}

/**
 * @name: 通过letter shell执行一行命令
 * @msg: 
 * @return {*}
 * @param {char} *cmd
 */
void LSUserExec(const char *cmd)
{
    LSUserPrintf("Exec: '%s'==========\r\n", cmd);
    shellRun(&shell_object, cmd);
    LSUserPrintf("================\r\n");
    return;
}

void LSUserSetResult(u32 result)
{
    shell_last_result = result;
}

u32 LSUserGetResult()
{
    return shell_last_result;
}

void LSUserSlient(boolean slient)
{
    shell_object.slient = slient ? 1 : 0;
}

boolean LSUserIsSlient(void)
{
    return (1 == shell_object.slient);
}

void LSUShowVersion(void)
{
    shellWriteVersion(&shell_object);
}
