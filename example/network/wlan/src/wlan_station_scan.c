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
 * FilePath: wlan_station_scan.c
 * Date: 2022-07-12 09:53:00
 * LastEditTime: 2024-04-30 09:53:02
 * Description:  This file is for providing functions used in cmd_sf.c file.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  zhugengyu  2023/10/19    first commit
 *  2.0  liyilun    2024/4/30     add no letter shell mode, adapt to auto test system
 */
/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "fdebug.h"
#include "fsleep.h"
#include "fkernel.h"
#include "fassert.h"
#include <queue.h>
#include "wlan_common.h"
#include "wlan_station_scan.h"

/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/

static QueueHandle_t xQueue = NULL;

/***************** Macros (Inline Functions) Definitions *********************/
#define WR_TIMER_PERIOD         pdMS_TO_TICKS(60000U)

#define FWLAN_DEBUG_TAG "FFreeRTOSWlan"
#define FWLAN_ERROR(format, ...)   FT_DEBUG_PRINT_E(FWLAN_DEBUG_TAG, format, ##__VA_ARGS__)
#define FWLAN_WARN(format, ...)    FT_DEBUG_PRINT_W(FWLAN_DEBUG_TAG, format, ##__VA_ARGS__)
#define FWLAN_INFO(format, ...)    FT_DEBUG_PRINT_I(FWLAN_DEBUG_TAG, format, ##__VA_ARGS__)
#define FWLAN_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FWLAN_DEBUG_TAG, format, ##__VA_ARGS__)
/************************** Function Prototypes ******************************/

/*****************************************************************************/
/* Link lost callback */
static void LinkStatusChangeCallback(bool linkState)
{
    if (linkState == false)
    {
        printf("-------- LINK LOST --------\r\n");
    }
    else
    {
        printf("-------- LINK REESTABLISHED --------\r\n");
    }
}

void WlanStationScanTask(void *param)
{
    FWlanRetStatus result;
    char *scan_result;
    result = FWlanInit();
    if (result != FWLAN_RET_SUCCESS)
    {
        printf("[!] FWlanInit: Failed, error: %d\r\n", (uint32_t)result);
        goto task_exit;
    }

    result = FWlanStart(LinkStatusChangeCallback); 
    if (result != FWLAN_RET_SUCCESS)
    {
        printf("[!] FWlanStart: Failed, error: %d\r\n", (uint32_t)result);
        goto task_exit;
    }

    scan_result = FWlanScan();
    if (scan_result == NULL)
    {
        printf("[!] FWlanScan: Failed to scan\r\n");
        result = FWLAN_NOT_FOUND;
        goto task_exit;
    }

    vPortFree(scan_result);

task_exit:
    xQueueSend(xQueue, &result, 0);
    vTaskDelete(NULL);
}

BaseType_t FFreeRTOSWlanStationScanTaskCreate(void)
{
    xQueue  = xQueueCreate(1, sizeof(int));
    if(xQueue == NULL)
    {
        FWLAN_ERROR("xQueue Create failed.");
        goto err_ret;
    }
    BaseType_t xReturn;
    int task_ret;
    /* Create the main Task */
    FWlanDeinit();
    xReturn = xTaskCreate(WlanStationScanTask, 
                    "WlanStationScanTask", 
                    4096, 
                    NULL, 
                    (UBaseType_t)configMAX_PRIORITIES - 1, 
                    NULL);
    if(xReturn != pdPASS)
    {
        FWLAN_ERROR("WlanStationScanTask create failed.");
        goto err_ret;
    }

    xReturn = xQueueReceive(xQueue, &task_ret, WR_TIMER_PERIOD);
    vQueueDelete(xQueue);
    
    if(xReturn != pdPASS)
    {
        FWLAN_ERROR("xQueue receive failed.\r\n");
        goto err_ret;
    }
    
    if(task_ret == FWLAN_RET_SUCCESS)
    {
        printf("%s@%d: Wlan scan example [success].\r\n", __func__, __LINE__);
        return pdPASS;
    }
err_ret:
    printf("%s@%d: Wlan scan example [failure].\r\n", __func__, __LINE__);
    return pdFAIL;
}