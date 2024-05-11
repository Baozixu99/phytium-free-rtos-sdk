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
 * FilePath: cmd_ddma.c
 * Date: 2022-07-14 14:06:43
 * LastEditTime: 2024-04-19 14:06:43
 * Description:  This file is for DDMA command interface.
 *
 * Modify History:
 *  Ver    Who          Date         Changes
 * -----  ------       --------     --------------------------------------
 *  1.0   zhugengyu    2022/7/27    init commit
 *  1.1   liqiaozhong  2023/11/10   synchronous update with standalone sdk
 *  2.0   liyilun      2024/4/19    add no letter shell mode, adapt to auto test system
 */

/***************************** Include Files *********************************/
#include <string.h>
#include <stdio.h>
#include "sdkconfig.h"
#include "ddma_spi_loopback.h"
#include "strto.h"
#include "FreeRTOS.h"
#include "task.h"

#if defined(CONFIG_E2000Q_DEMO_BOARD) || defined(CONFIG_E2000D_DEMO_BOARD)
#define USED_SPI_ID FSPI2_ID
#else
#define USED_SPI_ID FSPI0_ID
#endif

#ifdef CONFIG_USE_LETTER_SHELL
#include "../src/shell.h"

static void DdmaCmdUsage(void)
{
    printf("Usage:\r\n");
    printf("ddma spi-loopback\r\n");
    printf("-- Run ddma spi loopback example, default transfer 32 bytes\r\n");
    printf("ddma spi-loopback N\r\n");
    printf("-- Run ddma spi loopback example, N is a number, N <= 128 =bytes, default transfer N bytes\r\n");
}

static int DdmaCmdEntry(int argc, char *argv[])
{
    int ret = 0;
    u32 bytes = 32;
    u32 spi_id = USED_SPI_ID;
    if (!strcmp(argv[1], "spi-loopback"))
    {
        if (argc >= 3)
        {
            bytes = (u32)simple_strtoul(argv[2], NULL, 10);
        }

        ret = FFreeRTOSRunDDMASpiLoopback(spi_id, bytes);
    }else
    {
        DdmaCmdUsage();
    }

    return ret;
}
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), ddma, DdmaCmdEntry, test freertos ddma driver);

#else
void DdmaTasksEntry(void)
{
    u32 bytes = 32;
    u32 spi_id = USED_SPI_ID;
    FFreeRTOSRunDDMASpiLoopback(spi_id, bytes);

    /* end flag */
    printf("[test_end]\r\n");
    vTaskDelete(NULL);
}
#endif