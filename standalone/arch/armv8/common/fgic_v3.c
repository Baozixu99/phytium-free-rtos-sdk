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
 * FilePath: fgic_v3.c
 * Date: 2023-10-7 09:30:23
 * LastEditTime: 2023-107 09:30:24
 * Description: This file is for the minimum required function implementations for this driver.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   wangxiaodong  2023/10/7   init commit
 * 1.1   huanghe       2024/08/09  add lpi
 * 1.2   wangxiaodong  2024/08/16  refine some function
 */
#include "fparameters.h"
#include "fgic_v3.h"
#include "fgic_its.h"
#include "ftypes.h"
#include "fkernel.h"
#include "fassert.h"
#include "fdebug.h"


#define FGIC_DEBUG_TAG          "FGIC_V3"
#define FGIC_ERROR(format, ...) FT_DEBUG_PRINT_E(FGIC_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGIC_DEBUG_I(format, ...) \
    FT_DEBUG_PRINT_I(FGIC_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGIC_DEBUG_D(format, ...) \
    FT_DEBUG_PRINT_D(FGIC_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGIC_DEBUG_W(format, ...) \
    FT_DEBUG_PRINT_W(FGIC_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGIC_DEBUG_E(format, ...) \
    FT_DEBUG_PRINT_E(FGIC_DEBUG_TAG, format, ##__VA_ARGS__)


#define FGIC_SPI_START_ID    32
#define FGIC_PPI_END_ID      31
#define FGIC_SGI_END_ID      15

#define FGIC_GICD_32_PER_REG 32
#define FGIC_GICD_16_PER_REG 16
#define FGIC_GICD_4_PER_REG  4

#define FGIC_INT_DEFAULT_PRI_X4 \
    0xa0a0a0a0 /* 考虑到当前一般程序工作于EL1，对于NS 或 S 安全状态 ，0x80 - 0xff 的优先级都有存在的可能性 */
#define FGIC_CPU_INTERFACE_DEFAULT_FILTER 0xFF

FGicConfig fgic_config[FGIC_NUM] = {{
    .instance_id = 0,                         /* Id of device */
    .dist_base = GICV3_DISTRIBUTOR_BASE_ADDR, /* Distributor base address      */
    .its_base = GICV3_ITS_BASE_ADDR,          /* ITS base address            */
}};

/**
 * @name: FGicLookupConfig
 * @msg:  Gets the default configuration parameters in the current GIC
 * @param {u32} instance_id
 * @return {*}
 */
FGicConfig *FGicLookupConfig(u32 instance_id)
{
    FGicConfig *ptr = NULL;
    u32 index;

    for (index = 0; index < (u32)FGIC_NUM; index++)
    {
        if (fgic_config[index].instance_id == instance_id)
        {
            ptr = &fgic_config[index];
            break;
        }
    }

    return (FGicConfig *)ptr;
}


/**
 * @name: FGicWaitRwp
 * @msg:    Wait for register write pending
 * @param {uintptr} ctrl_base is a GICD_CTLR address
 * @param {WAIT_RWP_MODE} wait_mode
 * @return {*}
 */
FError FGicWaitRwp(uintptr ctrl_base, WAIT_RWP_MODE wait_mode)
{
    u32 rwp_mask;
    u32 timeout_cnt = 0;
    if (GICD_CTLR_RWP_WAIT == wait_mode)
    {
        rwp_mask = (u32)FGIC_GICD_CTLR_RWP_MASK;
    }
    else if (GICR_CTLR_RWP_WAIT == wait_mode)
    {
        rwp_mask = (u32)FGIC_GICR_CTLR_RWP_MASK;
    }
    else
    {
        FGIC_DEBUG_E(" wait_mode not in WAIT_RWP_MODE.");
        return FGIC_CTLR_ERR_TYPE;
    }

    while (FGIC_READREG32(ctrl_base, GICR_CTLR) & rwp_mask)
    {
        if (timeout_cnt++ >= 0xffffff)
        {
            FGIC_DEBUG_E(" Wait rwp timeout.");
            return FGIC_CTLR_ERR_IN_GET;
        }
    }

    return FGIC_SUCCESS;
}

/**
 * @name: FGicDistrubutiorInit
 * @msg:  Initialize Distrubutior
 * @param {FGic} *instance_p is a pointer to the FGic instance.
 */
void FGicDistrubutiorInit(FGic *instance_p)
{
    FASSERT(instance_p);
    FASSERT(instance_p->is_ready == FT_COMPONENT_IS_READY);
    u32 max_ints_mun, int_id;
    uintptr dist_base = instance_p->config.dist_base;
    max_ints_mun = instance_p->max_spi_num;
    /* Disable the distributor */
    FGIC_WRITEREG32(dist_base, FGIC_GICD_CTLR_OFFSET, 0);
    FASSERT(FGicWaitRwp(dist_base + FGIC_GICD_CTLR_OFFSET, GICD_CTLR_RWP_WAIT) == FGIC_SUCCESS);

    if (instance_p->security == ONE_NS_SECURITY_STATE)
    {
        /* Make  GICD_CTLR.DS = 1 ,Non-secure accesses are permitted to access and modify registers that control Group 0 interrupts */
        FGIC_SETBIT(dist_base, FGIC_GICD_CTLR_OFFSET, FGIC_GICD_CTLR_DS_MASK);
        FASSERT(FGIC_READREG32(dist_base, FGIC_GICD_CTLR_OFFSET) & FGIC_GICD_CTLR_DS_MASK);
    }

    /* 关闭所有中断，并将中断分组默认为group 1 */
    for (int_id = FGIC_SPI_START_ID; int_id < max_ints_mun; int_id += FGIC_GICD_32_PER_REG)
    {
        /* Disables forwarding of the corresponding interrupt. */
        FGIC_GICD_ICENABLER_WRITE_N_MASK(dist_base, int_id, FGIC_GICD_ICENABLER_DEFAULT_MASK);
        /*  Changes the state of the corresponding interrupt from pending to inactive, or from active and pending to active. */
        FGIC_GICD_ICPENDR_WRITE_N_MASK(dist_base, int_id, FGIC_GICD_ICPENDR_DEFAULT_MASK);

        if (instance_p->security == ONE_NS_SECURITY_STATE)
        {
            FGIC_GICD_IGROUPR_WRITE_N_MASK(dist_base, int_id, FGIC_GICD_ICPENDR_DEFAULT_MASK);
        }
        else
        {
            FGIC_GICD_IGROUPR_WRITE_N_MASK(dist_base, int_id, FGIC_GICD_ICPENDR_DEFAULT_MASK);
        }
        FGIC_GICD_IGRPMODR_WRITE_N_MASK(dist_base, int_id, FGIC_GICD_IGRPMODR_DEFAULT_MASK);
    }

    FASSERT(FGicWaitRwp(dist_base + FGIC_GICD_CTLR_OFFSET, GICD_CTLR_RWP_WAIT) == FGIC_SUCCESS);

    for (int_id = FGIC_SPI_START_ID; int_id < max_ints_mun; int_id += FGIC_GICD_4_PER_REG)
    {
        FGIC_GICD_IPRIORITYR_WRITE_N_MASK(dist_base, int_id, FGIC_INT_DEFAULT_PRI_X4);
    }

    for (int_id = FGIC_SPI_START_ID; int_id < max_ints_mun; int_id += FGIC_GICD_16_PER_REG)
    {
        FGIC_GICD_ICFGR_WRITE_N_MASK(dist_base, int_id, 0); /* level-sensitive */
    }

    if (instance_p->security == ONE_NS_SECURITY_STATE)
    {
        FGIC_GICD_CTLR_WRITE(dist_base, GICD_CTLR_BIT_ARE_S | GICD_CTLR_ENABLE_GRP1_NS);
    }
    else
    {
        FGIC_GICD_CTLR_WRITE(dist_base, GICD_CTLR_BIT_ARE_NS | GICD_CTLR_ENABLE_GRP1_NS);
    }
}

/**
 * @name: FGicRedistributorInit
 * @msg:  Initialize Redistributor
 * @param {uintptr} redist_base is the redistributor address of cpu core
 * @return {FError} FGIC_SUCCESS is success ,FGIC_ERR_IN_TIMEOUT is timeout
 */
FError __attribute__((optimize("O0"))) FGicRedistributorInit(uintptr redist_base)
{
    FASSERT(redist_base);
    s32 int_id;
    u32 timeout = 0;
    uintptr sgi_base = redist_base + FGIC_GICR_SGI_BASE_OFFSET;

    /* Clear processor sleep and wait till childasleep is cleard */

    FGIC_GICR_WAKER_CLEAR_BIT(redist_base, FGIC_GICR_WAKER_PROCESSOR_SLEEP_MASK);
    while (FGIC_GICR_WAKER_READ(redist_base) & FGIC_GICR_WAKER_CHILDREN_ASLEEP_MASK)
    {
        timeout++;
        if (timeout >= 0xfffffff)
        {
            return FGIC_ERR_IN_TIMEOUT;
        }
    }

    FASSERT(FGicWaitRwp(redist_base, GICR_CTLR_RWP_WAIT) == FGIC_SUCCESS);
    FGIC_GICR_ICENABLER0_WRITE(sgi_base, FGIC_GICR_ICENABLER0_DEFAULT_MASK); /* Disable all sgi ppi */

    /* Clear pending */
    FGIC_GICR_ICPENDR0_WRITE(sgi_base, FGIC_GICR_ICPENDR0_DEFAULT_MASK);

    /* Set sgi ppi route to different security group */
    FGIC_GICR_IGROUPR0_WRITE(sgi_base, FGIC_GICR_IGROUPR0_DEFAULT_MASK);
    FGIC_GICR_IGRPMODR0_WRITE(sgi_base, FGIC_GICR_IGRPMODR0_DEFAULT_MASK);

    /* 默认所有优先级为0xa0 */
    for (int_id = 0; int_id < FGIC_SPI_START_ID; int_id += FGIC_GICD_4_PER_REG)
    {
        FGIC_GICR_IPRIORITYR_WRITE(sgi_base, int_id, FGIC_INT_DEFAULT_PRI_X4);
    }

    FGIC_GICR_ICFGR0_WRITE(sgi_base, 0); /* set level-sensitive */
    FGIC_GICR_ICFGR1_WRITE(sgi_base, 0);

    return FGIC_SUCCESS;
}

/**
 * @name: FGicCpuInterfaceInit
 * @msg:  Initialize Cpu interface  of current core
 */
void FGicCpuInterfaceInit(void)
{
    u32 reg;
    reg = FGicGetICC_SRE_EL1();

    if (!(reg & GICC_SRE_SRE))
    {
        reg |= (GICC_SRE_SRE | GICC_SRE_DFB | GICC_SRE_DIB);
        FGicSetICC_SRE_EL1(reg);
        reg = FGicGetICC_SRE_EL1();
        FASSERT(reg & GICC_SRE_SRE);
    }

    FGicSetICC_PMR(FGIC_CPU_INTERFACE_DEFAULT_FILTER);

    FGicEnableGroup1_EL1();

    FGicSetICC_CTLR_EL1(GICC_CTLR_CBPR); /* ICC_BPR0_EL1 determines the preemption group for both Group 0 and Group 1 interrupts. */
}

/**
 * @name: uintptr FGicRedistributorIterate(void)
 * @msg:  Get gic redistributor base address of current core
 * @param {void}  
 * @return {uintptr} redistributor address
 */
uintptr FGicRedistributorIterate(void)
{
    uintptr redist_base = 0;
    fsize_t mpidr_aff = (GetAffinity() & CORE_AFF_MASK);
    u32 gicr_typer_aff = 0;

    for (redist_base = GICV3_RD_BASE_ADDR;
         redist_base < GICV3_RD_BASE_ADDR + GICV3_RD_SIZE; redist_base += GICV3_RD_OFFSET)
    {
        gicr_typer_aff = FGIC_GICR_TYPER_H_READ(redist_base);
        if (mpidr_aff == gicr_typer_aff)
        {
            return redist_base;
        }
    }

    // FASSERT_MSG(0, "mpidr_aff and gicr_typer_aff is not match");
    return GICV3_RD_BASE_ADDR;
}

/**
 * @name: FError FGicCfgInitialize(FGic *instance_p, const FGicConfig *input_config_p , uintptr redist_base)
 * @msg:  Initialize the GIC driver instance based on the incoming configuration
 * @param {FGic} *instance_p is a pointer to the FGic instance.
 * @param {FGicConfig} *input_config_p Configuration items that need to be applied in the instance
 * @param {uintptr} redist_base is the redistributor address of cpu core
 * @return {*}
 */
FError FGicCfgInitialize(FGic *instance_p, const FGicConfig *input_config_p, uintptr redist_base)
{
    u32 max_ints_mun;
    uintptr dist_base;

    FASSERT(instance_p && input_config_p && redist_base);

    instance_p->config = *input_config_p;
    instance_p->redist_base = redist_base;
    instance_p->is_ready = FT_COMPONENT_IS_READY;

    dist_base = instance_p->config.dist_base;

    max_ints_mun = FGIC_READREG32(dist_base, FGIC_GICD_TYPER_OFFSET);
    max_ints_mun &= FGIC_GICD_TYPER_ITLINESNUMBER_MASK;
    max_ints_mun = ((max_ints_mun + 1) << 5) - 1; /* If the value of this field is N, the maximum SPI INTID is 32(N+1) minus 1. */
    instance_p->max_spi_num = max_ints_mun;

    return FGIC_SUCCESS;
}

/**
 * @name: FError FGicIntEnable(FGic *instance_p,s32 int_id)
 * @msg:  Enables the interrupt function based on the interrupt number
 * @param {FGic} *instance_p is a pointer to the FGic instance.
 * @param {s32} int_id is interrupt id
 * @return {*}
 */
FError FGicIntEnable(FGic *instance_p, s32 int_id)
{
    FASSERT(instance_p);
    FASSERT(instance_p->is_ready == FT_COMPONENT_IS_READY);

    if (int_id <= FGIC_PPI_END_ID)
    {
        uintptr redist_base = FGicRedistributorIterate();
        FGicEnablePrivateInt(redist_base, int_id);
    }
    else if (int_id <= instance_p->max_spi_num)
    {
        uintptr dist_base = instance_p->config.dist_base;
        FGicEnableSPI(dist_base, int_id);
    }
    else if ((int_id >= FGIC_LPI_START_ID) && (int_id < instance_p->max_lpi_num))
    {
#ifdef CONFIG_ENABLE_GIC_ITS
        FGicItsLpiSetup(instance_p, int_id, 1);
#else
        FGIC_DEBUG_E("The GIC ITS module is not enabled.");
        return (u32)FGIC_ITS_DEVICE_NOT_EXIST;
#endif
    }
    else
    {
        FGIC_DEBUG_E("int_id is over max spi num for FGicIntEnable.");
        return FGIC_CTLR_ERR_NUM;
    }

    return FGIC_SUCCESS;
}


/**
 * @name: FError FGicIntEnable(FGic *instance_p,s32 int_id)
 * @msg:  Disable the interrupt function based on the interrupt number
 * @param {FGic} *instance_p is a pointer to the FGic instance.
 * @param {s32} int_id is interrupt id
 */
FError FGicIntDisable(FGic *instance_p, s32 int_id)
{
    FASSERT(instance_p);
    FASSERT(instance_p->is_ready == FT_COMPONENT_IS_READY);

    if (int_id <= FGIC_PPI_END_ID)
    {
        uintptr redist_base = FGicRedistributorIterate();
        FGicDisablePrivateInt(redist_base, int_id);
    }
    else if (int_id <= instance_p->max_spi_num)
    {
        uintptr dist_base = instance_p->config.dist_base;
        FGicDisableSPI(dist_base, int_id);
    }
    else if ((int_id >= FGIC_LPI_START_ID) && (int_id < instance_p->max_lpi_num))
    {
#ifdef CONFIG_ENABLE_GIC_ITS
        FGicItsLpiSetup(instance_p, int_id, 0);
#else
        FGIC_DEBUG_E("The GIC ITS module is not enabled.");
        return (u32)FGIC_ITS_DEVICE_NOT_EXIST;
#endif
    }
    else
    {
        FGIC_DEBUG_E("int_id is over max spi num for FGicIntDisable.");
        return FGIC_CTLR_ERR_NUM;
    }

    return FGIC_SUCCESS;
}


/**
 * @name:   FError FGicSetPriority(FGic *instance_p,s32 int_id,u32 priority)
 * @msg:    Sets the current interrupt priority value based on the interrupt number
 * @param {FGic} *instance_p is a pointer to the FGic instance.
 * @param {s32} int_id  is interrupt id
 * @param {u32} priority is priority value ,valid bit is bit[0:7]
 * @return {*}
 */
FError FGicSetPriority(FGic *instance_p, s32 int_id, u32 priority)
{
    FASSERT(instance_p);
    FASSERT(instance_p->is_ready == FT_COMPONENT_IS_READY);

    if (int_id <= FGIC_PPI_END_ID)
    {
        uintptr redist_base = FGicRedistributorIterate();
        FGicSetPrivatePriority(redist_base, int_id, priority);
    }
    else if (int_id <= instance_p->max_spi_num)
    {
        uintptr dist_base = instance_p->config.dist_base;
        FGicSetSpiPriority(dist_base, int_id, priority);
    }
    else if ((int_id >= FGIC_LPI_START_ID) && (int_id < instance_p->max_lpi_num))
    {
#ifdef CONFIG_ENABLE_GIC_ITS
        FGicItsLpiSetPriority(instance_p, int_id, priority);
#else
        FGIC_DEBUG_E("The GIC ITS module is not enabled.");
        return (u32)FGIC_ITS_DEVICE_NOT_EXIST;
#endif
    }
    else
    {
        FGIC_DEBUG_E("int_id is over max spi num for FGicSetPriority.");
        return FGIC_CTLR_ERR_NUM;
    }

    return FGIC_SUCCESS;
}

/**
 * @name:  u32 FGicGetPriority(FGic *instance_p,s32 int_id)
 * @msg:   Gets the current interrupt priority value based on the interrupt number
 * @param {FGic} *instance_p is a pointer to the FGic instance
 * @param {s32} int_id is interrupt id
 * @return {u32} priority value ,valid bit is bit[0:7]
 */
u32 FGicGetPriority(FGic *instance_p, s32 int_id)
{
    u32 priority = 0;
    FASSERT(instance_p);
    FASSERT(instance_p->is_ready == FT_COMPONENT_IS_READY);

    if (int_id <= FGIC_PPI_END_ID)
    {
        uintptr redist_base = FGicRedistributorIterate();
        priority = FGicGetPrivatePriority(redist_base, int_id);
    }
    else if (int_id <= instance_p->max_spi_num)
    {
        uintptr dist_base = instance_p->config.dist_base;
        priority = FGicGetSpiPriority(dist_base, int_id);
    }
    else if ((int_id >= FGIC_LPI_START_ID) && (int_id < instance_p->max_lpi_num))
    {
#ifdef CONFIG_ENABLE_GIC_ITS
        priority = FGicItsLpiGetPriority(instance_p, int_id);
#else
        FGIC_DEBUG_E("The GIC ITS module is not enabled.");
        return (u32)FGIC_ITS_DEVICE_NOT_EXIST;
#endif
    }
    else
    {
        FGIC_DEBUG_E("int_id is over max spi num for FGicGetPriority.");
        return (u32)FGIC_CTLR_ERR_IN_GET;
    }

    return priority;
}

/**
 * @name: FError FGicSetTriggerLevel(FGic *instance_p,s32 int_id,TRIGGER_LEVEL trigger_way)
 * @msg:  Sets the interrupt triggering mode based on the current interrupt number
 * @param {FGic} *instance_p is a pointer to the FGic instance
 * @param {s32} int_id is interrupt id
 * @param {TRIGGER_LEVEL} trigger_way is trigger mode
 * @return {*}
 */
FError FGicSetTriggerLevel(FGic *instance_p, s32 int_id, TRIGGER_LEVEL trigger_way)
{
    FASSERT(instance_p);
    FASSERT(instance_p->is_ready == FT_COMPONENT_IS_READY);

    if (int_id > instance_p->max_spi_num)
    {
        FGIC_DEBUG_E("int_id is over max spi num for FGicSetTriggerLevel.");
        return FGIC_CTLR_ERR_IN_SET;
    }

    if (int_id <= FGIC_SGI_END_ID)
    {
        uintptr redist_base = FGicRedistributorIterate();
        FGicSetSgiLevel(redist_base, int_id, trigger_way);
    }
    else if (int_id <= FGIC_PPI_END_ID)
    {
        uintptr redist_base = FGicRedistributorIterate();
        FGicSetPpiLevel(redist_base, int_id, trigger_way);
    }
    else
    {
        uintptr dist_base = instance_p->config.dist_base;
        FGicSetSpiLevel(dist_base, int_id, trigger_way);
    }

    return FGIC_SUCCESS;
}

/**
 * @name: u32 FGicGetTriggerLevel(FGic *instance_p,s32 int_id)
 * @msg:  Gets the interrupt triggering mode based on the current interrupt number
 * @param {FGic} *instance_p is a pointer to the FGic instance
 * @param {s32} int_id is interrupt id
 * @return {u32} triggering mode
 */
u32 FGicGetTriggerLevel(FGic *instance_p, s32 int_id)
{
    u32 trigger_way;
    FASSERT(instance_p);
    FASSERT(instance_p->is_ready == FT_COMPONENT_IS_READY);


    if (int_id > instance_p->max_spi_num)
    {
        FGIC_DEBUG_E("int_id is over max spi num for FGicGetTriggerLevel.");
        return (u32)FGIC_CTLR_ERR_IN_GET;
    }

    if (int_id <= FGIC_SGI_END_ID)
    {
        uintptr redist_base = FGicRedistributorIterate();
        trigger_way = FGicGetSgiLevel(redist_base, int_id);
    }
    else if (int_id <= FGIC_PPI_END_ID)
    {
        uintptr redist_base = FGicRedistributorIterate();
        trigger_way = FGicGetPpiLevel(redist_base, int_id);
    }
    else
    {
        uintptr dist_base = instance_p->config.dist_base;
        trigger_way = FGicGetSpiLevel(dist_base, int_id);
    }

    return trigger_way;
}

/**
 * @name: FGicSetSpiAffinityRouting
 * @msg:  Set intermediate routing information for a specific SPI interrupt
 * @param {FGic} *instance_p is a pointer to the FGic instance.
 * @param {u32} int_id is interrupt vector for spi
 * @param {SPI_ROUTING_MODE} route_mode is the interrupt routing mode.
 * @param {u64} affinity  is the affinity level ，format is
 * |--------[bit39-32]-------[bit23-16]-------------[bit15-8]--------[bit7-0]
 * |--------Affinity level3-----Affinity level2-----Affinity level1---Affinity level0
 * @return {*}
 */
FError FGicSetSpiAffinityRouting(FGic *instance_p, s32 int_id, SPI_ROUTING_MODE route_mode, u64 affinity)
{
    FASSERT(instance_p);
    FASSERT(instance_p->is_ready == FT_COMPONENT_IS_READY);
    uintptr dist_base = instance_p->config.dist_base;

    if (int_id <= FGIC_PPI_END_ID)
    {
        FGIC_DEBUG_E("%s:int_id %d is out of range.", __func__, int_id);
        return FGIC_CTLR_ERR_IN_SET;
    }

    FGicSetSpiRoute(dist_base, int_id, route_mode, affinity);
    return FGIC_SUCCESS;
}


/**
 * @name: FGicGetAffinityRouting
 * @msg:  Get intermediate routing information for a specific SPI interrupt
 * @param {FGic} *instance_p is a pointer to the FGic instance.
 * @param {u32} int_id  is interrupt vector for spi
 * @param {SPI_ROUTING_MODE} *route_mode_p is a pointer to get interrupt routing mode.
 * @param {u64} *affinity_p is pointer to get affinity level ,format is
 * |--------[bit39-32]-------[bit23-16]-------------[bit15-8]--------[bit7-0]
 * |--------Affinity level3-----Affinity level2-----Affinity level1---Affinity level0
 * @return {FError}
 */
FError FGicGetAffinityRouting(FGic *instance_p, s32 int_id,
                              SPI_ROUTING_MODE *route_mode_p, u64 *affinity_p)
{
    FASSERT(instance_p);
    FASSERT(instance_p->is_ready == FT_COMPONENT_IS_READY);
    FASSERT(route_mode_p != NULL);
    FASSERT(affinity_p != NULL);
    uintptr dist_base = instance_p->config.dist_base;
    u64 reg;

    if ((int_id > instance_p->max_spi_num) || (int_id <= FGIC_PPI_END_ID))
    {
        FGIC_DEBUG_E("%s:int_id %d is out of range.", __func__, int_id);
        return (u32)FGIC_CTLR_ERR_IN_GET;
    }

    reg = FGicGetSpiRoute(dist_base, int_id);
    *route_mode_p = reg & SPI_ROUTING_TO_ANY;
    *affinity_p = reg & FGIC_GICD_IROUTER_AFFINITY_MASK;
    return FGIC_SUCCESS;
}


/**
 * @name: FGicGenerateSgi
 * @msg:  This interface is used for software generated interrupts
 * @param {u32} int_id  is interrupt vector for spi
 * @param {u32} target_list is the set of PEs for which SGI interrupts will be generated. Each bit corresponds to the PE within a cluster with an Affinity 0 value equal to the bit number.
 * @param {SGI_ROUTING_MODE} routing_mode is Interrupt Routing Mode.
 * @param {u64} affinity is the affinity level ,format is
 * |--------[bit55-48]-------[bit39-32]-------------[bit23-16]
 * |--------Affinity level3-----Affinity level2-----Affinity level1
 * @return {*}
 */
FError FGicGenerateSgi(s32 int_id, u32 target_list, SGI_ROUTING_MODE routing_mode, u64 affinity)
{
    if (int_id > FGIC_SGI_END_ID)
    {
        FGIC_DEBUG_E("%s:int_id %d is out of range.", __func__, int_id);
        return FGIC_CTLR_ERR_IN_SET;
    }

    FGicSetICC_SGI1R((int_id & FGIC_ICC_SGI1R_INTID_MASK) << 24, target_list, routing_mode, affinity);
    return FGIC_SUCCESS;
}

/**
 * @name: FGicDeactionInterrupt
 * @msg:  Deactive Interruption of the current active state
 * @param {s32} int_id is interrupt id
 */
void FGicDeactionInterrupt(s32 int_id)
{
    FGicSetICC_EOIR1(int_id);
}

/**
 * @name: FGicAcknowledgeInt
 * @msg:  Acknowledge pending interrupt
 * @return {s32}  interrupt id
 */
s32 FGicAcknowledgeInt(void)
{
    return FGicGetICC_IAR1();
}


/**
 * @name: FGicSetPriorityFilter
 * @msg:  By setting the parameter of ICC_PMR, the interrupt range that the interrupt controller can respond to is determined
 * @param {u32} priority_mask . If the priority of an interrupt is lower than the value indicated by this field, the interface signals the interrupt to the PE.
 * The reference values  of priority_mask  are as follows
 * |priority_mask---------------256-------254--------252------248-------240
 * |Implemented priority bits---[7:0]----[7:1]------[7:2]-----[7:3]-----[7:4]
 * |priority the growing steps--any-----even value----4---------8--------16
 */
void FGicSetPriorityFilter(u32 priority_mask)
{
    FGicSetICC_PMR(priority_mask);
}


/**
 * @name: FGicGetPriorityFilter
 * @msg:  Gets the current priority filtering value
 * @return {u32} Priority Mask for the CPU interface . If the priority of an interrupt is lower than the value
 * indicated by this field, the interface signals the interrupt to the PE.
 * The reference values  of priority_mask  are as follows
 * |return value----------------256-------254--------252------248-------240
 * |Implemented priority bits---[7:0]----[7:1]------[7:2]-----[7:3]-----[7:4]
 * |priority the growing steps--any-----even value----4---------8--------16
 */
u32 FGicGetPriorityFilter(void)
{
    return FGicGetICC_PMR();
}


/**
 * @name: FGicGetPriorityGroup
 * @msg:  Get Binary point value
 * @return {u32} The relationship between Binary point value and grouping is as follows
 * |return value----------------0-------1--------2------3-------4------5------6-------7
 * |Group priority field------[---]----[7:1]---[7:2]--[7:3]---[7:4]---[7:5]---[7:6]---[7]
 * |Subpriority field---------[---]-----[0]----[1:0]--[2:0]---[3:0]---[4:0]---[5:0]---[6:0]
 */
u32 FGicGetPriorityGroup(void)
{
    return FGicGetICC_BPR1();
}

/**
 * @name: FGicSetPriorityGroup
 * @msg:  Sets the value of the current group priority
 * @param {u32} binary_point : The value of this field controls how the 8-bit interrupt priority field is split into a group priority field
 *  The relationship between binary_point value and grouping is as follows
 * |binary_point----------------0-------1--------2------3-------4------5------6-------7
 * |Group priority field------[---]----[7:1]---[7:2]--[7:3]---[7:4]---[7:5]---[7:6]---[7]
 * |Subpriority field---------[---]-----[0]----[1:0]--[2:0]---[3:0]---[4:0]---[5:0]---[6:0]
 */
void FGicSetPriorityGroup(u32 binary_point)
{
    FGicSetICC_BPR1(binary_point);
}
