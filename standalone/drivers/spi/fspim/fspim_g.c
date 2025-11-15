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
 * FilePath: fspim_g.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 09:07:55
 * Description:  This file is for providing spim basic information.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2021/12/3   init commit
 * 1.1   zhugengyu  2022/4/15   support test mode
 */

/***************************** Include Files *********************************/

#include "ftypes.h"
#include "fparameters.h"

#include "fspim.h"
#include "fspim_hw.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

const FSpimConfig FSPIM_CONFIG_TBL[FSPI_NUM] =
{
    [FSPI0_ID] =
    {
        .instance_id  = FSPI0_ID, /* Id of device*/
        .base_addr    = FSPI0_BASE_ADDR,
        .irq_num      = FSPI0_IRQ_NUM,
        .irq_prority  = 0,
        .work_mode    = FSPIM_DEV_MASTER_MODE,
        .slave_dev_id = FSPIM_SLAVE_DEV_0,
        .sclk_hz     = FSPI_DEFAULT_SCLK,
        .n_bytes      = 1,
        .en_test      = FALSE,
        .en_dma       = FALSE,
        .caps         = FSPIM0_DMA_CAPACITY,
        .tx_fifo_threshold  = 0,
        .rx_fifo_threshold  = 0,
        .rx_dma_level       = FSPIM_RX_DMA_LEVEL,
        .tx_dma_level       = FSPIM_TX_DMA_LEVEL,

    },
    [FSPI1_ID] =
    {
        .instance_id  = FSPI1_ID, /* Id of device*/
        .base_addr    = FSPI1_BASE_ADDR,
        .irq_num      = FSPI1_IRQ_NUM,
        .irq_prority  = 0,
        .work_mode    = FSPIM_DEV_MASTER_MODE,
        .slave_dev_id = FSPIM_SLAVE_DEV_0,
        .sclk_hz      = FSPI_DEFAULT_SCLK,
        .n_bytes      = 1,
        .en_test      = FALSE,
        .en_dma       = FALSE,
        .caps         = FSPIM1_DMA_CAPACITY,
        .tx_fifo_threshold  = 0,
        .rx_fifo_threshold  = 0,
        .rx_dma_level       = FSPIM_RX_DMA_LEVEL,
        .tx_dma_level       = FSPIM_TX_DMA_LEVEL,
    },
#if defined(FSPI2_ID)
    [FSPI2_ID] =
    {
        .instance_id  = FSPI2_ID, /* Id of device*/
        .base_addr    = FSPI2_BASE_ADDR,
        .irq_num      = FSPI2_IRQ_NUM,
        .irq_prority  = 0,
        .work_mode    = FSPIM_DEV_MASTER_MODE,
        .slave_dev_id = FSPIM_SLAVE_DEV_0,
        .sclk_hz      = FSPI_DEFAULT_SCLK,
        .n_bytes      = 1,
        .en_test      = FALSE,
        .en_dma       = FALSE,
        .caps         = FSPIM2_DMA_CAPACITY,
        .tx_fifo_threshold  = 0,
        .rx_fifo_threshold  = 0,
        .rx_dma_level       = FSPIM_RX_DMA_LEVEL,
        .tx_dma_level       = FSPIM_TX_DMA_LEVEL,
    },
#endif

#if defined(FSPI3_ID)
    [FSPI3_ID] =
    {
        .instance_id  = FSPI3_ID, /* Id of device*/
        .base_addr    = FSPI3_BASE_ADDR,
        .irq_num      = FSPI3_IRQ_NUM,
        .irq_prority  = 0,
        .work_mode    = FSPIM_DEV_MASTER_MODE,
        .slave_dev_id = FSPIM_SLAVE_DEV_0,
        .sclk_hz      = FSPI_DEFAULT_SCLK,
        .n_bytes      = 1,
        .en_test      = FALSE,
        .en_dma       = FALSE,
        .caps         = FSPIM3_DMA_CAPACITY,
        .tx_fifo_threshold  = 0,
        .rx_fifo_threshold  = 0,
        .rx_dma_level       = FSPIM_RX_DMA_LEVEL,
        .tx_dma_level       = FSPIM_TX_DMA_LEVEL,
    },
#endif
};


/*****************************************************************************/
