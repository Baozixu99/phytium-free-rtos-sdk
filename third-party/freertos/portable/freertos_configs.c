/*
 * @ : Copyright (c) 2021 Phytium Information Technology, Inc. 
 *  
 * SPDX-License-Identifier: Apache-2.0.
 * 
 * @Date: 2021-07-05 08:35:41
 * @LastEditTime: 2021-07-21 19:04:40
 * @Description:  This files is for 
 * 
 * @Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */
#include "FreeRTOS.h"
#include "task.h"
#include "ft_types.h"
#include "parameters.h"
#include "gicv3.h"
#include "generic_timer.h"
#include "interrupt.h"
#include "trap.h"
#include <stdio.h>

void vMainAssertCalled(const char *pcFileName, uint32_t ulLineNumber)
{
    printf("Assert Error is %s : %d \r\n", pcFileName, ulLineNumber);
    for (;;)
        ;
}

void vApplicationMallocFailedHook(void)
{
    printf("Malloc Failed\r\n");
    while (1)
        ;
}

void vApplicationTickHook(void)
{

}

void vApplicationIdleHook(void)
{

}

u32 PlatformGetGicDistBase(void)
{
    return GICV3_BASEADDRESS;
}

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

    // Set as the lowest priority
    InterruptSetPriority(GENERIC_TIMER_NS_IRQ_NUM, configKERNEL_INTERRUPT_PRIORITY);
    InterruptUmask(GENERIC_TIMER_NS_IRQ_NUM);

    GenericTimerStart();
}

void vClearTickInterrupt(void)
{
    GenericTimerCompare(cntfrq / configTICK_RATE_HZ);
}

volatile unsigned int gCpuRuntime;

#ifdef __aarch64__
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
        gCpuRuntime++;
        FreeRTOS_Tick_Handler();
    }
    else
    {
        SystemIrqHandler(ulInterruptID);
    }
}
#else
void vApplicationFPUSafeIRQHandler(uint32_t ulICCIAR)
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
        gCpuRuntime++;
        FreeRTOS_Tick_Handler();
        
    }
    else
    {
        SystemIrqHandler(ulInterruptID);
    }    
}
#endif



void InitIrq()
{
    /* interrupt init */
    ArmGicRedistAddressSet(0, GICV3_RD_BASEADDRESS + 0, 0);
    InterruptInit(GICV3_BASEADDRESS, 0);
}