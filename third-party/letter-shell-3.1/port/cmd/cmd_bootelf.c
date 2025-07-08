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
 * FilePath: cmd_bootelf.c
 * Date: 2022-02-24 18:24:53
 * LastEditTime: 2022-03-21 17:03:44
 * Description:  This file is for the bootelf command functions
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe    2022/3/25   first release
 */

#include <stdio.h>
#include "../src/shell.h"
#include "strto.h"
#include "felf.h"

static unsigned long image_load_addr = 0x80100000;

static void BootelfCmdUsage()
{
    printf("usage:\r\n");
    printf("    bootelf [-p|-s] [address] \r\n");
    printf("    load ELF image at [address] via program headers (-p)\r\n");
    printf("    or via section headers (-s)\r\n");
}

static int BootelfCmdEntry(int argc, char *argv[])
{
    unsigned long addr; /* Address of the ELF image */
    unsigned long rc = 0; /* Return value from user code */
    unsigned long load_flg = 0;
    char *sload = NULL;

    if (argc < 2)
    {
        BootelfCmdUsage();
        return -1;
    }

    /* Consume 'bootelf' */
    argc--;
    argv++;

    /* Check for flag. */
    if (argc >= 1 && (argv[0][0] == '-' && (argv[0][1] == 'p' || argv[0][1] == 's')))
    {
        sload = argv[0];
        /* Consume flag. */
        argc--;
        argv++;
    }


    if (argc >= 1 && (argv[0][0] == '-' && (argv[0][1] == 'l')))
    {
        /* Consume flag. */
        argc--;
        argv++;
        load_flg = 1;
    }

    /* Check for address. */
    if (argc >= 1 && strict_strtoul(argv[0], 16, &addr) == 0)
    {
        /* Consume address */
        argc--;
        argv++;
    }
    else
    {
        addr = image_load_addr;
    }

    if (!ElfIsImageValid(addr))
    {
        return 1;
    }


    if (sload && sload[1] == 'p')
    {
        addr = ElfLoadElfImagePhdr(addr);
    }
    else
    {
        addr = ElfLoadElfImageShdr(addr);
    }

    /*
     * pass address parameter as argv[0] (aka command name),
     * and all remaining args
     */
    if (load_flg == 1)
    {
        printf("## Load application at 0x%08lx ...\n", addr);
    }
    else
    {
        printf("## Starting application at 0x%08lx ...\n", addr);
        rc = ElfExecBootElf((void *)addr, argc, argv);
    }


    printf("## Application terminated, rc = 0x%lx.\n", rc);
    return (int)rc;
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
                 bootelf, BootelfCmdEntry, Boot from an ELF image in memory);
