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
 * FilePath: finterrupt.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 08:24:20
 * Description:  This file is for interrupt functionality related apis
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe    2021/4/1       first release
 */

#include <stdio.h>
#include <string.h>
#include "finterrupt.h"
#include "sdkconfig.h"
#include "fcpu_info.h"
#include "fparameters.h"
#include "fassert.h"
#include "fprintk.h"
#include "fdebug.h"
#include "fgic_v3.h"
#include "fgic_its.h"
#include "fgeneric_timer.h"
#include "fsleep.h"
#include "fcpu_info.h"

#define MAX_HANDLERS 1024

#ifdef CONFIG_USE_VIRTUAL_GTIMER

#define RPR_TEST_TIMER_ID      GENERIC_TIMER_ID1
#define RPR_TEST_TIMER_IRQ_NUM GENERIC_VTIMER_IRQ_NUM

#else

#define RPR_TEST_TIMER_ID      GENERIC_TIMER_ID0
#define RPR_TEST_TIMER_IRQ_NUM GENERIC_TIMER_NS_IRQ_NUM

#endif

#define TIMER_PRIORITY 8

u32 rpr_array[2] = {0};


static u8 need_translate = 0;

/* exception and interrupt handler table */
struct IrqDesc isr_table[MAX_HANDLERS];
struct IrqDesc lpi_isr_table[FGIC_DEVICE_MAX_ID * FGIC_ITS_DEVICEID_MAX_EVENT];
static InterruptDrvType *interrupt_handler_p = NULL; /* pointer to  */
static InterruptDrvType interrupt_instance;

/**
 * @name: InterruptMask
 * @msg:  Close the corresponding interrupt based on the interrupt ID
 * @param {int} int_id is interrupt id
 */
void InterruptMask(int int_id)
{
    FASSERT_MSG(interrupt_handler_p != NULL, "Please init interrupt component");
    FGicIntDisable(interrupt_handler_p, int_id);
}


/**
 * @name: InterruptUmask
 * @msg:  This function will un-mask a interrupt.
 * @param {int} int_id is interrupt id
 */
void InterruptUmask(int int_id)
{
    FASSERT_MSG(interrupt_handler_p != NULL, "Please init interrupt component");
    FGicIntEnable(interrupt_handler_p, int_id);
}

int InterruptGetAck(void)
{
    return FGicAcknowledgeInt();
}

void InterruptDeactivation(int int_id)
{
    FGicDeactionInterrupt(int_id);
}

/**
 * @name: InterruptSetTargetCpus
 * @msg:  Route interrupts to specific cpu, or to all cpus
 * @param {int} int_id  is interrupt id ，id range is 32-1019
 * @param {u32} cpu_id is The number to be routed to the CPU, if the value is INTERRUPT_CPU_TARGET_ALL_SET, to all cpus on the chip that can receive this interrupt
 * @return {FError}  FINT_SUCCESS: the setting is successful. FINT_INT_NUM_NOT_FIT: The interrupt number is inconsistent with the actual situation. FINT_SET_TARGET_ERR: The CPU does not have this ID when the CPU ID is involved
 */
FError InterruptSetTargetCpus(int int_id, u32 cpu_id)
{
    u64 cluster, temp_cluster;
    FError ret;

    FASSERT_MSG(interrupt_handler_p != NULL, "Please init interrupt component");

    if (cpu_id == INTERRUPT_CPU_TARGET_ALL_SET)
    {
        ret = FGicSetSpiAffinityRouting(interrupt_handler_p, int_id, SPI_ROUTING_TO_ANY, 0);

        if (ret != FGIC_SUCCESS)
        {
            f_printk("FGicSetSpiAffinityRouting all cpu set is error");
            return FINT_INT_NUM_NOT_FIT;
        }
    }
    else
    {
        if (GetCpuAffinity(cpu_id, &cluster) != ERR_SUCCESS)
        {
            f_printk("This cpu num[%d] is not in board", cpu_id);
            return FINT_SET_TARGET_ERR;
        }

        /* Change the format of the cluster to that required by the API */
        temp_cluster = (cluster >> 24) & 0xFFULL;
        cluster &= ~(0xFFULL << 24);
        cluster |= temp_cluster;
        ret = FGicSetSpiAffinityRouting(interrupt_handler_p, int_id, SPI_ROUTING_TO_SPECIFIC, cluster);

        if (ret != FGIC_SUCCESS)
        {
            f_printk("FGicSetSpiAffinityRouting specific set is error");
            return FINT_INT_NUM_NOT_FIT;
        }
    }

    return FINT_SUCCESS;
}


/**
 * @name: InterruptGetTargetCpus
 * @msg:  Obtain the interrupt routing information based on the interrupt ID
 * @param {int} int_id is interrupt id
 * @param {u32} *cpu_p to get routing cpu id
 * @return {FError} FINT_SUCCESS: the setting is successful. FINT_INT_NUM_NOT_FIT: The interrupt number is inconsistent with the actual situation. FINT_SET_TARGET_ERR: The CPU does not have this ID when the CPU ID is involved
 */
FError InterruptGetTargetCpus(int int_id, u32 *cpu_p)
{
    SPI_ROUTING_MODE route_mode;
    u64 affinity, temp_affinity;
    FError ret;
    FASSERT_MSG(interrupt_handler_p != NULL, "Please init interrupt component");
    ret = FGicGetAffinityRouting(interrupt_handler_p, int_id, &route_mode, &affinity);

    if (ret != FGIC_SUCCESS)
    {
        f_printk("FGicGetAffinityRouting is error");
        return FINT_INT_NUM_NOT_FIT;
    }

    if (route_mode == SPI_ROUTING_TO_ANY)
    {
        *cpu_p = INTERRUPT_CPU_TARGET_ALL_SET;
    }
    else
    {
        /* Change the format of the affinity level to that required by the API */
        temp_affinity = (affinity >> 24) & 0xFFULL;
        affinity &= ~(0xFFULL << 24);
        affinity |= temp_affinity;

        ret = UseAffinityGetCpuId(affinity, cpu_p);

        if (ret != ERR_SUCCESS)
        {
            f_printk("UseAffinityGetCpuId is error");
            return FINT_INT_NUM_NOT_FIT;
        }
    }

    return FINT_SUCCESS;
}


/**
 * @name: void InterruptSetTrigerMode(int int_id, unsigned int mode)
 * @msg:  This function set interrupt triger mode.
 * @param {int} int_id is the interrupt number
 * @param {unsigned int} mode is interrupt triger mode; 0: level triger, 1: edge triger
 */
void InterruptSetTrigerMode(int int_id, unsigned int mode)
{
    FASSERT_MSG(interrupt_handler_p != NULL, "Please init interrupt component");
    FGicSetTriggerLevel(interrupt_handler_p, int_id, mode);
}

/**
 * This function get interrupt triger mode.
 * @param int_id: the interrupt number
 * @return interrupt triger mode; 0: level triger, 1: edge triger
 */
unsigned int InterruptGetTrigerMode(int int_id)
{
    FASSERT_MSG(interrupt_handler_p != NULL, "Please init interrupt component");
    return FGicGetTriggerLevel(interrupt_handler_p, int_id);
}

/**
 * @name: InterruptSetPriority
 * @msg:  This function set interrupt priority value.
 * @param {int} int_id is the interrupt number
 * @param {unsigned int} priority ,use IRQ_PRIORITY_VALUE_*
 */
void InterruptSetPriority(int int_id, unsigned int priority)
{
    FASSERT_MSG(interrupt_handler_p != NULL, "Please init interrupt component");
    FGicSetPriority(interrupt_handler_p, int_id, (priority << IRQ_PRIORITY_OFFSET));
}

/**
 * This function get interrupt priority.
 * @param int_id: the interrupt number
 * @return interrupt priority value
 */
unsigned int InterruptGetPriority(int int_id)
{
    FASSERT_MSG(interrupt_handler_p != NULL, "Please init interrupt component");
    return FGicGetPriority(interrupt_handler_p, int_id);
}


/**
 * @name: InterruptSetPriorityMask
 * @msg:  Set the priority mask
 * @param {unsigned int} priority :Use IRQ_PRIORITY_MASK_*,If the priority of an interrupt is higher than the value indicated by this field, the interface signals the interrupt to the PE.
 * The reference values  of priority are as follows
 * |priority_mask---------------256-------254--------252------248-------240
 * |Implemented priority bits---[7:0]----[7:1]------[7:2]-----[7:3]-----[7:4]
 * |priority the growing steps--any-----even value----4---------8--------16
 */
void InterruptSetPriorityMask(unsigned int priority)
{
    if (need_translate == 1)
    {
        priority = PRIORITY_TRANSLATE_SET(priority);
    }
    FGicSetPriorityFilter(priority);
}

/**
 * @name: InterruptGetPriorityMask
 * @msg:  get the priority mask
 * @return priority :If the priority of an interrupt is higher than the value indicated by this field, the interface signals the interrupt to the PE.
 * The reference values  of priority are as follows
 * |priority_mask---------------256-------254--------252------248-------240
 * |Implemented priority bits---[7:0]----[7:1]------[7:2]-----[7:3]-----[7:4]
 * |priority the growing steps--any-----even value----4---------8--------16
 */
unsigned int InterruptGetPriorityMask(void)
{
    u32 priority = FGicGetPriorityFilter();
    if (need_translate == 1)
    {
        priority = PRIORITY_TRANSLATE_GET(priority);
    }
    return (unsigned int)priority;
}

/**
 * @name: InterruptGetCurrentPriority
 * @msg:  Get current interrupt priority ICC_RPR and translate when interrupt occur
 * @param {void} 
 * @return {u32} current interrupt priority
 */
u32 InterruptGetCurrentPriority(void)
{
    u32 icc_rpr = FGicGetICC_RPR();
    if (need_translate == 1)
    {
        return PRIORITY_TRANSLATE_GET(icc_rpr);
    }
    else
    {
        return icc_rpr;
    }
}

/**
 * @name: InterruptSetPriorityGroupBits
 * @msg:  Sets the interrupt group priority bit
 * @param {unsigned int} bits use IRQ_GROUP_PRIORITY_*
 */
void InterruptSetPriorityGroupBits(unsigned int bits)
{
    FGicSetPriorityGroup(bits);
}

/**
 * This function get priority grouping field split point.
 * @param none
 * @return priority grouping field split point
 */
unsigned int InterruptGetPriorityGroupBits(void)
{
    unsigned int bp;
    bp = FGicGetPriorityGroup() & 0x07;
    return bp;
}


/**
 * @name: InterruptInstall
 * @msg:  This function registers the custom interrupt callback function and callback parameters into the corresponding interrupt ID data structure
 * @param {int} int_id is the interrupt number
 * @param {IrqHandler} handler is interrupt the callback function
 * @param {void} *param is interrupt the callback paramters
 * @param {char} *name
 * @return {*}
 */
void InterruptInstall(int int_id, IrqHandler handler, void *param, const char *name)
{
    (void)name;
    if (int_id < MAX_HANDLERS)
    {
        if (handler != NULL)
        {
            isr_table[int_id].handler = handler;
            isr_table[int_id].param = param;
        }
    }
    else if (int_id >= FGIC_LPI_START_ID)
    {
        lpi_isr_table[int_id - FGIC_LPI_START_ID].handler = handler;
        lpi_isr_table[int_id - FGIC_LPI_START_ID].param = param;
    }
}

/**
 * @name: InterruptCoreInterSend
 * @msg:  Intercore interrupt trigger function
 * @param {int} int_id is the interrupt number ，number range is 0~15
 * @param {u64} cpu_mask is each bit of cpu_mask represents a selected CPU, for example, 0x3 represents core0 and CORE1 .
 */
FError InterruptCoreInterSend(int int_id, u64 cpu_mask)
{
    u32 cluster_id, target_list;
    FError ret = FINT_SUCCESS;
    if (cpu_mask == INTERRUPT_CPU_ALL_SELECT)
    {
        ret = FGicGenerateSgi(int_id, 0, SGI_ROUTING_TO_ANY, 0);
    }
    else
    {
        while (GetCpuMaskToAffval((u32 *)&cpu_mask, &cluster_id, &target_list))
        {
            ret = FGicGenerateSgi(int_id, target_list, SGI_ROUTING_TO_SPECIFIC,
                                  ((((cluster_id >> 8) & 0xFFULL) << FGIC_RSGI_AFF1_OFFSET) |
                                   (((cluster_id >> 16) & 0xFFULL) << FGIC_RSGI_AFF2_OFFSET) |
                                   (((cluster_id >> 24) & 0xFFull) << FGIC_RSGI_AFF3_OFFSET)));
        }
    }

    return (ret == FGIC_SUCCESS) ? FINT_SUCCESS : FINT_SET_TARGET_ERR;
}

/**
 * @name: InterruptEarlyInit
 * @msg:  Interrupt preinitialization function. This function is usually called in assembly code. When the user sets the default initialization mode, this function will use CORE0 as the main CORE and initialize all components in the interrupt driver, and other cores as slave cores will initialize the necessary components in the interrupt driver.
 * @return {*}
 */
void InterruptEarlyInit(void)
{

#if defined(CONFIG_USE_DEFAULT_INTERRUPT_CONFIG)

#if defined(CONFIG_INTERRUPT_ROLE_SLAVE)
    InterruptInit(&interrupt_instance, INTERRUPT_DRV_INTS_ID, INTERRUPT_ROLE_SLAVE);
#elif defined(CONFIG_INTERRUPT_ROLE_MASTER)
    InterruptInit(&interrupt_instance, INTERRUPT_DRV_INTS_ID, INTERRUPT_ROLE_MASTER);
#else

#endif

#endif
}


/**
 * @name: InterruptStepHandler
 * @msg:  Generic timer interrupt handler function, executed twice
 * @param vector Irq num, Don't need attention
 * @param param  contains a pointer to the driver instance
 * @return void
 */
static void InterruptStepHandler(s32 vector, void *param)
{
    static u16 interrupt_count = 0;
    if (vector != RPR_TEST_TIMER_IRQ_NUM)
    {
        return;
    }
    rpr_array[interrupt_count] = FGicGetICC_RPR();

    interrupt_count++;
    InterruptSetPriority(RPR_TEST_TIMER_IRQ_NUM, TIMER_PRIORITY + 1);
    GenericTimerSetTimerValue(RPR_TEST_TIMER_ID, 1);

    if (interrupt_count == 2)
    {
        GenericTimerStop(RPR_TEST_TIMER_ID);
        GenericTimerInterruptDisable(RPR_TEST_TIMER_ID);
        InterruptMask(RPR_TEST_TIMER_IRQ_NUM);
        interrupt_count = 0;
    }
}

/**
 * @name: InterruptStep
 * @msg:  Get interrupt rpr step, used for priority conversion under different firmware
 * @param void
 * @return {u32} interrupt rpr step
 */
static u32 InterruptStep(void)
{
    /* stop timer */
    GenericTimerStop(RPR_TEST_TIMER_ID);

    /* setup and enable interrupt */
    InterruptSetPriority(RPR_TEST_TIMER_IRQ_NUM, TIMER_PRIORITY);
    InterruptInstall(RPR_TEST_TIMER_IRQ_NUM, InterruptStepHandler, NULL, NULL);
    InterruptUmask(RPR_TEST_TIMER_IRQ_NUM);

    GenericTimerSetTimerValue(RPR_TEST_TIMER_ID, 1000);
    GenericTimerInterruptEnable(RPR_TEST_TIMER_ID);
    GenericTimerStart(RPR_TEST_TIMER_ID);
    INTERRUPT_ENABLE();
    fsleep_microsec(100);
    INTERRUPT_DISABLE();
    GenericTimerStop(RPR_TEST_TIMER_ID);
    GenericTimerInterruptDisable(RPR_TEST_TIMER_ID);
    InterruptMask(RPR_TEST_TIMER_IRQ_NUM);

    u32 step = rpr_array[1] - rpr_array[0];
    return step;
}

/**
 * @name: InterruptInit
 * @msg:  Initializes the interrupt module
 * @param {InterruptDrvType *} int_driver_p is a pointer to the interrupt driver instance.
 * @param {u32} instance_id is driver instance id
 * @param {INTERRUPT_ROLE_SELECT} role_select ,Select the initialization method
 */
void InterruptInit(InterruptDrvType *int_driver_p, u32 instance_id, INTERRUPT_ROLE_SELECT role_select)
{
    FASSERT(int_driver_p != NULL);
    FASSERT(role_select <= INTERRUPT_ROLE_NONE);
    interrupt_handler_p = int_driver_p;

    if (INTERRUPT_ROLE_NONE == role_select)
    {
        return;
    }

    FGicConfig *config_p = FGicLookupConfig(instance_id);
    /* get gic redistributor base address of current core */
    uintptr redist_base = FGicRedistributorIterate();
    FGicCfgInitialize(interrupt_handler_p, config_p, redist_base);

    if (INTERRUPT_ROLE_MASTER == role_select)
    {
        /* initialize exceptions table */
        memset(isr_table, 0x00, sizeof(isr_table));
        /* per cpu core need to run */
        FGicDistrubutiorInit(interrupt_handler_p);
    }

    FASSERT(FGicRedistributorInit(redist_base) == FGIC_SUCCESS);

    if (INTERRUPT_ROLE_MASTER == role_select)
    {
#ifdef CONFIG_ENABLE_GIC_ITS
        FGicLpisSetup(interrupt_handler_p);
#endif
    }

    FGicCpuInterfaceInit();

    /* ICC_BPR1_EL1 use gggg.ssss group field */
    InterruptSetPriorityGroupBits(IRQ_GROUP_PRIORITY_4);

    u32 step = InterruptStep();

    if (step == 8)
    {
        need_translate = 1;
    }
    else if (step == 16)
    {
        need_translate = 0;
    }
    else
    {
        need_translate = 0;
    }

    if (role_select == INTERRUPT_ROLE_MASTER)
    {
#ifdef CONFIG_ENABLE_GIC_ITS
        /* its、lpi init */
        interrupt_handler_p->its.base = interrupt_handler_p->config.its_base;
        FGicItsInit(interrupt_handler_p);
#endif
    }
}

/**
 * @name: InterruptSecondaryInit
 * @msg:  Initializes the secondary core interrupt module
 * @param: void
 * @return: void
 */
void InterruptSecondaryInit(void)
{
    uintptr redist_base = FGicRedistributorIterate();
    FASSERT(FGicRedistributorInit(redist_base) == FGIC_SUCCESS);
    FGicCpuInterfaceInit();

    /* ICC_BPR1_EL1 use gggg.ssss group field */
    InterruptSetPriorityGroupBits(IRQ_GROUP_PRIORITY_4);
}

/**
 * @name: InterruptGetPriorityConfig
 * @msg:  Get current interrupt priority config, whether need translate
 * @param {void} 
 * @return {u8} current interrupt priority
 */
u8 InterruptGetPriorityConfig(void)
{
    return need_translate;
}


void *InterruptGetInstance(void)
{
    return interrupt_handler_p;
}

/**
 * @name: FGicSetSpiMessage
 * @msg:  Send a message interrupt to a specified SPI interrupt(or stop a a message interrupt)
 * @param {int} int id of the irq that message to 
 * @param {boolean} stop intr message: 0, send intr message: 1
 * @return {void} no return
 */
void FGicSetSpiMessage(int int_id, boolean send)
{
    FASSERT_MSG(interrupt_handler_p != NULL, "Please init interrupt component");
    uintptr dist_base = interrupt_handler_p->config.dist_base;
    if (send)
    {
        FGIC_WRITEREG32(dist_base, FGIC_GICD_SETSPI_NSR_OFFSET, FGIC_GICD_SETSPI_NSR_INTID(int_id));
    }
    else
    {
        FGIC_WRITEREG32(dist_base, FGIC_GICD_CLRSPI_NSR_OFFSET, FGIC_GICD_CLRSPI_NSR_INTID(int_id));
    }
}

/**
 * @name: FGicGetMbis
 * @msg:  To tell if MBIS is enable or not
 * @param {void} no param
 * @return {int} 0 for MBIS disable, 1 for MBIS enable
 */
int FGicGetMbis(void)
{
    FASSERT_MSG(interrupt_handler_p != NULL, "Please init interrupt component");
    uintptr dist_base = interrupt_handler_p->config.dist_base;
    u32 reg = FGIC_READREG32(dist_base, FGIC_GICD_TYPER_OFFSET);
    if ((reg & FGIC_GICD_TYPER_MBIS) == FGIC_GICD_TYPER_MBIS)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}