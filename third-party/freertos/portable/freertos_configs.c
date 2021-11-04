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
#include "ft_assert.h"

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


void vApplicationStackOverflowHook(xTaskHandle pxTask, signed char *pcTaskName)
{
  (void) pxTask;
  (void) pcTaskName;

  taskDISABLE_INTERRUPTS();
  FT_ASSERTNONERETURN(FALSE);
  
}

/* configSUPPORT_STATIC_ALLOCATION is set to 1, so the application must provide an
 * implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
 * used by the Idle task. */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  /* If the buffers to be provided to the Idle task are declared inside this
   * function then they must be declared static - otherwise they will be allocated on
   * the stack and so not exists after this function exits. */
  static StaticTask_t xIdleTaskTCB;
  static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

  /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
    state will be stored. */
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

  /* Pass out the array that will be used as the Idle task's stack. */
  *ppxIdleTaskStackBuffer = uxIdleTaskStack;

  /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configMINIMAL_STACK_SIZE is specified in words, not bytes. */
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

/* configSUPPORT_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
 * application must provide an implementation of vApplicationGetTimerTaskMemory()
 * to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize )
{
  /* If the buffers to be provided to the Timer task are declared inside this
   * function then they must be declared static - otherwise they will be allocated on
   * the stack and so not exists after this function exits. */
  static StaticTask_t xTimerTaskTCB;
  static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

  /* Pass out a pointer to the StaticTask_t structure in which the Timer
    task's state will be stored. */
  *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

  /* Pass out the array that will be used as the Timer task's stack. */
  *ppxTimerTaskStackBuffer = uxTimerTaskStack;

  /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configTIMER_TASK_STACK_DEPTH is specified in words, not bytes. */
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
