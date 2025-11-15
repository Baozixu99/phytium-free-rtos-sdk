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
 * FilePath: cmd_codeloader.c
 * Date: 2022-02-10 14:53:43
 * LastEditTime: 2022-02-25 11:47:13
 * Description:  This file is for loadelf command implmentation
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe    2021-10-18   init commit
 */

#include <stdio.h>
#include "../src/shell.h"
#include "strto.h"
#include "felf.h"

static unsigned long image_load_addr = 0x80100000;

static void LoadElfCmdUsage()
{
    printf("usage:\r\n");
    printf("    loadelf [-p|-s] [address] \r\n");
    printf("    load ELF image at [address] via program headers (-p)\r\n");
    printf("    or via section headers (-s)\r\n");
}

static int LoadElfCmdEntry(int argc, char *argv[])
{
    unsigned long addr; /* Address of the ELF image */
    char *sload = NULL;

    if (argc < 2)
    {
        LoadElfCmdUsage();
        return -1;
    }
    /* Consume 'LoadElf' */
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
    printf("## Load application at 0x%08lx ...\n", addr);

    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), loadelf, LoadElfCmdEntry,
                 Load from an ELF image in memory);