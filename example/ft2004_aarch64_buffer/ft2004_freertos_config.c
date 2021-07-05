/*
 * @ : Copyright (c) 2021 Phytium Information Technology, Inc. 
 *  
 * SPDX-License-Identifier: Apache-2.0.
 * 
 * @Date: 2021-07-02 11:09:17
 * @LastEditTime: 2021-07-02 11:23:11
 * @Description:  This files is for 
 * 
 * @Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */
#include "ft_types.h"
#include "FreeRTOS.h"
#include "task.h"
#include "generic_timer.h"
#include "interrupt.h"

static u32 cntfrq; /* System frequency */

void vConfigureTickInterrupt(void)
{
    // Disable the timer
    GenericTimerStop();
    // Get system frequency
    cntfrq = GenericTimerFrequecy();

    // Set tick rate
    GenericTimerCompare(cntfrq / configTICK_RATE_HZ);
    GenericTimerInterruptEnable();

    InterruptSetPriority(30, configKERNEL_INTERRUPT_PRIORITY);
    InterruptUmask(30);
    GenericTimerStart();
}

void vClearTickInterrupt(void)
{
    GenericTimerCompare(cntfrq / configTICK_RATE_HZ);
}
