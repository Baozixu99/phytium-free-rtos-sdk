/*
 * Copyright (C) 2024, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: cmd_mmu.c
 * Created Date: 2024-05-21 10:50:34
 * Last Modified: 2024-05-21 11:17:24
 * Description:  This file is for
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../src/shell.h"
#include "fmmu.h"


static void MwUsage()
{
	printf("usage:\r\n");
	printf("    mmu_display [detailed] \r\n");
}

static void MmuDisplay(int argc, char *argv[])
{   
    int print_detailed = (argc > 1 && strcmp(argv[1], "detailed") == 0) ? 1 : 0;

    FMmuPrintReconstructedTables(print_detailed) ;
    DisplayMmuUsage();
    
    MwUsage() ;
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), 
                 mmu_display, MmuDisplay, Display MMU usage and reconstructed tables);



