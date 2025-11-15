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
 * FilePath: crt0.S
 * Date: 2022-02-10 14:53:41
 * LastEditTime: 2022-02-17 17:28:18
 * Description:  This file is for 32bit C run-time code
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  huanghe   2021-11       initialization
 *  1.1  zhugengyu	2022/06/05		add debugging information		   
 */


#include "sdkconfig.h"
.set FPEXC_EN,		0x40000000		/* FPU enable bit, (1 << 30) */
    .section ".got2","aw"
    .align 2

    .text
.LsbssStart:
    .long __sbss_start__

.LsbssEnd:
    .long __sbss_end__


.LbssStart:
    .long __bss_start__

.LbssEnd:
    .long __bss_end__


.LdataStart:
    .long __data_start__

.LdataEnd:
    .long  __data_end__

.LRomEnd:
    .long __rom_end__

    .globl _startup
_startup:
    mov r0,#0

/* clear sbss  */
ldr r1,.LsbssStart
ldr r2,.LsbssEnd

.LloopsBss:
	cmp	r1,r2
	bge	.LenclsBss		    /* If no SBSS, no clearing required */
	str	r0, [r1], #4
	b	.LloopsBss

.LenclsBss:
    /* clear bss */
	ldr	r1,.LbssStart		/* calculate beginning of the BSS */
	ldr	r2,.LbssEnd		    /* calculate end of the BSS */

.LloopBss:
	cmp	r1,r2
	bge	.LenclBss		/* If no BSS, no clearing required */
	str	r0, [r1], #4
	b	.LloopBss

.LenclBss:

    ldr r0,.LRomEnd
    ldr r1,.LdataStart
    ldr r2,.LdataEnd

.LdataCopy:
    cmp r1,r2
    ldrlt r3, [r0], #4 /* t load 4 bytes of data from ROM into R3 */
    strlt r3, [r1], #4
    blt .LdataCopy
    
    /* Enable access to FP registers. */
    mov r1, #0xfffffff
    mcr p15, 0, r1, c1, c0, 2 /* CPACR full access to cp11 and cp10. */

    /* cache flush */

    #ifndef CONFIG_USE_AARCH64_L1_TO_AARCH32

    bl FCacheDCacheFlush

    #endif

    /* enable vfp */
    vmrs	r1, FPEXC			/* read the exception register */
	orr	r1,r1, #FPEXC_EN		/* set VFP enable bit, leave the others in orig state */
	vmsr	FPEXC, r1			/* write back the exception register */

	/* enable MMU and cache */
	bl MmuInit
    bl FEarlyUartProbe
    
    bl InterruptEarlyInit
    bl FPsciInit
    bl FBootCore
    cpsie   i                   /* enable irq */

#ifdef CONFIG_ENABLE_CXX
    /* Call constructors of all cpp global objects */
    bl  FCxxInitGlobals
#endif

    bl  FTraceCEntry

    /* make sure argc and argv are valid */
	mov	r0, #0
	mov	r1, #0
    
    bl main

#ifdef CONFIG_ENABLE_CXX
    /* Call deconstructors of all cpp global objects */
    bl  FCxxDeInitGlobals
#endif

/* print after main.c end */
.weak FEndPrint

.Lexit: /* should never get here */
    b FEndPrint

.end:
    b .end