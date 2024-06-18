/*
 * Copyright : (C) 2024 Phytium Information Technology, Inc.
 * All Rights Reserved.
 * 
 * This program is OPEN SOURCE software: you can redistribute it and/or modify it
 * under the terms of the Phytium Public License as published by the Phytium Technology Co.,Ltd,
 * either version 1.0 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the Phytium Public License for more details.
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



