/*
 * @ : Copyright (c) 2021 Phytium Information Technology, Inc. 
 *  
 * SPDX-License-Identifier: Apache-2.0.
 * 
 * @Date: 2021-07-05 08:35:41
 * @LastEditTime: 2021-07-05 11:35:19
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
    Aarch32TimerStop();
    // Get system frequency
    cntfrq = Aarch32TimerFrequecy();

    // Set tick rate
    Aarch32TimerCompare(cntfrq / configTICK_RATE_HZ);
    Aarch32TimerInterruptEnable();

    // Set as the lowest priority
    InterruptSetPriority(GEN_TIMER_PHYSICAL_NOSECURE_IRQN, configKERNEL_INTERRUPT_PRIORITY);
    InterruptUmask(GEN_TIMER_PHYSICAL_NOSECURE_IRQN);

    Aarch32TimerStart();
    printf("tick setup done\r\n");
}

void vClearTickInterrupt(void)
{
    Aarch32TimerCompare(cntfrq / configTICK_RATE_HZ);
}

volatile unsigned int gCpuRuntime;
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
        //printf("run time %d\r\n", gCpuRuntime);
        FreeRTOS_Tick_Handler();
    }
    else
    {
        printf("\n%s(): IRQ happend (%u)\n", __func__, ulInterruptID);
        SystemIrqHandler(ulInterruptID);
    }    
}

void InitIrq()
{
    /* interrupt init */
    ArmGicRedistAddressSet(0, GICV3_RD_BASEADDRESS + 0, 0);
    InterruptInit();
}