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
 * FilePath: fsdif_g.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 08:37:44
 * Description:  This file is for static init
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2021/12/2    init
 * 1.1   zhugengyu  2022/6/6     modify according to tech manual.
 * 2.0   zhugengyu  2023/9/16    rename as sdif, support SD 3.0 and rework clock timing
 * 2.1   huangjin   2023/12/22   modify according to tech manual
 */

/***************************** Include Files *********************************/

#include "ftypes.h"
#include "fparameters.h"
#include "fsdif.h"
#include "fsdif_hw.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

const FSdifConfig FSDIF_CONFIG_TBL[FSDIF_NUM] =
{
    [FSDIF0_ID] =
    {
        .instance_id  = FSDIF0_ID,
        .base_addr    = FSDIF0_BASE_ADDR,
        .irq_num      = FSDIF0_IRQ_NUM,
        .trans_mode   = FSDIF_IDMA_TRANS_MODE,
        .non_removable  = FALSE,
        .src_clk_rate = FSDIF_CLK_FREQ_HZ,
    },

#ifdef FSDIF1_ID
    [FSDIF1_ID] =
    {
        .instance_id = FSDIF1_ID,
        .base_addr   = FSDIF1_BASE_ADDR,
        .irq_num     = FSDIF1_IRQ_NUM,
        .trans_mode  = FSDIF_IDMA_TRANS_MODE,
        .non_removable  = FALSE,
        .src_clk_rate = FSDIF_CLK_FREQ_HZ,
    }
#endif
};


/*****************************************************************************/
