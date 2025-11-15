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
 * FilePath: fxmac_msg_g.c
 * Date: 2024-10-28 14:46:52
 * LastEditTime: 2024-10-28 14:46:52
 * Description:  This file provide a template for user to define their own hardware settings.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huangjin   2024/10/28  first release
 */

#include "fxmac_msg.h"
#include "fparameters.h"
#include "ftypes.h"
/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Prototypes ******************************/

/*
 * The configuration table for emacps device
 */

const FXmacMsgConfig fxmac_msg_cfg_tbl[FXMAC_MSG_NUM] =
{
    [FXMAC0_MSG_ID] =
    {
        .instance_id = (u32)FXMAC0_MSG_ID,
        .msg =
        {
            .regfile = FXMAC0_MSG_REGFILE_ADDR,
            .shmem = FXMAC0_MSG_SHARE_MEM_ADDR
        },
        .interface = FXMAC_MSG_PHY_INTERFACE_MODE_SGMII,
        .speed = 1000,
        .duplex = 1,
        .auto_neg = 1,
        .max_queue_num = 16,
        .tx_queue_id = 0,
        .rx_queue_id = 0,
        .dma_brust_length = 16,
        .network_default_config = FXMAC_MSG_DEFAULT_OPTIONS,
        .queue_irq_num =
        {
            FXMAC0_MSG_QUEUE0_IRQ_NUM,
            FXMAC0_MSG_QUEUE1_IRQ_NUM,
            FXMAC0_MSG_QUEUE2_IRQ_NUM,
            FXMAC0_MSG_QUEUE3_IRQ_NUM,
#if defined(FXMAC0_MSG_QUEUE4_IRQ_NUM)
            FXMAC0_MSG_QUEUE4_IRQ_NUM,
#endif
#if defined(FXMAC0_MSG_QUEUE5_IRQ_NUM)
            FXMAC0_MSG_QUEUE5_IRQ_NUM,
#endif
#if defined(FXMAC0_MSG_QUEUE6_IRQ_NUM)
            FXMAC0_MSG_QUEUE6_IRQ_NUM,
#endif
#if defined(FXMAC0_MSG_QUEUE7_IRQ_NUM)
            FXMAC0_MSG_QUEUE7_IRQ_NUM
#endif
        },
#if defined(FXMAC_MSG_CAPS)
       .caps = FXMAC_MSG_CAPS ,
#endif
    }
};
