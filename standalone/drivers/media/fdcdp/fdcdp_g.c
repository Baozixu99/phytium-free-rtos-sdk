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
 * FilePath: fdcdp_g.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 08:28:45
 * Description:  This files is for the dcdp default configuration
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   wangzq  2022/5/26  first release
 */

#include "fdc.h"
#include "fdp.h"
#include "fparameters.h"

const FDcConfig fdc_config[FDC_INSTANCE_NUM] =
{
    [FDCDP_ID0] =
    {
        .instance_id = FDCDP_ID0,                  /* DC id */
        .dcch_base_addr = FDC0_CHANNEL_BASE_OFFSET, /* DC channel register address*/
        .dcctrl_base_addr = FDC_CTRL_BASE_OFFSET,   /* DC control register address */
        .irq_num = FDCDP_IRQ_NUM,                   /* Device interrupt id */
    },
    [FDCDP_ID1] =
    {
        .instance_id = FDCDP_ID1,                  /* DC id */
        .dcch_base_addr = FDC1_CHANNEL_BASE_OFFSET, /* DC channel register address*/
        .dcctrl_base_addr = FDC_CTRL_BASE_OFFSET,   /* DC control register address */
        .irq_num = FDCDP_IRQ_NUM,            /* Device interrupt id */
    }
};

const FDpConfig fdp_config[FDP_INSTANCE_NUM] =
{
    [FDCDP_ID0] =
    {
        .instance_id = FDCDP_ID0,                 /* DP id */
        .dp_channe_base_addr = FDP0_CHANNEL_BASE_OFFSET, /* DP channel register address*/
        .dp_phy_base_addr = FDP0_PHY_BASE_OFFSET,    /* DP phy register address */
        .irq_num = FDCDP_IRQ_NUM,           /* Device interrupt id */
    },

    [FDCDP_ID1] =
    {
        .instance_id = FDCDP_ID1,                 /* DP id */
        .dp_channe_base_addr = FDP1_CHANNEL_BASE_OFFSET, /* DP channel register address*/
        .dp_phy_base_addr = FDP1_PHY_BASE_OFFSET,    /* DP phy register address */
        .irq_num = FDCDP_IRQ_NUM,           /* Device interrupt id */
    }

};
