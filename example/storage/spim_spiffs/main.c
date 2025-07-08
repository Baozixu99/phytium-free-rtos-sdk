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
 * Description:  This file is for the main functions.
 * 
 * Modify History: 
 *  Modify History: 
 *  Ver     Who      Date         Changes
 * -----   ------  --------   --------------------------------------
 * 1.0 liqiaozhong 2022/11/2  first commit
 * 2.0  liyilun     2024/04/25 add no letter shell mode, adapt to auto test system
 */

#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "spim_spiffs_example.h"

#ifdef CONFIG_USE_LETTER_SHELL
#include "shell.h"
#include "shell_port.h"
#endif

int main(void)
{
    BaseType_t ret;
#ifdef CONFIG_USE_LETTER_SHELL
    ret = LSUserShellTask() ;
    if(ret != pdPASS)
        goto FAIL_EXIT;
#else

    ret = xTaskCreate((TaskFunction_t)SpimSpiffsExampleEntry,
                    (const char *)"SpimSpiffsExampleEntry",
                    4096,
                    NULL,
                    (UBaseType_t)2,
                    NULL);
#endif
    vTaskStartScheduler(); /* 启动任务，开启调度 */   
    while (1); /* 正常不会执行到这里 */
    
FAIL_EXIT:
    printf("failed 0x%x \r\n", ret);  
    return 0;
}
