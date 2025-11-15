/*
 * Copyright (C) 2023, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: fpcie_ecam.c
 * Created Date: 2023-07-24 11:39:40
 * Last Modified: 2025-08-05 14:13:18
 * Description:  This file is for
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 * 1.0     huanghe    2023/08/06        first release
 * 1.1     huanghe    2025/07/17        add msi-x support
 */

#include <stdio.h>
#include <string.h>
#include "fpcie_ecam.h"
#include "fpcie_ecam_common.h"
#include "fparameters.h"
#include "fdrivers_port.h"


#define upper_32_bits(n) ((u32)(((n) >> 16) >> 16))


/***************** Macros (Inline Functions) Definitions *********************/

#define FPCIE_DEBUG_TAG  "FPCIE"
#define FPCIE_DEBUG_D(format, ...) \
    FT_DEBUG_PRINT_D(FPCIE_DEBUG_TAG, format, ##__VA_ARGS__)
#define FPCIE_DEBUG_I(format, ...) \
    FT_DEBUG_PRINT_I(FPCIE_DEBUG_TAG, format, ##__VA_ARGS__)
#define FPCIE_DEBUG_W(format, ...) \
    FT_DEBUG_PRINT_W(FPCIE_DEBUG_TAG, format, ##__VA_ARGS__)
#define FPCIE_DEBUG_E(format, ...) \
    FT_DEBUG_PRINT_E(FPCIE_DEBUG_TAG, format, ##__VA_ARGS__)


/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
static u32 FPcieBridgeProbeBus(FPcieEcam *instance_p, u8 bus, u8 device, u8 function);
/************************** Variable Definitions *****************************/

static void FPcieShowRegion(const char *name, struct FPcieRegion *region)
{
    FPCIE_DEBUG_I("Pci auto config: bus %s region: [0x%llx-0x%llx],\r\n"
                  "\t\tphysical memory [0x%llx-0x%llx]",
                  name, (unsigned long long)region->bus_start,
                  (unsigned long long)(region->bus_start + region->size - 1),
                  (unsigned long long)region->phys_start,
                  (unsigned long long)(region->phys_start + region->size - 1));
    FPCIE_DEBUG_I("Bus lower is 0x%llx", (unsigned long long)region->bus_lower);
    FPCIE_DEBUG_I("Bus flg is 0x%x", region->flags);
}

/**
 * @name: FPcieRegionConfigInit
 * @msg:  初始化PEU 用于分配的地址空间
 * @param {FPcie} *instance_p is a pointer to the FPcie instance.
 * @param {FPcieRegion} *regs 地址空间对应的指针
 * @param {u32} regs_num 传入regs 结构体的数量
 */
static void FPcieRegionConfigInit(FPcieEcam *instance_p, struct FPcieRegion *regs, u32 regs_num)
{
    u32 i;

    for (i = 0; i < regs_num; i++)
    {
        switch (regs[i].flags)
        {
            case FPCIE_REGION_IO:
                memset(&instance_p->mem_io, 0, sizeof(struct FPcieRegion));
                memcpy(&instance_p->mem_io, regs, sizeof(struct FPcieRegion));
                instance_p->mem_io.exist_flg = FPCIE_REGION_EXIST_FLG;
                instance_p->mem_io.bus_lower = instance_p->mem_io.phys_start;
                FPcieShowRegion("I/O", &instance_p->mem_io);
                break;
            case FPCIE_REGION_MEM:
                memset(&instance_p->mem, 0, sizeof(struct FPcieRegion));
                memcpy(&instance_p->mem, regs, sizeof(struct FPcieRegion));
                instance_p->mem.exist_flg = FPCIE_REGION_EXIST_FLG;
                instance_p->mem.bus_lower = instance_p->mem.phys_start;
                FPcieShowRegion("Memory", &instance_p->mem);
                break;
            case (PCI_REGION_PREFETCH | FPCIE_REGION_MEM):
                memset(&instance_p->mem_prefetch, 0, sizeof(struct FPcieRegion));
                memcpy(&instance_p->mem_prefetch, regs, sizeof(struct FPcieRegion));
                instance_p->mem_prefetch.exist_flg = FPCIE_REGION_EXIST_FLG;
                instance_p->mem_prefetch.bus_lower = instance_p->mem_prefetch.phys_start;
                FPcieShowRegion("Prefetchable Mem", &instance_p->mem_prefetch);
                break;
            default:
                break;
        }
    }
}


/**
 * @name: FPcieGetEcamConfigAddress
 * @msg: 组合外部配置地址（ECAM格式）。
 * @param {u8} Bus：总线号
 * @param {u8} Device：设备号
 * @param {u8} Function：函数号
 * @param {u16} Offset：寄存器偏移量
 * @return {u32} Location：组合后的外部配置地址
 * @note: 此函数用于将总线号、设备号、函数号和寄存器偏移量组合成外部配置地址（ECAM格式）。
 */
static u32 FPcieGetEcamConfigAddress(u8 bus, u8 device, u8 function, u16 offset)
{
    u32 location = 0;

    location |= ((((u32)bus) << FPCIE_ECAM_BUS_SHIFT));
    location |= ((((u32)device) << FPCIE_ECAM_DEV_SHIFT));
    location |= ((((u32)function) << FPCIE_ECAM_FUN_SHIFT));
    location |= ((((u32)offset << FPCIE_ECAM_REG_SHIFT)));

    return location;
}


/**
 * @name: FPcieEcamReadConfigSpace
 * @msg: 读取PCIe配置空间寄存器的值。
 * @param {FPcie*} instance_p：指向FPcie实例的指针
 * @param {u8} bus：总线号
 * @param {u8} device：设备号
 * @param {u8} function：函数号
 * @param {u16} offset：寄存器偏移量 ,请使用 FPCIE_CCR_XXX_REGS 系列寄存器
 * @param {u32*} data：指向存储读取数据的变量的指针
 * @return {FError} ret：函数执行结果，返回错误码
 * @note: 此函数用于读取PCIe配置空间寄存器的值。根据给定的总线号、设备号、函数号和寄存器偏移量，
 *       使用ECAM地址计算函数获取寄存器的值。如果传入的总线号超出最大支持总线号范围，
 *       将返回FPCIE_EXCEED_BUS错误码。如果总线号为0且设备号和函数号都为0，
 *       将返回FPCIE_CCR_INVALID_DATA，表示未定义的寄存器数据。
 *       如果成功读取到寄存器的值，将通过data参数返回该值，并返回FT_SUCCESS。
 */
FError FPcieEcamReadConfigSpace(FPcieEcam *instance_p, u8 bus, u8 device, u8 function,
                                u16 offset, u32 *data)
{
    FASSERT(instance_p != NULL);
    FASSERT(data != NULL);
    FASSERT(instance_p->is_ready == FT_COMPONENT_IS_READY);
    u32 ecam_location = 0;
    FError ret = FT_SUCCESS;


    if (bus > instance_p->config.max_bus_num)
    {
        *data = FPCIE_CCR_INVALID_DATA;
        ret = FPCIE_EXCEED_BUS;
    }
    else
    {
        ecam_location = FPcieGetEcamConfigAddress(bus, device, function, offset);
        DSB();
        *data = FPCIE_READECAM_REG(instance_p->config.ecam, ecam_location);
    }

    return ret;
}

/**
 * @name: FPcieEcamWriteConfigSpace
 * @msg: 写入PCIe配置空间寄存器的值。
 * @param {FPcie*} instance_p：指向FPcie实例的指针
 * @param {u8} bus：总线号
 * @param {u8} device：设备号
 * @param {u8} function：函数号
 * @param {u16} offset：寄存器偏移量
 * @param {u32} data：要写入寄存器的值
 * @return {FError} ret：函数执行结果，返回错误码（如果有）
 * @note: 此函数用于将给定的值写入PCIe配置空间寄存器。根据给定的总线号、设备号、函数号和寄存器偏移量，
 *       使用ECAM地址计算函数获取寄存器的地址。如果传入的总线号为0，则直接返回FT_SUCCESS。
 *       如果传入的总线号超出最大支持总线号范围，将返回FPCIE_EXCEED_BUS错误码。
 *       如果成功写入寄存器的值，将返回FT_SUCCESS。
 */
FError FPcieEcamWriteConfigSpace(FPcieEcam *instance_p, u8 bus, u8 device, u8 function,
                                 u16 offset, u32 data)
{
    FASSERT(instance_p != NULL);
    FASSERT(instance_p->is_ready == FT_COMPONENT_IS_READY);
    u32 ecam_location = 0;
    FError ret = FT_SUCCESS;

    if (bus > instance_p->config.max_bus_num)
    {
        ret = FPCIE_EXCEED_BUS;
        return ret;
    }
    else
    {
        ecam_location = FPcieGetEcamConfigAddress(bus, device, function, offset);
        FPCIE_WRITEECAM_REG(instance_p->config.ecam, ecam_location, data);
    }

    return ret;
}


/**
 * @name: FPcieGetEcamConfigAddressEx
 * @msg: 组合外部配置地址（ECAM格式），支持字节级偏移
 * @param {u8} bus：总线号
 * @param {u8} device：设备号
 * @param {u8} function：函数号
 * @param {u16} offset：字节偏移量
 * @return {u32} Location：组合后的外部配置地址
 * @note: 此函数用于将总线号、设备号、函数号和字节偏移量组合成外部配置地址
 */
static u32 FPcieGetEcamConfigAddressEx(u8 bus, u8 device, u8 function, u16 offset)
{
    u32 location = 0;

    location |= (((u32)bus) << FPCIE_ECAM_BUS_SHIFT);
    location |= (((u32)device) << FPCIE_ECAM_DEV_SHIFT);
    location |= (((u32)function) << FPCIE_ECAM_FUN_SHIFT);
    location |= ((u32)offset); /* 直接使用字节偏移，不再左移 */

    return location;
}
/**
 * @brief 扩展的PCIe ECAM配置空间读取函数
 * @msg: 该函数支持不同尺寸（1/2/4字节）的配置空间访问
 * @param instance_p ECAM实例指针
 * @param bus PCI总线号
 * @param device PCI设备号
 * @param function PCI功能号
 * @param offset 配置空间偏移量（0-4095）
 * @param data [输出] 读取数据的缓冲区指针
 * @param access_size 访问尺寸（PCI_SIZE_8/PCI_SIZE_16/PCI_SIZE_32）
 * @return 错误码（FT_SUCCESS表示成功）
 */
FError FPcieEcamReadConfigSpaceEx(FPcieEcam *instance_p, u8 bus, u8 device, u8 function,
                                  u16 offset, u32 *data, enum FPcieSize access_size /* 新增访问尺寸参数：1/2/4字节 */
)
{
    /* 参数有效性检查 */
    FASSERT(instance_p != NULL);
    FASSERT(data != NULL);
    FASSERT(instance_p->is_ready == FT_COMPONENT_IS_READY);

    u32 ecam_location = 0;
    FError ret = FT_SUCCESS;
    u8 *data_ptr = (u8 *)data; /* 用于字节访问的指针 */

    /* 总线号有效性检查 */
    if (bus > instance_p->config.max_bus_num)
    {
        *data = FPCIE_CCR_INVALID_DATA;
        return FPCIE_EXCEED_BUS;
    }

    /* 偏移量范围检查（配置空间范围0~4095） */
    if (offset > 0xFFF)
    {
        *data = FPCIE_CCR_INVALID_DATA;
        return FPCIE_INVALID_OFFSET;
    }

    /* 内存访问对齐检查 */
    if (((access_size >= 2) && (offset & 0x1)) || /* 2/4字节访问需要2字节对齐 */
        ((access_size == 4) && (offset & 0x3)))
    { /* 4字节访问需要4字节对齐 */
        *data = FPCIE_CCR_INVALID_DATA;
        return FPCIE_INVALID_OFFSET;
    }

    /* 边界检查（防止跨页访问越界） */
    if ((u32)offset + access_size > 0x1000)
    { /* 4096字节边界 */
        *data = FPCIE_CCR_INVALID_DATA;
        return FPCIE_ACCESS_VIOLATION;
    }

    /* 计算ECAM内存映射地址 */
    ecam_location = FPcieGetEcamConfigAddressEx(bus, device, function, offset);
    u8 *ecam_addr = (u8 *)(instance_p->config.ecam + ecam_location);

    /* 根据访问尺寸执行读取操作 */
    switch (access_size)
    {
        case PCI_SIZE_8:
            /* 单字节读取 */
            *data_ptr = FPCIE_READECAM_BYTE(instance_p->config.ecam, ecam_location);
            break;

        case PCI_SIZE_16:
            {
                /* 双字节读取（注意小端模式处理） */
                u16 val = *((u16 *)ecam_addr); /* 直接内存访问 */
                *data_ptr = (u8)val;
                *(data_ptr + 1) = (u8)(val >> 8);
                break;
            }

        case PCI_SIZE_32:
            /* 四字节读取 */
            *data = FPCIE_READECAM_REG(instance_p->config.ecam, ecam_location);
            break;

        default:
            /* 无效访问尺寸处理 */
            *data = FPCIE_CCR_INVALID_DATA;
            ret = FPCIE_INVALID_ACCESS_SIZE;
            break;
    }

    return ret;
}


/**
 * @brief 扩展的PCIe ECAM配置空间写入函数
 * @msg: 该函数支持不同尺寸（1/2/4字节）的配置空间写入操作
 * @param instance_p ECAM实例指针
 * @param bus PCI总线号
 * @param device PCI设备号
 * @param function PCI功能号
 * @param offset 配置空间偏移量（0-4095）
 * @param data 待写入的数据值
 * @param access_size 访问尺寸（PCI_SIZE_8/PCI_SIZE_16/PCI_SIZE_32）
 * @return 错误码（FT_SUCCESS表示成功）
 */
FError FPcieEcamWriteConfigSpaceEx(FPcieEcam *instance_p, u8 bus, u8 device, u8 function,
                                   u16 offset, u32 data, enum FPcieSize access_size)
{
    /* 参数有效性检查 */
    FASSERT(instance_p != NULL);
    FASSERT(instance_p->is_ready == FT_COMPONENT_IS_READY);

    u32 ecam_location = 0;
    FError ret = FT_SUCCESS;

    /* 总线号有效性检查 */
    if (bus > instance_p->config.max_bus_num)
    {
        return FPCIE_EXCEED_BUS;
    }

    /* 偏移量范围检查（配置空间范围0~4095字节） */
    if (offset > 0xFFF)
    {
        return FPCIE_INVALID_OFFSET;
    }

    /* 访问尺寸合法性检查（仅支持1/2/4字节访问） */
    if (access_size != PCI_SIZE_8 && access_size != PCI_SIZE_16 && access_size != PCI_SIZE_32)
    {
        return FPCIE_INVALID_ACCESS_SIZE;
    }

    /* 内存访问对齐检查 */
    if ((access_size == PCI_SIZE_16 && (offset & 0x1)) || /* 2字节访问需2字节对齐 */
        (access_size == PCI_SIZE_32 && (offset & 0x3)))
    { /* 4字节访问需4字节对齐 */
        return FPCIE_INVALID_OFFSET;
    }

    /* 边界检查（防止跨页访问越界） */
    if ((u32)offset + access_size > 0x1000)
    {
        return FPCIE_ACCESS_VIOLATION;
    }

    /* 计算ECAM内存映射地址 */
    ecam_location = FPcieGetEcamConfigAddressEx(bus, device, function, offset);
    /* u8 *ecam_addr = (u8 *)(instance_p->config.ecam + ecam_location); */

    /* 根据访问尺寸执行写入操作 */
    switch (access_size)
    {
        case PCI_SIZE_8:
            /* 单字节写入 */
            FPCIE_WRITEECAM_BYTE(instance_p->config.ecam, ecam_location, data);
            break;

        case PCI_SIZE_16:
            /* 双字节写入 */
            FPCIE_WRITEECAM_WORD(instance_p->config.ecam, ecam_location, data);
            break;

        case PCI_SIZE_32:
            /* 四字节写入 */
            FPCIE_WRITEECAM_REG(instance_p->config.ecam, ecam_location, data);
            break;

        default:
            /* 理论上不会执行（前面已做参数检查） */
            ret = FPCIE_INVALID_ACCESS_SIZE;
            break;
    }

    return ret;
}


static void FPcieBdfInsert(FPcieEcam *instance_p, u8 bus, u8 device, u8 function)
{
    FASSERT(instance_p != NULL);
    FASSERT(instance_p->is_ready == FT_COMPONENT_IS_READY);

    if (instance_p->scans_bdf_count >= FPCIE_MAX_SCAN_NUMBER)
    {
        FPCIE_DEBUG_W("Scans bdf is over %d ", FPCIE_MAX_SCAN_NUMBER);
        instance_p->scans_bdf_count = FPCIE_MAX_SCAN_NUMBER - 1;
    }

    instance_p->scans_bdf[instance_p->scans_bdf_count].bus = bus;
    instance_p->scans_bdf[instance_p->scans_bdf_count].device = device;
    instance_p->scans_bdf[instance_p->scans_bdf_count].function = function;
    instance_p->scans_bdf_count++;
}

static void FPcieAutoRegionAlign(struct FPcieRegion *res, fpcisize_t size)
{
    res->bus_lower = ((res->bus_lower - 1) | (size - 1)) + 1;
}

/**
 * @name: FPcieRegionAllocate
 * @msg: This function allocates a region of memory from a PCIe resource.
 * @param {struct FPcieRegion} *res - Pointer to the PCIe resource structure.
 * @param {fpcisize_t} size - Size of the memory region to allocate.(bytes)
 * @param {fpciaddr_t} *bar_addr - Pointer to store the allocated memory address.
 * @return {FError} - FT_SUCCESS on success, or an error code on failure.
 */
static FError FPcieRegionAllocate(struct FPcieRegion *res, fpcisize_t size, fpciaddr_t *bar_addr)
{
    fpciaddr_t addr;

    if (!res)
    {
        FPCIE_DEBUG_E("No resource.");
        goto error;
    }


    addr = ((res->bus_lower - 1) | (size - 1)) + 1;

    if (addr - res->bus_start + size > res->size)
    {
        FPCIE_DEBUG_E("No room in resource.");
        goto error;
    }

    res->bus_lower = addr + size;

    *bar_addr = addr;
    return FT_SUCCESS;

error:
    *bar_addr = (fpciaddr_t)-1;
    return FPCIE_READCONFIG_ERROR;
}

/**
 * @name: FPcieAllocBarSpace
 * @msg: This function allocates BAR (Base Address Registers) space for a PCIe device.
 * @param {FPcieEcam} *instance_p - Pointer to the FPcieEcam instance.
 * @param {u8} header_type - The type of header (Type 0 or Type 1) of the PCIe device.
 * @param {u8} bus - The PCI bus of the device.
 * @param {u8} device - The PCI device of the device.
 * @param {u8} function - The PCI function of the device.
 * @return {FError} - FT_SUCCESS on success, or an error code on failure.
 */
static FError FPcieAllocBarSpace(FPcieEcam *instance_p, u8 header_type, u8 bus, u8 device, u8 function)
{
    u32 cmdstat; /* command status */
    u32 max_bar, bar_num;
    u32 bar_reg;
    u32 bar_addr64;
    fpcisize_t bar_size;
    fpciaddr_t bar_addr;
    u32 rom_addr;
    int mem64_flg;
    struct FPcieRegion *bar_mem_region = NULL;

    /* FPCIE_CCR_CMD_STATUS_REGS: clean io and mem access  */
    if (FPcieEcamReadConfigSpace(instance_p, bus, device, function,
                                 FPCIE_CCR_CMD_STATUS_REGS, &cmdstat) != FT_SUCCESS)
    {
        FPCIE_DEBUG_E("%s:%d,Failed to read config space", __FUNCTION__, __LINE__);
        return FPCIE_READCONFIG_ERROR;
    }

    cmdstat &= ~(FPCIE_CCR_CMD_IO_ACCESS_ENABLED | FPCIE_CCR_CMD_MEMORY_ACCESS_ENABLED);

    if (header_type == FPCIE_CCR_HEAD_TYPE0)
    {
        max_bar = 6;
    }
    else
    {
        max_bar = 2;
    }

    for (bar_num = 0; bar_num < max_bar; bar_num++)
    {
        /* Write FPCIE_BAR_ADDR_MASK to bar address space */
        FPcieEcamWriteConfigSpace(instance_p, bus, device, function,
                                  FPCIE_CCR_BAR_ADDR0_REGS + bar_num, FPCIE_BAR_ADDR_MASK);
        /* FPCIE_CCR_CMD_STATUS_REGS: clean io and mem access  */
        if (FPcieEcamReadConfigSpace(instance_p, bus, device, function,
                                     FPCIE_CCR_BAR_ADDR0_REGS + bar_num, &bar_reg) != FT_SUCCESS)
        {
            FPCIE_DEBUG_E("%s:%d,Failed to read config space", __FUNCTION__, __LINE__);
            return FPCIE_READCONFIG_ERROR;
        }

        if (bar_reg == 0)
        {
            continue;
        }

        mem64_flg = 0;
        if (bar_reg & FPCIE_CCR_BAR_IO_REQ_MASK)
        {
            /* io mem */
            bar_size = FPCIE_CCR_BAR_IO_SIZE(bar_reg);
            bar_mem_region = &instance_p->mem_io;
        }
        else
        {
            /* mem */
            if (bar_reg & FPCIE_CCR_BAR_MEM_64_MASK)
            {
                /* 64-bit decoding */
                u32 bar_reg_upper32;
                u64 bar_reg_64;
                bar_addr64 = FPCIE_CCR_BAR_ADDR0_REGS + bar_num + 1;
                /* Write FPCIE_BAR_ADDR_MASK to bar address space */
                FPcieEcamWriteConfigSpace(instance_p, bus, device, function, bar_addr64,
                                          FPCIE_BAR_ADDR_MASK);

                /* FPCIE_CCR_CMD_STATUS_REGS: clean io and mem access  */
                if (FPcieEcamReadConfigSpace(instance_p, bus, device, function,
                                             bar_addr64, &bar_reg_upper32) != FT_SUCCESS)
                {
                    FPCIE_DEBUG_E("%s:%d,Failed to read config space", __FUNCTION__, __LINE__);
                    return FPCIE_READCONFIG_ERROR;
                }

                bar_reg_64 = ((u64)bar_reg_upper32 << 32) | bar_reg;
                bar_size = FPCIE_CCR_BAR_MEM_SIZE(bar_reg_64);

                mem64_flg = 1;
            }
            else
            {
                bar_size = (u32)FPCIE_CCR_BAR_MEM_SIZE(bar_reg);
            }

            if ((instance_p->mem_prefetch.exist_flg & FPCIE_REGION_EXIST_FLG) &
                ((bar_reg & FPCIE_CCR_BAR_MEM_PREFETCH_MASK) || (bar_reg & FPCIE_CCR_BAR_MEM_64_MASK)))
            {
                bar_mem_region = &instance_p->mem_prefetch;
            }
            else
            {
                bar_mem_region = &instance_p->mem;
            }
        }

        if (FPcieRegionAllocate(bar_mem_region, bar_size, &bar_addr) == FT_SUCCESS)
        {
            /* Write FPCIE_BAR_ADDR_MASK to bar address space */
            FPcieEcamWriteConfigSpace(instance_p, bus, device, function,
                                      FPCIE_CCR_BAR_ADDR0_REGS + bar_num, bar_addr);
            if (mem64_flg)
            {
#ifdef CONFIG_SYS_PCI_64BIT
                FPcieEcamWriteConfigSpace(instance_p, bus, device, function, bar_addr64,
                                          (u32)(bar_addr >> 32));
#else
                FPcieEcamWriteConfigSpace(instance_p, bus, device, function, bar_addr64, 0);
#endif

                bar_num++;
            }
        }
        cmdstat |= (bar_reg & FPCIE_CCR_BAR_IO_REQ_MASK) ? FPCIE_CCR_CMD_IO_ACCESS_ENABLED
                                                         : FPCIE_CCR_CMD_MEMORY_ACCESS_ENABLED;
    }

    /* configure the expansion ROM address */

    if (header_type != FPCIE_CCR_HEAD_TYPE2)
    {

        rom_addr = (header_type == FPCIE_CCR_HEAD_TYPE0) ? FPCIE_CCR_TYPE0_EXPANSION_ROM_REGS
                                                         : FPCIE_CCR_TYPE1_EXPANSION_ROM_REGS;
        /* Write FPCIE_BAR_ADDR_MASK to bar address space */
        FPcieEcamWriteConfigSpace(instance_p, bus, device, function, rom_addr, 0xfffffffe);

        if (FPcieEcamReadConfigSpace(instance_p, bus, device, function, rom_addr, &bar_reg) != FT_SUCCESS)
        {
            FPCIE_DEBUG_E("%s:%d,Failed to read config space", __FUNCTION__, __LINE__);
            return FPCIE_READCONFIG_ERROR;
        }

        if (bar_reg)
        {

            bar_size = -(bar_reg & ~1);
            if (FPcieRegionAllocate(&instance_p->mem, bar_size, &bar_addr) == FT_SUCCESS)
            {
                FPcieEcamWriteConfigSpace(instance_p, bus, device, function, rom_addr, bar_addr);
            }
            FPCIE_DEBUG_I("PCI Autoconfig: ROM, size=%#x, ", (unsigned int)bar_size);

            cmdstat |= FPCIE_CCR_CMD_MEMORY_ACCESS_ENABLED;
        }
    }


    /* FPCIE_CCR_CMD_STATUS_REGS: write io or mem access  */
    FPcieEcamWriteConfigSpace(instance_p, bus, device, function, FPCIE_CCR_CMD_STATUS_REGS, cmdstat);
    if (FPcieEcamReadConfigSpace(instance_p, bus, device, function,
                                 FPCIE_CCR_CLS_LT_HT_REGS, &cmdstat) != FT_SUCCESS)
    {
        FPCIE_DEBUG_E("%s:%d,Failed to read config space", __FUNCTION__, __LINE__);
        return FPCIE_READCONFIG_ERROR;
    }
    cmdstat &= ~(FPCIE_CCR_CACHELINE_MASK(0xf) | FPCIE_CCR_LATENCY_TIMER_MASK(0xf));
    FPcieEcamWriteConfigSpace(instance_p, bus, device, function, FPCIE_CCR_CLS_LT_HT_REGS,
                              FPCIE_CCR_CACHELINE_MASK(8) |
                                  FPCIE_CCR_LATENCY_TIMER_MASK(0x8) | cmdstat);

    return FT_SUCCESS;
}

/**
 * @name: FPciePreScanSetupBridge
 * @msg: This function performs pre-enumeration setup for a PCIe bridge.
 * @param {FPcieEcam} *instance_p - Pointer to the FPcieEcam instance.
 * @param {u8} secondary_bus - Secondary bus number of the bridge.
 * @param {u8} bus - The PCI bus of the bridge.
 * @param {u8} device - The PCI device of the bridge.
 * @param {u8} function - The PCI function of the bridge.
 */
static void FPciePreScanSetupBridge(FPcieEcam *instance_p, u8 secondary_bus, u8 bus,
                                    u8 device, u8 function)
{
    u32 bar_reg;
    u32 cmdstat; /* command status */
    u32 prefechable_64 = 0;

    if (FPcieEcamReadConfigSpace(instance_p, bus, device, function,
                                 FPCIE_CCR_CMD_STATUS_REGS, &cmdstat) != FT_SUCCESS)
    {
        FPCIE_DEBUG_E("%s:%d,Failed to read config space", __FUNCTION__, __LINE__);
        return;
    }

    if (FPcieEcamReadConfigSpace(instance_p, bus, device, function,
                                 FPCIE_CCR_PMEMB_PMEML_REGS, &prefechable_64) != FT_SUCCESS)
    {
        FPCIE_DEBUG_E("%s:%d,Failed to read config space", __FUNCTION__, __LINE__);
        return;
    }

    prefechable_64 &= FPCIE_CCR_PMEMB_64BIT_MASK;


    /* configure FPCIE_CCR_PB_SEC_SUB_SECL_REGS */
    FPcieEcamWriteConfigSpace(instance_p, bus, device, function, FPCIE_CCR_PB_SEC_SUB_SECL_REGS,
                              FPCIE_CCR_PRIMARY_NUM_MASK(bus) |
                                  FPCIE_CCR_SECONDARY_NUM_MASK(secondary_bus) |
                                  FPCIE_CCR_SUB_NUM_MASK(0Xff));

    if (instance_p->mem.exist_flg & FPCIE_REGION_EXIST_FLG)
    {
        /* Round memory allocator to 1MB boundary */
        FPcieAutoRegionAlign(&instance_p->mem, 0x100000);

        /* Config Memory base & limit */
        if (FPcieEcamReadConfigSpace(instance_p, bus, device, function,
                                     FPCIE_CCR_MEMB_MEML_REGS, &bar_reg) != FT_SUCCESS)
        {
            FPCIE_DEBUG_E("%s:%d,Failed to read config space", __FUNCTION__, __LINE__);
            return;
        }
        bar_reg &= ~FPCIE_CCR_MEM_BASE_MASK(0xffff);
        /* keep bit32~20 of Non-Prefetchable base address */
        FPcieEcamWriteConfigSpace(instance_p, bus, device, function, FPCIE_CCR_MEMB_MEML_REGS,
                                  bar_reg | FPCIE_CCR_MEM_BASE_MASK((instance_p->mem.bus_lower & 0xfff00000) >>
                                                                    16));

        cmdstat |= FPCIE_CCR_CMD_MEMORY_ACCESS_ENABLED;
    }

    if (instance_p->mem_prefetch.exist_flg & FPCIE_REGION_EXIST_FLG)
    {
        /* Round memory allocator to 1MB boundary */
        FPcieAutoRegionAlign(&instance_p->mem_prefetch, 0x100000);

        /* Config Memory base & limit */
        if (FPcieEcamReadConfigSpace(instance_p, bus, device, function,
                                     FPCIE_CCR_PMEMB_PMEML_REGS, &bar_reg) != FT_SUCCESS)
        {
            FPCIE_DEBUG_E("%s:%d,Failed to read config space", __FUNCTION__, __LINE__);
            return;
        }
        bar_reg &= ~FPCIE_CCR_PMEM_BASE_MASK(0xffff);

        /* keep bit32~20 of Prefetchable base address */
        FPcieEcamWriteConfigSpace(instance_p, bus, device, function, FPCIE_CCR_PMEMB_PMEML_REGS,
                                  bar_reg |
                                      FPCIE_CCR_PMEM_BASE_MASK((instance_p->mem_prefetch.bus_lower & 0xfff00000) >>
                                                               16));


        if (prefechable_64)
        {
#ifdef CONFIG_SYS_PCI_64BIT
            FPcieEcamWriteConfigSpace(instance_p, bus, device, function, FPCIE_CCR_PREBU32_REGS,
                                      instance_p->mem_prefetch.bus_lower >> 32);
#else
            FPcieEcamWriteConfigSpace(instance_p, bus, device, function, FPCIE_CCR_PREBU32_REGS, 0);
#endif
        }
        cmdstat |= FPCIE_CCR_CMD_MEMORY_ACCESS_ENABLED;
    }
    else
    {
        /* We don't support prefetchable memory for now, so disable */

        /* keep bit32~20 of Prefetchable base address */
        FPcieEcamWriteConfigSpace(instance_p, bus, device, function, FPCIE_CCR_PMEMB_PMEML_REGS,
                                  FPCIE_CCR_MEM_LIMIT_MASK(0) | FPCIE_CCR_MEM_BASE_MASK(0x1000));
        if (prefechable_64)
        {
            FPcieEcamWriteConfigSpace(instance_p, bus, device, function, FPCIE_CCR_PREBU32_REGS, 0);
            FPcieEcamWriteConfigSpace(instance_p, bus, device, function,
                                      FPCIE_CCR_PRELIMITU32_REGS, 0);
        }
    }

    if (instance_p->mem_io.exist_flg & FPCIE_REGION_EXIST_FLG)
    {

        /* Round memory allocator to 4KB boundary */
        FPcieAutoRegionAlign(&instance_p->mem_io, 0x1000);


        /* Config IO mem  base */
        if (FPcieEcamReadConfigSpace(instance_p, bus, device, function,
                                     FPCIE_CCR_IOB_IOL_SECS_REGS, &bar_reg) != FT_SUCCESS)
        {
            FPCIE_DEBUG_E("%s:%d,Failed to read config space", __FUNCTION__, __LINE__);
            return;
        }

        bar_reg &= ~FPCIE_CCR_IO_BASE_MASK(0xff);

        /* keep bit32~20 of Prefetchable base address */
        FPcieEcamWriteConfigSpace(instance_p, bus, device, function, FPCIE_CCR_IOB_IOL_SECS_REGS,
                                  bar_reg | FPCIE_CCR_IO_BASE_MASK((instance_p->mem_io.bus_lower & 0x0000f000) >>
                                                                   8));

        /* Config IO mem base upper 16 bits */
        if (FPcieEcamReadConfigSpace(instance_p, bus, device, function,
                                     FPCIE_CCR_IOU16_IOL16_REGS, &bar_reg) != FT_SUCCESS)
        {
            FPCIE_DEBUG_E("%s:%d,Failed to read config space", __FUNCTION__, __LINE__);
            return;
        }

        bar_reg &= ~FPCIE_CCR_IOB_UP16_MASK(0xffff);

        /* keep bit32~20 of Prefetchable base address */
        FPcieEcamWriteConfigSpace(instance_p, bus, device, function, FPCIE_CCR_IOU16_IOL16_REGS,
                                  bar_reg | FPCIE_CCR_IOB_UP16_MASK((instance_p->mem_io.bus_lower & 0xffff0000) >>
                                                                    16));
        cmdstat |= FPCIE_CCR_CMD_IO_ACCESS_ENABLED;
    }

    /* Enable memory and I/O accesses, enable bus master */
    FPcieEcamWriteConfigSpace(instance_p, bus, device, function, FPCIE_CCR_CMD_STATUS_REGS,
                              cmdstat | FPCIE_CCR_CMD_BUS_MASTER_ENABLED);
}

/**
 * @name: FPciePostscanSetupBridge
 * @msg: This function performs post-enumeration setup for a PCIe bridge.
 * @param {FPcieEcam} *instance_p - Pointer to the FPcieEcam instance.
 * @param {u8} sub_bus - Subordinate bus number of the bridge.
 * @param {u8} bus - The PCI bus of the bridge.
 * @param {u8} device - The PCI device of the bridge.
 * @param {u8} function - The PCI function of the bridge.
 */
static void FPciePostscanSetupBridge(FPcieEcam *instance_p, u8 sub_bus, u8 bus, u8 device, u8 function)
{
    u32 bar_reg;
    u32 prefechable_64 = 0;

    /* Configure Subordinate bus number registers */

    if (FPcieEcamReadConfigSpace(instance_p, bus, device, function,
                                 FPCIE_CCR_PB_SEC_SUB_SECL_REGS, &bar_reg) != FT_SUCCESS)
    {
        FPCIE_DEBUG_E("%s:%d,Failed to read config space", __FUNCTION__, __LINE__);
        return;
    }

    bar_reg &= ~FPCIE_CCR_SUB_NUM_MASK(0xff);

    FPcieEcamWriteConfigSpace(instance_p, bus, device, function, FPCIE_CCR_PB_SEC_SUB_SECL_REGS,
                              bar_reg | FPCIE_CCR_SUB_NUM_MASK(sub_bus));

    if (FPcieEcamReadConfigSpace(instance_p, bus, device, function,
                                 FPCIE_CCR_PMEMB_PMEML_REGS, &prefechable_64) != FT_SUCCESS)
    {
        FPCIE_DEBUG_E("%s:%d,Failed to read config space", __FUNCTION__, __LINE__);
        return;
    }

    prefechable_64 &= FPCIE_CCR_PMEMB_64BIT_MASK;


    if (instance_p->mem.exist_flg & FPCIE_REGION_EXIST_FLG)
    {
        /* Round memory allocator to 1MB boundary */
        FPcieAutoRegionAlign(&instance_p->mem, 0x100000);

        /* Config Memory base & limit */
        if (FPcieEcamReadConfigSpace(instance_p, bus, device, function,
                                     FPCIE_CCR_MEMB_MEML_REGS, &bar_reg) != FT_SUCCESS)
        {
            FPCIE_DEBUG_E("%s:%d,Failed to read config space", __FUNCTION__, __LINE__);
            return;
        }
        bar_reg &= ~FPCIE_CCR_MEM_LIMIT_MASK(0xffff);
        /* keep bit32~20 of Non-Prefetchable base address */
        FPcieEcamWriteConfigSpace(instance_p, bus, device, function, FPCIE_CCR_MEMB_MEML_REGS,
                                  bar_reg | FPCIE_CCR_MEM_LIMIT_MASK((instance_p->mem.bus_lower - 1) >> 16));
    }
    /* write  */
    if (instance_p->mem_prefetch.exist_flg & FPCIE_REGION_EXIST_FLG)
    {
        FPcieAutoRegionAlign(&instance_p->mem_prefetch, 0x100000);

        /* Config Memory base & limit */
        if (FPcieEcamReadConfigSpace(instance_p, bus, device, function,
                                     FPCIE_CCR_PMEMB_PMEML_REGS, &bar_reg) != FT_SUCCESS)
        {
            FPCIE_DEBUG_E("%s:%d,Failed to read config space", __FUNCTION__, __LINE__);
            return;
        }

        bar_reg &= ~FPCIE_CCR_PMEM_LIMIT_MASK(0xffff);

        /* keep bit32~20 of Prefetchable base address */
        FPcieEcamWriteConfigSpace(
            instance_p, bus, device, function, FPCIE_CCR_PMEMB_PMEML_REGS,
            bar_reg | FPCIE_CCR_PMEM_LIMIT_MASK((instance_p->mem_prefetch.bus_lower - 1) >> 16));


        if (prefechable_64)
        {
#ifdef CONFIG_SYS_PCI_64BIT
            FPcieEcamWriteConfigSpace(instance_p, bus, device, function, FPCIE_CCR_PRELIMITU32_REGS,
                                      (instance_p->mem_prefetch.bus_lower - 1) >> 32);
#else

            FPcieEcamWriteConfigSpace(instance_p, bus, device, function,
                                      FPCIE_CCR_PRELIMITU32_REGS, 0);
#endif
        }
        else
        {
            FPcieEcamWriteConfigSpace(instance_p, bus, device, function,
                                      FPCIE_CCR_PRELIMITU32_REGS, 0);
        }
    }

    if (instance_p->mem_io.exist_flg & FPCIE_REGION_EXIST_FLG)
    {

        /* Round I/O allocator to 4KB boundary */
        FPcieAutoRegionAlign(&instance_p->mem_io, 0x1000);

        /* Config IO mem  base */
        if (FPcieEcamReadConfigSpace(instance_p, bus, device, function,
                                     FPCIE_CCR_IOB_IOL_SECS_REGS, &bar_reg) != FT_SUCCESS)
        {
            FPCIE_DEBUG_E("%s:%d,Failed to read config space", __FUNCTION__, __LINE__);
            return;
        }

        bar_reg &= ~FPCIE_CCR_IO_LIMIT_MASK(0xff);

        /* keep bit32~20 of Prefetchable base address */
        FPcieEcamWriteConfigSpace(instance_p, bus, device, function, FPCIE_CCR_IOB_IOL_SECS_REGS,
                                  bar_reg | FPCIE_CCR_IO_LIMIT_MASK(((instance_p->mem_io.bus_lower - 1) &
                                                                     0x0000f000) >>
                                                                    8));

        /* Config IO mem base upper 16 bits */
        if (FPcieEcamReadConfigSpace(instance_p, bus, device, function,
                                     FPCIE_CCR_IOU16_IOL16_REGS, &bar_reg) != FT_SUCCESS)
        {
            FPCIE_DEBUG_E("%s:%d,Failed to read config space", __FUNCTION__, __LINE__);
            return;
        }

        bar_reg &= ~FPCIE_CCR_IOB_LIMIT16_MASK(0xffff);

        /* keep bit32~20 of Prefetchable base address */
        FPcieEcamWriteConfigSpace(instance_p, bus, device, function, FPCIE_CCR_IOU16_IOL16_REGS,
                                  bar_reg |
                                      FPCIE_CCR_IOB_LIMIT16_MASK(((instance_p->mem_io.bus_lower - 1) &
                                                                  0xffff0000) >>
                                                                 16));
    }
}

#ifdef FPCIE_NEED_SKIP_TYPE0

FError FPcieEcamSkipType0(FPcieEcam *instance_p, u8 bus, u8 device, u8 function)
{
    u8 pos, id;
    u16 capreg;
    u8 port_type;
    u32 config_data;
    u8 head_type;
    uintptr addr = instance_p->config.ecam;
    addr += FPcieGetEcamConfigAddress(bus, device, function, 0);

    if (!(bus > 0) || !(device > 0))
    {
        return FT_SUCCESS;
    }

    /* read header type */
    if (FPcieEcamReadConfigSpace(instance_p, bus, device, function, FPCIE_CCR_CLS_LT_HT_REGS, &config_data))
    {
        FPCIE_DEBUG_E("%s:%d,Failed to read config space", __FUNCTION__, __LINE__);
        return FPCIE_READCONFIG_ERROR;
    }
    head_type = FPCIE_CCR_HEAD_TYPE_MASK(config_data);

    if (head_type == FPCIE_CCR_HEAD_TYPE1)
    {
        return FT_SUCCESS;
    }
    /* read vendor id */
    if (FPcieEcamReadConfigSpace(instance_p, bus, device, function, FPCIE_CCR_ID_REG, &config_data))
    {
        FPCIE_DEBUG_E("%s:%d,Failed to read config space", __FUNCTION__, __LINE__);
        return FPCIE_READCONFIG_ERROR;
    }

    if (FPCIE_CCR_VENDOR_ID_MASK(config_data) != 0x1d17)
    {
        return FPCIE_NEED_SKIP;
    }

    pos = 0x34;
    while (1)
    {
        pos = FPCIE_READECAM_BYTE(addr, pos);
        if (pos < 0x40) /* 超过Capability Pointer所代表的空间offset最大范围 */
        {
            break;
        }
        pos &= ~3;                           /* offset 第两位对齐 */
        id = FPCIE_READECAM_BYTE(addr, pos); /* PCI Express Cap ID */
        if (id == 0xff)
        {
            break;
        }

        if (id == 0x10) /* 找到 PCIe设备的cap structure */
        {
            capreg = FPCIE_READECAM_REG(addr, pos + 2);
            port_type = (capreg >> 4) & 0xf; /* Device/Port type */
            if ((port_type == 0x5))
            {
                return FPCIE_NEED_SKIP;
            }
            else
            {
                return FT_SUCCESS;
            }
        }
        pos++;
    }

    return FT_SUCCESS;
}


#endif

/**
 * @name: FPcieEcamEnumerateBusRecursion
 * @msg: This function recursively enumerates PCIe devices and functions on a bus.
 * @param {FPcieEcam} *instance_p - Pointer to the FPcieEcam instance.
 * @param {u8} bus - The PCI bus to be enumerated.
 * @return FError - Error code indicating the success or failure of the enumeration process.
 */
static FError FPcieEcamEnumerateBusRecursion(FPcieEcam *instance_p, u8 bus)
{
    u32 config_data;
    u16 device_id;
    u8 head_type, mult_func;

    FASSERT(instance_p != NULL);
    FASSERT(instance_p->is_ready == FT_COMPONENT_IS_READY);

    /* device loop */
    for (u32 pcie_dev_num = 0; pcie_dev_num < instance_p->config.max_dev_num; pcie_dev_num++)
    {
        /* function loop  */
        for (u32 pcie_func_num = 0; pcie_func_num < instance_p->config.max_fun_num; pcie_func_num++)
        {
#ifdef FPCIE_NEED_SKIP_TYPE0


            if (FPcieEcamSkipType0(instance_p, bus, pcie_dev_num, pcie_func_num))
            {
                continue;
            }

#endif

            if (instance_p->need_skip_fun)
            {
                if (instance_p->need_skip_fun(instance_p, bus, pcie_dev_num, pcie_func_num))
                {
                    continue;
                }
            }

            /* read vendor id */
            if (FPcieEcamReadConfigSpace(instance_p, bus, pcie_dev_num, pcie_func_num,
                                         FPCIE_CCR_ID_REG, &config_data))
            {
                FPCIE_DEBUG_E("%s:%d,Failed to read config space", __FUNCTION__, __LINE__);
                return FPCIE_READCONFIG_ERROR;
            }

            device_id = FPCIE_CCR_DEVICE_ID_MASK(config_data);


            if (device_id == FPCIE_DEVICE_NOT_PRESENT)
            {
                continue;
            }


            /* read header type */
            if (FPcieEcamReadConfigSpace(instance_p, bus, pcie_dev_num, pcie_func_num,
                                         FPCIE_CCR_CLS_LT_HT_REGS, &config_data))
            {
                FPCIE_DEBUG_E("%s:%d,Failed to read config space", __FUNCTION__, __LINE__);
                return FPCIE_READCONFIG_ERROR;
            }
            head_type = FPCIE_CCR_HEAD_TYPE_MASK(config_data);
            mult_func = FPCIE_CCR_MULT_FUNTION_MASK(config_data);

            /* type0 header */
            if (head_type == FPCIE_CCR_HEAD_TYPE0)
            {
                FPcieBdfInsert(instance_p, bus, pcie_dev_num, pcie_func_num);
                /* 开始决定分配内存 */
                FPcieAllocBarSpace(instance_p, FPCIE_CCR_HEAD_TYPE0, bus, pcie_dev_num, pcie_func_num);
            }
            else if (head_type == FPCIE_CCR_HEAD_TYPE1)
            {
                /*  type1 header */
                /* 先分配内存 */
                /* 递归执行枚举过程 */

                FPcieBdfInsert(instance_p, bus, pcie_dev_num, pcie_func_num);
                /* 开始决定分配内存 */
                FPcieAllocBarSpace(instance_p, FPCIE_CCR_HEAD_TYPE1, bus, pcie_dev_num, pcie_func_num);

                /* 桥下面device 枚举完成之后，最后进行bridge的设置 */
                FPcieBridgeProbeBus(instance_p, bus, pcie_dev_num, pcie_func_num);
            }
            else
            {
                FPCIE_DEBUG_W("Other type %d not surpport", head_type);

                FPcieBdfInsert(instance_p, bus, pcie_dev_num, pcie_func_num);
                /* 开始决定分配内存 */
                FPcieAllocBarSpace(instance_p, FPCIE_CCR_HEAD_TYPE0, bus, pcie_dev_num, pcie_func_num);
            }

            if (mult_func == 0 && pcie_func_num == 0)
            {
                break;
            }
        }
    }

    return FT_SUCCESS;
}

/**
 * @name: FPcieBridgeProbeBus
 * @msg: This function probes a PCIe bridge and sets up buses for enumeration.
 * @param {FPcieEcam} *instance_p - Pointer to the FPcieEcam instance.
 * @param {u8} bus - The PCI bus of the bridge.
 * @param {u8} device - The PCI device of the bridge.
 * @param {u8} function - The PCI function of the bridge.
 * @return u32 - The subordinate bus number of the bridge.
 */
static u32 FPcieBridgeProbeBus(FPcieEcam *instance_p, u8 bus, u8 device, u8 function)
{
    u32 secondary_bus, subordinate_bus;
    instance_p->bus_max++;
    secondary_bus = instance_p->bus_max;

    FPciePreScanSetupBridge(instance_p, secondary_bus, bus, device, function);

    FPcieEcamEnumerateBusRecursion(instance_p, secondary_bus);
    subordinate_bus = instance_p->bus_max;
    FPciePostscanSetupBridge(instance_p, subordinate_bus, bus, device, function);
    return subordinate_bus;
}


/**
 * @name: FPcieEcamEnumerateBus
 * @msg: This function initiates the enumeration process for a specific bus.
 * @param {FPcieEcam} *instance_p - Pointer to the FPcieEcam instance.
 * @param {u8} bus - The target PCI bus to enumerate.
 * @return FError - Error code indicating the success or failure of the enumeration process.
 */
FError FPcieEcamEnumerateBus(FPcieEcam *instance_p, u8 bus)
{
    FError ret;
    instance_p->bus_max = bus;
    instance_p->scans_bdf_count = 0;
    ret = FPcieEcamEnumerateBusRecursion(instance_p, bus);

    /* debug print */
    FPcieEcamInfoPrint(instance_p);
    return ret;
}


FError FPcieEcamGetBar(FPcieEcam *instance_p, u8 bus, u8 device, u8 function,
                       u32 bar_index, struct FPcieBarInfo *bar)
{
    uint32_t reg = bar_index + FPCIE_CCR_BAR_ADDR0_REGS;
    uint32_t cmd_reg;
    uint32_t phys_addr;
    uint32_t size_high = 0;
    uint64_t size = 0;
    uint32_t orig_bar_val;
    uint32_t orig_bar_high = 0;
    int is_64bit_bar = 0;
    int bar_modified = 0;
    FError err;

    /* 检查 BAR 索引是否有效 */
    if (reg > FPCIE_CCR_BAR_ADDR5_REGS)
    {
        return FPCIE_INVALID_PARAM;
    }

    /* 读取原始 BAR 值 */
    err = FPcieEcamReadConfigSpace(instance_p, bus, device, function, reg, &phys_addr);
    if (err != FT_SUCCESS)
    {
        return FPCIE_READCONFIG_ERROR;
    }
    orig_bar_val = phys_addr;

    /* 检查是否为64位BAR并保存高位原始值 */
    if (FPCIE_CONF_BAR_MEM(phys_addr) && FPCIE_CONF_BAR_64(phys_addr))
    {
        if (reg >= FPCIE_CCR_BAR_ADDR5_REGS)
        {
            return FPCIE_TYPE_NOT_FIT;
        }
        is_64bit_bar = 1;
        bar->is_64bit = 1;

        err = FPcieEcamReadConfigSpace(instance_p, bus, device, function, reg + 1, &orig_bar_high);
        if (err != FT_SUCCESS)
        {
            return FPCIE_READCONFIG_ERROR;
        }
    }

    /* 检查无效的标志位 */
    if (FPCIE_CONF_BAR_INVAL_FLAGS(phys_addr))
    {
        return FPCIE_TYPE_NOT_FIT;
    }

    /* 读取并修改命令寄存器 */
    err = FPcieEcamReadConfigSpace(instance_p, bus, device, function,
                                   FPCIE_CCR_CMD_STATUS_REGS, &cmd_reg);
    if (err != FT_SUCCESS)
    {
        return FPCIE_READCONFIG_ERROR;
    }

    /* 禁用内存和 I/O 解码 */
    err = FPcieEcamWriteConfigSpace(instance_p, bus, device, function, FPCIE_CCR_CMD_STATUS_REGS,
                                    cmd_reg & ~(FPCIE_CCR_CMD_IO_ACCESS_ENABLED |
                                                FPCIE_CCR_CMD_MEMORY_ACCESS_ENABLED));
    if (err != FT_SUCCESS)
    {
        goto err_exit;
    }

    /* 写入全1以确定 BAR 大小 */
    err = FPcieEcamWriteConfigSpace(instance_p, bus, device, function, reg, 0xFFFFFFFF);
    if (err != FT_SUCCESS)
    {
        goto err_exit;
    }
    bar_modified = 1;

    if (is_64bit_bar)
    {
        /* 写入高位BAR全1 */
        err = FPcieEcamWriteConfigSpace(instance_p, bus, device, function, reg + 1, 0xFFFFFFFF);
        if (err != FT_SUCCESS)
        {
            goto err_exit;
        }
    }

    /* 读取大小掩码 */
    err = FPcieEcamReadConfigSpace(instance_p, bus, device, function, reg, &phys_addr);
    if (err != FT_SUCCESS)
    {
        goto err_exit;
    }

    /* 处理 64 位 BAR */
    if (is_64bit_bar)
    {
        bar->mem_type = FPCIE_CONF_BAR_PREFETCH(phys_addr) ? MEM_TYPE_PREFETCH : MEM_TYPE_MEM;
        /* 读取高位大小 */
        err = FPcieEcamReadConfigSpace(instance_p, bus, device, function, reg + 1, &size_high);
        if (err != FT_SUCCESS)
        {
            goto err_exit;
        }

        size = ((uint64_t)size_high << 32) | FPCIE_CONF_BAR_ADDR(phys_addr);
    }
    else
    {
        if (FPCIE_CONF_BAR_IO(phys_addr))
        {
            bar->mem_type = MEM_TYPE_IO;
            size = FPCIE_CONF_BAR_IO_ADDR(phys_addr);
        }
        else
        {
            bar->mem_type = MEM_TYPE_MEM;
            size = FPCIE_CONF_BAR_ADDR(phys_addr);
        }
    }

    /* 计算实际大小 (取反加1) */
    size = ~size + 1;
    bar->size = size & ~(size - 1);

    /* 恢复原始 BAR 值 */
    err = FPcieEcamWriteConfigSpace(instance_p, bus, device, function, reg, orig_bar_val);
    if (err != FT_SUCCESS)
    {
        goto err_exit;
    }

    if (is_64bit_bar)
    {
        /* 恢复高位 BAR 值 */
        err = FPcieEcamWriteConfigSpace(instance_p, bus, device, function, reg + 1, orig_bar_high);
        if (err != FT_SUCCESS)
        {
            goto err_exit;
        }

        /* 组合64位物理地址 */
        bar->phys_addr = ((uint64_t)orig_bar_high << 32) | FPCIE_CONF_BAR_ADDR(orig_bar_val);
    }
    else
    {
        if (FPCIE_CONF_BAR_IO(orig_bar_val))
        {
            bar->phys_addr = FPCIE_CONF_BAR_IO_ADDR(orig_bar_val);
        }
        else
        {
            bar->phys_addr = FPCIE_CONF_BAR_ADDR(orig_bar_val);
        }
    }

    err = FT_SUCCESS;

err_exit:
    /* 恢复命令寄存器 */
    FPcieEcamWriteConfigSpace(instance_p, bus, device, function, FPCIE_CCR_CMD_STATUS_REGS, cmd_reg);

    /* 如果BAR被修改但未恢复（错误路径）*/
    if (bar_modified && err != FT_SUCCESS)
    {

        FPcieEcamWriteConfigSpace(instance_p, bus, device, function, reg, orig_bar_val);
        if (is_64bit_bar)
        {

            FPcieEcamWriteConfigSpace(instance_p, bus, device, function, reg + 1, orig_bar_high);
        }
    }
    return err;
}

/**
 * @name: FPcieEcamCfgInitialize
 * @msg: This function initializes the config space and PCIe bridge.
 * @param {FPcie} *instance_p is a pointer to the FPcie instance.
 * @param {FPcieEcamConfig} *config_p pointer to FPcieEcamConfig instrance Pointer.
 * @return FError
 */
FError FPcieEcamCfgInitialize(FPcieEcam *instance_p, FPcieEcamConfig *config_p,
                              FPcieEcamNeedSkip need_skip_fun) //用于从全局配置数据中获取数据，初始化instance_p
{
    struct FPcieRegion mem_region = {0};
    struct FPcieRegion io_region = {0};

    /* Assert arguments */
    FASSERT(instance_p != NULL);
    FASSERT(config_p != NULL);

    /* Clear instance memory and make copy of configuration */
    memset(instance_p, 0, sizeof(FPcieEcam));
    memcpy(&instance_p->config, config_p, sizeof(FPcieEcamConfig));

    /* 为枚举过程中，涉及的配置空间提供地址划分 */
    /* mem32 地址 */
    mem_region.phys_start = instance_p->config.npmem_base_addr;
    mem_region.bus_start = instance_p->config.npmem_base_addr;
    mem_region.size = instance_p->config.npmem_size;
    mem_region.flags = FPCIE_REGION_MEM;

    /* memio 地址 */
    io_region.phys_start = instance_p->config.io_base_addr;
    io_region.bus_start = instance_p->config.io_base_addr;
    io_region.size = instance_p->config.io_size;
    io_region.flags = FPCIE_REGION_IO;

    /* scaned bdf array clean */
    instance_p->scans_bdf_count = 0;

    /* skip fun */
    instance_p->need_skip_fun = need_skip_fun;

    FPcieRegionConfigInit(instance_p, &mem_region, 1);

#if defined(FAARCH64_USE)
    struct FPcieRegion prefetch_region = {0};

    /* mem64 地址 */
    prefetch_region.phys_start = instance_p->config.pmem_base_addr;
    prefetch_region.bus_start = instance_p->config.pmem_base_addr;
    prefetch_region.size = instance_p->config.pmem_size;
    prefetch_region.flags = (PCI_REGION_PREFETCH | FPCIE_REGION_MEM);

    FPcieRegionConfigInit(instance_p, &prefetch_region, 1);
#endif

    FPcieRegionConfigInit(instance_p, &io_region, 1);

    instance_p->is_ready = FT_COMPONENT_IS_READY;

    return (FT_SUCCESS);
}
