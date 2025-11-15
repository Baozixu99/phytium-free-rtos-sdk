/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * Date: 2025-01-14 14:53:42
 * LastEditTime: 2025-03-27 14:53:42
 * Description:  This file is for
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0   LiuSM      2025-01-14  First version.
 */

#ifndef BSP_BOARD_PS2316_PARAMETERS_H
#define BSP_BOARD_PS2316_PARAMETERS_H

#ifdef __cplusplus
extern "C"
{
#endif

#if !defined(__ASSEMBLER__)
#include "ftypes.h"
#endif

#define CORE0_AFF                      0x0
#define CORE1_AFF                      0x10000
#define CORE2_AFF                      0x20000
#define CORE3_AFF                      0x30000
#define CORE4_AFF                      0x40000
#define CORE5_AFF                      0x50000
#define CORE6_AFF                      0x60000
#define CORE7_AFF                      0x70000
#define CORE8_AFF                      0x80000
#define CORE9_AFF                      0x90000
#define CORE10_AFF                     0xa0000
#define CORE11_AFF                     0xb0000
#define CORE12_AFF                     0xc0000
#define CORE13_AFF                     0xd0000
#define CORE14_AFF                     0xe0000
#define CORE15_AFF                     0xf0000

#define FCPU_CONFIG_ARM64_PA_BITS      47

#define FCORE_NUM                      16
#define CORE_AFF_MASK                  0xFFFFF


/* cache */
#define CACHE_LINE_ADDR_MASK           0x3FUL
#define CACHE_LINE                     64U


/* PCIE ECAM */

/* Pci express  */
#define FPCIE_ECAM_INSTANCE_NUM        1
#define FPCIE_ECAM_INSTANCE0           0

#define FPCIE_ECAM_MAX_OUTBOUND_NUM    8

/* Bus, Device and Function */
#define FPCIE_ECAM_CFG_MAX_NUM_OF_BUS  256
#define FPCIE_ECAM_CFG_MAX_NUM_OF_DEV  32
#define FPCIE_ECAM_CFG_MAX_NUM_OF_FUN  8

#define FPCIE_ECAM_INTA_IRQ_NUM        60
#define FPCIE_ECAM_INTB_IRQ_NUM        61
#define FPCIE_ECAM_INTC_IRQ_NUM        62
#define FPCIE_ECAM_INTD_IRQ_NUM        63

/*  max scan*/
#define FPCIE_MAX_SCAN_NUMBER          128

/* memory space */
#define FPCI_ECAM_CONFIG_BASE_ADDR     0x40000000 /* ecam */
#define FPCI_ECAM_CONFIG_REG_LENGTH    0x10000000

#define FPCI_ECAM_IO_CONFIG_BASE_ADDR  0x50000000 /* io address space */
#define FPCI_ECAM_IO_CONFIG_REG_LENGTH 0x08000000

#define FPCI_ECAM_MEM32_BASE_ADDR      0x58000000 /* mmio 32 */
#define FPCI_ECAM_MEM32_REG_LENGTH     0x27ffffff

#define FPCI_ECAM_MEM64_BASE_ADDR      0x40000000000 /* mmio 64 */
#define FPCI_ECAM_MEM64_REG_LENGTH     0x1fffffffff

/* ACPI */
#define FACPI_CPU_MAX_FREQ_MHZ         2100U
#define FACPI_CPU_MIN_FREQ_MHZ         50U

/* SCMI and MHU */
#define FSCMI_MHU_BASE_ADDR            0x38001020
#define FSCMI_MHU_IRQ_ADDR             0x3800150c
#define FSCMI_MHU_IRQ_NUM              (16U + 42U)
#define FSCMI_SHR_MEM_ADDR             0x38004C00

#define FSCMI_MSG_SIZE                 128
#define FSCMI_MAX_STR_SIZE             16
#define FSCMI_MAX_NUM_SENSOR           16
#define FSCMI_MAX_CLOCK_RATES          16
#define FSCMI_MAX_PROTOCOLS_IMP        16
#define FSCMI_MAX_PERF_DOMAINS         8
#define FSCMI_MAX_OPPS                 14
#define FSCMI_MAX_POWER_DOMAINS        40
#define FSCMI_MAX_CLOCK_DOMAINS        32
#define FSCMI_MAX_RESET_DOMAINS        32

/* UART */
#define FUART0_ID                      0
#define FUART1_ID                      1
#define FUART2_ID                      2
#define FUART_NUM                      3

#define FUART0_IRQ_NUM                 48
#define FUART0_BASE_ADDR               0x20000000
#define FUART0_CLK_FREQ_HZ             48000000

#define FUART1_IRQ_NUM                 47
#define FUART1_BASE_ADDR               0x20001000
#define FUART1_CLK_FREQ_HZ             48000000

#define FUART2_IRQ_NUM                 46
#define FUART2_BASE_ADDR               0x20002000
#define FUART2_CLK_FREQ_HZ             48000000

#define FT_STDOUT_BASE_ADDR            FUART1_BASE_ADDR
#define FT_STDIN_BASE_ADDR             FUART1_BASE_ADDR


/* GPIO */
#if !defined(__ASSEMBLER__)
enum
{
    FGPIO0_ID = 0,

    FGPIO_NUM
};
#endif

#define FGPIO_WITH_PIN_IRQ             1U /* max id of gpio assign irq for each pin */

#define FGPIO0_BASE_ADDR               0x2000A000

#define FGPIO_CTRL_PIN_NUM             16U

#define FGPIO_PIN_IRQ_BASE             37U
#define FGPIO_PIN_IRQ_NUM_GET(id, pin) (FGPIO_PIN_IRQ_BASE)

/* iopad */
#define FIOPAD_BASE_ADDR               0x38003000U

#if !defined(__ASSEMBLER__)
/* IOPAD */
enum
{
    FIOPAD0_ID = 0,

    FIOPAD_NUM
};
#endif

/* PMU */
#define FPMU_IRQ_NUM 23

/* QSPI */
#if !defined(__ASSEMBLER__)
enum
{
    FQSPI0_ID = 0,

    FQSPI_NUM
};

#define FQSPI_BASE_ADDR 0x1A100000U

/* FQSPI cs 0_3, chip number */
enum
{
    FQSPI_CS_0 = 0,
    FQSPI_CS_1 = 1,
    FQSPI_CS_2 = 2,
    FQSPI_CS_3 = 3,
    FQSPI_CS_NUM
};

#endif

#define FQSPI_MEM_START_ADDR      0x0U
#define FQSPI_MEM_END_ADDR        0x07FFFFFFU /* 128MB */
#define FQSPI_CAP_FLASH_NUM_MASK  GENMASK(4, 3)
#define FQSPI_CAP_FLASH_NUM(data) ((data) << 3) /* Flash number */

/* WDT */
#if !defined(__ASSEMBLER__)
enum
{
    FWDT0_ID = 0,
    FWDT1_ID,

    FWDT_NUM
};
#endif

#define FWDT0_REFRESH_BASE_ADDR     0x1A102000U
#define FWDT1_REFRESH_BASE_ADDR     0x1A104000U

#define FWDT_CONTROL_BASE_ADDR(x)   ((x) + 0x1000)

#define FWDT0_IRQ_NUM               50
#define FWDT1_IRQ_NUM               49

#define FWDT_CLK_FREQ_HZ            48000000U /* 48MHz */

/****** GIC v3  *****/
#define FT_GICV3_INSTANCES_NUM      1U
#define GICV3_REG_LENGTH            0x00009000U

/*
 * The maximum priority value that can be used in the GIC.
 */
#define GICV3_MAX_INTR_PRIO_VAL     240U
#define GICV3_INTR_PRIO_MASK        0x000000f0U

#define ARM_GIC_NR_IRQS             270U
#define ARM_GIC_IRQ_START           0U
#define FGIC_NUM                    1U

#define ARM_GIC_IPI_COUNT           16U /* MPCore IPI count         */
#define SGI_INT_MAX                 16U
#define SPI_START_INT_NUM           32U   /* SPI start at ID32        */
#define PPI_START_INT_NUM           16U   /* PPI start at ID16        */
#define GIC_INT_MAX_NUM             1020U /* GIC max interrupts count */

#define GICV3_BASE_ADDR             0x22000000U
#define GICV3_ITS_BASE_ADDR         (GICV3_BASE_ADDR + 0X40000U)
#define GICV3_DISTRIBUTOR_BASE_ADDR (GICV3_BASE_ADDR + 0)
#define GICV3_RD_BASE_ADDR          (GICV3_BASE_ADDR + 0x0C0000U)
#define GICV3_RD_OFFSET             (2U << 16)
#define GICV3_RD_SIZE               (16U << 17)
#define FT_GICV3_VECTORTABLE_NUM    GIC_INT_MAX_NUM


/* generic timer */
/* non-secure physical timer int id */
#define GENERIC_TIMER_NS_IRQ_NUM    30U

/* virtual timer int id */
#define GENERIC_VTIMER_IRQ_NUM      27U

#if !defined(__ASSEMBLER__)
enum
{
    GENERIC_TIMER_ID0 = 0, /* non-secure physical timer */
    GENERIC_TIMER_ID1 = 1, /* virtual timer */

    GENERIC_TIMER_NUM
};
#endif

#ifdef __cplusplus
}
#endif

#endif // !