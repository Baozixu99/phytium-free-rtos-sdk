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
 * FilePath: e1000e_g.c
 * Date: 2025-01-02 14:53:42
 * LastEditTime: 2025-01-02 08:25:09
 * Description:  This file is for e1000e driver static config
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huangjin   2025/01/02    first release
 */


/*  - This file contains a configuration table that specifies the configuration
- 驱动全局变量定义，包括静态配置参数 */

/***************************** Include Files *********************************/

#include "ftypes.h"
#include "fparameters.h"
#include "e1000e.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
/*
 * The configuration table for e1000e device
 */
FE1000EConfig FE1000E_CONFIG_TBL[FE1000E_NUM] =
{
    {
        .instance_id = 0, /* Id of device*/
        .base_addr = 0,
        .interface = FE1000E_PHY_INTERFACE_MODE_SGMII,
        .speed = 1000,
        .duplex = 1,
        .auto_neg = 1,
    },
};


/*****************************************************************************/
