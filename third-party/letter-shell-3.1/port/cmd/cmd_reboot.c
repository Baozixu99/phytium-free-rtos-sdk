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
 * FilePath: cmd_reboot.c
 * Date: 2022-02-24 18:24:53
 * LastEditTime: 2022-03-21 17:04:01
 * Description:  This file is for the reboot command functions
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe    2022/3/25   first release
 */

#include "../src/shell.h"
#include "ftypes.h"
#include "fpsci.h"

int RebootCmdEntry()
{
    FPsciSystemReset(FPSCI_SYSTEM_RESET_TYPE_COLD);
    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), reboot,
                 RebootCmdEntry, reboot board by psci);