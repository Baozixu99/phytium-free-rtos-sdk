/*
 * @ : Copyright (c) 2021 Phytium Information Technology, Inc. 
 *  
 * SPDX-License-Identifier: Apache-2.0.
 * 
 * @Date: 2021-07-02 11:09:17
 * @LastEditTime: 2021-07-02 11:22:05
 * @Description:  This files is for 
 * 
 * @Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */
#include "irq_test.h"
#include "parameters.h"
#include "ft_types.h"
#include "gicv3.h"
#include "interrupt.h"

extern void FreeRTOS_Tick_Handler(void);
extern void SystemIrqHandler(int ir);

u32 PlatformGetGicDistBase(void)
{
    return GICV3_BASEADDRESS;
}

void irq_test_init(void)
{
    /* interrupt init */
    ArmGicRedistAddressSet(0, GICV3_RD_BASEADDRESS + 0, 0);
    InterruptInit();
}

void vApplicationIRQHandler(uint32_t ulICCIAR)
{
    int ulInterruptID;

    /* Interrupts cannot be re-enabled until the source of the interrupt is
	cleared. The ID of the interrupt is obtained by bitwise ANDing the ICCIAR
	value with 0x3FF. */
    ulInterruptID = ulICCIAR & 0x3FFUL;

    /* call handler function */
    if (ulInterruptID == 30)
    {
        /* Generic Timer */
        FreeRTOS_Tick_Handler();
    }
    else
    {
        // printf("\n%s(): IRQ happend (%u)\n", __func__, ulInterruptID);
        SystemIrqHandler(ulInterruptID);
    }
}