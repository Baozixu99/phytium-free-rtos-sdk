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
 * Date: 2022-02-24 22:03:27
 * LastEditTime: 2022-02-24 22:03:28
 * Description:  This file is for the shell port related functions
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe    2022/3/25   first release
 */


#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "fassert.h"
#include "shell_port.h"
#include "shell.h"
#include "FreeRTOS.h"
#include "task.h"


Shell shell_object;
u32 shell_last_result = 0;
char shell_buffer[4096];
extern void LSUserShellWrite(char data);
extern signed char LSUserShellRead(char *data);

#if defined(CONFIG_LETTER_SHELL_UART_INTERRUPT_MODE) || defined(CONFIG_LETTER_SHELL_UART_POLLED_MODE)
extern void LSSerialConfig();
extern void LSSerialWaitLoop();
#endif

#if defined (CONFIG_LETTER_SHELL_UART_MSG_INTR_MODE) || defined (CONFIG_LETTER_SHELL_UART_MSG_POLLED_MODE)
extern void LSSerialMsgConfig();
extern void LSSerialMsgWaitLoop();
#endif


void LSUserShellTaskCreate(void *args)
{
    BaseType_t ret;
#if defined(CONFIG_LETTER_SHELL_UART_INTERRUPT_MODE) || defined(CONFIG_LETTER_SHELL_UART_POLLED_MODE)
    LSSerialConfig();
#endif

#if defined(CONFIG_LETTER_SHELL_UART_MSG_INTR_MODE) || defined(CONFIG_LETTER_SHELL_UART_MSG_POLLED_MODE)
    LSSerialMsgConfig();
#endif

    shell_object.echo = 1;
    shell_object.write = LSUserShellWrite;
    shell_object.read = LSUserShellRead;
    shellInit(&shell_object, shell_buffer, 4096);

#if defined(CONFIG_LETTER_SHELL_UART_INTERRUPT_MODE) || defined(CONFIG_LETTER_SHELL_UART_POLLED_MODE)
    ret = xTaskCreate((TaskFunction_t)LSSerialWaitLoop,  /* 任务入口函数 */
                      (const char *)"LSSerialWaitLoop",/* 任务名字 */
                      1024,  /* 任务栈大小 */
                      (void *)NULL,/* 任务入口函数参数 */
                      (UBaseType_t)2,  /* 任务的优先级 */
                      NULL); /* 任务控制块指针 */
#endif

#if defined(CONFIG_LETTER_SHELL_UART_MSG_INTR_MODE) || defined(CONFIG_LETTER_SHELL_UART_MSG_POLLED_MODE)
    ret = xTaskCreate((TaskFunction_t)LSSerialMsgWaitLoop,  /* 任务入口函数 */
                      (const char *)"LSSerialMsgWaitLoop",/* 任务名字 */
                      1024,  /* 任务栈大小 */
                      (void *)NULL,/* 任务入口函数参数 */
                      (UBaseType_t)2,  /* 任务的优先级 */
                      NULL); /* 任务控制块指针 */

#endif

    FASSERT_MSG(ret == pdPASS, "LSUserShellTask create is failed");

    vTaskDelete(NULL);
}

/**
 * @brief 用户shell初始化
 *
 * Note: Call this function will lead to infinite create freertos task
 */
BaseType_t LSUserShellTask(void)
{
    return xTaskCreate((TaskFunction_t)LSUserShellTaskCreate,  /* 任务入口函数 */
                       (const char *)"LSUserShellTaskCreate",/* 任务名字 */
                       1024,  /* 任务栈大小 */
                       (void *)NULL,/* 任务入口函数参数 */
                       (UBaseType_t)2,  /* 任务的优先级 */
                       NULL); /* 任务控制块指针 */
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
    LSUserPrintf("exec: '%s'==========\r\n", cmd);
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