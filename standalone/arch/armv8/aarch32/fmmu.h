/*
 * Copyright (C) 2024, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: fmmu.h
 * Created Date: 2024-05-11 11:58:19
 * Last Modified: 2024-06-06 09:36:27
 * Description:  This file is for mmu map and unmap function.
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 *  1.0  huanghe	2024-06-06			initialization
 */

#ifndef FMMU_H
#define FMMU_H

#include "ftypes.h"
#include "fprintk.h"
#include "ferror_code.h"

#ifdef __cplusplus
extern "C"
{
#endif


#ifdef CONFIG_MMU_DEBUG_PRINTS
/* To dump page table entries while filling them, set DUMP_PTE macro */
#define DUMP_PTE            1
#define MMU_DEBUG(fmt, ...) f_printk(fmt, ##__VA_ARGS__)
#else
#define MMU_DEBUG(...)
#endif

#define MMU_WRNING(fmt, ...)       f_printk(fmt, ##__VA_ARGS__)


#define FMMU_SUCCESS               FT_SUCCESS
#define FMMU_L2_PAGE_NOT_INVALID   (FMMU_SUCCESS + 1)
#define FMMU_L1_PAGE_NOT_INVALID   (FMMU_SUCCESS + 2)
#define FMMU_VIRT_ADDR_NOT_INVALID (FMMU_SUCCESS + 3)
#define FMMU_SIZE_NOT_INVALID      (FMMU_SUCCESS + 4)


#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) ((long)((sizeof(array) / sizeof((array)[0]))))
#endif


/* 0~5  bit 0-bit2 */
#define MT_STRONGLY_ORDERED_INDEX    0U
#define MT_DEVICE_INDEX              1U
#define MT_NORMAL_INDEX              2U
#define MT_MASK                      3


/* bit 3-bit14 */
/* Bit positions for memory attributes */
#define MT_SHAREABLE_SHIFT           3U // 定义共享属性的位位置（第3位）
#define MT_CACHE_OUTER_SHIFT         4U // 定义外部缓存属性的起始位位置（第4位），位大小为2
#define MT_CACHE_INNER_SHIFT         6U // 定义内部缓存属性的起始位位置（第6位），位大小为2
#define MT_NON_SECURE_SHIFT          8U // 定义非安全属性的位位置（第8位）
#define MT_NERVER_EXECUTE_SHIFT      9U  // 定义执行权限的位位置（第9位）
#define MT_WRITE_SHIFT               10U // 定义写权限的位位置（第10位）
#define MT_READ_SHIFT                11U // 定义读权限的位位置（第11位）
#define MT_USERUNPRIVILEGED_SHIFT    12U // 定义用户非特权模式的位位置（第12位）
#define MT_NON_GLOBAL_SHIFT          13U // 定义非全局属性的位位置（第13位）
#define MT_MAP_L1_SECTION_SHIFT      14U // 使用映射L1段的位位置（第14位）
#define MT_MAP_L1_NO_OVERWRITE_SHIFT 15U // 使用映射L1覆盖的位位置（第15位）

/* Definitions for setting specific memory attributes */
#define MT_SHAREABLE                 (1U << MT_SHAREABLE_SHIFT) // 设置共享属性的位掩码
#define MT_CACHE_OUTER_WB_WA \
    (0U << MT_CACHE_OUTER_SHIFT) // 设置外部缓存为Write-Back, Write-Allocate
#define MT_CACHE_OUTER_WT_NWA \
    (1U << MT_CACHE_OUTER_SHIFT) // 设置外部缓存为Write-Through, No Write-Allocate
#define MT_CACHE_OUTER_WB_NWA \
    (2U << MT_CACHE_OUTER_SHIFT) // 设置外部缓存为Write-Back, No Write-Allocate

#define MT_CACHE_INNER_WB_WA \
    (0U << MT_CACHE_INNER_SHIFT) // 设置内部缓存为Write-Back, Write-Allocate
#define MT_CACHE_INNER_WT_NWA \
    (1U << MT_CACHE_INNER_SHIFT) // 设置内部缓存为Write-Through, No Write-Allocate
#define MT_CACHE_INNER_WB_NWA \
    (2U << MT_CACHE_INNER_SHIFT) // 设置内部缓存为Write-Back, No Write-Allocate

#define MT_NON_SECURE     (1U << MT_NON_SECURE_SHIFT) // 设置安全属性的位掩码
#define MT_NERVER_EXECUTE (1U << MT_NERVER_EXECUTE_SHIFT) // 设置执行权限的位掩码
#define MT_WRITE          (1U << MT_WRITE_SHIFT)          // 设置写权限的位掩码
#define MT_READ           (1U << MT_READ_SHIFT)           // 设置读权限的位掩码
#define MT_USERUNPRIVILEGED \
    (1U << MT_USERUNPRIVILEGED_SHIFT) // 设置用户非特权模式的位掩码

#define MT_NON_GLOBAL     (1U << MT_NON_GLOBAL_SHIFT) // 设置全局属性的位掩码

#define MT_MAP_L1_SECTION (1U << MT_MAP_L1_SECTION_SHIFT) // 设置映射L1段的位掩码


/* MT_P_RW_U_RW: Read/Write permissions for both privileged and unprivileged levels,
 * no execution allowed in any mode. */
#define MT_P_RW_U_RW      (MT_WRITE | MT_READ | MT_NERVER_EXECUTE | MT_USERUNPRIVILEGED)
/* MT_P_RW_U_NA: Read/Write permissions for privileged level only, no access from
 * unprivileged level, no execution allowed in any mode. */
#define MT_P_RW_U_NA      (MT_WRITE | MT_READ | MT_NERVER_EXECUTE)
/* MT_P_RO_U_RO: Read-Only permissions for both privileged and unprivileged levels,
 * no execution allowed in any mode. */
#define MT_P_RO_U_RO      (MT_READ | MT_NERVER_EXECUTE | MT_USERUNPRIVILEGED)
/* MT_P_RO_U_NA: Read-Only permission for privileged level only, no access from
 * unprivileged level, no execution allowed in any mode. */
#define MT_P_RO_U_NA      (MT_READ | MT_NERVER_EXECUTE)
/* MT_P_RX_U_RX: Read and Execute permissions for both privileged and unprivileged levels. */
#define MT_P_RX_U_RX      (MT_READ | MT_USERUNPRIVILEGED)
/* MT_P_RX_U_NA: Read and Execute permissions for privileged level, no access for
 * unprivileged level, execution allowed only in privileged mode. */
#define MT_P_RX_U_NA      (MT_READ)

#define MT_P_RWX_U_NA     (MT_READ | MT_WRITE)


#define MT_DEVICE_NGNRNE  (MT_STRONGLY_ORDERED_INDEX)
#define MT_NORMAL         (MT_NORMAL_INDEX | MT_SHAREABLE)
#define MT_NS             MT_NON_SECURE
#define MT_NO_OVERWRITE   (1U << MT_MAP_L1_NO_OVERWRITE_SHIFT)

/* Convenience macros to represent the ARMv8-A-specific
* configuration for memory access permission and
* cache-ability attribution.
*/

#define MMU_REGION_ENTRY(_name, _base_pa, _base_va, _size, _attrs)                               \
    {                                                                                            \
        .name = _name, .base_pa = _base_pa, .base_va = _base_va, .size = _size, .attrs = _attrs, \
    }

#define MMU_REGION_FLAT_ENTRY(name, adr, sz, attrs) \
    MMU_REGION_ENTRY(name, adr, adr, sz, attrs)


struct ArmMmuRegion
{
    /* Region Base Physical Address */
    uintptr_t base_pa;
    /* Region Base Virtual Address */
    uintptr_t base_va;
    /* Region size */
    size_t size;
    /* Region Name */
    const char *name;
    /* Region Attributes */
    fsize_t attrs;
};

/* MMU configuration data structure */
struct ArmMmuConfig
{
    /* Number of regions */
    unsigned int num_regions;
    /* Regions */
    const struct ArmMmuRegion *mmu_regions;
};

void MmuInit(void);
int FMmuMap(uintptr virt, uintptr_t phys, fsize_t size, u32 flags);
int FMmuUnMap(uintptr addr, fsize_t size);


/* debug print */
int FMmuPrintReconstructedTables(int print_detailed);
void DisplayMmuUsage(void);


static inline void FMmuMapRetopology(void)
{
}

fsize_t FMmuGetMappingByVA(uintptr va, struct ArmMmuRegion *mmu_region);
fsize_t FMmuGetMappingsByPA(uintptr pa, struct ArmMmuRegion *mmu_region, int max_ranges);

fsize_t FMmuGetAllVaByPA(uintptr_t pa, uintptr_t *va_out, fsize_t max_mappings);
fsize_t FMmuGetPAByVA(uintptr_t va, uintptr_t *pa_out);

#ifdef __cplusplus
}
#endif


#endif /* FMMU_H */