/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc.
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
 * FilePath: exception_test.c
 * Date: 2022-02-24 13:42:19
 * LastEditTime: 2022-03-21 17:02:53
 * Description:  This file is for exception debug examples.
 *
 * Modify History:
 *  Ver   Who            Date         Changes
 * ----- ------        --------    --------------------------------------
 *  1.0  wangxiaodong  2022/11/1    init commit
 */
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "ftypes.h"
#include "fmmu.h"
#include "fio.h"
#include "fkernel.h"
#include "sdkconfig.h"
#include "faarch.h"

int FExcOpsInvalidMemAccess(void) // FMmuAssignL2Table
{
    static char buf[4096] __attribute__((aligned(4096))) = {0};
    buf[0] = 'A';
    buf[32] = 'B';
    printf("Buf @%p buf[0] = %c, buf[32] = %c\r\n",
           buf, buf[0], buf[32]);

    FMmuMap((uintptr)buf, (uintptr)buf, 4096, MT_NORMAL | MT_P_RO_U_NA | MT_NS);

    buf[0] = 'a'; /* cause sync error */
    return 0;
}

int FExcOpsUndefInstr(void)
{
    printf("Exception debug FExcOpsUndefInstr func\n");

#ifdef __aarch64__
    AARCH64_READ_SYSREG(SCTLR_EL3);
#else
    #define UNDEFINED_INS	15, 0, 3, 2, 1
    AARCH32_READ_SYSREG_32(UNDEFINED_INS);
#endif

    return 0;
}

int FExcOpsDataAbort(void)
{
    printf("Exception debug FExcOpsDataAbort func\n");
    uintptr data_addr = 0x02000000;
    FtOut32(data_addr, 12); /* invalid data access to trap Data abort */

    return 0;
}

