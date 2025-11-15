/*
 * Copyright : (C) 2024 Phytium Information Technology, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2024/6/26 first commit
 */
/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>

#include "sdkconfig.h"

#include "fassert.h"
#include "finterrupt.h"
#include "fcpu_info.h"
#include "fdebug.h"
#include "fcache.h"
#include "fmemory_pool.h"

#include "usbh_core.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
static void usb_hc_xhci_interrupt_handler(s32 vector, void *param)
{
    extern void USBH_IRQHandler(uint8_t busid);
    USBH_IRQHandler((uint8_t)(uintptr_t)param);
}

static uint32_t usb_hc_get_xhci_irq_num(u32 id)
{
#if defined(CONFIG_TARGET_PE2204) || defined(CONFIG_TARGET_PE2202)
    uint32_t irq_num[FUSB3_NUM] = {
        FUSB3_0_IRQ_NUM,
        FUSB3_1_IRQ_NUM
    };
    FASSERT(id < FUSB3_NUM);
#endif

    return irq_num[id];
}

void usb_hc_setup_xhci_interrupt(u32 id)
{
    u32 cpu_id;
    u32 irq_priority = 13U;
    u32 irq_num = usb_hc_get_xhci_irq_num(id);

    GetCpuId(&cpu_id);
    InterruptSetTargetCpus(irq_num, cpu_id);

    InterruptSetPriority(irq_num, irq_priority);

    /* register intr callback */
    InterruptInstall(irq_num,
                     usb_hc_xhci_interrupt_handler,
                     (void *)(uintptr_t)id,
                     NULL);
}

void usb_hc_disable_interrupt(u32 id)
{
    u32 irq_num = usb_hc_get_xhci_irq_num(id);

    InterruptMask(irq_num);
}

void usb_hc_enable_interrupt(u32 id)
{
    u32 irq_num = usb_hc_get_xhci_irq_num(id);

    InterruptUmask(irq_num);
}

unsigned long usb_hc_get_register_base(uint32_t id)
{
#if defined(CONFIG_TARGET_PE2204) || defined(CONFIG_TARGET_PE2202)
    unsigned long base_reg[FUSB3_NUM] = {
        FUSB3_0_BASE_ADDR + FUSB3_XHCI_OFFSET,
        FUSB3_1_BASE_ADDR + FUSB3_XHCI_OFFSET
    };
    FASSERT(id < FUSB3_NUM);
#endif

    return base_reg[id];
}