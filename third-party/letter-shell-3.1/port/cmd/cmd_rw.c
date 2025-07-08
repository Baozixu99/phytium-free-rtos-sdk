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
 * FilePath: cmd_rw.c
 * Date: 2022-02-24 18:24:53
 * LastEditTime: 2022-03-21 17:04:05
 * Description:  This file is for the rw command functions
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
#include "fio.h"
#include "ftypes.h"
#include "fdebug.h"

static int RwWriteRegister(u32 reg_addr, u32 reg_val, u32 bit_width)
{
    if (bit_width > 4)
    {
        return -1;
    }

    if (4 == bit_width)
    {
        FtOut32((uintptr)reg_addr, reg_val);
    }
    else if (2 == bit_width)
    {
        FtOut16((uintptr)reg_addr, (u16)reg_val);
    }
    else if (1 == bit_width)
    {
        FtOut8((uintptr)reg_addr, (u8)reg_val);
    }

    return 0;
}

static int RwReadRegister(u32 reg_addr, u32 *reg_val_p, u32 bit_width, u32 len)
{
    if ((bit_width > 4) || (NULL == reg_val_p))
    {
        return -1;
    }

    if (4 == bit_width)
    {
        *reg_val_p = FtIn32(reg_addr);
    }
    else if (2 == bit_width)
    {
        *reg_val_p = FtIn16(reg_addr);
    }
    else if (1 == bit_width)
    {
        *reg_val_p = FtIn8(reg_addr);
    }

    if (1 < len)
    {
        FtDumpHexByte((u8 *)(uintptr)reg_addr, len);
    }

    return 0;
}

static void RwCmdUsage()
{
    printf("usage:\r\n");
    printf("    rw [-w|-r] address register_val \r\n");
    printf("    rw -w [bit width] [address] [value]: write bits to address with value\r\n");
    printf("	rw -r [bit width] [address] [cnt=1]: read bits from address and print, by default read 1 bits\r\n");
}

static int RwCmdEntry(int argc, char *argv[])
{
    int ret = 0;
    u32 reg_addr = 0;
    u32 reg_val = 0;
    u32 bit_width = 0;
    u32 read_cnt = 1;

    if (argc < 4)
    {
        RwCmdUsage();
        return -1;
    }

    if (!strcmp(argv[1], "-w"))
    {
        if (argc < 5)
        {
            RwCmdUsage();
            return -2;
        }

        bit_width = strtoul(argv[2], NULL, 0);
        reg_addr = strtoul(argv[3], NULL, 0);
        reg_val = strtoul(argv[4], NULL, 0);
        ret = RwWriteRegister(reg_addr, reg_val, bit_width);

        if (0 != ret)
        {
            printf("Write failed: %d.\r\n", ret);
        }
        else
        {
            LSUserPrintf("Set 0x%x = 0x%x.\r\n", reg_addr, reg_val);
        }
    }
    else if (!strcmp(argv[1], "-r"))
    {
        if (argc < 4)
        {
            RwCmdUsage();
            return -3;
        }

        bit_width = strtoul(argv[2], NULL, 0);
        reg_addr = strtoul(argv[3], NULL, 0);
        reg_val = 0;

        if (argc = 4)
        {
            read_cnt = strtoul(argv[4], NULL, 0);
        }

        ret = RwReadRegister(reg_addr, &reg_val, bit_width, read_cnt);

        if (0 != ret)
        {
            printf("Read failed: %d.\r\n", ret);
        }
        else
        {
            LSUserPrintf("Get 0x%x = 0x%x.\r\n", reg_addr, reg_val);
            LSUserSetResult(reg_val);
        }
    }

    return ret;
}
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), rw, RwCmdEntry, read or write register value);