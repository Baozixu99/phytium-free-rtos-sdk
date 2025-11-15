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
 * FilePath: fi2c_g.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 08:36:14
 * Description:  This file is for I2C static configuration
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  zhugengyu 2021/11/1  first commit
 * 1.1  liushengming 2022/02/18  add e2000 configs
 */


/*  - This file contains a configuration table that specifies the configuration
- 驱动全局变量定义，包括静态配置参数 */

/***************************** Include Files *********************************/

#include "ftypes.h"
#include "fparameters.h"
#include "fi2c.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
/**
 * @name: FI2C_CONFIG_TBL
 * @msg: I2C驱动的默认配置参数
 */
#if defined(FI2C_NUM)
const FI2cConfig FI2C_CONFIG_TBL[FI2C_NUM] =
{
    [FI2C0_ID] =
    {
        .instance_id = FI2C0_ID,
        .base_addr = FI2C0_BASE_ADDR,
        .irq_num = FI2C0_IRQ_NUM,
        .irq_prority = 0,
        .ref_clk_hz = FI2C_CLK_FREQ_HZ,
        .work_mode = FI2C_MASTER,
        .slave_addr = 0,
        .use_7bit_addr = TRUE,
        .speed_rate = FI2C_SPEED_STANDARD_RATE,
        .auto_calc = TRUE
    },
#if defined(FI2C1_ID)
    [FI2C1_ID] =
    {
        .instance_id  = FI2C1_ID,
        .base_addr = FI2C1_BASE_ADDR,
        .irq_num  = FI2C1_IRQ_NUM,
        .irq_prority = 0,
        .ref_clk_hz = FI2C_CLK_FREQ_HZ,
        .work_mode = FI2C_MASTER,
        .slave_addr = 0,
        .use_7bit_addr = TRUE,
        .speed_rate = FI2C_SPEED_STANDARD_RATE,
        .auto_calc = TRUE
    },
#endif

#if defined(FI2C2_ID)

    [FI2C2_ID] =
    {
        .instance_id  = FI2C2_ID,
        .base_addr = FI2C2_BASE_ADDR,
        .irq_num  = FI2C2_IRQ_NUM,
        .irq_prority = 0,
        .ref_clk_hz = FI2C_CLK_FREQ_HZ,
        .work_mode = FI2C_MASTER,
        .slave_addr = 0,
        .use_7bit_addr = TRUE,
        .speed_rate = FI2C_SPEED_STANDARD_RATE,
        .auto_calc = TRUE
    },
#endif     

#if defined(FI2C3_ID)
    [FI2C3_ID] =
    {
        .instance_id  = FI2C3_ID,
        .base_addr = FI2C3_BASE_ADDR,
        .irq_num  = FI2C3_IRQ_NUM,
        .irq_prority = 0,
        .ref_clk_hz = FI2C_CLK_FREQ_HZ,
        .work_mode = FI2C_MASTER,
        .slave_addr = 0,
        .use_7bit_addr = TRUE,
        .speed_rate = FI2C_SPEED_STANDARD_RATE,
        .auto_calc = TRUE
    }
#endif    
};
#endif