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
 * FilePath: fpcie_c_dma.c
 * Created Date: 2023-08-02 09:39:56
 * Last Modified: 2023-08-06 09:53:25
 * Description:  This file is for pciec dma implementation
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 * 1.0     huanghe    2023/08/06        first release
 */
#include "fpciec_dma.h"
#include "fpciec.h"
#include "fpciec_hw.h"
#include "fassert.h"
#include "fdrivers_port.h"


#define FPCIE_DMA_DEBUG_TAG "FPCIE_DMA"
#define FPCIE_DMA_DEBUG_D(format, ...) \
    FT_DEBUG_PRINT_D(FPCIE_DMA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FPCIE_DMA_DEBUG_I(format, ...) \
    FT_DEBUG_PRINT_I(FPCIE_DMA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FPCIE_DMA_DEBUG_W(format, ...) \
    FT_DEBUG_PRINT_W(FPCIE_DMA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FPCIE_DMA_DEBUG_E(format, ...) \
    FT_DEBUG_PRINT_E(FPCIE_DMA_DEBUG_TAG, format, ##__VA_ARGS__)


/* end */
/**
 * @name: FPcieCDmaStatusGet
 * @msg: Gets the status of a DMA channel.
 * @param {FPcieC *} instance_p - Pointer to the FPcieC instance.
 * @param {u32} channel_index - DMA channel index.
 * @return {u32} - The status of the DMA channel.
 */
u32 FPcieCDmaStatusGet(FPcieC *instance_p, u32 channel_index)
{
    uintptr_t dma_engine_base = 0;
    FASSERT(instance_p != NULL);
    dma_engine_base = instance_p->config.dma_engine_base;
    FASSERT(dma_engine_base != (uintptr_t)NULL);
    FASSERT(channel_index + 1 < instance_p->config.dma_max_num);

    return FPCIEC_HW_READW(dma_engine_base, FPCIEC_DMA_CH_ENGINE_OFFSET(channel_index) +
                                                FPCIEC_DMA_STATUS_OFFSET);
}


/* processing */

/* 使能发送 */
/**
 * @name: FPcieCDmaStart
 * @msg: Starts a DMA transfer on a specific channel.
 * @param {FPcieC *} instance_p - Pointer to the FPcieC instance.
 * @param {u32} channel_index - DMA channel index.
 * @return {FError} - FT_SUCCESS if successful, error code otherwise.
 */
FError FPcieCDmaStart(FPcieC *instance_p, u32 channel_index)
{
    u32 reg;

    uintptr_t dma_engine_base = 0;
    FASSERT(instance_p != NULL);
    dma_engine_base = instance_p->config.dma_engine_base;
    FASSERT(dma_engine_base != (uintptr_t)NULL);
    FASSERT(channel_index + 1 < instance_p->config.dma_max_num);

    /* DMA_CONTROL */

    /* start transfer */
    reg = FPCIEC_HW_READW(dma_engine_base, FPCIEC_DMA_CH_ENGINE_OFFSET(channel_index) +
                                               FPCIEC_DMA_CONTROL_OFFSET);
    reg |= FPCIEC_DMA_START_MASK;
    FPCIEC_HW_WRITEW(dma_engine_base,
                     FPCIEC_DMA_CH_ENGINE_OFFSET(channel_index) + FPCIEC_DMA_CONTROL_OFFSET, reg);
    return FT_SUCCESS;
}

/* config */
/* direct 发送模式 */
/**
 * @name: FPcieCDmaDirectConfig
 * @msg: Configures a DMA channel for direct data transfer.
 * @param {FPcieC *} instance_p - Pointer to the FPcieC instance.
 * @param {u32} channel_index - DMA channel index.
 * @param {uintptr_t} src_addr - Source address.
 * @param {uintptr_t} dest_addr - Destination address.
 * @param {u32} length - Transfer length.
 * @param {u32} direct - Transfer direction (FPCIEC_DMA_TYPE_READ or FPCIEC_DMA_TYPE_WRITE).
 * @return {FError} - FT_SUCCESS if successful, error code otherwise.
 */
FError FPcieCDmaDirectConfig(FPcieC *instance_p, u32 channel_index, uintptr_t src_addr,
                             uintptr_t dest_addr, u32 length, u32 direct)
{
    u32 reg;
    uintptr_t dma_engine_base = 0;
    FASSERT(instance_p != NULL);
    dma_engine_base = instance_p->config.dma_engine_base;
    FASSERT(dma_engine_base != (uintptr_t)NULL);
    FASSERT(channel_index + 1 < instance_p->config.dma_max_num);

    /* DMA_CONTROL */
    reg = FPCIEC_HW_READW(dma_engine_base, FPCIEC_DMA_CONTROL_OFFSET +
                                               FPCIEC_DMA_CH_ENGINE_OFFSET(channel_index));

    reg &= ~(FPCIEC_DMA_START_MASK);        /* disable transfer */
    reg &= ~FPCIEC_DMA_ENABLE_SG_MODE_MASK; /* direct <=====> direct */

    reg &= ~FPCIEC_DMA_SG_ID_MASK;  /* pcie interface */
    reg &= ~FPCIEC_DMA_SG2_ID_MASK; /* pcie interface */
    reg &= ~FPCIEC_DMA_IRQ_MASK;    /* no irq */

    reg |= FPCIEC_DMA_IRQ_END_MASK | FPCIEC_DMA_IRQ_ERROR_MASK | FPCIEC_DMA_IRQ_AXI_STREAM_EOP_MASK;

    reg |= FPCIEC_DMA_IRQ_LOCAL_PROC_MASK;

    reg |= (FPCIEC_DMA_STOP_DMA_LENGTH_MASK | FPCIEC_DMA_STOP_AXI_STREAM_EOP_MASK |
            FPCIEC_DMA_ABORT_ON_ERROR_MASK);


    FPCIEC_HW_WRITEW(dma_engine_base,
                     FPCIEC_DMA_CH_ENGINE_OFFSET(channel_index) + FPCIEC_DMA_CONTROL_OFFSET, reg);

    /* DMA_SRCADDR */
    FPCIEC_HW_WRITEW(dma_engine_base, FPCIEC_DMA_CH_ENGINE_OFFSET(channel_index) + FPCIEC_DMA_SRCADDR_UPPER32_OFFSET,
                     UPPER_32_BITS(src_addr));
    FPCIEC_HW_WRITEW(dma_engine_base, FPCIEC_DMA_CH_ENGINE_OFFSET(channel_index) + FPCIEC_DMA_SRCADDR_OFFSET,
                     LOWER_32_BITS(src_addr));

    FPCIEC_HW_WRITEW(dma_engine_base, FPCIEC_DMA_CH_ENGINE_OFFSET(channel_index) + FPCIEC_DMA_DMA_DESTADDR_OFFSET,
                     LOWER_32_BITS(dest_addr));
    FPCIEC_HW_WRITEW(dma_engine_base, FPCIEC_DMA_CH_ENGINE_OFFSET(channel_index) + FPCIEC_DMA_DMA_DESTADDR_UPEER32_OFFSET,
                     UPPER_32_BITS(dest_addr));

    FPCIEC_HW_WRITEW(dma_engine_base, FPCIEC_DMA_CH_ENGINE_OFFSET(channel_index) + FPCIEC_DMA_DMA_LENGTH_OFFSET,
                     length);

    if (FPCIEC_DMA_TYPE_READ == direct)
    {
        FPCIEC_HW_WRITEW(dma_engine_base, FPCIEC_DMA_CH_ENGINE_OFFSET(channel_index) + FPCIEC_DMA_SRCPARAM_OFFSET,
                         FPCIEC_DMA_SRCID_PCIE);
        FPCIEC_HW_WRITEW(dma_engine_base, FPCIEC_DMA_CH_ENGINE_OFFSET(channel_index) + FPCIEC_DMA_DMA_DESTPARAM_OFFSET,
                         (FPCIEC_DMA_DEST_ID_AXI_MASTER_X(0)));
    }
    else
    {

        FPCIEC_HW_WRITEW(dma_engine_base, FPCIEC_DMA_CH_ENGINE_OFFSET(channel_index) + FPCIEC_DMA_SRCPARAM_OFFSET,
                         (FPCIEC_DMA_SRC_ID_AXI_MASTER_X(0)));
        FPCIEC_HW_WRITEW(dma_engine_base, FPCIEC_DMA_CH_ENGINE_OFFSET(channel_index) + FPCIEC_DMA_DMA_DESTPARAM_OFFSET,
                         FPCIEC_DMA_SRCID_PCIE);
    }
    return FT_SUCCESS;
}