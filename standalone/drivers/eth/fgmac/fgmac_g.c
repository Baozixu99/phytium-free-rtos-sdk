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
 * FilePath: fgmac_g.c
 * Date: 2022-04-06 14:46:52
 * LastEditTime: 2022-04-06 14:46:58
 * Description:  This file is for configuration table that specifies the configuration of
 * ethernet devices .
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe    2021/07/13    first release
 */

/*  - This file contains a configuration table that specifies the configuration
- 驱动全局变量定义，包括静态配置参数 */

/***************************** Include Files *********************************/

#include "fparameters.h"
#include "fgmac.h"
#include "fgmac_phy.h"

/************************** Constant Definitions *****************************/

const FGmacConfig FGMAC_CONFIG_TBL[FGMAC_NUM] =
{
    [FGMAC0_ID] =
    {
        .instance_id  = FGMAC0_ID,
        .base_addr = FGMAC0_BASE_ADDR,
        .irq_num = FGMAC0_IRQ_NUM,
        .irq_prority = 0,
        .cheksum_mode = FGMAC_CHECKSUM_BY_SOFTWARE,
        .max_packet_size = FGMAC_MAX_PACKET_SIZE,
        .mdc_clk_hz = FGMAC_PHY_MII_ADDR_CR_250_300MHZ,
    },

    [FGMAC1_ID] =
    {
        .instance_id  = FGMAC1_ID,
        .base_addr = FGMAC1_BASE_ADDR,
        .irq_num =  FGMAC1_IRQ_NUM,
        .irq_prority = 0,
        .cheksum_mode = FGMAC_CHECKSUM_BY_SOFTWARE,
        .max_packet_size = FGMAC_MAX_PACKET_SIZE,
        .mdc_clk_hz = FGMAC_PHY_MII_ADDR_CR_250_300MHZ,
    }
};

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
