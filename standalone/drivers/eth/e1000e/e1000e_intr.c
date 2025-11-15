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
 * FilePath: e1000e_intr.c
 * Date: 2025-08-15 14:55:11
 * LastEditTime: 2025-08-15 09:03:57
 * Description:  This file is for interrupt function of E1000E ctrl
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   haungjin   2025/8/15    first release
 */

#include "fassert.h"
#include "fdrivers_port.h"
#include "e1000e.h"
#include "e1000e_hw.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

#define E1000E_DEBUG_TAG "E1000E_INTR"
#define E1000E_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(E1000E_DEBUG_TAG, format, ##__VA_ARGS__)
#define E1000E_WARN(format, ...) \
    FT_DEBUG_PRINT_W(E1000E_DEBUG_TAG, format, ##__VA_ARGS__)
#define E1000E_INFO(format, ...) \
    FT_DEBUG_PRINT_I(E1000E_DEBUG_TAG, format, ##__VA_ARGS__)
#define E1000E_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(E1000E_DEBUG_TAG, format, ##__VA_ARGS__)

/**
 * @name: FE1000EIrqEnable
 * @msg: enable e1000e interrupt mask
 * @param {FE1000ECtrl} *instance_p, pointer to the FE1000ECtrl instance
 * @param {u32} int_mask, interrupt enable mask
 * @return {void}
 */
void FE1000EIrqEnable(FE1000ECtrl *instance_p, u32 int_mask)
{
    FASSERT(instance_p != NULL);
    FASSERT(instance_p->is_ready == FT_COMPONENT_IS_READY);

    FE1000E_WRITEREG32(instance_p->config.base_addr, E1000_IMS, int_mask);
}

/**
 * @name: FE1000EIrqDisable
 * @msg: disable e1000e interrupt mask
 * @param {FE1000ECtrl} *instance_p, pointer to the FE1000ECtrl instance
 * @param {u32} int_mask, interrupt disable mask
 * @return {void}
 */
void FE1000EIrqDisable(FE1000ECtrl *instance_p, u32 int_mask)
{
    FASSERT(instance_p != NULL);
    FASSERT(instance_p->is_ready == FT_COMPONENT_IS_READY);

    FE1000E_WRITEREG32(instance_p->config.base_addr, E1000_IMC, int_mask);
}

/**
 * @name: FE1000ERegisterEvtHandler
 * @msg: 注册FE1000E中断事件响应函数
 * @return {*}
 * @param {FGmac} *instance_p 驱动控制数据
 * @param {u32} event 中断事件类型
 * @param {FE1000EEvtHandler} handler 中断事件响应函数
 * @note 注册的函数handler会在中断上下文执行
 */
void FE1000ERegisterEvtHandler(FE1000ECtrl *instance_p, u32 event, FE1000EEvtHandler handler)
{
    FASSERT((NULL != instance_p) && (FE1000E_INTR_EVT_NUM > event));
    instance_p->evt_handler[event] = handler;
}

/**
 * @name: FE1000EIrqHandler
 * @msg: e1000e interrupt handler entry
 * @param {void} *param, function parameters, users can set
 * @return {void}
 */
void FE1000EIrqHandler(s32 vector, void *param)
{
    FE1000ECtrl *instance_p = (FE1000ECtrl *)param;
    FASSERT(instance_p != NULL);
    FASSERT(instance_p->is_ready == FT_COMPONENT_IS_READY);
    u32 status;
    uintptr base_addr = instance_p->config.base_addr;

    status = FE1000E_READREG32(base_addr, E1000_ICR);
    E1000E_DEBUG("E1000E interrupt status is 0x%08x", status);

    if (status & IMS_LSC)
    {
        E1000E_DEBUG("---------------------link change event---------------------");
        if (instance_p->evt_handler[FE1000E_LINK_STATUS_EVT])
        {
            instance_p->evt_handler[FE1000E_LINK_STATUS_EVT](instance_p);
        }

        FE1000E_WRITEREG32(base_addr, E1000_ICR, IMS_LSC);
    }

    if (status & IMS_RXQ0)
    {
        E1000E_DEBUG("---------------------receive done event---------------------");
        if (instance_p->evt_handler[FE1000E_RX_COMPLETE_EVT])
        {
            instance_p->evt_handler[FE1000E_RX_COMPLETE_EVT](instance_p);
        }

        FE1000E_WRITEREG32(base_addr, E1000_ICR, IMS_RXQ0);
    }

    if (status & IMS_RXT0)
    {
        E1000E_DEBUG("---------------------Receiver Timer "
                     "Interrupt---------------------");
        FE1000E_WRITEREG32(base_addr, E1000_ICR, IMS_RXT0);
    }


    if (status & IMS_TXQ0)
    {
        E1000E_DEBUG("---------------------Transmit Queue 0 "
                     "Interrupt---------------------");
        FE1000E_WRITEREG32(base_addr, E1000_ICR, IMS_TXQ0);
    }

    if (status & IMS_TXDW)
    {
        E1000E_DEBUG("---------------------transmit done event---------------------");
        if (instance_p->evt_handler[FE1000E_TX_COMPLETE_EVT])
        {
            instance_p->evt_handler[FE1000E_TX_COMPLETE_EVT](instance_p);
        }

        FE1000E_WRITEREG32(base_addr, E1000_ICR, IMS_TXDW);
    }
}
