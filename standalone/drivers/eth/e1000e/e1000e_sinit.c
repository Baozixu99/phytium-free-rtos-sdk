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
 * FilePath: e1000e_sinit.c
 * Date: 2025-01-02 14:53:42
 * LastEditTime: 2025-01-02 08:25:15
 * Description:  This file is for e1000e driver static init
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huangjin   2025/01/02    first release
 */


/* - This file contains the implementation of driver's static initialization functionality.
- 驱动静态初始化  */

/***************************** Include Files *********************************/

#include "ftypes.h"
#include "fparameters.h"
#include "e1000e.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

extern FE1000EConfig FE1000E_CONFIG_TBL[FE1000E_NUM];

/************************** Function Prototypes ******************************/
/**
 * @name: FE1000ELookupConfig
 * @msg: 获取E1000E驱动的默认配置参数
 * @return {FE1000EConfig *}, 驱动默认配置
 * @param {u32} instance_id, 驱动控制器号
 * @note 返回E1000E的默认配置，复制后修改配置
 *       需要确认当前平台支持输入的instance_id
 */
FE1000EConfig *FE1000ELookupConfig(u32 instance_id)
{
    FE1000EConfig *ptr = NULL;
    u32 index;

    for (index = 0; index < (u32)FE1000E_NUM; index++)
    {
        if (FE1000E_CONFIG_TBL[index].instance_id == instance_id)
        {
            ptr = &FE1000E_CONFIG_TBL[index];
            break;
        }
    }

    return (FE1000EConfig *)ptr;
}