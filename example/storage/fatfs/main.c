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
 * FilePath: main.c
 * Date: 2022-06-17 08:17:59
 * LastEditTime: 2022-06-17 08:17:59
 * Description:  This file is for fatfs main entry.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  zhugengyu  2022/12/7    init commit
 */

#include <stdio.h>
#include "sdkconfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "fatfs_examples.h"
#if defined(CONFIG_FATFS_SDMMC_FSDIF_TF) || defined(CONFIG_FATFS_SDMMC_FSDIF_EMMC)
#include "fsdif_timing.h"
#include "fsl_sdmmc.h"
#endif

#if defined CONFIG_USE_LETTER_SHELL
#include "shell.h"
#include "shell_port.h"
#endif

int main(void)
{
    BaseType_t ret;

#if defined(CONFIG_FATFS_SDMMC_FSDIF_TF) || defined(CONFIG_FATFS_SDMMC_FSDIF_EMMC)
    FSdifTimingInit();
    SDMMC_OSAInit();
#endif

#if defined CONFIG_USE_LETTER_SHELL
    ret = LSUserShellTask() ;
    if (ret != pdPASS)
    {
        goto FAIL_EXIT;
    }
#else
    ret = xTaskCreate((TaskFunction_t)FatfsExampleEntry,
                        (const char *)"FatfsExampleEntry",
                        4096,
                        NULL,
                        (UBaseType_t)2,
                        NULL);
    
    if (ret != pdPASS)
    {
        goto FAIL_EXIT;
    }
#endif

    vTaskStartScheduler(); /* 启动任务，开启调度 */
    while (1); /* 正常不会执行到这里 */

#if defined(CONFIG_FATFS_SDMMC_FSDIF_TF) || defined(CONFIG_FATFS_SDMMC_FSDIF_EMMC)
    SDMMC_OSADeInit();
    FSdifTimingDeinit();
#endif

FAIL_EXIT:
    printf("Failed,the ret value is 0x%x. \r\n", ret);
    return 0;
}
