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
 * FilePath: fspim_msg_g.c
 * Date: 2024-10-14 11:27:42
 * LastEditTime: 2024-10-24 17:46:03
 * Description:  This file is for providing spi msg basic information.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   liyilun   2024/10/24   first release
 */

/***************************** Include Files *********************************/

#include "ftypes.h"
#include "fparameters.h"
#include "fspim_msg_hw.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

const FSpiMsgConfig FSPI_MSG_CONFIG_TBL[FSPI_MSG_NUM] =
{
    [FSPI0_MSG_ID] =
    {
        .instance_id  = FSPI0_MSG_ID, /* Id of device*/
        .irq_num      = FSPI0_MSG_IRQ_NUM,
        .slave_dev_id = FSPI_MSG_SLAVE_DEV_0,
        .irq_priority  = 0,
        .clk_div      = FSPI_MSG_DEFAULT_CLK_DIV,
        .n_bytes      = FSPIM_1_BYTE,
        .mode         = FSPI_MSG_MODE_0, /* cpol=0, cpha=0 */
        .tmod         = FSPI_MSG_CTRL_TMOD_TR,
        .en_dma       = FALSE,

        .spi_msg =
        {
            .regfile = FSPI0_MSG_REGFILE_BASE_ADDR,
            .shmem = FSPI0_MSG_SHARE_MEM_BASE_ADDR
        }
    },

    [FSPI1_MSG_ID] =
    {
        .instance_id  = FSPI1_MSG_ID, /* Id of device*/
        .irq_num      = FSPI1_MSG_IRQ_NUM,
        .slave_dev_id = FSPI_MSG_SLAVE_DEV_0,
        .irq_priority  = 0,
        .clk_div      = FSPI_MSG_DEFAULT_CLK_DIV,
        .n_bytes      = FSPIM_1_BYTE, /* 1 byte data width */
        .mode         = FSPI_MSG_MODE_0, /* cpol=0, cpha=0 */
        .tmod         = FSPI_MSG_CTRL_TMOD_TR,
        .en_dma       = FALSE,
        .spi_msg =
        {
            .regfile = FSPI1_MSG_REGFILE_BASE_ADDR,
            .shmem = FSPI1_MSG_SHARE_MEM_BASR_ADDR
        }
    },
};