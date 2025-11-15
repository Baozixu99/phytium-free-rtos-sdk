/*
 * Copyright (C) 2021, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: fi3c_intr.c
 * Date: 2021-11-01 14:53:42
 * LastEditTime: 2022-02-18 08:36:38
 * Description:  This file is for I3C interrupt operation
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  zhangyan 2023/9/11  first commit
 */

#include <string.h>
#include "fio.h"
#include "ferror_code.h"
#include "ftypes.h"
#include "fdebug.h"
#include "fi3c_hw.h"
#include "fi3c.h"

#define FI3C_DEBUG_TAG          "I3C_INTR"
#define FI3C_ERROR(format, ...) FT_DEBUG_PRINT_E(FI3C_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI3C_INFO(format, ...)  FT_DEBUG_PRINT_I(FI3C_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI3C_DEBUG(format, ...) FT_DEBUG_PRINT_D(FI3C_DEBUG_TAG, format, ##__VA_ARGS__)

#define FI3C_CALL_INTR_EVENT_HANDLDER(instance_p, event) \
    if (instance_p->intr_event[event].handler)           \
    instance_p->intr_event[event].handler(instance_p->intr_event[event].param)

/**
 * @name: FI3cMasterRegisterIntrHandler
 * @msg: 注册I3C主机中断事件函数
 * @return {*}
 * @param {FI3c} *instance_p I3C驱动实例数据
 * @param {u32} evt 中断事件，参考 FI3C_MASTER_INTR_EVT_NUM
 * @param {FI3cEvtHandler} handler 中断事件回调函数
 */
void FI3cMasterRegisterIntrHandler(FI3c *instance_p, FI3cIntrEventConfig *intr_event_p)
{
    FASSERT(instance_p);
    FASSERT(intr_event_p);
    FI3cIntrEventType type = intr_event_p->type;
    FASSERT(intr_event_p->type < FI3C_MASTER_INTR_EVENT_NUM);
    instance_p->intr_event[type].type = type;
    instance_p->intr_event[type].handler = intr_event_p->handler;
    instance_p->intr_event[type].param = intr_event_p->param;
}
/**
 * @name: FI3cMasterIntrHandler
 * @msg: 主机模式下的I3C中断响应函数
 * @return {*}
 * @param {s32} vector
 * @param {void} *param, 中断输入参数
 */
void FI3cMasterIntrHandler(s32 vector, void *param)
{
    FASSERT(param);
    FI3c *instance_p = (FI3c *)param;
    uintptr base_addr = instance_p->config.base_addr;
    u32 irq_reg_val;
    u32 intr_status = FI3C_READ_REG32(base_addr, FI3C_MST_ISR_OFFSET);

    if (0 == intr_status)
    {
        return;
    }

    if (FI3C_MST_INT_CMDD_EMP_MASK & intr_status)
    {
        FI3C_CALL_INTR_EVENT_HANDLDER(instance_p, FI3C_INTR_EVENT_CMDD_EMP);
        FI3C_DEBUG("Command Descriptor Queue Empty!!!.");
        irq_reg_val = FI3C_READ_REG32(base_addr, FI3C_MST_IDR_OFFSET);
        FI3C_WRITE_REG32(base_addr, FI3C_MST_IDR_OFFSET, irq_reg_val | FI3C_MST_INT_CMDD_EMP_MASK);
        irq_reg_val = FI3C_READ_REG32(base_addr, FI3C_MST_ICR_OFFSET);
        FI3C_WRITE_REG32(base_addr, FI3C_MST_ICR_OFFSET, irq_reg_val | FI3C_MST_INT_CMDD_EMP_MASK);
    }
}
