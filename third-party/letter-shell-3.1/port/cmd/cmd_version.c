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
 * FilePath: cmd_version.c
 * Date: 2022-02-24 18:24:53
 * LastEditTime: 2022-03-21 17:04:14
 * Description:  This file is for the version command functions
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe    2022/3/25   first release
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../src/shell.h"
#include "../shell_port.h"


static int VersionCmdEntry(int argc, char *argv[])
{
    LSUShowVersion();
    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), version,
                 VersionCmdEntry, show version of cur shell);