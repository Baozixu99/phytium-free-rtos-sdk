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
 * FilePath: fnand_intr.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 08:56:46
 * Description:  This file contains functions related to fnand interrupt handling.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe    2022/05/10    first release
 */


#include "fnand.h"
#include "fnand_hw.h"
#include "fassert.h"
#include "ferror_code.h"

#include "fdrivers_port.h"
#define FNAND_INTR_DEBUG_TAG "FNAND_INTR"
#define FNAND_INTR_DEBUG_D(format, ...) \
    FT_DEBUG_PRINT_D(FNAND_INTR_DEBUG_TAG, format, ##__VA_ARGS__)
#define FNAND_INTR_DEBUG_I(format, ...) \
    FT_DEBUG_PRINT_I(FNAND_INTR_DEBUG_TAG, format, ##__VA_ARGS__)
#define FNAND_INTR_DEBUG_W(format, ...) \
    FT_DEBUG_PRINT_W(FNAND_INTR_DEBUG_TAG, format, ##__VA_ARGS__)
#define FNAND_INTR_DEBUG_E(format, ...) \
    FT_DEBUG_PRINT_E(FNAND_INTR_DEBUG_TAG, format, ##__VA_ARGS__)


/**
 * @name: FNandIsrEnable
 * @msg:  Enable the corresponding interrupt based on the interrupt mask
 * @return {*}
 * @note:
 * @param {FNand} *instance_p is the pointer to the FNand instance
 * @param {u32} int_mask is interrupt mask
 */
void FNandIsrEnable(FNand *instance_p, u32 int_mask)
{
    FASSERT(instance_p != NULL);
    FASSERT(instance_p->is_ready == FT_COMPONENT_IS_READY);
    FNandConfig *config_p;
    config_p = &instance_p->config;

    FNAND_CLEARBIT(config_p->base_address, FNAND_INTRMASK_OFFSET, int_mask);
}

/**
 * @name: FNandIrqDisable
 * @msg:  Disable the corresponding interrupt based on the interrupt mask
 * @note:
 * @param {FNand} *instance_p is the pointer to the FNand instance
 * @param {u32} int_mask is interrupt mask
 * @return {*}
 */
void FNandIrqDisable(FNand *instance_p, u32 int_mask)
{
    FASSERT(instance_p != NULL);
    FASSERT(instance_p->is_ready == FT_COMPONENT_IS_READY);
    FNandConfig *config_p;
    config_p = &instance_p->config;

    FNAND_SETBIT(config_p->base_address, FNAND_INTRMASK_OFFSET, int_mask);
}


/**
 * @name: FNandSetIsrHandler
 * @msg:  Initializes isr event callback function
 * @note:
 * @param {FNand} *instance_p is the pointer to the FNand instance.
 * @param {FnandIrqEventHandler} event_p is callback function used to respond to the interrupt event
 * @param {void} *irq_args  is the arguments of event callback
 * @return {*}
 */
void FNandSetIsrHandler(FNand *instance_p, FnandIrqEventHandler event_p, void *irq_args)
{
    FASSERT(instance_p != NULL);
    FASSERT(instance_p->is_ready == FT_COMPONENT_IS_READY);
    instance_p->irq_event_fun_p = event_p;
    instance_p->irq_args = irq_args;
}


/**
 * @name: FNandIrqHandler
 * @msg:  Nand driver isr handler
 * @note:
 * @param {s32} vector is interrupt number
 * @param {void} *param is argument
 * @return {*}
 */
void FNandIrqHandler(s32 vector, void *param)
{
    FNand *instance_p = (FNand *)param;
    FNandConfig *config_p;
    u32 status;
    u32 en_irq;
    (void)vector;
    FASSERT(instance_p != NULL);
    FASSERT(instance_p->is_ready == FT_COMPONENT_IS_READY);
    config_p = &instance_p->config;

    status = FNAND_READREG(config_p->base_address, FNAND_INTR_OFFSET);
    en_irq = (~FNAND_READREG(config_p->base_address, FNAND_INTRMASK_OFFSET)) & FNAND_INTRMASK_ALL_INT_MASK;

    if ((status & en_irq) == 0)
    {
        FNAND_INTR_DEBUG_E("No irq exit");
        return;
    }

    FNandIrqDisable(instance_p, status & FNAND_INTRMASK_ALL_INT_MASK);
    FNAND_WRITEREG(config_p->base_address, 0xfd0, 0);
    FNAND_WRITEREG(config_p->base_address, FNAND_INTR_OFFSET, status);

    if (instance_p->irq_event_fun_p)
    {
        if (status & FNAND_INTR_BUSY_MASK)
        {

            instance_p->irq_event_fun_p(instance_p->irq_args, FNAND_IRQ_BUSY_EVENT);
        }

        if (status & FNAND_INTR_DMA_BUSY_MASK)
        {

            instance_p->irq_event_fun_p(instance_p->irq_args, FNAND_IRQ_DMA_BUSY_EVENT);
        }

        if (status & FNAND_INTR_DMA_PGFINISH_MASK)
        {
            instance_p->irq_event_fun_p(instance_p->irq_args, FNAND_IRQ_DMA_PGFINISH_EVENT);
        }

        if (status & FNAND_INTR_DMA_FINISH_MASK)
        {
            instance_p->irq_event_fun_p(instance_p->irq_args, FNAND_IRQ_DMA_FINISH_EVENT);
        }

        if (status & FNAND_INTR_FIFO_EMP_MASK)
        {
            instance_p->irq_event_fun_p(instance_p->irq_args, FNAND_IRQ_FIFO_EMP_EVENT);
        }

        if (status & FNAND_INTR_FIFO_FULL_MASK)
        {
            instance_p->irq_event_fun_p(instance_p->irq_args, FNAND_IRQ_FIFO_FULL_EVENT);
        }

        if (status & FNAND_INTR_FIFO_TIMEOUT_MASK)
        {
            instance_p->irq_event_fun_p(instance_p->irq_args, FNAND_IRQ_FIFO_TIMEOUT_EVENT);
        }

        if (status & FNAND_INTR_CMD_FINISH_MASK)
        {
            instance_p->irq_event_fun_p(instance_p->irq_args, FNAND_IRQ_CMD_FINISH_EVENT);
        }

        if (status & FNAND_INTR_PGFINISH_MASK)
        {
            instance_p->irq_event_fun_p(instance_p->irq_args, FNAND_IRQ_PGFINISH_EVENT);
        }

        if (status & FNAND_INTR_RE_MASK)
        {
            instance_p->irq_event_fun_p(instance_p->irq_args, FNAND_IRQ_RE_EVENT);
        }

        if (status & FNAND_INTR_DQS_MASK)
        {
            instance_p->irq_event_fun_p(instance_p->irq_args, FNAND_IRQ_DQS_EVENT);
        }

        if (status & FNAND_INTR_RB_MASK)
        {
            instance_p->irq_event_fun_p(instance_p->irq_args, FNAND_IRQ_RB_EVENT);
        }

        if (status & FNAND_INTR_ECC_FINISH_MASK)
        {
            instance_p->irq_event_fun_p(instance_p->irq_args, FNAND_IRQ_ECC_FINISH_EVENT);
        }

        if (status & FNAND_INTR_ECC_ERR_MASK)
        {
            FNAND_WRITEREG(config_p->base_address, FNAND_ERROR_CLEAR_OFFSET,
                           FNAND_ERROR_CLEAR_ECC_ERR_CLR_MASK);
            FNAND_WRITEREG(config_p->base_address, FNAND_FIFO_FREE_OFFSET, FNAND_FIFO_FREE_MASK);
            instance_p->irq_event_fun_p(instance_p->irq_args, FNAND_IRQ_ECC_ERR_EVENT);
        }
    }
}
