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
 * FilePath: fparameters.h
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:58:51
 * Description:  This file is for
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */

#ifndef QEMU_VIRT_FPARAMETERS_H
#define QEMU_VIRT_FPARAMETERS_H

#ifdef __cplusplus
extern "C"
{
#endif

#if !defined(__ASSEMBLER__)
#include "ftypes.h"
#endif

#define SOC_TARGET_QEMU_VIRT

#define CORE0_AFF                   0x0
#define CORE1_AFF                   0x1
#define CORE2_AFF                   0x2
#define CORE3_AFF                   0x3
#define CORE_AFF_MASK               0xF
#define FCORE_NUM                   4
#define FCPU_CONFIG_ARM64_PA_BITS   44
/* cache */
#define CACHE_LINE_ADDR_MASK        0x3FUL
#define CACHE_LINE                  64U

/* UART */
#define FUART0_ID                   0
#define FUART1_ID                   1
#define FUART2_ID                   2
#define FUART_NUM                   3


#define FUART0_IRQ_NUM              33
#define FUART0_BASE_ADDR            0x09000000
#define FUART0_CLK_FREQ_HZ          24000000

#define FUART1_IRQ_NUM              42
#define FUART1_BASE_ADDR            0x09001000
#define FUART1_CLK_FREQ_HZ          24000000

#define FUART2_IRQ_NUM              43
#define FUART2_BASE_ADDR            0x09002000
#define FUART2_CLK_FREQ_HZ          24000000

#define FT_STDOUT_BASE_ADDR         FUART0_BASE_ADDR
#define FT_STDIN_BASE_ADDR          FUART0_BASE_ADDR

/* GIC v3 */
#define ARM_GIC_NR_IRQS             1024
#define ARM_GIC_IRQ_START           0
#define FGIC_NUM                    1


#define GICV3_BASE_ADDR             0x08000000
#define GICV3_DISTRIBUTOR_BASE_ADDR (GICV3_BASE_ADDR + 0)
#define GICV3_RD_BASE_ADDR          (0x080A0000)
#define GICV3_RD_OFFSET             (2U << 16)
#define GICV3_RD_SIZE               (16U << 16)

#define GICV3_ITS_BASE_ADDR         0x08080000

/*
 * The maximum priority value that can be used in the GIC.
 */
#define GICV3_MAX_INTR_PRIO_VAL     240U
#define GICV3_INTR_PRIO_MASK        0x000000f0U

#define ARM_GIC_IPI_COUNT           16 /* MPCore IPI count  */
#define SGI_INT_MAX                 16
#define SPI_START_INT_NUM           32   /* SPI start at ID32        */
#define PPI_START_INT_NUM           16   /* PPI start at ID16        */
#define GIC_INT_MAX_NUM             1020 /* GIC max interrupts count */

/* generic timer */
/* non-secure physical timer int id */
#define GENERIC_TIMER_NS_IRQ_NUM    30U

/* virtual timer int id */
#define GENERIC_VTIMER_IRQ_NUM      27U


#define GENERIC_TIMER_ID0           0 /* non-secure physical timer */
#define GENERIC_TIMER_ID1           1 /* virtual timer */
#define GENERIC_TIMER_NUM           2

/* PMU */
#define FPMU_IRQ_NUM                23


#ifdef __cplusplus
}
#endif

#endif // !