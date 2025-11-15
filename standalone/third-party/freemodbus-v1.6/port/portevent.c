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
 * FilePath: portevent.c
 * Date: 2022-09-29 18:08:02
 * LastEditTime: 2022-09-29 18:08:02
 * Description:  This file is for port event function
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  liushengming 2022/09/29    first commit
 */

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

/* ----------------------- Variables ----------------------------------------*/
static eMBEventType queued_event;
static BOOL event_in_queue;

/* ----------------------- Start implementation -----------------------------*/
BOOL xMBPortEventInit(void)
{
    event_in_queue = FALSE;
    return TRUE;
}

BOOL xMBPortEventPost(eMBEventType event)
{
    event_in_queue = TRUE;
    queued_event = event;
    return TRUE;
}

BOOL xMBPortEventGet(eMBEventType *event)
{
    BOOL event_happened = FALSE;

    if (event_in_queue)
    {
        *event = queued_event;
        event_in_queue = FALSE;
        event_happened = TRUE;
    }
    return event_happened;
}
