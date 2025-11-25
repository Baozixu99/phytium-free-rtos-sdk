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
 * FilePath: hyperamp_cmd.c
 * Description: Shell commands for HyperAMP server control
 */

#include <stdio.h>
#include "shell.h"
#include "hyperamp_server.h"

/* Shell 命令: 获取 HyperAMP 状态 */
static int HyperAmpStatusCmd(int argc, char *argv[])
{
    HyperAmpServerGetStatus();
    return 0;
}

/* Shell 命令: 停止 HyperAMP 服务端 */
static int HyperAmpStopCmd(int argc, char *argv[])
{
    HyperAmpServerStop();
    return 0;
}

/* Shell 命令: 启动 HyperAMP 服务端 */
static int HyperAmpStartCmd(int argc, char *argv[])
{
    if (HyperAmpServerCreateTask() == 0) {
        printf("[HyperAMP] Server started\r\n");
    } else {
        printf("[HyperAMP] Failed to start server\r\n");
    }
    return 0;
}

/* 注册 Shell 命令 */
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                 hyperamp_status, HyperAmpStatusCmd, Get HyperAMP server status);

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                 hyperamp_stop, HyperAmpStopCmd, Stop HyperAMP server);

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                 hyperamp_start, HyperAmpStartCmd, Start HyperAMP server);
