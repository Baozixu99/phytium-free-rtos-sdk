/*
 * Copyright (C) 2021, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: fi3c_sinit.c
 * Date: 2021-11-01 14:53:42
 * LastEditTime: 2022-02-18 08:36:52
 * Description:  This file is for implementation of driver's static initialization functionality
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
* 1.0  zhangyan     2023/9/11  first commit
 */
/***************************** Include Files *********************************/

#include "ftypes.h"
#include "fparameters.h"
#include "fi3c.h"
#include "sdkconfig.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

extern const FI3cConfig FI3C_CONFIG_TBL[FI3C_NUM];
/************************** Function Prototypes ******************************/
/**
 * @name: FI3cLookupConfig
 * @msg: 获取I3C驱动的默认配置参数
 * @return {const FI3cConfig*} 驱动默认参数
 * @param {u32} instance_id, 当前控制的I3C控制器实例号
 */
const FI3cConfig *FI3cLookupConfig(u32 instance_id)
{
    const FI3cConfig *ptr = NULL;
    u32 index;

    for (index = 0; index < (u32)FI3C_NUM; index++)
    {
        if (FI3C_CONFIG_TBL[index].instance_id == instance_id)
        {
            ptr = &FI3C_CONFIG_TBL[index];
            break;
        }
    }

    return (const FI3cConfig *)ptr;
}
