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
 * FilePath: fexception.h
 * Date: 2022-02-10 14:53:41
 * LastEditTime: 2022-02-17 17:29:56
 * Description:  This file contains low-level driver functions for the processor exception
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe      2021/7/3    first release
 * 1.1   zhugengyu	  2022/6/5	  add debugging information	
 * 1.2   wangxiaodong 2023/2/23	  add nested interrupt enable and disable	
 */


#ifndef ARCH_ARMV8_AARCH32_EXCEPTION_H
#define ARCH_ARMV8_AARCH32_EXCEPTION_H

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/
#include "ftypes.h"
#include "fassert.h"
#include "fcpumode.h"

/************************** Constant Definitions *****************************/
#define FEXC_FRAME_SIZE 68U  /* sizeof(FExcFrame) */
#define FEXC_IRQ_MASK   0x80 /* bit 7 of CPSR */

/****************************************************************************/
/**
* @brief	Enable nested interrupts by setting the IRQ not mask bit, and the PE mode to System in CPSR.
* @note     Caution: 
            This macro must be used in interrupt handler.
            The value parameter must be a global or static variable, not a local variable.
            Use simple operations between FInterruptNestedEnable and FInterruptNestedDisable.
******************************************************************************/
#define FInterruptNestedEnable(value)                                              \
    __asm__ __volatile__("mov %0, lr" : "=r"(value[0]));                           \
    __asm__ __volatile__("mrs %0, spsr" : "=r"(value[1]));                         \
    __asm__ __volatile__("msr cpsr_c, %0" ::"i"((~FEXC_IRQ_MASK) & ARM_SYS_MODE)); \
    __asm__ __volatile__("mov %0, lr" : "=r"(value[2]));

/****************************************************************************/
/**
* @brief	Disable the nested interrupts by setting the the IRQ mask bit, and the PE mode to IRQ in CPSR.
* @note     Caution: 
            This macro must be used in interrupt handler.
            The value parameter must be a global or static variable, not a local variable.
            Use simple operations between FInterruptNestedEnable and FInterruptNestedDisable.
******************************************************************************/
#define FInterruptNestedDisable(value)                                             \
    __asm__ __volatile__("mov lr,%0 " ::"r"(value[2]));                            \
    __asm__ __volatile__("msr cpsr_c, %0" ::"i"(FEXC_IRQ_MASK | NESTED_IRQ_MODE)); \
    __asm__ __volatile__("msr spsr, %0" ::"r"(value[1]));                          \
    __asm__ __volatile__("mov lr,%0 " ::"r"(value[0]));

/**************************** Type Definitions *******************************/
typedef struct
{
    u32 r0;
    u32 r1;
    u32 r2;
    u32 r3;
    u32 r4;
    u32 r5;
    u32 r6;
    u32 r7;
    u32 r8;
    u32 r9;
    u32 r10;
    u32 r11;
    u32 r12;
    u32 sp;
    u32 lr;
    u32 pc;
    u32 cpsr;
} FExcFrame;

FASSERT_STATIC(sizeof(FExcFrame) == FEXC_FRAME_SIZE);

typedef void (*FExcInterruptEndHandler)(void);
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
void FExcRegisterDataAbortEndHandler(FExcInterruptEndHandler handler);

/************************** Variable Definitions *****************************/

/*****************************************************************************/
void FExceptionInterruptHandler(void *temp);

#ifdef __cplusplus
}
#endif

#endif