/*
 * Copyright (C) 2023, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: fi2s_sinit.c
 * Created Date: 2023-06-01 16:46:27
 * Last Modified: 2023-06-13 17:27:07
 * Description:  This file is for
 * 
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------  -------- --------------------------------------
 *  1.0  Wangzq     2023/05/09  Modify the format and establish the version
 */

/***************************** Include Files *********************************/

#include "ftypes.h"
#include "fparameters.h"
#include "fi2s.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

extern const FI2sConfig FI2S_CONFIG_TBL[FI2S_NUM];
/************************** Function Prototypes ******************************/
/**
 * @name: FI2sLookupConfig
 * @msg: 获取I2C驱动的默认配置参数
 * @return {const FI2sConfig*} 驱动默认参数
 * @param {u32} instance_id, 当前控制的I2S控制器实例号
 */
const FI2sConfig *FI2sLookupConfig(u32 instance_id)
{
    const FI2sConfig *ptr = NULL;
    u32 index;

    for (index = 0; index < (u32)FI2S_NUM; index++)
    {
        if (FI2S_CONFIG_TBL[index].instance_id == instance_id)
        {
            ptr = &FI2S_CONFIG_TBL[index];
            break;
        }
    }

    return (const FI2sConfig *)ptr;
}
