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
 * FilePath: fgdma_g.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 08:25:09
 * Description:  This file is for static variables definition
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe    2021/11/5    init commit
 * 1.1   zhugengyu  2022/5/16    modify according to tech manual.
 */

/***************************** Include Files *********************************/
#include "ftypes.h"
#include "fparameters.h"

#include "fgdma.h"
#include "fgdma_hw.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
const FGdmaConfig fgdma_cfg_tbl[FGDMA_INSTANCE_NUM] =
{
    [FGDMA0_ID] =
    {
        .instance_id = FGDMA0_ID,
        .irq_num ={
                    FGDMA0_CHANNEL0_IRQ_NUM,
                    #if defined(FGDMA0_CHANNEL1_IRQ_NUM)
                    FGDMA0_CHANNEL1_IRQ_NUM,
                    #endif
                    #if defined(FGDMA0_CHANNEL2_IRQ_NUM)
                    FGDMA0_CHANNEL2_IRQ_NUM,
                    #endif
                    #if defined(FGDMA0_CHANNEL3_IRQ_NUM)
                    FGDMA0_CHANNEL3_IRQ_NUM,
                    #endif
                    #if defined(FGDMA0_CHANNEL4_IRQ_NUM)
                    FGDMA0_CHANNEL4_IRQ_NUM,
                    #endif
                    #if defined(FGDMA0_CHANNEL5_IRQ_NUM)
                    FGDMA0_CHANNEL5_IRQ_NUM,
                    #endif
                    #if defined(FGDMA0_CHANNEL6_IRQ_NUM)
                    FGDMA0_CHANNEL6_IRQ_NUM,
                    #endif
                } , 
        .irq_prority = 0,
        .caps = FGDMA0_CAPACITY,
        .base_addr = FGDMA0_BASE_ADDR,
        .rd_qos = FGDMA_OPER_NONE_PRIORITY_POLL,
        .wr_qos = FGDMA_OPER_NONE_PRIORITY_POLL,
        .wait_mode = FGDMA_WAIT_INTR
    }
};


/*****************************************************************************/
