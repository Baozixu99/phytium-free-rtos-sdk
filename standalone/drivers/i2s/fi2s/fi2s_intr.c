/*
 * Copyright (C) 2023, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: fi2s_intr.c
 * Created Date: 2023-06-20 11:06:02
 * Last Modified: 2023-09-06 09:28:07
 * Description:  This file is for
 *
 * Modify History:
 *   Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 */
#include "fparameters.h"

#include <stdio.h>
#include "fassert.h"
#include "fi2s.h"
#include "fi2s_hw.h"


#define FI2S_CALL_INTR_EVENT_HANDLDER(instance_p, event) \
    if (instance_p->event_handler[event])                \
    instance_p->event_handler[event](instance_p->event_param[event])

/**
 * @name: FI2sRegisterInterruptHandler
 * @msg: register i2s interrupt handler function
 * @param {FI2s} *instance_p, pointer to the i2s instance
 * @param {FI2SIntrEventType} event_type, interrupt event type
 * @param {FI2SIntrEventHandler} handler, interrupt event handler
 * @param {void} *param, contains a pointer to the driver instance
 * @return {*}
 */
void FI2sRegisterInterruptHandler(FI2s *instance_p, FI2SIntrEventType event_type,
                                  FI2SIntrEventHandler handler, void *param)
{
    FASSERT(instance_p);
    FASSERT(event_type < FI2S_INTR_EVENT_NUM);
    instance_p->event_handler[event_type] = handler;
    instance_p->event_param[event_type] = param;
}

void FI2sIntrHandler(s32 vector, void *args)
{
    FI2s *instance_p = (FI2s *)args;
    u32 status = 0;

    uintptr index = instance_p->config.instance_id;
    uintptr base_addr = instance_p->config.base_addr;

    status = FI2S_READ_REG32(base_addr, FI2S_ISR0(index));

    if (status & FI2S_INTR_TRANS_FO_MASK)
    {
        FI2S_READ_REG32(base_addr, FI2S_TOR0(index));
        FI2S_CALL_INTR_EVENT_HANDLDER(instance_p, FI2S_INTR_TRANS_FO);
    }
    if (status & FI2S_INTR_TRANS_FE_MASK)
    {
        FI2S_READ_REG32(base_addr, FI2S_TOR0(index));
        FI2S_CALL_INTR_EVENT_HANDLDER(instance_p, FI2S_INTR_TRANS_FE);
    }
    if (status & FI2S_INTR_RECRIVE_FO_MASK)
    {
        FI2S_READ_REG32(base_addr, FI2S_R0R0(index));
        FI2S_CALL_INTR_EVENT_HANDLDER(instance_p, FI2S_INTR_RECEIVE_FO);
    }
    if ((status & FI2S_INTR_RECRIVE_FE_MASK))
    {
        FI2S_READ_REG32(base_addr, FI2S_R0R0(index));
        FI2S_CALL_INTR_EVENT_HANDLDER(instance_p, FI2S_INTR_RECEIVE_FE);
    }
}
