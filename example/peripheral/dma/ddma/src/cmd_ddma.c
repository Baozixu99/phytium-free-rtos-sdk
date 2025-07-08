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
    printf("-- Run ddma spi loopback example, N is a number, N <= 128 =bytes, N % 4 == 0, default transfer N bytes\r\n");
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