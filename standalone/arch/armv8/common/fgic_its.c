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
 * FilePath: fgic_its.h
 * Created Date: 2024-07-24 11:29:55
 * Last Modified: 2025-06-12 10:06:24
 * Description:  This file is for
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 * 1.0     huanghe	  2024/08/09   		first version
 * 1.1     huanghe    2025/06/04        fix cmd_queue size bug
 */
#include "fgic_its.h"
#include "ftypes.h"
#include "fio.h"
#include "fsleep.h"
#include "fgic_v3.h"
#include "fcpu_info.h"
#include "fprintk.h"
#include "fparameters.h"
#include "tlsf.h"
#include "fkernel.h"
#include "string.h"
#include "fassert.h"
#include "faarch.h"
#include "fdebug.h"
#include "fmmu.h"


#define FGIC_DEBUG_TAG          "FGIC_V3_ITS"
#define FGIC_ERROR(format, ...) FT_DEBUG_PRINT_E(FGIC_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGIC_DEBUG_I(format, ...) \
    FT_DEBUG_PRINT_I(FGIC_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGIC_DEBUG_D(format, ...) \
    FT_DEBUG_PRINT_D(FGIC_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGIC_DEBUG_W(format, ...) \
    FT_DEBUG_PRINT_W(FGIC_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGIC_DEBUG_E(format, ...) \
    FT_DEBUG_PRINT_E(FGIC_DEBUG_TAG, format, ##__VA_ARGS__)

#define FGIC_CMD_POS_CMD_TIMEOUT_US 1000000

#define KB(x)                       ((x) << 10)

#define SIZE_256                    256
#define SIZE_4K                     SZ_4K
#define SIZE_16K                    SZ_16K
#define SIZE_64K                    SZ_64K

/* wait 500ms & wakeup every millisecond */
#define WAIT_QUIESCENT_MS           500
#define ITS_CMD_QUEUE_SIZE          SIZE_64K
#define ITS_CMD_QUEUE_NR_ENTRIES    (ITS_CMD_QUEUE_SIZE / sizeof(struct FGicItsCmdBlock))
#define MAX(a, b)                   (((a) > (b)) ? (a) : (b))
/**
 * @brief Value of @p x rounded up to the next multiple of @p align.
 */
#define ROUND_UP(x, align)                                                          \
    ((((unsigned long)(x) + ((unsigned long)(align)-1)) / (unsigned long)(align)) * \
     (unsigned long)(align))


#define ITS_MAX_LPI_NRBITS      16 /* 64K LPIs */

#define LPI_PROPBASE_SZ(nrbits) ROUND_UP(BIT(nrbits), KB(64))
#define LPI_PENDBASE_SZ(nrbits) ROUND_UP(BIT(nrbits) / 8, KB(64))

static u8 lpi_prop_table[LPI_PROPBASE_SZ(16)] __attribute__((aligned(64 * 1024)));
static u8 lpi_pend_table[LPI_PENDBASE_SZ(16)] __attribute__((aligned(64 * 1024)));

/* Used for  */
static u8 memp_buf[SZ_4M] = {0};


static void FGicItsLpiIdInit(FGicLpiIntrId *lpi_id)
{
    for (int i = 0; i < (LPI_INTR_MAX_NUM + 31) / 32; i++)
    {
        lpi_id->bitmap[i] = 0;
    }
}

/**
 * @name: FGicItsLpiIdAlloc
 * @msg: Allocate an LPI (Locality-specific Peripheral Interrupt) ID.
 * @return {u32} Allocated LPI ID or 0 if no ID is available.
 * @note: 
 * @param {FGic} *instanse_p - Pointer to the FGic instance.
 */
u32 FGicItsLpiIdAlloc(FGic *instanse_p)
{
    FASSERT(instanse_p);
    FGicItsData *data = &instanse_p->its;
    FGicLpiIntrId *lpi_id = &data->lpi_id;

    for (int i = 0; i < LPI_INTR_MAX_NUM; i++)
    {
        int index = i / 32;
        int bit = i % 32;
        if ((lpi_id->bitmap[index] & (1 << bit)) == 0)
        {
            lpi_id->bitmap[index] |= (1 << bit);
            return i + FGIC_LPI_START_ID;
        }
    }
    return 0; /* 所有中断号都已被分配 */
}

/**
 * @name: FGicItsLpiIdFree
 * @msg: Free a previously allocated LPI ID.
 * @return {*}
 * @note: 
 * @param {FGic} *instanse_p - Pointer to the FGic instance.
 * @param {u32} int_id - Interrupt ID to be freed.FGicItsSendSyncCmd
 */
void FGicItsLpiIdFree(FGic *instanse_p, u32 int_id)
{
    FASSERT(instanse_p);
    FGicItsData *data = &instanse_p->its;
    FGicLpiIntrId *lpi_id = &data->lpi_id;

    if (int_id < FGIC_LPI_START_ID || int_id >= (LPI_INTR_MAX_NUM + FGIC_LPI_START_ID))
    {
        FGIC_DEBUG_E("Invalid LPI int_id number: %d\n", int_id);
        return;
    }
    u32 index = (int_id - FGIC_LPI_START_ID) / 32;
    u32 bit = (int_id - FGIC_LPI_START_ID) % 32;
    lpi_id->bitmap[index] &= ~(1 << bit);
}


static uintptr_t FGicItsGetRdbase(FGic *instanse_p)
{
    FGicItsData *data = &instanse_p->its;
    u64 typer = FtIn64(data->base + GITS_TYPER);
    if (GITS_TYPER_PTA_GET(typer))
    {
        return instanse_p->redist_base;
    }
    else
    {
        return GICR_TYPER_PROCESSOR_NUMBER_GET((u64)(FtIn64(instanse_p->redist_base + GICR_TYPER)));
    }
}


static FError FGicItsPostCmd(FGicItsData *data, struct FGicItsCmdBlock *cmd)
{
    u64 wr_idx, rd_idx, idx;
    u32 count = FGIC_CMD_POS_CMD_TIMEOUT_US; /* 1s! */

    wr_idx = (data->cmd_write - data->cmd_base) * sizeof(*cmd);
    rd_idx = FtIn32(data->base + GITS_CREADR);

    FtOut32(data->base + GITS_CWRITER, wr_idx);
    DSB();
    while (1)
    {
        idx = FtIn32(data->base + GITS_CREADR);

        if (idx == wr_idx)
        {
            break;
        }

        count--;
        if (!count)
        {
            FGIC_DEBUG_E("data->base is %p.", data->base);
            FGIC_DEBUG_E("ITS queue timeout (rd %lld => %lld => wr %lld).", rd_idx, idx, wr_idx);
            return FGIC_ITS_CMD_TIMEOUT;
        }
        fsleep_microsec(1);
    }

    return FGIC_SUCCESS;
}


static int FGicItsQueueFull(FGicItsData *data)
{
    int widx;
    int ridx;

    widx = data->cmd_write - data->cmd_base;
    ridx = FtIn32(data->base + GITS_CREADR) / sizeof(struct FGicItsCmdBlock);
    /* This is incredibly unlikely to happen, unless the ITS locks up. */
    return (((widx + 1) % ITS_CMD_QUEUE_NR_ENTRIES) == ridx);
}

static struct FGicItsCmdBlock *FGicItsAllocateEntry(FGicItsData *data)
{
    struct FGicItsCmdBlock *cmd = NULL;
    u32 count = FGIC_CMD_POS_CMD_TIMEOUT_US; /* 1s! */

    while (FGicItsQueueFull(data))
    {
        count--;
        if (!count)
        {
            FGIC_DEBUG_E("ITS queue not draining");
            return NULL;
        }
        fsleep_microsec(1);
    }

    cmd = data->cmd_write++;

    /* Handle queue wrapping */
    if (data->cmd_write == (data->cmd_base + ITS_CMD_QUEUE_NR_ENTRIES))
    {
        data->cmd_write = data->cmd_base;
    }

    /* Clear command  */
    cmd->raw_cmd[0] = 0;
    cmd->raw_cmd[1] = 0;
    cmd->raw_cmd[2] = 0;
    cmd->raw_cmd[3] = 0;

    return cmd;
}


/**
 * @name: FGicItsSendSyncCmd
 * @msg: Send a SYNC command to the ITS.
 * @return {FError} Result of the operation.
 * @note: 
 * @param {FGicItsData} *data - Pointer to the ITS data.
 * @param {uintptr_t} rd_addr - Address of the Redistributor.
 */
static FError FGicItsSendSyncCmd(FGicItsData *data, uintptr_t rd_addr)
{
    struct FGicItsCmdBlock *cmd = FGicItsAllocateEntry(data);

    if (!cmd)
    {
        return FGIC_ITS_CMD_GET_ERR;
    }

    cmd->raw_cmd[0] = MASK_SET(GITS_CMD_ID_SYNC, GITS_CMD_ID);
    cmd->raw_cmd[2] = MASK_SET((u64)rd_addr >> GITS_CMD_RDBASE_ALIGN, GITS_CMD_RDBASE);
    return FGicItsPostCmd(data, cmd);
}

static FError FGicItsSendInvallCmd(FGicItsData *data, u32 icid)
{
    struct FGicItsCmdBlock *cmd = FGicItsAllocateEntry(data);

    if (!cmd)
    {
        return FGIC_ITS_CMD_GET_ERR;
    }

    cmd->raw_cmd[0] = MASK_SET(GITS_CMD_ID_INVALL, GITS_CMD_ID);
    cmd->raw_cmd[2] = MASK_SET(icid, GITS_CMD_ICID);
    return FGicItsPostCmd(data, cmd);
}

/**
 * @name: FGicItsLpiSetup
 * @msg: Setup the LPI configuration.
 * @return {*}
 * @note: 
 * @param {FGic} *instanse_p - Pointer to the FGic instance.
 * @param {u32} int_id - Interrupt ID.
 * @param {u8} enable - Enable flag.
 */
void FGicItsLpiSetup(const FGic *instanse_p, u32 int_id, u8 enable)
{
    FASSERT(instanse_p);
    uint8_t *cfg = &((uint8_t *)instanse_p->lpi_global_table_base)[int_id - FGIC_LPI_START_ID];
    int ret;
    u32 cpu_id = 0;
    GetCpuId(&cpu_id);
    FGicItsData *data = (FGicItsData *)&instanse_p->its;


    if (enable)
    {
        *cfg |= BIT(0);
    }
    else
    {
        *cfg &= ~BIT(0);
    }

    DSB();
    ret = FGicItsSendInvallCmd(data, cpu_id);
    if (ret)
    {
        FGIC_DEBUG_E("FGicItsSendInvallCmd failed.");
    }

    FGicItsSendSyncCmd(data, FGicItsGetRdbase((FGic *)instanse_p));
}


/**
 * @name: FGicItsLpiSetPriority
 * @msg: Set the priority of an LPI.
 * @return {*}
 * @note: 
 * @param {FGic} *instanse_p - Pointer to the FGic instance.
 * @param {u32} int_id - Interrupt ID.
 * @param {u32} prio - Priority value.
 */
void FGicItsLpiSetPriority(const FGic *instanse_p, u32 int_id, u32 prio)
{
    FASSERT(instanse_p);
    uint8_t *cfg = &((uint8_t *)instanse_p->lpi_global_table_base)[int_id - FGIC_LPI_START_ID];
    FGicItsData *data = (FGicItsData *)&instanse_p->its;
    int ret;
    u32 cpu_id = 0;
    GetCpuId(&cpu_id);

    *cfg &= 0xfc;
    *cfg |= prio & 0xfc;

    DSB();

    ret = FGicItsSendInvallCmd(data, cpu_id);
    if (ret)
    {
        FGIC_DEBUG_E("FGicItsSendInvallCmd failed.");
    }

    FGicItsSendSyncCmd(data, FGicItsGetRdbase((FGic *)instanse_p));
}


u8 FGicItsLpiGetPriority(const FGic *instanse_p, u32 int_id)
{
    FASSERT(instanse_p);
    u8 *cfg = &((uint8_t *)instanse_p->lpi_global_table_base)[int_id - FGIC_LPI_START_ID];
    *cfg &= 0xfc;
    return *cfg;
}


static FError FGicItsSendMapcCmd(FGicItsData *data, u32 icid, uintptr_t rd_addr, u8 valid)
{
    struct FGicItsCmdBlock *cmd = FGicItsAllocateEntry(data);

    if (!cmd)
    {
        return FGIC_ITS_CMD_GET_ERR;
    }

    cmd->raw_cmd[0] = MASK_SET(GITS_CMD_ID_MAPC, GITS_CMD_ID);
    cmd->raw_cmd[2] = MASK_SET(icid, GITS_CMD_ICID) | MASK_SET(rd_addr, GITS_CMD_RDBASE) |
                      MASK_SET(valid ? (u64)1 : (u64)0, GITS_CMD_VALID);

    return FGicItsPostCmd(data, cmd);
}


/* Probe the BASER(i) to get the largest supported page size */
static size_t FGicItsProbeBaserPageSize(FGicItsData *data, int i)
{
    u64 page_size = GITS_BASER_PAGE_SIZE_64K;

    while (page_size > GITS_BASER_PAGE_SIZE_4K)
    {
        u64 reg = FtIn64(data->base + GITS_BASER(i));

        reg &= ~MASK(GITS_BASER_PAGE_SIZE);
        reg |= MASK_SET(page_size, GITS_BASER_PAGE_SIZE);

        FtOut64(data->base + GITS_BASER(i), reg);

        reg = FtIn64(data->base + GITS_BASER(i));

        if (MASK_GET(reg, GITS_BASER_PAGE_SIZE) == page_size)
        {
            break;
        }

        switch (page_size)
        {
            case GITS_BASER_PAGE_SIZE_64K:
                page_size = GITS_BASER_PAGE_SIZE_16K;
                break;
            default:
                page_size = GITS_BASER_PAGE_SIZE_4K;
        }
    }

    switch (page_size)
    {
        case GITS_BASER_PAGE_SIZE_64K:
            return SIZE_64K;
        case GITS_BASER_PAGE_SIZE_16K:
            return SIZE_16K;
        default:
            return SIZE_4K;
    }
}

static void FGicItsSetupCmdQueue(FGicItsData *data)
{
    u64 reg = 0;
    struct FGicItsCmdBlock *cmd_queue = NULL;
    cmd_queue = (struct FGicItsCmdBlock *)tlsf_memalign(data->mem_poll_p, ITS_CMD_QUEUE_SIZE,
                                                        ITS_CMD_QUEUE_NR_ENTRIES *
                                                            sizeof(struct FGicItsCmdBlock));
    /* Zero out cmd table */
    memset(cmd_queue, 0, ITS_CMD_QUEUE_NR_ENTRIES * sizeof(struct FGicItsCmdBlock));
    reg |= MASK_SET((ITS_CMD_QUEUE_NR_ENTRIES * sizeof(struct FGicItsCmdBlock)) / SIZE_4K - 1,
                    GITS_CBASER_SIZE);
    reg |= MASK_SET(GIC_BASER_SHARE_INNER, GITS_CBASER_SHAREABILITY);
    reg |= MASK_SET((uintptr_t)cmd_queue >> GITS_CBASER_ADDR_SHIFT, GITS_CBASER_ADDR);
    reg |= MASK_SET(GIC_BASER_CACHE_RAWAWB, GITS_CBASER_OUTER_CACHE);
    reg |= MASK_SET(GIC_BASER_CACHE_RAWAWB, GITS_CBASER_INNER_CACHE);
    reg |= MASK_SET(1ULL, GITS_CBASER_VALID);
    FtOut64(data->base + GITS_CBASER, reg);

    data->cmd_base = (struct FGicItsCmdBlock *)cmd_queue;
    data->cmd_write = data->cmd_base;

    FtOut64(data->base + GITS_CWRITER, 0);
}


static FError FGicItsForceQuiescent(FGicItsData *data)
{
    u32 count = WAIT_QUIESCENT_MS;

    u32 reg = FtIn32(data->base + GITS_CTLR);

    if (GITS_CTLR_ENABLED_GET(reg))
    {
        /* Disable ITS */
        reg &= ~MASK(GITS_CTLR_ENABLED);

        FtOut32(data->base + GITS_CTLR, reg);
    }

    while (1)
    {
        if (GITS_CTLR_QUIESCENT_GET(reg))
        {
            return FGIC_SUCCESS;
        }

        count--;
        if (!count)
        {
            return FGIC_ITS_ERR_QUIETSCENT;
        }

        fsleep_millisec(1);
        reg = FtIn32(data->base + GITS_CTLR);
    }

    return FGIC_SUCCESS;
}

static void FGicItsAllocTables(FGicItsData *data)
{
    /* 获取device id 的最大位宽 */
    u32 device_ids = GITS_TYPER_DEVBITS_GET(FtIn64(data->base + GITS_TYPER)) + 1;
    int i;

    for (i = 0; i < GITS_BASER_NR_REGS; ++i)
    {
        u64 reg = FtIn64(data->base + GITS_BASER(i));
        u64 type = GITS_BASER_TYPE_GET(reg);
        size_t page_size, entry_size, page_cnt, lvl2_width = 0;
        u8 indirect = 0;
        void *alloc_addr;
        entry_size = GITS_BASER_ENTRY_SIZE_GET(reg) + 1;
        switch (GITS_BASER_PAGE_SIZE_GET(reg))
        {
            case GITS_BASER_PAGE_SIZE_4K:
                page_size = SIZE_4K;
                break;
            case GITS_BASER_PAGE_SIZE_16K:
                page_size = SIZE_16K;
                break;
            case GITS_BASER_PAGE_SIZE_64K:
                page_size = SIZE_64K;
                break;
            default:
                page_size = SIZE_4K;
        }
        FGIC_DEBUG_D("page_size is %ld, entry_size is %ld, type is %ld \r\n", page_size,
                     entry_size, type);

        switch (type)
        {
            case GITS_BASER_TYPE_DEVICE: /* 设备 */
                if (device_ids > 16)
                {
                    /* Use the largest possible page size for indirect */
                    page_size = FGicItsProbeBaserPageSize(data, i);

                    /*
				 * lvl1 table size:
				 * subtract ID bits that sparse lvl2 table from 'ids'
				 * which is reported by ITS hardware times lvl1 table
				 * entry size.
				 */
                    lvl2_width = fls(page_size / entry_size) - 1;
                    device_ids -= (u32)lvl2_width + 1;

                    /* The level 1 entry size is a 64bit pointer */
                    entry_size = sizeof(u64);

                    indirect = 1;
                }

                page_cnt = ROUND_UP(entry_size << device_ids, page_size) / page_size;
                break;
            case GITS_BASER_TYPE_COLLECTION: /* 中断集合 */
                page_cnt = ROUND_UP(entry_size * FCORE_NUM, page_size) / page_size;
                break;
            default:
                continue;
        }

        FGIC_DEBUG_D("page_size = %ld,page_size*page_cnt = %ld.", page_size, page_size * page_cnt);
        alloc_addr = tlsf_memalign(data->mem_poll_p, page_size, page_size * page_cnt);

        switch (page_size)
        {
            case SIZE_4K:
                reg = MASK_SET(GITS_BASER_PAGE_SIZE_4K, GITS_BASER_PAGE_SIZE);
                break;
            case SIZE_16K:
                reg = MASK_SET(GITS_BASER_PAGE_SIZE_16K, GITS_BASER_PAGE_SIZE);
                break;
            case SIZE_64K:
                reg = MASK_SET(GITS_BASER_PAGE_SIZE_64K, GITS_BASER_PAGE_SIZE);
                break;
        }

        reg |= MASK_SET(page_cnt - 1, GITS_BASER_SIZE);
        reg |= MASK_SET(GIC_BASER_SHARE_INNER, GITS_BASER_SHAREABILITY);
        reg |= MASK_SET(((uintptr)alloc_addr) >> GITS_BASER_ADDR_SHIFT, GITS_BASER_ADDR);
        reg |= MASK_SET(GIC_BASER_CACHE_RAWAWB, GITS_BASER_OUTER_CACHE);
        reg |= MASK_SET(GIC_BASER_CACHE_RAWAWB, GITS_BASER_INNER_CACHE);
        reg |= MASK_SET(indirect ? (u64)1 : (u64)0, GITS_BASER_INDIRECT);
        reg |= MASK_SET((u64)1, GITS_BASER_VALID);
        FtOut64(data->base + GITS_BASER(i), reg);

        if (type == GITS_BASER_TYPE_DEVICE && indirect)
        {
            data->dev_table_is_indirect = indirect;
            data->indirect_dev_lvl1_table = (u64 *)alloc_addr;
            data->indirect_dev_lvl1_width = device_ids;
            data->indirect_dev_lvl2_width = lvl2_width;
            data->indirect_dev_page_size = page_size;
        }
    }
}


static int FGicItsSendMaptiCmd(FGicItsData *data, u32 device_id, u32 event_id, u32 int_id, u32 icid)
{
    struct FGicItsCmdBlock *cmd = FGicItsAllocateEntry(data);

    if (!cmd)
    {
        return FGIC_ITS_CMD_GET_ERR;
    }

    cmd->raw_cmd[0] = MASK_SET(GITS_CMD_ID_MAPTI, GITS_CMD_ID) |
                      MASK_SET((u64)device_id, GITS_CMD_DEVICEID);
    cmd->raw_cmd[1] = MASK_SET(event_id, GITS_CMD_EVENTID) | MASK_SET((u64)int_id, GITS_CMD_PINTID);
    cmd->raw_cmd[2] = MASK_SET(icid, GITS_CMD_ICID);


    return FGicItsPostCmd(data, cmd);
}


static FError FGicItsSendMapdCmd(FGicItsData *data, u32 device_id, u32 size,
                                 uintptr_t itt_addr, u32 valid)
{
    struct FGicItsCmdBlock *cmd = FGicItsAllocateEntry(data);

    if (!cmd)
    {
        return FGIC_ITS_CMD_GET_ERR;
    }

    cmd->raw_cmd[0] = MASK_SET(GITS_CMD_ID_MAPD, GITS_CMD_ID) |
                      MASK_SET((u64)device_id, GITS_CMD_DEVICEID);
    cmd->raw_cmd[1] = MASK_SET(size, GITS_CMD_SIZE);
    cmd->raw_cmd[2] = MASK_SET((u64)itt_addr >> GITS_CMD_ITTADDR_ALIGN, GITS_CMD_ITTADDR) |
                      MASK_SET(valid ? (u64)1 : (u64)0, GITS_CMD_VALID);

    return FGicItsPostCmd(data, cmd);
}


static FError FGicItsSendDiscardCmd(FGicItsData *data, u32 device_id, u32 event_id)
{
    struct FGicItsCmdBlock *cmd = FGicItsAllocateEntry(data);

    if (!cmd)
    {
        return FGIC_ITS_CMD_GET_ERR;
    }

    cmd->raw_cmd[0] = ((u64)device_id << 32) | GITS_CMD_ID_DISCARD;
    cmd->raw_cmd[1] = event_id;

    return FGicItsPostCmd(data, cmd);
}


u32 FGicItsGetTransAddr(const FGic *instanse_p)
{
    FASSERT(instanse_p);
    FGicItsData *data = (FGicItsData *)&instanse_p->its;
    return (data->base + GITS_TRANSLATER);
}


/**
 * @name: FGicItsSendIntCmd
 * @msg: Send an INT command to the ITS.
 * @return {FError} Result of the operation.
 * @note: 
 * @param {FGicItsData} *data - Pointer to the ITS data.
 * @param {u32} device_id - Device ID.
 * @param {u32} event_id - Event ID.
 */
FError FGicItsSendIntCmd(FGicItsData *data, u32 device_id, u32 event_id)
{
    struct FGicItsCmdBlock *cmd = FGicItsAllocateEntry(data);

    if (!cmd)
    {
        return FGIC_ITS_CMD_GET_ERR;
    }

    cmd->raw_cmd[0] = MASK_SET(GITS_CMD_ID_INT, GITS_CMD_ID) |
                      MASK_SET((u64)device_id, GITS_CMD_DEVICEID);
    cmd->raw_cmd[1] = MASK_SET(event_id, GITS_CMD_EVENTID);

    return FGicItsPostCmd(data, cmd);
}

/**
 * @name: FGicItsDeviceIdInit
 * @msg: Initialize a device ID.
 * @return {FError} Result of the operation.
 * @note: 
 * @param {FGic} *instanse_p - Pointer to the FGic instance.
 * @param {u32} device_id - Device ID.
 * @param {u32} nites - Number of ITT entries.
 */
FError FGicItsDeviceIdInit(const FGic *instanse_p, u32 device_id, u32 nites)
{
    FASSERT(instanse_p);
    FGicItsData *data = (FGicItsData *)&instanse_p->its;
    size_t entry_size, alloc_size;
    uintptr alloc_addr = 0;
    uintptr itt;
    int nr_ites;
    FError ret;
    int i;

    entry_size = GITS_TYPER_ITT_ENTRY_SIZE_GET(FtIn64(data->base + GITS_TYPER)) + 1;
    if (data->dev_table_is_indirect)
    {
        size_t offset = device_id >> data->indirect_dev_lvl2_width;

        /* Check if DeviceID can fit in the Level 1 table */
        if (offset > (1 << data->indirect_dev_lvl1_width))
        {
            return FGIC_ITS_DEVICEID_OVER_LV1_WIDTH;
        }

        /* Check if a Level 2 table has already been allocated for the DeviceID */
        if (!data->indirect_dev_lvl1_table[offset])
        {
            FGIC_DEBUG_D("Allocating Level 2 Device %ldK table", data->indirect_dev_page_size / 1024);

            alloc_addr = (uintptr)tlsf_memalign((tlsf_t)data->mem_poll_p, data->indirect_dev_page_size,
                                                data->indirect_dev_page_size);
            if (!alloc_addr)
            {
                return FGIC_ITS_CMD_GET_ERR;
            }
            memset((void *)alloc_addr, 0, data->indirect_dev_page_size);

            data->indirect_dev_lvl1_table[offset] = (uintptr_t)alloc_addr |
                                                    MASK_SET(1, GITS_BASER_VALID);

            DSB();
        }
    }

    /* ITT must be of power of 2 */
    nr_ites = MAX(2, nites);
    alloc_size = ROUND_UP(nr_ites * entry_size, 256);

    itt = (uintptr)tlsf_memalign(data->mem_poll_p, 256, alloc_size);
    if (!itt)
    {
        if (alloc_addr)
        {
            tlsf_free((tlsf_t)data->mem_poll_p, (void *)alloc_addr);
        }
        FGIC_DEBUG_E("Failed to allocate ITT table\n");
        return FGIC_ITS_MEM_POLL_ALLOC_ERR;
    }

    for (i = 0; i < FGIC_DEVICE_MAX_ID; i++)
    {
        if (data->device_table[i].valid_flg == 0)
        {
            data->device_table[i].itt_table_base = (uintptr_t)itt;
            data->device_table[i].lvl1_table_base = (uintptr_t)alloc_addr;
            data->device_table[i].device_id = device_id;
            data->device_table[i].valid_flg = 1;
            data->device_table[i].event_id_num = nites;
            break;
        }
    }

    if (i == FGIC_DEVICE_MAX_ID)
    {
        if (alloc_addr)
        {
            tlsf_free((tlsf_t)data->mem_poll_p, (void *)alloc_addr);
        }
        tlsf_free((tlsf_t)data->mem_poll_p, (void *)itt);
        return FGIC_ITS_DEVICE_TABLE_IS_EXHAUST;
    }
    else if (!data->device_table[i].valid_flg)
    {
        if (alloc_addr)
        {
            tlsf_free((tlsf_t)data->mem_poll_p, (void *)alloc_addr);
        }
        tlsf_free((tlsf_t)data->mem_poll_p, (void *)itt);
        return FGIC_ITS_DEVICE_TABLE_IS_EXHAUST;
    }

    /* size is log2(ites) - 1, equivalent to (fls(ites) - 1) - 1 */
    ret = FGicItsSendMapdCmd(data, device_id, fls(nr_ites) - 2, (uintptr_t)itt, 1);
    if (ret)
    {
        FGIC_DEBUG_E("Failed to map device id %x ITT table.", device_id);
        return ret;
    }

    return FGIC_SUCCESS;
}


/**
 * @name: FGicItsDeviceIdDeinit
 * @msg: Deinitialize a device ID.
 * @return {FError} Result of the operation.
 * @note: 
 * @param {FGic} *instanse_p - Pointer to the FGic instance.
 * @param {u32} device_id - Device ID.
 */
FError FGicItsDeviceIdDeinit(const FGic *instanse_p, u32 device_id)
{
    FASSERT(instanse_p);
    FGicItsData *data = (FGicItsData *)&instanse_p->its;
    int i, event_id_cnt;
    FError ret;
    /* 先找到device_id 对应的数据结果 */

    for (i = 0; i < FGIC_DEVICE_MAX_ID; i++)
    {
        if (data->device_table[i].valid_flg)
        {
            if (device_id == data->device_table[i].device_id)
            {
                break;
            }
        }
    }

    if ((!data->device_table[i].valid_flg) || (i == FGIC_DEVICE_MAX_ID))
    {
        FGIC_DEBUG_E("Device id %x not exist\n", i);
        return FGIC_ITS_DEVICE_NOT_EXIST;
    }

    /* 先发discard  */
    for (event_id_cnt = 0; event_id_cnt < data->device_table[i].event_id_num; event_id_cnt++)
    {
        ret = FGicItsSendDiscardCmd(data, data->device_table[i].device_id, event_id_cnt);
        if (ret)
        {
            FASSERT(0);
        }
    }

    /* 释放内存 */
    tlsf_free((tlsf_t)data->mem_poll_p, (void *)data->device_table[i].itt_table_base);
    if (data->dev_table_is_indirect)
    {
        if (data->device_table[i].lvl1_table_base)
        {
            tlsf_free((tlsf_t)data->mem_poll_p, (void *)data->device_table[i].lvl1_table_base);
        }
    }

    data->device_table[i].valid_flg = 0;
    return FGIC_SUCCESS;
}


/**
 * @name: FGicItsMapIntId
 * @msg: Map an interrupt ID.
 * @return {FError} Result of the operation.
 * @note: 
 * @param {FGic} *instanse_p - Pointer to the FGic instance.
 * @param {u32} device_id - Device ID.
 * @param {u32} event_id - Event ID.
 * @param {u32} int_id - Interrupt ID.
 */
FError FGicItsMapIntId(const FGic *instanse_p, u32 device_id, u32 event_id, u32 int_id)
{
    FASSERT(instanse_p);
    FGicItsData *data = (FGicItsData *)&instanse_p->its;
    u32 cpu_id = 0;
    GetCpuId(&cpu_id);
    int ret;

    if (int_id < FGIC_LPI_START_ID)
    {
        return FGIC_ITS_INTID_NOT_FIT;
    }

    /* The CPU id directly maps as ICID for the current CPU redistributor */
    ret = FGicItsSendMaptiCmd(data, device_id, event_id, int_id, cpu_id);
    if (ret)
    {
        FGIC_DEBUG_E("Failed to map eventid %d to int_id %d for deviceid %x .",
                     event_id, int_id - FGIC_LPI_START_ID, device_id);
        return ret;
    }

    return FGicItsSendSyncCmd(data, FGicItsGetRdbase((FGic *)instanse_p));
}

/**
 * @name: FGicLpisSetup
 * @msg: Setup LPIs.Set GICR_PENDBASER and GICR_PROPBASER registers.
 * @return {*}
 * @note: 
 * @param {FGic} *instanse_p - Pointer to the FGic instance.
 */
void FGicLpisSetup(FGic *instanse_p)
{
    FASSERT(instanse_p);
    uintptr_t rdist = instanse_p->redist_base;
    u64 reg;
    u32 ctlr;
    u32 lpi_id_bits = MIN(FGIC_GICD_TYPER_IDBITS(FGIC_READREG32(GICV3_BASE_ADDR, FGIC_GICD_TYPER_OFFSET)),
                          ITS_MAX_LPI_NRBITS);
    instanse_p->max_lpi_num = FGIC_LPI_START_ID + (1 << (lpi_id_bits + 1)) - 1;

    FASSERT(FGicWaitRwp(rdist, GICR_CTLR_RWP_WAIT) == FGIC_SUCCESS);
    ctlr = FtIn32(rdist + GICR_CTLR);
    ctlr &= ~GICR_CTLR_ENABLE_LPIS; /* 关闭lpis */
    FtOut32(rdist + GICR_CTLR, ctlr);
    FASSERT(FGicWaitRwp(rdist, GICR_CTLR_RWP_WAIT) == FGIC_SUCCESS);

    /* Lpi  Configuration table set */
    reg = (GIC_BASER_SHARE_INNER << GITR_PROPBASER_SHAREABILITY_SHIFT) |
          (GIC_BASER_CACHE_RAWAWB << GITR_PROPBASER_INNER_CACHE_SHIFT) |
          ((uintptr_t)&lpi_prop_table & (GITR_PROPBASER_ADDR_MASK << GITR_PROPBASER_ADDR_SHIFT)) |
          (GIC_BASER_CACHE_RAWAWB << GITR_PROPBASER_OUTER_CACHE_SHIFT) |
          ((lpi_id_bits - 1) & GITR_PROPBASER_ID_BITS_MASK);
    FGIC_DEBUG_D("LPI Configuration table set reg = 0x%llx .", reg);
    FtOut64(rdist + GICR_PROPBASER, reg); /* 设置 LPI Configuration table */
    FGIC_DEBUG_D("FtOut64(rdist + GICR_PROPBASER,reg) reg = 0x%llx .", FtIn64(rdist + GICR_PROPBASER));
    /* PENDBASE */
    /* Lpi pending table set */
    reg = (GIC_BASER_SHARE_INNER << GITR_PENDBASER_SHAREABILITY_SHIFT) |
          (GIC_BASER_CACHE_RAWAWB << GITR_PENDBASER_INNER_CACHE_SHIFT) |
          ((uintptr_t)&lpi_pend_table & (GITR_PENDBASER_ADDR_MASK << GITR_PENDBASER_ADDR_SHIFT)) |
          (GIC_BASER_CACHE_RAWAWB << GITR_PENDBASER_OUTER_CACHE_SHIFT) | GITR_PENDBASER_PTZ;
    FGIC_DEBUG_D("LPI pending table set reg = 0x%llx .", reg);
    FtOut64(rdist + GICR_PENDBASER, reg); /* 设置 LPI pending table */
    FGIC_DEBUG_D("FtOut64(rdist + GICR_PENDBASER,reg) reg = 0x%llx .", FtIn64(rdist + GICR_PENDBASER));


    FASSERT(FGicWaitRwp(rdist, GICR_CTLR_RWP_WAIT) == FGIC_SUCCESS);
    ctlr = FtIn32(rdist + GICR_CTLR);
    ctlr |= GICR_CTLR_ENABLE_LPIS;
    instanse_p->lpi_global_table_base = (uintptr)lpi_prop_table;
    instanse_p->lpi_global_pending_base = (uintptr)lpi_pend_table;
    FtOut32(rdist + GICR_CTLR, ctlr);
    FASSERT(FGicWaitRwp(rdist, GICR_CTLR_RWP_WAIT) == FGIC_SUCCESS);
    DSB();
}

/**
 * @name: FGicItsInit
 * @msg: Initialize the ITS.
 * @return {FError} Result of the operation.
 * @note: 
 * @param {FGic} *instanse_p - Pointer to the FGic instance.
 */
FError FGicItsInit(FGic *instanse_p)
{
    FASSERT(instanse_p);
    FError ret = 0;
    u32 reg = 0;
    u32 cpu_id = 0;

    FGicItsData *data = (FGicItsData *)&instanse_p->its;

    data->mem_poll_p = tlsf_create_with_pool(memp_buf, sizeof(memp_buf));
    if (!data->mem_poll_p)
    {
        FGIC_DEBUG_E("Failed to create tlsf pool for memory polling\n");
        return FGIC_ITS_MEM_POLL_CREATE_ERR;
    }

    ret = FGicItsForceQuiescent(data);
    /*  需要考虑多ITS的情况，强制ITS 模块关闭 */

    if (ret)
    {
        FGIC_DEBUG_E("GICv3 ITS force quiescent failed\n");
        return ret;
    }
    /* 申请its 表 */
    FGicItsAllocTables(data);
    /* 设置ITS cmd 队列 */
    FGicItsSetupCmdQueue(data);

    /* 开启ITS*/
    reg = FtIn32(data->base + GITS_CTLR);
    reg |= MASK_SET(1, GITS_CTLR_ENABLED);
    FtOut32(data->base + GITS_CTLR, reg);

    GetCpuId(&cpu_id);
    ret = FGicItsSendMapcCmd(data, cpu_id, FGicItsGetRdbase(instanse_p), 1);
    if (ret)
    {
        FGIC_DEBUG_E("Failed to map boot CPU redistributor");
        return ret;
    }

    FGicItsLpiIdInit(&data->lpi_id);

    return 0;
}
