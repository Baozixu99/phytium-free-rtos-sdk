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
 * FilePath: fdcdp_intr.c
 * Date: 2024-11-20 17:31:47
 * LastEditTime: 2024/11/21 10:02:47
 * Description:  This file is for defining the dp interrupt and function
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------  -------- --------------------------------------
 *  1.0  Wangzq     2024/11/20  Modify the format and establish the version
 */
#include "fio.h"
#include "fdrivers_port.h"
#include "fassert.h"
#include "ftypes.h"

#include "fdc.h"
#include "fdp_hw.h"

#include "fdcdp.h"
#include "fdcdp_reg.h"
/***************** Macros (Inline Functions) Definitions *********************/

#define FDCDP_CALL_INTR_EVENT_HANDLDER(instance, event, index) \
    if (instance->intr_event[event].handler)                   \
    instance->intr_event[event].handler(instance->intr_event[event].param, index)

/************************** Function Prototypes ******************************/

void FDcDpRegisterHandler(FDcDp *instance, FDcDpIntrEventType type,
                          FDCDPIntrHandler handler, void *param)
{
    FASSERT(instance != NULL);
    FASSERT(type < FDCDP_INTR_MAX_NUM);
    instance->intr_event[type].type = type;
    instance->intr_event[type].handler = handler;
    instance->intr_event[type].param = param;
}

void FDcDpIrqEnable(FDcDp *instance, u32 index, FDcDpIntrEventType intr_event_p)
{
    FASSERT(instance != NULL);
    u32 irq_mask = 0;
    uintptr addr = instance->dp_instance_p[index].config.dp_channe_base_addr;
    switch (intr_event_p)
    {
        case FDCDP_HPD_IRQ_CONNECTED:
            irq_mask = FDpChannelRegRead(addr, PHYTIUM_DP_INTERRUPT_MASK);
            irq_mask &= ~PHYTIUM_DP_TX_HPD_EVENT_MASK;
            irq_mask &= ~PHYTIUM_DP_TX_HPD_INTR_MASK;
            break;
        case FDCDP_HPD_IRQ_DISCONNECTED:
            irq_mask = FDpChannelRegRead(addr, PHYTIUM_DP_INTERRUPT_MASK);
            irq_mask &= ~PHYTIUM_DP_TX_HPD_EVENT_MASK;
            irq_mask &= ~PHYTIUM_DP_TX_HPD_INTR_MASK;
            break;
        case FDCDP_AUX_REPLY_ERROR:
            irq_mask = FDpChannelRegRead(addr, PHYTIUM_DP_INTERRUPT_MASK);
            irq_mask &= ~PHYTIUM_DP_TX_AUX_ERROR_MASK;
            break;
        case FDCDP_AUX_REPLY_TIMEOUT:
            irq_mask = FDpChannelRegRead(addr, PHYTIUM_DP_INTERRUPT_MASK);
            irq_mask &= ~PHYTIUM_DP_TX_AUX_TIMEOUT_MASK;
            break;
        default:
            break;
    }
    FDpChannelRegWrite(addr, PHYTIUM_DP_INTERRUPT_MASK, irq_mask);
}

void FDcDpInterruptHandler(s32 vector, void *args)
{
    FASSERT(args != NULL);
    FDcDp *instance = (FDcDp *)args;
    u32 hpd_status = 0;
    u32 sink_status = 0;
    u32 hpd_mask;
    u32 index;

    for (index = 0; index < FDP_INSTANCE_NUM; index++)
    {
        uintptr addr = instance->dp_instance_p[index].config.dp_channe_base_addr;
        hpd_mask = FDpChannelRegRead(addr, PHYTIUM_DP_INTERRUPT_MASK);
        hpd_status = FDpChannelRegRead(addr, PHYTIUM_DP_INTERRUPT_RAW_STATUS);
        sink_status = FDpChannelRegRead(addr, PHYTIUM_DP_SINK_HPD_STATE);

        if (0 == (hpd_mask & PHYTIUM_DP_TX_HPD_EVENT_MASK))
        {
            if ((hpd_status & PHYTIUM_DP_TX_STATUS_HPD_EVENT) && (sink_status == 1))
            {
                FDCDP_CALL_INTR_EVENT_HANDLDER(instance, FDCDP_HPD_IRQ_CONNECTED, index);
            }
            if ((hpd_status & PHYTIUM_DP_TX_STATUS_HPD_EVENT) && (sink_status == 0))
            {
                FDCDP_CALL_INTR_EVENT_HANDLDER(instance, FDCDP_HPD_IRQ_DISCONNECTED, index);
            }
        }
        if (0 == (hpd_mask & PHYTIUM_DP_TX_AUX_TIMEOUT_MASK))
        {
            if ((hpd_status & PHYTIUM_DP_TX_STATUS_AUX_TIMEOUT) && (sink_status == 1))
            {
                FDCDP_CALL_INTR_EVENT_HANDLDER(instance, FDCDP_AUX_REPLY_TIMEOUT, index);
            }
        }
        if (0 == (hpd_mask & PHYTIUM_DP_TX_AUX_ERROR_MASK))
        {
            if ((hpd_status & PHYTIUM_DP_TX_STATUS_AUX_ERROR) && (sink_status == 1))
            {
                FDCDP_CALL_INTR_EVENT_HANDLDER(instance, FDCDP_AUX_REPLY_ERROR, index);
            }
        }
    }
}
