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
 * FilePath: sys.c
 * Date: 2022-01-03 13:04:02
 * LastEditTime: 2022-01-06 21:48:13
 * Description:  This file is for  machine specific system primitives implementation.
 * 
 * Modify History: 
 *  Ver   Who  Date   Changes
 * ----- ------  -------- --------------------------------------
 * 1.0	huanghe 2022/02/05 first release
 */


#include <metal/compiler.h>
#include <metal/io.h>
#include <metal/sys.h>
#include <stdint.h>
#include "faarch.h"
#include "fmmu.h"
#include "fcache.h"

#define _DISABLE_INTERRUPTS()           \
	__asm volatile("MSR DAIFSET, #2" :: \
					   : "memory");     \
	__asm volatile("DSB SY");           \
	__asm volatile("ISB SY");

#define _ENABLE_INTERRUPTS()            \
	__asm volatile("MSR DAIFCLR, #2" :: \
					   : "memory");     \
	__asm volatile("DSB SY");           \
	__asm volatile("ISB SY");

void sys_irq_restore_enable(unsigned int flags)
{
#ifdef __aarch64__
	_ENABLE_INTERRUPTS();
#else
	MTCPSR(flags);
#endif
}

unsigned int sys_irq_save_disable(void)
{
	unsigned int state = 0;

#ifdef __aarch64__
	_DISABLE_INTERRUPTS();
#else
	state = MFCPSR();
	MTCPSR(state | 0xc0);
#endif

	return state;
}

void metal_machine_cache_flush(void *addr, unsigned int len)
{

	if (!addr && !len)
		FCacheDCacheFlush();
	else
		FCacheDCacheFlushRange((uintptr_t)addr, len);
	
}

void metal_machine_cache_invalidate(void *addr, unsigned int len)
{
	if (!addr && !len)
		FCacheDCacheInvalidate();
	else
		FCacheDCacheInvalidateRange((uintptr_t)addr, len);
}

/**
 * @brief poll function until some event happens
 */
void metal_weak metal_generic_default_poll(void)
{
	metal_asm volatile("wfi");
}



void *metal_machine_io_mem_map(void *va, metal_phys_addr_t pa,
							   size_t size, unsigned int flags)
{	
	if (!flags)
		return va;
		
	FMmuMap(pa,pa,size,flags) ;
	return va;
}

