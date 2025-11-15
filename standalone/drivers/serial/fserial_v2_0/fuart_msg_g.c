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
 * FilePath: fuart_msg_g.c
 * Created Date: 2024-10-10 20:06:47
 * Last Modified: 2025-06-09 17:00:12
 * Description:  This file is for
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 */

/***************************** Include Files *********************************/

#include "fuart_msg.h"
#include "fparameters.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

const FUartMsgConfig FUartMsgConfigTable[FUART_MSG_NUM] =
{
    {
        .instance_id = FUART0_MSG_ID,
        .msg =
        {
            .regfile = FUART0_MSG_REGFILE_BASE,
            .shmem = FUART0_MSG_SHARE_MEM_ADDR
        },
        .irq_num = FUART0_MSG_IRQ_NUM,
        .baudrate = FUART_MSG_DEFAULT_BAUDRATE,
        .ref_clk_freq = FUART0_MSG_CLK_FREQ_HZ
    },
#if defined(FUART1_MSG_ID)
    {
        .instance_id = FUART1_MSG_ID,
        .msg =
        {
            .regfile = FUART1_MSG_REGFILE_BASE,
            .shmem = FUART1_MSG_SHARE_MEM_ADDR
        },
        .irq_num = FUART1_MSG_IRQ_NUM,
        .baudrate = FUART_MSG_DEFAULT_BAUDRATE,
        .ref_clk_freq = FUART1_MSG_CLK_FREQ_HZ
    },
#endif
    {
        .instance_id = FUART2_MSG_ID,
        .msg =
        {
            .regfile = FUART2_MSG_REGFILE_BASE,
            .shmem = FUART2_MSG_SHARE_MEM_ADDR
        },
        .irq_num = FUART2_MSG_IRQ_NUM,
        .baudrate = FUART_MSG_DEFAULT_BAUDRATE,
        .ref_clk_freq = FUART2_MSG_CLK_FREQ_HZ
    },
};
