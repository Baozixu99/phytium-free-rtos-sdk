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
 * FilePath: fmmu.c
 * Created Date: 2024-05-11 11:58:12
 * Last Modified: 2024-08-14 14:55:13
 * Description:  This file is for mmu map and unmap function.
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 *  1.0  huanghe	2024-06-06			initialization
 *  1.2   huanghe   2024-07-24    add mmu init for secondary cpu
 */

#include "fmmu.h"
#include "ftypes.h"
#include "fkernel.h"
#include "fassert.h"
#include "faarch.h"
#include "sdkconfig.h"
#include "fcache.h"
#include <string.h>

#ifndef CONFIG_MMU_PAGE_SIZE
#define CONFIG_MMU_PAGE_SIZE 4096
#endif // !CONFIG_MMU_PAGE_SIZE

/*
 * Comp.:
 * ARM Architecture Reference Manual, ARMv7-A and ARMv7-R edition
 * ARM document ID DDI0406C Rev. d, March 2018
 * L1 / L2 page table entry formats and entry type IDs:
 * Chapter B3.5.1, fig. B3-4 and B3-5, p. B3-1323 f.
 */

#define FMMU_PT_L1_NUM_ENTRIES               4096
#define FMMU_PT_L2_NUM_ENTRIES               256

#define FMMU_PTE_L1_INDEX_PA_SHIFT           20
#define FMMU_PTE_L1_INDEX_MASK               0xFFF
#define FMMU_PTE_L2_INDEX_PA_SHIFT           12
#define FMMU_PTE_L2_INDEX_MASK               0xFF
#define FMMU_PT_L2_ADDR_SHIFT                10
#define FMMU_PT_L2_ADDR_MASK                 0x3FFFFF
#define FMMU_PTE_L2_SMALL_PAGE_ADDR_SHIFT    12
#define FMMU_PTE_L2_SMALL_PAGE_ADDR_MASK     0xFFFFF
#define FMMU_ADDR_BELOW_PAGE_GRAN_MASK       0xFFF

#define FMMU_PTE_ID_INVALID                  0x0
#define FMMU_PTE_ID_L2_PT                    0x1
#define FMMU_PTE_ID_SECTION                  0x2
#define FMMU_PTE_ID_LARGE_PAGE               0x1
#define FMMU_PTE_ID_SMALL_PAGE               0x2

#define FMMU_PERMS_AP2_DISABLE_WR            0x2
#define FMMU_PERMS_AP1_ENABLE_PL0            0x1
#define FMMU_TEX2_CACHEABLE_MEMORY           0x4

#define FMMU_TEX_CACHE_ATTRS_WB_WA           0x1
#define FMMU_TEX_CACHE_ATTRS_WT_nWA          0x2
#define FMMU_TEX_CACHE_ATTRS_WB_nWA          0x3
#define FMMU_C_CACHE_ATTRS_WB_WA             0
#define FMMU_B_CACHE_ATTRS_WB_WA             1
#define FMMU_C_CACHE_ATTRS_WT_nWA            1
#define FMMU_B_CACHE_ATTRS_WT_nWA            0
#define FMMU_C_CACHE_ATTRS_WB_nWA            1
#define FMMU_B_CACHE_ATTRS_WB_nWA            1

/*
 * The following defines might vary if support for CPUs without
 * the multiprocessor extensions was to be implemented:
 */

#define FMMU_TTBR_IRGN0_BIT_MP_EXT_ONLY      BIT(6)
#define FMMU_TTBR_NOS_BIT                    BIT(5)
#define FMMU_TTBR_RGN_OUTER_NON_CACHEABLE    0x0
#define FMMU_TTBR_RGN_OUTER_WB_WA_CACHEABLE  0x1
#define FMMU_TTBR_RGN_OUTER_WT_CACHEABLE     0x2
#define FMMU_TTBR_RGN_OUTER_WB_nWA_CACHEABLE 0x3
#define FMMU_TTBR_RGN_SHIFT                  3
#define FMMU_TTBR_SHAREABLE_BIT              BIT(1)
#define FMMU_TTBR_IRGN1_BIT_MP_EXT_ONLY      BIT(0)
#define FMMU_TTBR_CACHEABLE_BIT_NON_MP_ONLY  BIT(0)

/* <-- end MP-/non-MP-specific */

#define FMMU_DOMAIN_OS                       0
#define FMMU_DOMAIN_DEVICE                   1
#define FMMU_DACR_ALL_DOMAINS_CLIENT         0x55555555

#define FMMU_SCTLR_AFE_BIT                   BIT(29)
#define FMMU_SCTLR_TEX_REMAP_ENABLE_BIT      BIT(28)
#define FMMU_SCTLR_HA_BIT                    BIT(17)
#define FMMU_SCTLR_ICACHE_ENABLE_BIT         BIT(12)
#define FMMU_SCTLR_DCACHE_ENABLE_BIT         BIT(2)
#define FMMU_SCTLR_CHK_ALIGN_ENABLE_BIT      BIT(1)
#define FMMU_SCTLR_MMU_ENABLE_BIT            BIT(0)

struct FMmuPermsAttrsShow
{
    u32 acc_ap     : 2;
    u32 acc_apx    : 1;
    u32 bufferable : 1;
    u32 cacheable  : 1;
    u32 not_global : 1;
    u32 non_sec    : 1;
    u32 shared     : 1;
    u32 tex        : 3;
    u32 exec_never : 1;
    u32 id_mask    : 2;
    u32 domain     : 4;
    u32 reserved   : 15;
};

struct FMmuTableRange
{
    u32 start_va;
    u32 end_va;
    u32 start_pa;
    u32 end_pa;
    struct FMmuPermsAttrsShow perms_attrs;
    u32 size;
    const char *table_type;
};


const char *l2_page_string = "L2 page";
const char *l1_section_string = "L1 section";


struct FMmuTableRange current_range = {0};
int range_active = 0;

extern const struct ArmMmuConfig mmu_config;
extern const struct ArmMmuConfig code_mmu_config;


_WEAK void FCacheL3CacheFlush(void)
{
    return;
}
_WEAK void FCacheL3CacheDisable(void)
{
    return;
}


#define FMMU_L2_PT_INDEX(pt) \
    ((u32)(pt) - (u32)l2_page_tables) / sizeof(struct FMmuL2PageTable);

union FMmuL1PageTableEntry
{
    struct
    {
        u32 id           : 2; /* [00] */
        u32 bufferable   : 1;
        u32 cacheable    : 1;
        u32 exec_never   : 1;
        u32 domain       : 4;
        u32 impl_def     : 1;
        u32 acc_perms10  : 2;
        u32 tex          : 3;
        u32 acc_perms2   : 1;
        u32 shared       : 1;
        u32 not_global   : 1;
        u32 zero         : 1;
        u32 non_sec      : 1;
        u32 base_address : 12; /* [31] */
    } l1_section_1m;
    struct
    {
        u32 id                    : 2; /* [00] */
        u32 zero0                 : 1; /* PXN if avail. */
        u32 non_sec               : 1;
        u32 zero1                 : 1;
        u32 domain                : 4;
        u32 impl_def              : 1;
        u32 l2_page_table_address : 22; /* [31] */
    } l2_page_table_ref;
    struct
    {
        u32 id       : 2;  /* [00] */
        u32 reserved : 30; /* [31] */
    } undefined;
    u32 word;
};

struct FMmuL1PageTable
{
    union FMmuL1PageTableEntry entries[FMMU_PT_L1_NUM_ENTRIES];
};

union FMmuL2PageTableEntry
{
    struct
    {
        u32 id          : 2; /* [00] */
        u32 bufferable  : 1;
        u32 cacheable   : 1;
        u32 acc_perms10 : 2;
        u32 tex         : 3;
        u32 acc_perms2  : 1;
        u32 shared      : 1;
        u32 not_global  : 1;
        u32 pa_base     : 20; /* [31] */
    } l2_page_4k;
    struct
    {
        u32 id          : 2; /* [00] */
        u32 bufferable  : 1;
        u32 cacheable   : 1;
        u32 acc_perms10 : 2;
        u32 zero        : 3;
        u32 acc_perms2  : 1;
        u32 shared      : 1;
        u32 not_global  : 1;
        u32 tex         : 3;
        u32 exec_never  : 1;
        u32 pa_base     : 16; /* [31] */
    } l2_page_64k;
    struct
    {
        u32 id       : 2;  /* [00] */
        u32 reserved : 30; /* [31] */
    } undefined;
    u32 word;
};

struct FMmuL2PageTable
{
    union FMmuL2PageTableEntry entries[FMMU_PT_L2_NUM_ENTRIES];
};

/*
 * Data structure for L2 table usage tracking, contains a
 * L1 index reference if the respective L2 table is in use.
 */

struct FMmuL2PageTableStatus
{
    u32 l1_index : 12;
    u32 entries  : 9;
    u32 reserved : 11;
};


/*
 * Data structure containing the memory attributes and permissions
 * data derived from a memory region's attr flags word in the format
 * required for setting up the corresponding PTEs.
 */
struct FMmuPermsAttrs
{
    u32 acc_perms  : 2;
    u32 bufferable : 1;
    u32 cacheable  : 1;
    u32 not_global : 1;
    u32 non_sec    : 1;
    u32 shared     : 1;
    u32 tex        : 3;
    u32 exec_never : 1;
    u32 id_mask    : 2;
    u32 domain     : 4;
    u32 reserved   : 14;
};

/* Level 1 page table: always required, must be 16k-aligned */
struct FMmuL1PageTable l1_page_table __aligned(KB(16)) = {0};
/*
 * Array of level 2 page tables with 4k granularity:
 * each table covers a range of 1 MB, the number of L2 tables
 * is configurable.
 */
static struct FMmuL2PageTable l2_page_tables[CONFIG_FMMU_NUM_L2_TABLES] __aligned(KB(1)) = {0};
/*
 * For each level 2 page table, a separate dataset tracks
 * if the respective table is in use, if so, to which 1 MB
 * virtual address range it is assigned, and how many entries,
 * each mapping a 4 kB page, it currently contains.
 */
static struct FMmuL2PageTableStatus l2_page_tables_status[CONFIG_FMMU_NUM_L2_TABLES] = {0};

/* Available L2 tables count & next free index for an L2 table request */
static u32 fmmu_l2_tables_free = CONFIG_FMMU_NUM_L2_TABLES;
static u32 fmmu_l2_next_free_table;

static void FMmmL2MapPage(u32 va, u32 pa, struct FMmuPermsAttrs perms_attrs);


static void FMmuInvalidateTlb(void)
{
    AARCH32_WRITE_SYSREG_32(TLBIALL, 0U);
    /* Invalidate all entries from branch predictors. */
    AARCH32_WRITE_SYSREG_32(BPIALL, 0U);

    DSB();
    ISB();
}

/**
 * @name: FMmuAssignL2Table
 * @msg: Allocates a free Level 2 (L2) page table and assigns it to manage a specific virtual address range.
 * @return: struct FMmuL2PageTable* - Pointer to the assigned L2 page table.
 * @note: This function ensures that a free L2 page table is available and then assigns it to manage a 1 MB virtual address range.
 *        It performs checks to ensure that the selected table is indeed free (i.e., has no entries), and it updates global tracking
 *        variables to manage the pool of free L2 tables. It handles wrapping around the L2 table pool when searching for the next
 *        free table.
 * @param {u32} va - The base virtual address that the new L2 table will manage.
 */
static struct FMmuL2PageTable *FMmuAssignL2Table(u32 va)
{
    struct FMmuL2PageTable *l2_page_table;

    FASSERT_MSG(fmmu_l2_tables_free > 0,
                "Cannot set up L2 page table for VA 0x%lx: "
                "no more free L2 page tables available\n",
                va);
    FASSERT_MSG(l2_page_tables_status[fmmu_l2_next_free_table].entries == 0,
                "Cannot set up L2 page table for VA 0x%lx: "
                "expected empty L2 table at index [%lu], but the "
                "entries value is %d\n",
                va, fmmu_l2_next_free_table,
                l2_page_tables_status[fmmu_l2_next_free_table].entries);

    /*
	 * Store in the status dataset of the L2 table to be returned
	 * which 1 MB virtual address range it is being assigned to.
	 * Set the current page table entry count to 0.
	 */
    l2_page_tables_status[fmmu_l2_next_free_table].l1_index = ((va >> FMMU_PTE_L1_INDEX_PA_SHIFT) &
                                                               FMMU_PTE_L1_INDEX_MASK);
    l2_page_tables_status[fmmu_l2_next_free_table].entries = 0;
    l2_page_table = &l2_page_tables[fmmu_l2_next_free_table];

    /*
	 * Decrement the available L2 page table count. As long as at
	 * least one more L2 table is available afterwards, update the
	 * L2 next free table index. If we're about to return the last
	 * available L2 table, calculating a next free table index is
	 * impossible.
	 */
    --fmmu_l2_tables_free;
    if (fmmu_l2_tables_free > 0)
    {
        do
        {
            fmmu_l2_next_free_table = (fmmu_l2_next_free_table + 1) % CONFIG_FMMU_NUM_L2_TABLES;
        } while (l2_page_tables_status[fmmu_l2_next_free_table].entries != 0);
    }

    return l2_page_table;
}


/**
 * @name: FMmuReleaseL2Table
 * @msg: Marks an L2 page table as free and resets its tracking status.
 * @return: void
 * @note: This function is called when all entries in an L2 table are removed, effectively marking it as free and ready for reuse.
 *        It resets the table's tracking status and updates the global counter and index for available L2 tables. If it's the first
 *        table being released when all were previously allocated, it also resets the index to the next available table.
 * @param {struct FMmuL2PageTable*} l2_page_table - Pointer to the L2 page table being released.
 */
static void FMmuReleaseL2Table(struct FMmuL2PageTable *l2_page_table)
{
    u32 l2_page_table_index = FMMU_L2_PT_INDEX(l2_page_table);

    l2_page_tables_status[l2_page_table_index].l1_index = 0;
    if (fmmu_l2_tables_free == 0)
    {
        fmmu_l2_next_free_table = l2_page_table_index;
    }
    ++fmmu_l2_tables_free;
}


/**
 * @name: FMmuIncL2TableEntries
 * @msg: Increments the count of active entries in an L2 page table.
 * @return: void
 * @note: This function increments the entry count for an L2 table, ensuring it doesn't exceed the maximum number of entries allowed.
 *        It validates that the increment operation is within the bounds of the table's capacity. If the table is at its maximum capacity,
 *        an assertion failure is triggered.
 * @param {struct FMmuL2PageTable*} l2_page_table - Pointer to the L2 page table whose entry count is to be incremented.
 */
static void FMmuIncL2TableEntries(struct FMmuL2PageTable *l2_page_table)
{
    u32 l2_page_table_index = FMMU_L2_PT_INDEX(l2_page_table);

    FASSERT_MSG(l2_page_tables_status[l2_page_table_index].entries < FMMU_PT_L2_NUM_ENTRIES,
                "Cannot increment entry count of the L2 page table at index "
                "[%lu] / addr %p / ref L1[%d]: maximum entry count already reached",
                l2_page_table_index, l2_page_table,
                l2_page_tables_status[l2_page_table_index].l1_index);

    ++l2_page_tables_status[l2_page_table_index].entries;
}


/**
 * @name: FMmuDecL2TableEntries
 * @msg: Decrements the count of active entries in an L2 page table.
 * @return: void
 * @note: This function decrements the entry count for an L2 table and releases the table if the count reaches zero, indicating the table is no longer in use.
 *        It ensures the operation does not decrement the count below zero, which would be an invalid operation. If decrementing results in zero entries,
 *        the table is passed to `FMmuReleaseL2Table` to be marked as free and available for reassignment.
 * @param {struct FMmuL2PageTable*} l2_page_table - Pointer to the L2 page table whose entry count is to be decremented.
 */
static void FMmuDecL2TableEntries(struct FMmuL2PageTable *l2_page_table)
{
    u32 l2_page_table_index = FMMU_L2_PT_INDEX(l2_page_table);

    FASSERT_MSG(l2_page_tables_status[l2_page_table_index].entries > 0,
                "Cannot decrement entry count of the L2 page table at index "
                "[%lu] / addr %p / ref L1[%d]: entry count is already zero",
                l2_page_table_index, l2_page_table,
                l2_page_tables_status[l2_page_table_index].l1_index);

    if (--l2_page_tables_status[l2_page_table_index].entries == 0)
    {
        FMmuReleaseL2Table(l2_page_table);
    }
}


/**
 * @name: FMmuConvertAttrFlags
 * @msg: Converts attribute flags to permission attributes for page table entries.
 * @return: struct FMmuPermsAttrs - Returns a structure filled with permission attributes.
 * @note: This function takes a set of memory attributes as input and translates them into the format used for MMU page table
 *        entries, handling various memory types and configurations (e.g., strongly ordered, device, normal). It performs checks
 *        to ensure the attribute flags are valid and consistent (e.g., write-permission requires read-permission, executable memory
 *        cannot be writable). The function also configures domain settings and security aspects based on the attributes. Errors in
 *        attribute flags result in assertions to prevent misconfiguration in memory management setups.
 * @param {u32} attrs - An unsigned 32-bit integer representing memory attribute flags.
 */
static struct FMmuPermsAttrs FMmuConvertAttrFlags(u32 attrs)
{
    struct FMmuPermsAttrs perms_attrs = {0};

    FASSERT_MSG(!((attrs & MT_WRITE) && !(attrs & MT_READ)),
                "attrs must not define write permission without read "
                "permission");

    /* mask memory type */
    switch (MT_MASK & attrs)
    {
        case MT_STRONGLY_ORDERED_INDEX:
            perms_attrs.tex = 0;
            perms_attrs.cacheable = 0;
            perms_attrs.bufferable = 0;
            perms_attrs.shared = 0;
            perms_attrs.domain = FMMU_DOMAIN_DEVICE;
            break;
        case MT_DEVICE_INDEX:
            /*
		 * Shareability of device memory is determined by TEX, C, B.
		 * The S bit is ignored. C is always 0 for device memory.
		 */
            perms_attrs.shared = 0;
            perms_attrs.cacheable = 0;
            perms_attrs.domain = FMMU_DOMAIN_DEVICE;

            if (attrs & MT_SHAREABLE)
            {
                perms_attrs.tex = 0;
                perms_attrs.bufferable = 1;
            }
            else
            {
                perms_attrs.tex = 2;
                perms_attrs.bufferable = 0;
            }
            break;
        case MT_NORMAL_INDEX:
            /*
		 * TEX[2] is always 1. TEX[1:0] contain the outer cache attri-
		 * butes encoding, C and B contain the inner cache attributes
		 * encoding.
		 */
            perms_attrs.tex |= FMMU_TEX2_CACHEABLE_MEMORY;
            perms_attrs.domain = FMMU_DOMAIN_OS;

            /* For normal memory, shareability depends on the S bit */
            if (attrs & MT_SHAREABLE)
            {
                perms_attrs.shared = 1;
            }

            if (attrs & MT_CACHE_OUTER_WT_NWA)
            {
                perms_attrs.tex |= FMMU_TEX_CACHE_ATTRS_WT_nWA;
            }
            else if (attrs & MT_CACHE_OUTER_WB_NWA)
            {
                perms_attrs.tex |= FMMU_TEX_CACHE_ATTRS_WB_nWA;
            }
            else
            {
                perms_attrs.tex |= FMMU_TEX_CACHE_ATTRS_WB_WA;
            }


            if (attrs & MT_CACHE_INNER_WT_NWA)
            {
                perms_attrs.cacheable = FMMU_C_CACHE_ATTRS_WT_nWA;
                perms_attrs.bufferable = FMMU_B_CACHE_ATTRS_WT_nWA;
            }
            else if (attrs & MT_CACHE_INNER_WB_NWA)
            {
                perms_attrs.cacheable = FMMU_C_CACHE_ATTRS_WB_nWA;
                perms_attrs.bufferable = FMMU_B_CACHE_ATTRS_WB_nWA;
            }
            else
            {
                perms_attrs.cacheable = FMMU_C_CACHE_ATTRS_WB_WA;
                perms_attrs.bufferable = FMMU_B_CACHE_ATTRS_WB_WA;
            }

            break;
        default:
            FASSERT_MSG(0, "unknown memory type");
            break;
    }

    if (attrs & MT_NON_SECURE)
    {
        perms_attrs.non_sec = 1;
    }

    if (attrs & MT_NON_GLOBAL)
    {
        perms_attrs.not_global = 1;
    }

    /*
	 * Up next is the consideration of the case that a PTE shall be configured
	 * for a page that shall not be accessible at all (e.g. guard pages), and
	 * therefore has neither read nor write permissions. In the AP[2:1] access
	 * permission specification model, the only way to indicate this is to
	 * actually mask out the PTE's identifier bits, as otherwise, read permission
	 * is always granted for any valid PTE, it can't be revoked explicitly,
	 * unlike the write permission.
    */

    if (!((attrs & MT_READ) || (attrs & MT_WRITE)))
    {
        perms_attrs.id_mask = 0x0;
    }
    else
    {
        perms_attrs.id_mask = 0x3;
    }
    if (!(attrs & MT_WRITE))
    {
        perms_attrs.acc_perms |= FMMU_PERMS_AP2_DISABLE_WR;
    }
    if (attrs & MT_USERUNPRIVILEGED)
    {
        perms_attrs.acc_perms |= FMMU_PERMS_AP1_ENABLE_PL0;
    }
    if ((attrs & MT_NERVER_EXECUTE))
    {
        perms_attrs.exec_never = 1;
    }


    if ((attrs & MT_NO_OVERWRITE))
    {
        perms_attrs.reserved |= 1;
    }

    return perms_attrs;
}


/**
 * @name: FMmuL1MapSection
 * @msg: Maps a 1 MB section in the level 1 page table with specified permissions and attributes.
 * @return: void
 * @note: This function sets up a 1 MB section mapping in the L1 page table. It assumes the L1 page table entry (PTE) at the
 *        calculated index is invalid before mapping, and asserts if this is not the case to prevent accidental overwriting of
 *        existing mappings. The function configures all relevant attributes of the section, such as access permissions, 
 *        cache settings, execution permissions, and security settings. It is primarily used for mapping large contiguous memory
 *        regions efficiently in the memory management unit (MMU).
 * @param {u32} va - The virtual address at which the section begins; must be aligned to a 1 MB boundary.
 * @param {u32} pa - The physical address of the section to map; also must be aligned to a 1 MB boundary.
 * @param {struct FMmuPermsAttrs} perms_attrs - The permissions and attributes to apply to the mapped section.
 */
static void FMmuL1MapSection(u32 va, u32 pa, struct FMmuPermsAttrs perms_attrs)
{
    u32 l1_index = (va >> FMMU_PTE_L1_INDEX_PA_SHIFT) & FMMU_PTE_L1_INDEX_MASK;

    if (perms_attrs.reserved & 0x1)
    {
        FASSERT_MSG((l1_page_table.entries[l1_index].undefined.id == FMMU_PTE_ID_INVALID),
                    "Unexpected non-zero L1 PTE ID %u for VA 0x%lx / PA 0x%lx",
                    l1_page_table.entries[l1_index].undefined.id, va, pa);
    }


    l1_page_table.entries[l1_index].l1_section_1m.id = (FMMU_PTE_ID_SECTION &
                                                        perms_attrs.id_mask);
    l1_page_table.entries[l1_index].l1_section_1m.bufferable = perms_attrs.bufferable;
    l1_page_table.entries[l1_index].l1_section_1m.cacheable = perms_attrs.cacheable;
    l1_page_table.entries[l1_index].l1_section_1m.exec_never = perms_attrs.exec_never;
    l1_page_table.entries[l1_index].l1_section_1m.domain = perms_attrs.domain;
    l1_page_table.entries[l1_index].l1_section_1m.impl_def = 0;
    l1_page_table.entries[l1_index].l1_section_1m.acc_perms10 = ((perms_attrs.acc_perms & 0x1) << 1) | 0x1;
    l1_page_table.entries[l1_index].l1_section_1m.tex = perms_attrs.tex;
    l1_page_table.entries[l1_index].l1_section_1m.acc_perms2 = (perms_attrs.acc_perms >> 1) & 0x1;
    l1_page_table.entries[l1_index].l1_section_1m.shared = perms_attrs.shared;
    l1_page_table.entries[l1_index].l1_section_1m.not_global = perms_attrs.not_global;
    l1_page_table.entries[l1_index].l1_section_1m.zero = 0;
    l1_page_table.entries[l1_index].l1_section_1m.non_sec = perms_attrs.non_sec;
    l1_page_table.entries[l1_index].l1_section_1m.base_address = (pa >> FMMU_PTE_L1_INDEX_PA_SHIFT);
}


/**
 * @name: FMmuRemapL1SectionToL2Table
 * @msg: Converts a 1 MB section mapping into a level 2 page table mapping, preserving the original permissions and attributes.
 * @return: void
 * @note: This function is critical for scenarios where finer granularity of memory management is required at a location 
 *        previously managed by a single 1 MB section. It disables interrupts to maintain system stability during the 
 *        transition, maps all individual 4 kB pages within the 1 MB section using the original section's permissions,
 *        and then re-enables interrupts after the operation. This function ensures the consistency and integrity of 
 *        memory mappings during dynamic changes in the memory management unit (MMU) configuration.
 * @param {u32} va - The base virtual address of the 1 MB section to be converted. The address will be aligned to the 1 MB boundary.
 * @param {struct FMmuL2PageTable *} l2_page_table - Pointer to the newly allocated L2 page table that will replace the 1 MB section.
 */
static void FMmuRemapL1SectionToL2Table(u32 va, struct FMmuL2PageTable *l2_page_table)
{
    struct FMmuPermsAttrs perms_attrs = {0};
    u32 l1_index = (va >> FMMU_PTE_L1_INDEX_PA_SHIFT) & FMMU_PTE_L1_INDEX_MASK;
    u32 rem_size = MB(1);
    int lock_key;

    /*
	 * Extract the permissions and attributes from the current 1 MB section entry.
	 * This data will be carried over to the resulting L2 page table.
	 */

    perms_attrs.acc_perms = (l1_page_table.entries[l1_index].l1_section_1m.acc_perms2 << 1) |
                            ((l1_page_table.entries[l1_index].l1_section_1m.acc_perms10 >> 1) & 0x1);
    perms_attrs.bufferable = l1_page_table.entries[l1_index].l1_section_1m.bufferable;
    perms_attrs.cacheable = l1_page_table.entries[l1_index].l1_section_1m.cacheable;
    perms_attrs.domain = l1_page_table.entries[l1_index].l1_section_1m.domain;
    perms_attrs.id_mask = (l1_page_table.entries[l1_index].l1_section_1m.id == FMMU_PTE_ID_INVALID)
                              ? 0x0
                              : 0x3;
    perms_attrs.not_global = l1_page_table.entries[l1_index].l1_section_1m.not_global;
    perms_attrs.non_sec = l1_page_table.entries[l1_index].l1_section_1m.non_sec;
    perms_attrs.shared = l1_page_table.entries[l1_index].l1_section_1m.shared;
    perms_attrs.tex = l1_page_table.entries[l1_index].l1_section_1m.tex;
    perms_attrs.exec_never = l1_page_table.entries[l1_index].l1_section_1m.exec_never;

    /*
	 * Disable interrupts - no interrupts shall occur before the L2 table has
	 * been set up in place of the former L1 section entry.
	 */

    lock_key = FArchIrqLock();

    /*
	 * Clear the entire L1 PTE & re-configure it as a L2 PT reference
	 * -> already sets the correct values for: zero0, zero1, impl_def.
	 */
    l1_page_table.entries[l1_index].word = 0;

    l1_page_table.entries[l1_index].l2_page_table_ref.id = FMMU_PTE_ID_L2_PT;
    l1_page_table.entries[l1_index].l2_page_table_ref.domain = perms_attrs.domain;
    l1_page_table.entries[l1_index].l2_page_table_ref.non_sec = perms_attrs.non_sec;
    l1_page_table.entries[l1_index].l2_page_table_ref.l2_page_table_address =
        (((u32)l2_page_table >> FMMU_PT_L2_ADDR_SHIFT) & FMMU_PT_L2_ADDR_MASK);

    /* Align the target VA to the base address of the section we're converting */
    va &= ~(MB(1) - 1);
    while (rem_size > 0)
    {
        FMmmL2MapPage(va, va, perms_attrs);
        rem_size -= KB(4);
        va += KB(4);
    }

    /* Remap complete, re-enable the MMU, unlock the interrupts. */

    FMmuInvalidateTlb();

    FArchIrqUnLock(lock_key);
}


/**
 * @name: FMmmL2MapPage
 * @msg: Maps a single 4 kB page in the level 2 page table, updating or establishing the necessary L1 and L2 table entries.
 * @return: void
 * @note: This function is responsible for updating the MMU's page tables to map a 4 kB physical page to a virtual address.
 *        It handles several scenarios: setting up a new L2 page table if one does not exist, adding a new page entry to an
 *        existing L2 page table, or converting a 1 MB section into a more granular L2 page table setup if the page overlaps
 *        an existing 1 MB section. The function ensures that the appropriate permissions and attributes are set according
 *        to the given parameters. It increments the entry usage count for new mappings and adjusts existing entries as needed.
 * @param {u32} va - The virtual address at which the page will be mapped.
 * @param {u32} pa - The physical address of the page to map.
 * @param {struct FMmuPermsAttrs} perms_attrs - The permissions and attributes to apply to the mapped page.
 */
static void FMmmL2MapPage(u32 va, u32 pa, struct FMmuPermsAttrs perms_attrs)
{
    struct FMmuL2PageTable *l2_page_table = NULL;
    u32 l1_index = (va >> FMMU_PTE_L1_INDEX_PA_SHIFT) & FMMU_PTE_L1_INDEX_MASK;
    u32 l2_index = (va >> FMMU_PTE_L2_INDEX_PA_SHIFT) & FMMU_PTE_L2_INDEX_MASK;

    /*
	 * Use the calculated L1 index in order to determine if a L2 page
	 * table is required in order to complete the current mapping.
	 * -> See below for an explanation of the possible scenarios.
	 */

    if (l1_page_table.entries[l1_index].undefined.id == FMMU_PTE_ID_INVALID ||
        (l1_page_table.entries[l1_index].undefined.id & FMMU_PTE_ID_SECTION) != 0)
    {
        l2_page_table = FMmuAssignL2Table(pa);
        FASSERT_MSG(l2_page_table != NULL, "Unexpected L2 page table NULL pointer for VA 0x%lx", va);
    }

    /*
	 * Check what is currently present at the corresponding L1 table entry.
	 * The following scenarios are possible:
	 * 1) The L1 PTE's ID bits are zero, as is the rest of the entry.
	 *    In this case, the L1 PTE is currently unused. A new L2 PT to
	 *    refer to in this entry has already been allocated above.
	 * 2) The L1 PTE's ID bits indicate a L2 PT reference entry (01).
	 *    The corresponding L2 PT's address will be resolved using this
	 *    entry.
	 * 3) The L1 PTE's ID bits may or may not be zero, and the rest of
	 *    the descriptor contains some non-zero data. This always indicates
	 *    an existing 1 MB section entry in this place. Checking only the
	 *    ID bits wouldn't be enough, as the only way to indicate a section
	 *    with neither R nor W permissions is to set the ID bits to 00 in
	 *    the AP[2:1] permissions model. As we're now about to map a single
	 *    page overlapping with the 1 MB section, the section has to be
	 *    converted into a L2 table. Afterwards, the current page mapping
	 *    can be added/modified.
	 */

    if (l1_page_table.entries[l1_index].word == 0)
    {
        /* The matching L1 PT entry is currently unused */
        l1_page_table.entries[l1_index].l2_page_table_ref.id = FMMU_PTE_ID_L2_PT;
        l1_page_table.entries[l1_index].l2_page_table_ref.zero0 = 0;
        l1_page_table.entries[l1_index].l2_page_table_ref.zero1 = 0;
        l1_page_table.entries[l1_index].l2_page_table_ref.impl_def = 0;
        l1_page_table.entries[l1_index].l2_page_table_ref.domain = 0;
        l1_page_table.entries[l1_index].l2_page_table_ref.non_sec = perms_attrs.non_sec;
        l1_page_table.entries[l1_index].l2_page_table_ref.l2_page_table_address =
            (((u32)l2_page_table >> FMMU_PT_L2_ADDR_SHIFT) & FMMU_PT_L2_ADDR_MASK);
    }
    else if (l1_page_table.entries[l1_index].undefined.id == FMMU_PTE_ID_L2_PT)
    {
        /* The matching L1 PT entry already points to a L2 PT */
        l2_page_table = (struct FMmuL2PageTable *)((l1_page_table.entries[l1_index].word &
                                                    (FMMU_PT_L2_ADDR_MASK << FMMU_PT_L2_ADDR_SHIFT)));
        /*
		 * The only configuration bit contained in the L2 PT entry is the
		 * NS bit. Set it according to the attributes passed to this function,
		 * warn if there is a mismatch between the current page's NS attribute
		 * value and the value currently contained in the L2 PT entry.
		 */
        if (l1_page_table.entries[l1_index].l2_page_table_ref.non_sec != perms_attrs.non_sec)
        {
            MMU_WRNING("NS bit mismatch in L2 PT reference at L1 index [%u], "
                       "re-configuring from %u to %u",
                       l1_index, l1_page_table.entries[l1_index].l2_page_table_ref.non_sec,
                       perms_attrs.non_sec);
            l1_page_table.entries[l1_index].l2_page_table_ref.non_sec = perms_attrs.non_sec;
        }
    }
    else if (l1_page_table.entries[l1_index].undefined.reserved != 0)
    {
        /*
		 * The matching L1 PT entry currently holds a 1 MB section entry
		 * in order to save a L2 table (as it's neither completely blank
		 * nor a L2 PT reference), but now we have to map an overlapping
		 * 4 kB page, so the section entry must be converted to a L2 table
		 * first before the individual L2 entry for the page to be mapped is
		 * accessed. A blank L2 PT has already been assigned above.
		 */
        FMmuRemapL1SectionToL2Table(va, l2_page_table);
    }

    /*
	 * If the matching L2 PTE is blank, increment the number of used entries
	 * in the L2 table. If the L2 PTE already contains some data, we're re-
	 * placing the entry's data instead, the used entry count remains unchanged.
	 * Once again, checking the ID bits might be misleading if the PTE declares
	 * a page which has neither R nor W permissions.
	 */
    if (l2_page_table->entries[l2_index].word == 0)
    {
        FMmuIncL2TableEntries(l2_page_table);
    }

    l2_page_table->entries[l2_index].l2_page_4k.id = (FMMU_PTE_ID_SMALL_PAGE &
                                                      perms_attrs.id_mask);
    l2_page_table->entries[l2_index].l2_page_4k.id |= perms_attrs.exec_never; /* XN in [0] */
    l2_page_table->entries[l2_index].l2_page_4k.bufferable = perms_attrs.bufferable;
    l2_page_table->entries[l2_index].l2_page_4k.cacheable = perms_attrs.cacheable;
    l2_page_table->entries[l2_index].l2_page_4k.acc_perms10 = ((perms_attrs.acc_perms & 0x1) << 1) | 0x1;
    l2_page_table->entries[l2_index].l2_page_4k.tex = perms_attrs.tex;
    l2_page_table->entries[l2_index].l2_page_4k.acc_perms2 = ((perms_attrs.acc_perms >> 1) & 0x1);
    l2_page_table->entries[l2_index].l2_page_4k.shared = perms_attrs.shared;
    l2_page_table->entries[l2_index].l2_page_4k.not_global = perms_attrs.not_global;
    l2_page_table->entries[l2_index].l2_page_4k.pa_base = ((pa >> FMMU_PTE_L2_SMALL_PAGE_ADDR_SHIFT) &
                                                           FMMU_PTE_L2_SMALL_PAGE_ADDR_MASK);
}


/**
 * @name: FMmuL2UnmapPage
 * @msg: Unmaps a virtual address from the level 2 page table, if applicable.
 * @return: void
 * @note: This function handles the unmapping of a single page from a level 2 page table. It checks if a level 2 page table
 *        exists for the given virtual address (VA) and verifies the page type before attempting to clear the entry. This
 *        function is tolerant of attempts to unmap addresses that are not currently mapped, reflecting scenarios such
 *        as unmapping guard pages in memory management routines. Warnings are issued if the unmapping operation attempts
 *        to clear an entry that does not conform to the expected small page format.
 * @param {u32} va - The virtual address of the page to unmap.
 */
static int FMmuL2UnmapPage(u32 va)
{
    struct FMmuL2PageTable *l2_page_table;
    u32 l1_index = (va >> FMMU_PTE_L1_INDEX_PA_SHIFT) & FMMU_PTE_L1_INDEX_MASK;
    u32 l2_index = (va >> FMMU_PTE_L2_INDEX_PA_SHIFT) & FMMU_PTE_L2_INDEX_MASK;

    if (l1_page_table.entries[l1_index].undefined.id != FMMU_PTE_ID_L2_PT)
    {
        /*
		 * No L2 PT currently exists for the given VA - this should be
		 * tolerated without an error, just as in the case that while
		 * a L2 PT exists, the corresponding PTE is blank - see explanation
		 * below, the same applies here.
		 */
        return -1;
    }

    l2_page_table = (struct FMmuL2PageTable *)((l1_page_table.entries[l1_index].word &
                                                (FMMU_PT_L2_ADDR_MASK << FMMU_PT_L2_ADDR_SHIFT)));

    if (l2_page_table->entries[l2_index].word == 0)
    {
        /*
		 * We're supposed to unmap a page at the given VA, but there currently
		 * isn't anything mapped at this address, the L2 PTE is blank.
		 * -> This is normal if a memory area is being mapped via k_mem_map,
		 * which contains two calls to FMmuMemUnmap (which effectively end up
		 * here) in order to unmap the leading and trailing guard pages.
		 * Therefore, it has to be expected that unmap calls are made for unmapped
		 * memory which hasn't been in use before.
		 * -> Just return, don't decrement the entry counter of the corresponding
		 * L2 page table, as we're not actually clearing any PTEs.
		 */
        return -2;
    }

    if ((l2_page_table->entries[l2_index].undefined.id & FMMU_PTE_ID_SMALL_PAGE) != FMMU_PTE_ID_SMALL_PAGE)
    {
        MMU_WRNING("Cannot unmap virtual memory at 0x%lx: invalid "
                   "page table entry type in level 2 page table at "
                   "L1 index [%u], L2 index [%u]",
                   va, l1_index, l2_index);
        return -3;
    }

    l2_page_table->entries[l2_index].word = 0;

    FMmuDecL2TableEntries(l2_page_table);
    return 0;
}

static int FMmuL1UnmapSection(u32 va)
{
    u32 l1_index = (va >> FMMU_PTE_L1_INDEX_PA_SHIFT) & FMMU_PTE_L1_INDEX_MASK;


    if (l1_page_table.entries[l1_index].undefined.id != FMMU_PTE_ID_SECTION)
    {
        return -1;
    }

    if (l1_page_table.entries[l1_index].word == 0)
    {
        return -2;
    }


    l1_page_table.entries[l1_index].word = 0;
    return 0;
}

/**
 * @name: MmuInit
 * @msg: Initializes the Memory Management Unit (MMU) for the system by setting up memory mappings and configuring cache settings.
 * @return: void
 * @note: This function performs several critical initializations for memory management:
 *        - Flushes caches and disables them before setting up new mappings.
 *        - Validates and asserts the configuration of the MMU page size.
 *        - Maps physical memory regions to virtual memory addresses both for code and SoC-specific regions.
 *        - Configures and activates the MMU and caches.
 *        The function locks interrupts during critical mappings and utilizes multiple levels of memory mapping,
 *        including direct 1 MB section mappings for efficiency. It also sets up various system registers (TTBR0, TTBR1,
 *        DACR, SCTLR) based on the memory attributes of the page tables and regions being mapped. Finally, it enables
 *        the MMU and both instruction and data caches with specific configurations.
 */
void MmuInit(void)
{
    u32 mem_range;
    u32 pa;
    u32 attrs;
    u32 pt_attrs = 0;
    int rem_size;
    u32 reg_val = 0;
    struct FMmuPermsAttrs perms_attrs;


#ifdef CONFIG_DISABLE_L3CACHE
    FCacheL3CacheDisable();
#else
    FCacheL3CacheFlush();
#endif
    FCacheDCacheFlush();
    FCacheICacheFlush();
    FCacheDCacheDisable();
    FCacheICacheDisable();

    FASSERT_MSG(KB(4) == CONFIG_MMU_PAGE_SIZE,
                "MMU_PAGE_SIZE value %u is invalid, only 4 kB pages are supported\n",
                CONFIG_MMU_PAGE_SIZE);

    for (mem_range = 0; mem_range < code_mmu_config.num_regions; mem_range++)
    {
        pa = code_mmu_config.mmu_regions[mem_range].base_pa;
        rem_size = code_mmu_config.mmu_regions[mem_range].size;
        attrs = code_mmu_config.mmu_regions[mem_range].attrs;
        perms_attrs = FMmuConvertAttrFlags(attrs);

        /*
		 * Check if the L1 page table is within the region currently
		 * being mapped. If so, store the permissions and attributes
		 * of the current section. This information is required when
		 * writing to the TTBR0 register.
		 */
        if (((u32)&l1_page_table >= pa) && ((u32)&l1_page_table < (pa + rem_size)))
        {
            pt_attrs = attrs;
        }

        while (rem_size > 0)
        {
            if (rem_size >= MB(1) && (pa & 0xFFFFF) == 0)
            {
                /*
				 * Remaining area size > 1 MB & matching alignment
				 * -> map a 1 MB section instead of individual 4 kB
				 * pages with identical configuration.
				 */
                FMmuL1MapSection(pa, pa, perms_attrs);
                rem_size -= MB(1);
                pa += MB(1);
            }
            else
            {
                FMmmL2MapPage(pa, pa, perms_attrs);
                rem_size -= (rem_size >= KB(4)) ? KB(4) : rem_size;
                pa += KB(4);
            }
        }
    }

    /* Set up the memory regions defined at the SoC level */
    for (mem_range = 0; mem_range < mmu_config.num_regions; mem_range++)
    {
        pa = (u32)(mmu_config.mmu_regions[mem_range].base_pa);
        rem_size = (u32)(mmu_config.mmu_regions[mem_range].size);
        attrs = mmu_config.mmu_regions[mem_range].attrs;
        perms_attrs = FMmuConvertAttrFlags(attrs);

        while (rem_size > 0)
        {
            if (rem_size >= MB(1) && (pa & 0xFFFFF) == 0)
            {
                /*
					* Remaining area size > 1 MB & matching alignment
					* -> map a 1 MB section instead of individual 4 kB
					* pages with identical configuration.
					*/
                FMmuL1MapSection(pa, pa, perms_attrs);
                rem_size -= MB(1);
                pa += MB(1);
            }
            else
            {
                FMmmL2MapPage(pa, pa, perms_attrs);
                rem_size -= (rem_size >= KB(4)) ? KB(4) : rem_size;
                pa += KB(4);
            }
        }
    }

    /* Clear TTBR1 */
    AARCH32_WRITE_SYSREG_32(TTBR1, 0);

    /* Write TTBCR: EAE, security not yet relevant, N[2:0] = 0 */
    AARCH32_WRITE_SYSREG_32(TTBCR, 0);

    /* Write TTBR0 */
    reg_val = ((u32)&l1_page_table.entries[0] & ~0x3FFF);

    /*
	 * Set IRGN, RGN, S in TTBR0 based on the configuration of the
	 * memory area the actual page tables are located in.
	 */
    if (pt_attrs & MT_SHAREABLE)
    {
        reg_val |= (u32)FMMU_TTBR_SHAREABLE_BIT;
    }

    if (pt_attrs & MT_CACHE_OUTER_WT_NWA)
    {
        reg_val |= (FMMU_TTBR_RGN_OUTER_WT_CACHEABLE << FMMU_TTBR_RGN_SHIFT);
    }
    else if (pt_attrs & MT_CACHE_OUTER_WB_NWA)
    {
        reg_val |= (FMMU_TTBR_RGN_OUTER_WB_nWA_CACHEABLE << FMMU_TTBR_RGN_SHIFT);
    }
    else
    {
        reg_val |= (FMMU_TTBR_RGN_OUTER_WB_WA_CACHEABLE << FMMU_TTBR_RGN_SHIFT);
    }


    if (pt_attrs & MT_CACHE_INNER_WT_NWA)
    {
        reg_val |= (u32)FMMU_TTBR_IRGN1_BIT_MP_EXT_ONLY;
    }
    else if (pt_attrs & MT_CACHE_INNER_WB_NWA)
    {
        reg_val |= (u32)FMMU_TTBR_IRGN0_BIT_MP_EXT_ONLY;
        reg_val |= (u32)FMMU_TTBR_IRGN1_BIT_MP_EXT_ONLY;
    }
    else
    {
        reg_val |= (u32)FMMU_TTBR_IRGN0_BIT_MP_EXT_ONLY;
    }

    AARCH32_WRITE_SYSREG_32(TTBR0, reg_val);

    /* Write DACR -> all domains to client = 01b. */
    reg_val = FMMU_DACR_ALL_DOMAINS_CLIENT;
    AARCH32_WRITE_SYSREG_32(DACR, reg_val);

    FMmuInvalidateTlb();

    /* Enable the MMU and Cache in SCTLR */
    reg_val = AARCH32_READ_SYSREG_32(SCTLR);
    reg_val |= (u32)FMMU_SCTLR_AFE_BIT;
    reg_val |= (u32)FMMU_SCTLR_ICACHE_ENABLE_BIT;
    reg_val |= (u32)FMMU_SCTLR_DCACHE_ENABLE_BIT;
    reg_val |= (u32)FMMU_SCTLR_MMU_ENABLE_BIT;
    AARCH32_WRITE_SYSREG_32(SCTLR, reg_val);

    FCacheDCacheInvalidate();
}


/**
 * @name: FMmuMemMap
 * @msg: Maps a block of physical memory to a corresponding virtual address space with specified attributes and flags.
 * @return: int - Returns 0 on successful completion, or an error code if an error occurs during the mapping process.
 * @note: This function performs memory mapping by subdividing the memory block into pages and mapping each page individually. 
 *        It locks interrupts during the mapping process to ensure that it is atomic and handles conditions where the size 
 *        is zero by returning an error. After mapping, it increments the virtual and physical addresses by the page size 
 *        until all requested memory has been mapped.
 * @param {uintptr_t} virt - Pointer to the virtual base address where the physical memory will be mapped.
 * @param {uintptr_t} phys - The physical base address of the memory to be mapped.
 * @param {size_t} size - The size of the memory region to be mapped in bytes.
 * @param {u32} flags - Mapping attributes that determine the access permissions and other attributes of the memory region.
 */
static int FMmuMemMap(uintptr_t virt, uintptr_t phys, size_t size, u32 flags)
{
    u32 va = (u32)virt;
    u32 pa = (u32)phys;
    u32 rem_size = (u32)size;
    struct FMmuPermsAttrs perms_attrs;
    int key;

    if (size == 0)
    {
        MMU_WRNING("Cannot map physical memory at 0x%lx: invalid "
                   "zero size",
                   (u32)phys);
        return FMMU_SIZE_NOT_INVALID;
    }

    perms_attrs = FMmuConvertAttrFlags(flags);

    key = FArchIrqLock();

    while (rem_size > 0)
    {

        if (rem_size >= MB(1) && (pa & 0xFFFFF) == 0)
        {
            /*
				* Remaining area size > 1 MB & matching alignment
				* -> map a 1 MB section instead of individual 4 kB
				* pages with identical configuration.
				*/
            FMmuL1MapSection(va, pa, perms_attrs);
            rem_size -= MB(1);
            pa += MB(1);
            va += MB(1);
        }
        else
        {
            FMmmL2MapPage(va, pa, perms_attrs);
            rem_size -= (rem_size >= KB(4)) ? KB(4) : rem_size;
            pa += KB(4);
            va += KB(4);
        }
    }
    FArchIrqUnLock(key);

    return 0;
}


/**
 * @name: FMmuMap
 * @msg: Maps a physical memory address to a virtual memory address with specified flags, and invalidates TLB if successful.
 * @return: {int}
 * @note: This function handles the mapping of physical memory addresses to virtual memory addresses with the use of flags to 
 *        specify the mapping attributes (such as read/write permissions, caching policies, etc.). If the mapping operation fails,
 *        it logs a warning and asserts failure. Successful mapping triggers invalidation of the TLB to ensure the system uses
 *        updated address translations.
 * @param {uintptr_t } virt - Pointer to the virtual base address where the physical memory will be mapped.
 * @param {uintptr_t} phys - The physical base address of the memory to be mapped.
 * @param {fsize_t} size - The size of the memory region to be mapped in bytes.
 * @param {u32} flags - Mapping attributes such as protection and caching behaviors.
 */
int FMmuMap(uintptr_t virt, uintptr_t phys, fsize_t size, u32 flags)
{
    /* address and size must be page-aligned */
    FASSERT((virt & (4096 - 1)) == 0);
    FASSERT((phys & (4096 - 1)) == 0);
    FASSERT((size & (4096 - 1)) == 0);
    FASSERT(size > 0);

    int ret = FMmuMemMap(virt, phys, size, flags);

    if (ret)
    {
        MMU_WRNING("FMmuMemMap() returned %d", ret);
        return ret;
    }
    else
    {
        FMmuInvalidateTlb();
    }
    return 0;
}


/**
 * @name: FMmuMemUnmap
 * @msg: Unmaps a region of memory and invalidates the TLB entries if successful.
 * @return: void
 * @note: This function attempts to unmap a specified memory region. If the unmap operation
 *        is successful, it proceeds to invalidate the Translation Lookaside Buffer (TLB)
 *        to ensure that stale mappings are removed. If the operation fails, an error is logged.
 * @param {uintptr_t } addr - Pointer to the base address of the memory region to be unmapped.
 * @param {fsize_t} size - The size of the memory region to be unmapped in bytes.
 */
static int FMmuMemUnmap(uintptr_t addr, fsize_t size)
{
    u32 va = (u32)addr;
    u32 rem_size = (u32)size;
    int key;
    int ret;
    u32 l1_index = (va >> FMMU_PTE_L1_INDEX_PA_SHIFT) & FMMU_PTE_L1_INDEX_MASK;


    if (addr == (uintptr_t)NULL)
    {
        MMU_WRNING("Cannot unmap virtual memory: invalid NULL pointer");
        return FMMU_VIRT_ADDR_NOT_INVALID;
    }

    if (size == 0)
    {
        MMU_WRNING("Cannot unmap virtual memory at 0x%lx: invalid "
                   "zero size",
                   (u32)addr);
        return FMMU_SIZE_NOT_INVALID;
    }

    key = FArchIrqLock();

    while (rem_size > 0)
    {
        l1_index = (va >> FMMU_PTE_L1_INDEX_PA_SHIFT) & FMMU_PTE_L1_INDEX_MASK;

        if (l1_page_table.entries[l1_index].undefined.id == FMMU_PTE_ID_SECTION)
        {
            if (rem_size >= MB(1))
            {
                /*
				* Remaining area size > 1 MB & matching alignment
				* -> map a 1 MB section instead of individual 4 kB
				* pages with identical configuration.
				*/
                ret = FMmuL1UnmapSection(va);
                if (ret != 0)
                {
                    goto exit;
                }
                rem_size -= MB(1);
                va += MB(1);
            }
            else
            {
                ret = -4;
                goto exit;
            }
        }
        else if (l1_page_table.entries[l1_index].undefined.id == FMMU_PTE_ID_L2_PT)
        {
            ret = FMmuL2UnmapPage(va);
            if (ret != 0)
            {
                goto exit;
            }
            rem_size -= (rem_size >= KB(4)) ? KB(4) : rem_size;
            va += KB(4);
        }
        else
        {
            ret = -5;
            goto exit;
        }
    }

exit:
    FArchIrqUnLock(key);

    return ret;
}


/**
 * @name: FMmuUnMap
 * @msg: Unmaps a region of memory and invalidates the TLB entries if successful.
 * @return: int
 * @note: This function attempts to unmap a specified memory region. If the unmap operation
 *        is successful, it proceeds to invalidate the Translation Lookaside Buffer (TLB)
 *        to ensure that stale mappings are removed. If the operation fails, an error is logged.
 * @param {uintptr_t} addr - Pointer to the base address of the memory region to be unmapped.
 * @param {size_t} size - The size of the memory region to be unmapped in bytes.
 */
int FMmuUnMap(uintptr_t addr, fsize_t size)
{
    int ret = FMmuMemUnmap(addr, size);

    if (ret)
    {
        MMU_WRNING("FMmuMemUnmap() returned %d", ret);
        return ret;
    }
    else
    {
        FMmuInvalidateTlb();
    }
    return 0;
}


/**
 * @name: FMmuPagePhysGet
 * @msg: Translates a virtual address to a physical address using a two-level page table system.
 * @return: Returns 0 if the physical address is successfully resolved; -1 if the virtual address is not mapped.
 * @note: This function is part of an operating system's memory management unit. It locks interrupts during operation to ensure atomicity.
 * @param {void*} virt - The virtual address for which the physical address needs to be resolved.
 * @param {uintptr_t*} phys - Pointer to a variable where the physical address will be stored if resolved successfully.
 */
int FMmuPagePhysGet(void *virt, uintptr_t *phys)
{
    u32 l1_index = ((u32)virt >> FMMU_PTE_L1_INDEX_PA_SHIFT) & FMMU_PTE_L1_INDEX_MASK;
    u32 l2_index = ((u32)virt >> FMMU_PTE_L2_INDEX_PA_SHIFT) & FMMU_PTE_L2_INDEX_MASK;
    struct FMmuL2PageTable *l2_page_table;

    u32 pa_resolved = 0;
    u32 l2_pt_resolved;

    int rc = 0;
    int key;

    key = FArchIrqLock();

    if (l1_page_table.entries[l1_index].undefined.id == FMMU_PTE_ID_SECTION)
    {

        pa_resolved = (u32)virt;
    }
    else if (l1_page_table.entries[l1_index].undefined.id == FMMU_PTE_ID_L2_PT)
    {
        /*
		 * The VA points to a level 1 PTE which re-directs to a level 2
		 * PT. -> Assemble the level 2 PT pointer and resolve the PA for
		 * the specified VA from there.
		 */
        l2_pt_resolved = l1_page_table.entries[l1_index].l2_page_table_ref.l2_page_table_address;
        l2_pt_resolved <<= FMMU_PT_L2_ADDR_SHIFT;
        l2_page_table = (struct FMmuL2PageTable *)l2_pt_resolved;

        /*
		 * Check if the PTE for the specified VA is actually in use before
		 * assembling & returning the corresponding PA. k_mem_unmap will
		 * call this function for the leading & trailing guard pages when
		 * unmapping a VA. As those guard pages were explicitly unmapped
		 * when the VA was originally mapped, their L2 PTEs will be empty.
		 * In that case, the return code of this function must not be 0.
		 */
        if (l2_page_table->entries[l2_index].word == 0)
        {
            rc = FMMU_L2_PAGE_NOT_INVALID;
        }

        pa_resolved = l2_page_table->entries[l2_index].l2_page_4k.pa_base;
        pa_resolved <<= FMMU_PTE_L2_SMALL_PAGE_ADDR_SHIFT;
        pa_resolved |= ((u32)virt & FMMU_ADDR_BELOW_PAGE_GRAN_MASK);
    }
    else
    {
        /* The level 1 PTE is invalid -> the specified VA is not mapped */
        rc = FMMU_L1_PAGE_NOT_INVALID;
    }

    FArchIrqUnLock(key);

    if (phys)
    {
        *phys = (uintptr_t)pa_resolved;
    }
    return rc;
}


static void FMmuPrintPermsAttrs(struct FMmuPermsAttrsShow *perms_attrs, u32 size,
                                const char *table_type);


void FMmuPrintMergedRange(u32 start_va, u32 end_va, u32 start_pa, u32 end_pa,
                          struct FMmuPermsAttrsShow perms_attrs, u32 size, const char *table_type)
{
    const char *mem_type;
    if (perms_attrs.tex == 0 && perms_attrs.cacheable == 0 && perms_attrs.bufferable == 0)
    {
        mem_type = "strongly ordered";
    }
    else if ((perms_attrs.tex == 0 && perms_attrs.cacheable == 0 && perms_attrs.bufferable == 1) ||
             (perms_attrs.tex == 2 && perms_attrs.cacheable == 0 && perms_attrs.bufferable == 0))
    {
        mem_type = "device";
    }
    else
    {
        mem_type = "normal";
    }

    const char *cache_policy = "";
    if (strcmp(mem_type, "normal") == 0)
    {
        if ((perms_attrs.tex & FMMU_TEX2_CACHEABLE_MEMORY) &&
            (perms_attrs.tex & 0x3) == FMMU_TEX_CACHE_ATTRS_WB_WA &&
            perms_attrs.cacheable == FMMU_C_CACHE_ATTRS_WB_WA &&
            perms_attrs.bufferable == FMMU_B_CACHE_ATTRS_WB_WA)
        {
            cache_policy = "I:wr-back/alloc O:wr-back/alloc";
        }
        else if ((perms_attrs.tex & FMMU_TEX2_CACHEABLE_MEMORY) &&
                 (perms_attrs.tex & 0x3) == FMMU_TEX_CACHE_ATTRS_WT_nWA &&
                 perms_attrs.cacheable == FMMU_C_CACHE_ATTRS_WT_nWA &&
                 perms_attrs.bufferable == FMMU_B_CACHE_ATTRS_WT_nWA)
        {
            cache_policy = "I:wr-thru/nalloc O:wr-thru/nalloc";
        }
        else if ((perms_attrs.tex & FMMU_TEX2_CACHEABLE_MEMORY) &&
                 (perms_attrs.tex & 0x3) == FMMU_TEX_CACHE_ATTRS_WB_nWA &&
                 perms_attrs.cacheable == FMMU_C_CACHE_ATTRS_WB_nWA &&
                 perms_attrs.bufferable == FMMU_B_CACHE_ATTRS_WB_nWA)
        {
            cache_policy = "I:wr-back/nalloc O:wr-back/nalloc";
        }
    }

    const char *p_rw, *u_rw;

    if (perms_attrs.acc_apx == 0)
    {
        if (perms_attrs.acc_ap == 0b00)
        {
            p_rw = "noaccess";
            u_rw = "noaccess";
        }
        else if (perms_attrs.acc_ap == 0b01)
        {
            p_rw = "readwrite";
            u_rw = "noaccess";
        }
        else if (perms_attrs.acc_ap == 0b10)
        {
            p_rw = "readwrite";
            u_rw = "readonly";
        }
        else if (perms_attrs.acc_ap == 0b11)
        {
            p_rw = "readwrite";
            u_rw = "readwrite";
        }
    }
    else
    {
        if (perms_attrs.acc_ap == 0b01)
        {
            p_rw = "readonly";
            u_rw = "noaccess";
        }
        else if (perms_attrs.acc_ap == 0b10)
        {
            p_rw = "readonly";
            u_rw = "readonly";
        }
        else
        {
            p_rw = "reserved";
            u_rw = "reserved";
        }
    }

    printf("N:%lx--%lx\tAN:%lx--%lx\tsize:%lx\tP:%s\tU:%s\t%s\t%s\t%s\t%s\t%s\t%s\t%"
           "s\n",
           start_va, end_va, start_pa, end_pa, size, p_rw, u_rw,
           perms_attrs.exec_never ? "notexec" : "exec",
           perms_attrs.shared ? "yes" : "no", perms_attrs.not_global ? "no" : "yes",
           perms_attrs.non_sec ? "non-secure" : "secure", mem_type, cache_policy, table_type);
}

static void FMmuParseL2Entry(struct FMmuL2PageTable *l2_table, u32 va_base, u32 no_sec, int print_detailed)
{
    for (int i = 0; i < FMMU_PT_L2_NUM_ENTRIES; ++i)
    {
        union FMmuL2PageTableEntry *l2_entry = &l2_table->entries[i];
        u32 l2_va = va_base + (i << FMMU_PTE_L2_INDEX_PA_SHIFT);
        if (l2_entry->undefined.id & FMMU_PTE_ID_SMALL_PAGE)
        {
            u32 l2_pa_base = l2_entry->l2_page_4k.pa_base << FMMU_PTE_L2_SMALL_PAGE_ADDR_SHIFT;
            struct FMmuPermsAttrsShow perms_attrs = {
                .acc_ap = l2_entry->l2_page_4k.acc_perms10,
                .acc_apx = l2_entry->l2_page_4k.acc_perms2,
                .bufferable = l2_entry->l2_page_4k.bufferable,
                .cacheable = l2_entry->l2_page_4k.cacheable,
                .id_mask = 0x3,
                .not_global = l2_entry->l2_page_4k.not_global,
                .shared = l2_entry->l2_page_4k.shared,
                .tex = l2_entry->l2_page_4k.tex,
                .non_sec = no_sec,
            };
            if (print_detailed)
            {
                printf("N:%lx--%lx\tAN:%lx--%lx\tL2 page ", l2_va, l2_va + 0xFFF,
                       l2_pa_base, l2_pa_base + 0xFFF);
                FMmuPrintPermsAttrs(&perms_attrs, 0x1000, l2_page_string);
            }
            else
            {

                if (range_active &&
                    (current_range.perms_attrs.acc_ap != perms_attrs.acc_ap ||
                     current_range.perms_attrs.acc_apx != perms_attrs.acc_apx ||
                     current_range.perms_attrs.bufferable != perms_attrs.bufferable ||
                     current_range.perms_attrs.cacheable != perms_attrs.cacheable ||
                     current_range.perms_attrs.not_global != perms_attrs.not_global ||
                     current_range.perms_attrs.non_sec != perms_attrs.non_sec ||
                     current_range.perms_attrs.shared != perms_attrs.shared ||
                     current_range.perms_attrs.tex != perms_attrs.tex ||
                     current_range.perms_attrs.exec_never != perms_attrs.exec_never ||
                     current_range.perms_attrs.id_mask != perms_attrs.id_mask ||
                     current_range.perms_attrs.domain != perms_attrs.domain ||
                     current_range.table_type != l2_page_string))
                {
                    FMmuPrintMergedRange(current_range.start_va, current_range.end_va,
                                         current_range.start_pa, current_range.end_pa,
                                         current_range.perms_attrs, current_range.size,
                                         current_range.table_type);
                    range_active = 0;
                }
                if (!range_active)
                {
                    current_range.start_va = l2_va;
                    current_range.end_va = l2_va + 0xFFF;
                    current_range.start_pa = l2_pa_base;
                    current_range.end_pa = l2_pa_base + 0xFFF;
                    current_range.perms_attrs = perms_attrs;
                    current_range.size = 0x1000;
                    current_range.table_type = l2_page_string;
                    range_active = 1;
                }
                else
                {
                    current_range.end_va = l2_va + 0xFFF;
                    current_range.end_pa = l2_pa_base + 0xFFF;
                    current_range.size += 0x1000;
                }
            }
        }
    }

    if (!print_detailed && range_active)
    {
        FMmuPrintMergedRange(current_range.start_va, current_range.end_va, current_range.start_pa,
                             current_range.end_pa, current_range.perms_attrs,
                             current_range.size, current_range.table_type);
        range_active = 0;
    }
}

void FMmuParseL1Entry(union FMmuL1PageTableEntry *l1_entry, u32 l1_va_base, int print_detailed)
{
    if (l1_entry->undefined.id == FMMU_PTE_ID_SECTION)
    {
        u32 l1_pa_base = l1_entry->l1_section_1m.base_address << FMMU_PTE_L1_INDEX_PA_SHIFT;
        struct FMmuPermsAttrsShow perms_attrs = {
            .id_mask = (l1_entry->l1_section_1m.id == FMMU_PTE_ID_SECTION) ? 0x3 : 0x0,
            .bufferable = l1_entry->l1_section_1m.bufferable,
            .cacheable = l1_entry->l1_section_1m.cacheable,
            .exec_never = l1_entry->l1_section_1m.exec_never,
            .domain = l1_entry->l1_section_1m.domain,
            .tex = l1_entry->l1_section_1m.tex,
            .shared = l1_entry->l1_section_1m.shared,
            .not_global = l1_entry->l1_section_1m.not_global,
            .non_sec = l1_entry->l1_section_1m.non_sec,
            .acc_ap = l1_entry->l1_section_1m.acc_perms10,
            .acc_apx = l1_entry->l1_section_1m.acc_perms2,
        };
        if (print_detailed)
        {
            printf("N:%lx--%lx\tAN:%lx--%lx\tL1 section ", l1_va_base,
                   l1_va_base + 0xFFFFF, l1_pa_base, l1_pa_base + 0xFFFFF);
            FMmuPrintPermsAttrs(&perms_attrs, 0x100000, l1_section_string);
        }
        else
        {

            if (range_active &&
                (current_range.perms_attrs.acc_ap != perms_attrs.acc_ap ||
                 current_range.perms_attrs.acc_apx != perms_attrs.acc_apx ||
                 current_range.perms_attrs.bufferable != perms_attrs.bufferable ||
                 current_range.perms_attrs.cacheable != perms_attrs.cacheable ||
                 current_range.perms_attrs.not_global != perms_attrs.not_global ||
                 current_range.perms_attrs.non_sec != perms_attrs.non_sec ||
                 current_range.perms_attrs.shared != perms_attrs.shared ||
                 current_range.perms_attrs.tex != perms_attrs.tex ||
                 current_range.perms_attrs.exec_never != perms_attrs.exec_never ||
                 current_range.perms_attrs.id_mask != perms_attrs.id_mask ||
                 current_range.perms_attrs.domain != perms_attrs.domain ||
                 current_range.table_type != l1_section_string))
            {
                FMmuPrintMergedRange(current_range.start_va, current_range.end_va,
                                     current_range.start_pa, current_range.end_pa,
                                     current_range.perms_attrs, current_range.size,
                                     current_range.table_type);
                range_active = 0;
            }
            if (!range_active)
            {
                current_range.start_va = l1_va_base;
                current_range.end_va = l1_va_base + 0xFFFFF;
                current_range.start_pa = l1_pa_base;
                current_range.end_pa = l1_pa_base + 0xFFFFF;
                current_range.perms_attrs = perms_attrs;
                current_range.size = 0x100000;
                current_range.table_type = l1_section_string;
                range_active = 1;
            }
            else
            {
                current_range.end_va = l1_va_base + 0xFFFFF;
                current_range.end_pa = l1_pa_base + 0xFFFFF;
                current_range.size += 0x100000;
            }
        }
    }
    else if (l1_entry->undefined.id == FMMU_PTE_ID_L2_PT)
    {
        if (print_detailed)
        {
            printf("N:%lx--%lx\tAN: L2 Table \n", l1_va_base, l1_va_base + 0xFFFFF);
            struct FMmuL2PageTable *l2_table =
                (struct FMmuL2PageTable *)(l1_entry->l2_page_table_ref.l2_page_table_address
                                           << FMMU_PT_L2_ADDR_SHIFT);
            FMmuParseL2Entry(l2_table, l1_va_base, l1_entry->l2_page_table_ref.non_sec, print_detailed);
        }
        else
        {

            if (range_active)
            {
                FMmuPrintMergedRange(current_range.start_va, current_range.end_va,
                                     current_range.start_pa, current_range.end_pa,
                                     current_range.perms_attrs, current_range.size,
                                     current_range.table_type);
                range_active = 0;
            }
            struct FMmuL2PageTable *l2_table =
                (struct FMmuL2PageTable *)(l1_entry->l2_page_table_ref.l2_page_table_address
                                           << FMMU_PT_L2_ADDR_SHIFT);
            FMmuParseL2Entry(l2_table, l1_va_base, l1_entry->l2_page_table_ref.non_sec, print_detailed);
        }
    }
}


void FMmuPrintPermsAttrs(struct FMmuPermsAttrsShow *perms_attrs, u32 size, const char *table_type)
{
    const char *mem_type;
    if (perms_attrs->tex == 0 && perms_attrs->cacheable == 0 && perms_attrs->bufferable == 0)
    {
        mem_type = "strongly ordered";
    }
    else if ((perms_attrs->tex == 0 && perms_attrs->cacheable == 0 && perms_attrs->bufferable == 1) ||
             (perms_attrs->tex == 2 && perms_attrs->cacheable == 0 && perms_attrs->bufferable == 0))
    {
        mem_type = "device";
    }
    else
    {
        mem_type = "normal";
    }

    const char *cache_policy = "";
    if (strcmp(mem_type, "normal") == 0)
    {
        if ((perms_attrs->tex & FMMU_TEX2_CACHEABLE_MEMORY) &&
            (perms_attrs->tex & 0x3) == FMMU_TEX_CACHE_ATTRS_WB_WA &&
            perms_attrs->cacheable == FMMU_C_CACHE_ATTRS_WB_WA &&
            perms_attrs->bufferable == FMMU_B_CACHE_ATTRS_WB_WA)
        {
            cache_policy = "I:wr-back/alloc O:wr-back/alloc";
        }
        else if ((perms_attrs->tex & FMMU_TEX2_CACHEABLE_MEMORY) &&
                 (perms_attrs->tex & 0x3) == FMMU_TEX_CACHE_ATTRS_WT_nWA &&
                 perms_attrs->cacheable == FMMU_C_CACHE_ATTRS_WT_nWA &&
                 perms_attrs->bufferable == FMMU_B_CACHE_ATTRS_WT_nWA)
        {
            cache_policy = "I:wr-thru/nalloc O:wr-thru/nalloc";
        }
        else if ((perms_attrs->tex & FMMU_TEX2_CACHEABLE_MEMORY) &&
                 (perms_attrs->tex & 0x3) == FMMU_TEX_CACHE_ATTRS_WB_nWA &&
                 perms_attrs->cacheable == FMMU_C_CACHE_ATTRS_WB_nWA &&
                 perms_attrs->bufferable == FMMU_B_CACHE_ATTRS_WB_nWA)
        {
            cache_policy = "I:wr-back/nalloc O:wr-back/nalloc";
        }
    }

    const char *p_rw, *u_rw;

    if (perms_attrs->acc_apx == 0)
    {
        if (perms_attrs->acc_ap == 0b00)
        {
            p_rw = "noaccess";
            u_rw = "noaccess";
        }
        else if (perms_attrs->acc_ap == 0b01)
        {
            p_rw = "readwrite";
            u_rw = "noaccess";
        }
        else if (perms_attrs->acc_ap == 0b10)
        {
            p_rw = "readwrite";
            u_rw = "readonly";
        }
        else if (perms_attrs->acc_ap == 0b11)
        {
            p_rw = "readwrite";
            u_rw = "readwrite";
        }
    }
    else
    {
        if (perms_attrs->acc_ap == 0b01)
        {
            p_rw = "readonly";
            u_rw = "noaccess";
        }
        else if (perms_attrs->acc_ap == 0b10)
        {
            p_rw = "readonly";
            u_rw = "readonly";
        }
        else
        {
            p_rw = "reserved";
            u_rw = "reserved";
        }
    }

    printf("P:%s\tU:%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n", p_rw, u_rw,
           perms_attrs->exec_never ? "notexec" : "exec",
           perms_attrs->shared ? "yes" : "no", perms_attrs->not_global ? "no" : "yes",
           perms_attrs->non_sec ? "non-secure" : "secure", mem_type, cache_policy, table_type);
}


void FMmuPackMmuRegion(struct FMmuTableRange *range, struct ArmMmuRegion *region_out)
{
    const char *mem_type;
    if (range->perms_attrs.tex == 0 && range->perms_attrs.cacheable == 0 &&
        range->perms_attrs.bufferable == 0)
    {
        mem_type = "strongly ordered";
    }
    else if ((range->perms_attrs.tex == 0 && range->perms_attrs.cacheable == 0 &&
              range->perms_attrs.bufferable == 1) ||
             (range->perms_attrs.tex == 2 && range->perms_attrs.cacheable == 0 &&
              range->perms_attrs.bufferable == 0))
    {
        mem_type = "device";
    }
    else
    {
        mem_type = "normal";
    }


    region_out->attrs = 0;
    if (strcmp(mem_type, "normal") == 0)
    {
        if ((range->perms_attrs.tex & FMMU_TEX2_CACHEABLE_MEMORY) &&
            (range->perms_attrs.tex & 0x3) == FMMU_TEX_CACHE_ATTRS_WB_WA &&
            range->perms_attrs.cacheable == FMMU_C_CACHE_ATTRS_WB_WA &&
            range->perms_attrs.bufferable == FMMU_B_CACHE_ATTRS_WB_WA)
        {
            region_out->attrs |= MT_NORMAL;
        }
        else if ((range->perms_attrs.tex & FMMU_TEX2_CACHEABLE_MEMORY) &&
                 (range->perms_attrs.tex & 0x3) == FMMU_TEX_CACHE_ATTRS_WT_nWA &&
                 range->perms_attrs.cacheable == FMMU_C_CACHE_ATTRS_WT_nWA &&
                 range->perms_attrs.bufferable == FMMU_B_CACHE_ATTRS_WT_nWA)
        {
            region_out->attrs |= (MT_NORMAL_INDEX | MT_CACHE_OUTER_WT_NWA | MT_CACHE_INNER_WT_NWA);
        }
        else if ((range->perms_attrs.tex & FMMU_TEX2_CACHEABLE_MEMORY) &&
                 (range->perms_attrs.tex & 0x3) == FMMU_TEX_CACHE_ATTRS_WB_nWA &&
                 range->perms_attrs.cacheable == FMMU_C_CACHE_ATTRS_WB_nWA &&
                 range->perms_attrs.bufferable == FMMU_B_CACHE_ATTRS_WB_nWA)
        {
            region_out->attrs |= (MT_NORMAL_INDEX | MT_CACHE_OUTER_WB_NWA | MT_CACHE_INNER_WB_NWA);
        }
    }


    if (range->perms_attrs.acc_apx == 0)
    {
        if (range->perms_attrs.acc_ap == 0b00)
        {
            region_out->attrs |= 0;
        }
        else if (range->perms_attrs.acc_ap == 0b01)
        {
            region_out->attrs |= MT_WRITE | MT_READ;
        }
        else if (range->perms_attrs.acc_ap == 0b10)
        {
            region_out->attrs |= MT_WRITE | MT_READ | MT_USERUNPRIVILEGED;
        }
        else if (range->perms_attrs.acc_ap == 0b11)
        {
            region_out->attrs |= MT_WRITE | MT_READ | MT_USERUNPRIVILEGED;
        }
    }
    else
    {
        if (range->perms_attrs.acc_ap == 0b01)
        {
            region_out->attrs |= MT_READ;
        }
        else if (range->perms_attrs.acc_ap == 0b10)
        {
            region_out->attrs |= MT_READ | MT_USERUNPRIVILEGED;
        }
        else
        {
            region_out->attrs |= 0;
        }
    }

    region_out->base_pa = range->start_pa;
    region_out->base_va = range->start_va;
    region_out->size = range->size;
    region_out->name = range->table_type;
    if (range->perms_attrs.non_sec)
    {
        region_out->attrs |= MT_NON_SECURE;
    }

    if (range->perms_attrs.exec_never)
    {
        region_out->attrs |= MT_NERVER_EXECUTE;
    }
}

int FMmuPrintReconstructedTables(int print_detailed)
{
    range_active = 0;
    for (int i = 0; i < FMMU_PT_L1_NUM_ENTRIES; ++i)
    {
        FMmuParseL1Entry(&l1_page_table.entries[i], i << FMMU_PTE_L1_INDEX_PA_SHIFT, print_detailed);
    }

    if (range_active)
    {
        FMmuPrintMergedRange(current_range.start_va, current_range.end_va, current_range.start_pa,
                             current_range.end_pa, current_range.perms_attrs,
                             current_range.size, current_range.table_type);
    }
    return 0;
}


void DisplayMmuUsage(void)
{
    u32 l1_section_count = 0;
    u32 l1_to_l2_count = 0;
    u32 l1_free_count = 0;
    u32 total_l2_entries = 0;
    u32 total_l2_table_count = CONFIG_FMMU_NUM_L2_TABLES;
    u32 used_l2_tables = 0;

    // 计算 L1 表的使用情况
    for (int i = 0; i < FMMU_PT_L1_NUM_ENTRIES; ++i)
    {
        if (l1_page_table.entries[i].word == 0)
        {
            l1_free_count++;
        }
        else if (l1_page_table.entries[i].undefined.id == FMMU_PTE_ID_SECTION)
        {
            l1_section_count++;
        }
        else if (l1_page_table.entries[i].undefined.id == FMMU_PTE_ID_L2_PT)
        {
            l1_to_l2_count++;
        }
    }

    // 计算 L2 表的使用情况
    for (int i = 0; i < total_l2_table_count; ++i)
    {
        if (l2_page_tables_status[i].entries > 0)
        {
            used_l2_tables++;
            total_l2_entries += l2_page_tables_status[i].entries;
        }
    }

    // 计算剩余可用内存空间
    u32 l1_free_memory_mb = l1_free_count;       // 每个 L1 条目对应 1 MB 内存
    u32 l1_section_memory_mb = l1_section_count; // 每个 section 条目对应 1 MB 内存
    u32 l2_free_entries = (total_l2_table_count * FMMU_PT_L2_NUM_ENTRIES) - total_l2_entries;
    u32 l2_free_memory_kb = l2_free_entries * 4; // 每个 L2 条目对应 4 KB 内存

    // 剩余 L2 页表数量
    u32 l2_free_tables = fmmu_l2_tables_free;

    // 输出结果
    printf("MMU Usage Information:\n");
    printf("L1 Table Usage:\n");
    printf("  Sections: %lu / %d entries used (Total: %lu MB)\n", l1_section_count,
           FMMU_PT_L1_NUM_ENTRIES, l1_section_memory_mb);
    printf("  L2 Table References: %lu entries\n", l1_to_l2_count);
    printf("  Free Entries: %lu entries (Total: %lu MB)\n", l1_free_count, l1_free_memory_mb);

    printf("L2 Tables Usage: %lu / %lu tables used\n", used_l2_tables, total_l2_table_count);
    printf("Total L2 Entries Usage: %lu entries\n", total_l2_entries);
    printf("Free L2 Tables: %lu tables\n", l2_free_tables);
    printf("Free L2 Entries: %lu entries (Total: %lu KB)\n", l2_free_entries, l2_free_memory_kb);

    u32 total_free_memory_kb = (l1_free_memory_mb * 1024) + l2_free_memory_kb;
    printf("Total Free Memory: %lu KB\n", total_free_memory_kb);
}


fsize_t FMmuGetMappingsByPA(uintptr pa, struct ArmMmuRegion *mmu_region, int max_ranges)
{
    struct FMmuTableRange range;
    fsize_t range_count = 0;
    for (int i = 0; i < FMMU_PT_L1_NUM_ENTRIES; ++i)
    {
        union FMmuL1PageTableEntry *l1_entry = &l1_page_table.entries[i];
        u32 l1_va_base = i << FMMU_PTE_L1_INDEX_PA_SHIFT;
        if (l1_entry->undefined.id == FMMU_PTE_ID_SECTION)
        {
            u32 l1_pa_base = l1_entry->l1_section_1m.base_address << FMMU_PTE_L1_INDEX_PA_SHIFT;
            if (pa >= l1_pa_base && pa <= l1_pa_base + 0xFFFFF)
            {
                if (range_count < max_ranges)
                {

                    range = (struct FMmuTableRange){
                        .start_va = l1_va_base,
                        .end_va = l1_va_base + 0xFFFFF,
                        .start_pa = l1_pa_base,
                        .end_pa = l1_pa_base + 0xFFFFF,
                        .perms_attrs =
                            {
                                .id_mask = (l1_entry->l1_section_1m.id == FMMU_PTE_ID_SECTION) ? 0x3 : 0x0,
                                .bufferable = l1_entry->l1_section_1m.bufferable,
                                .cacheable = l1_entry->l1_section_1m.cacheable,
                                .exec_never = l1_entry->l1_section_1m.exec_never,
                                .domain = l1_entry->l1_section_1m.domain,
                                .tex = l1_entry->l1_section_1m.tex,
                                .shared = l1_entry->l1_section_1m.shared,
                                .not_global = l1_entry->l1_section_1m.not_global,
                                .non_sec = l1_entry->l1_section_1m.non_sec,
                                .acc_ap = l1_entry->l1_section_1m.acc_perms10,
                                .acc_apx = l1_entry->l1_section_1m.acc_perms2,
                            },
                        .size = 0x100000,
                        .table_type = l1_section_string};
                    FMmuPackMmuRegion(&range, &mmu_region[range_count]);
                    range_count++;
                }
            }
        }
        else if (l1_entry->undefined.id == FMMU_PTE_ID_L2_PT)
        {
            u32 l2_table_base = l1_entry->l2_page_table_ref.l2_page_table_address << FMMU_PT_L2_ADDR_SHIFT;
            struct FMmuL2PageTable *l2_table = (struct FMmuL2PageTable *)l2_table_base;
            for (int j = 0; j < FMMU_PT_L2_NUM_ENTRIES; ++j)
            {
                union FMmuL2PageTableEntry *l2_entry = &l2_table->entries[j];
                u32 l2_va = l1_va_base + (j << FMMU_PTE_L2_INDEX_PA_SHIFT);
                if ((l2_entry->undefined.id & FMMU_PTE_ID_SMALL_PAGE) == FMMU_PTE_ID_SMALL_PAGE)
                {
                    u32 l2_pa_base = l2_entry->l2_page_4k.pa_base << FMMU_PTE_L2_SMALL_PAGE_ADDR_SHIFT;
                    if (pa >= l2_pa_base && pa <= l2_pa_base + 0xFFF)
                    {
                        if (range_count < max_ranges)
                        {
                            range = (struct FMmuTableRange){
                                .start_va = l2_va,
                                .end_va = l2_va + 0xFFF,
                                .start_pa = l2_pa_base,
                                .end_pa = l2_pa_base + 0xFFF,
                                .perms_attrs = {.acc_ap = l2_entry->l2_page_4k.acc_perms10,
                                                .acc_apx = l2_entry->l2_page_4k.acc_perms2,
                                                .bufferable = l2_entry->l2_page_4k.bufferable,
                                                .cacheable = l2_entry->l2_page_4k.cacheable,
                                                .id_mask = 0x3,
                                                .not_global = l2_entry->l2_page_4k.not_global,
                                                .shared = l2_entry->l2_page_4k.shared,
                                                .tex = l2_entry->l2_page_4k.tex,
                                                .non_sec = l1_entry->l2_page_table_ref.non_sec,
                                                .exec_never = l2_entry->undefined.id & 0x1},
                                .size = 0x1000,
                                .table_type = "L2 page"};

                            FMmuPackMmuRegion(&range, &mmu_region[range_count]);
                            range_count++;
                        }
                    }
                }
            }
        }
    }
    return range_count;
}

fsize_t FMmuGetMappingByVA(uintptr va, struct ArmMmuRegion *mmu_region)
{
    struct FMmuTableRange range;
    u32 l1_index = (u32)(va >> FMMU_PTE_L1_INDEX_PA_SHIFT);
    u32 l2_index = (u32)(va >> FMMU_PTE_L2_INDEX_PA_SHIFT) & (FMMU_PT_L2_NUM_ENTRIES - 1);
    union FMmuL1PageTableEntry *l1_entry = &l1_page_table.entries[l1_index];
    u32 l1_va_base = l1_index << FMMU_PTE_L1_INDEX_PA_SHIFT;
    if (l1_entry->undefined.id == FMMU_PTE_ID_SECTION)
    {
        u32 l1_pa_base = l1_entry->l1_section_1m.base_address << FMMU_PTE_L1_INDEX_PA_SHIFT;
        range = (struct FMmuTableRange){
            .start_va = l1_va_base,
            .end_va = l1_va_base + 0xFFFFF,
            .start_pa = l1_pa_base,
            .end_pa = l1_pa_base + 0xFFFFF,
            .perms_attrs =
                {
                    .id_mask = (l1_entry->l1_section_1m.id == FMMU_PTE_ID_SECTION) ? 0x3 : 0x0,
                    .bufferable = l1_entry->l1_section_1m.bufferable,
                    .cacheable = l1_entry->l1_section_1m.cacheable,
                    .exec_never = l1_entry->l1_section_1m.exec_never,
                    .domain = l1_entry->l1_section_1m.domain,
                    .tex = l1_entry->l1_section_1m.tex,
                    .shared = l1_entry->l1_section_1m.shared,
                    .not_global = l1_entry->l1_section_1m.not_global,
                    .non_sec = l1_entry->l1_section_1m.non_sec,
                    .acc_ap = l1_entry->l1_section_1m.acc_perms10,
                    .acc_apx = l1_entry->l1_section_1m.acc_perms2,
                },
            .size = 0x100000,
            .table_type = l1_section_string};

        FMmuPackMmuRegion(&range, mmu_region);
        return 1;
    }
    else if (l1_entry->undefined.id == FMMU_PTE_ID_L2_PT)
    {
        u32 l2_table_base = l1_entry->l2_page_table_ref.l2_page_table_address << FMMU_PT_L2_ADDR_SHIFT;
        struct FMmuL2PageTable *l2_table = (struct FMmuL2PageTable *)l2_table_base;
        union FMmuL2PageTableEntry *l2_entry = &l2_table->entries[l2_index];
        u32 l2_va_base = l1_va_base + (l2_index << FMMU_PTE_L2_INDEX_PA_SHIFT);
        if ((l2_entry->undefined.id & FMMU_PTE_ID_SMALL_PAGE) == FMMU_PTE_ID_SMALL_PAGE)
        {
            u32 l2_pa_base = l2_entry->l2_page_4k.pa_base << FMMU_PTE_L2_SMALL_PAGE_ADDR_SHIFT;
            range = (struct FMmuTableRange){
                .start_va = l2_va_base,
                .end_va = l2_va_base + 0xFFF,
                .start_pa = l2_pa_base,
                .end_pa = l2_pa_base + 0xFFF,
                .perms_attrs = {.acc_ap = l2_entry->l2_page_4k.acc_perms10,
                                .acc_apx = l2_entry->l2_page_4k.acc_perms2,
                                .bufferable = l2_entry->l2_page_4k.bufferable,
                                .cacheable = l2_entry->l2_page_4k.cacheable,
                                .id_mask = 0x3,
                                .not_global = l2_entry->l2_page_4k.not_global,
                                .shared = l2_entry->l2_page_4k.shared,
                                .tex = l2_entry->l2_page_4k.tex,
                                .non_sec = l1_entry->l2_page_table_ref.non_sec,
                                .exec_never = l2_entry->undefined.id & 0x1},
                .size = 0x1000,
                .table_type = "L2 page"};
            FMmuPackMmuRegion(&range, mmu_region);
            return 1;
        }
    }
    return 0;
}


fsize_t FMmuGetAllVaByPA(uintptr pa, uintptr_t *va_out, size_t max_va)
{
    uintptr_t offset = 0;
    fsize_t range_count = 0;
    for (int i = 0; i < FMMU_PT_L1_NUM_ENTRIES; ++i)
    {
        union FMmuL1PageTableEntry *l1_entry = &l1_page_table.entries[i];
        u32 l1_va_base = i << FMMU_PTE_L1_INDEX_PA_SHIFT;
        if (l1_entry->undefined.id == FMMU_PTE_ID_SECTION)
        {
            u32 l1_pa_base = l1_entry->l1_section_1m.base_address << FMMU_PTE_L1_INDEX_PA_SHIFT;
            if (pa >= l1_pa_base && pa <= l1_pa_base + 0xFFFFF)
            {
                if (range_count < max_va)
                {
                    offset = pa - l1_pa_base;
                    va_out[range_count++] = l1_va_base + offset;

                    range_count++;
                }
            }
        }
        else if (l1_entry->undefined.id == FMMU_PTE_ID_L2_PT)
        {
            u32 l2_table_base = l1_entry->l2_page_table_ref.l2_page_table_address << FMMU_PT_L2_ADDR_SHIFT;
            struct FMmuL2PageTable *l2_table = (struct FMmuL2PageTable *)l2_table_base;
            for (int j = 0; j < FMMU_PT_L2_NUM_ENTRIES; ++j)
            {
                union FMmuL2PageTableEntry *l2_entry = &l2_table->entries[j];
                u32 l2_va = l1_va_base + (j << FMMU_PTE_L2_INDEX_PA_SHIFT);
                if (l2_entry->undefined.id == FMMU_PTE_ID_SMALL_PAGE)
                {
                    u32 l2_pa_base = l2_entry->l2_page_4k.pa_base << FMMU_PTE_L2_SMALL_PAGE_ADDR_SHIFT;
                    if (pa >= l2_pa_base && pa <= l2_pa_base + 0xFFF)
                    {
                        if (range_count < max_va)
                        {
                            offset = pa - l2_pa_base;
                            va_out[range_count++] = l2_va + offset;
                            range_count++;
                        }
                    }
                }
            }
        }
    }
    return range_count;
}


fsize_t FMmuGetPAByVA(uintptr_t va, uintptr_t *pa_out)
{
    struct ArmMmuRegion mmu_region;

    if (FMmuGetMappingByVA(va, &mmu_region) == 1)
    {
        if (va >= mmu_region.base_va && va < mmu_region.base_va + mmu_region.size)
        {
            uintptr_t offset = va - mmu_region.base_va;
            *pa_out = mmu_region.base_pa + offset;
            return 1; // Successfully found the PA
        }
    }

    return 0; // No mapping found
}
