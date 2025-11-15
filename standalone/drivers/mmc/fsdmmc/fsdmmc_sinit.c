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
 * FilePath: fsdmmc_sinit.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 08:55:09
 * Description:  This file contains the implementation of the fsdmmc driver's static
 * initialization functionality.
 * 
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2021/12/2    init
 */


/* - This file contains the implementation of driver's static initialization functionality.
- 驱动静态初始化  */

/***************************** Include Files *********************************/

#include "ftypes.h"
#include "fparameters.h"

#include "fsdmmc.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

extern const FSdmmcConfig FSDMMC_CONFIG_TBL[FSDMMC_NUM];

/************************** Function Prototypes ******************************/
/**
 * @name: FSdmmcLookupConfig
 * @msg: 获取FSDMMC控制器默认配置
 * @return {const FSdmmcConfig *}  FSDMMC默认配置，返回NULL如果找不到默认配置
 * @param {u32} instance_id 驱动控制器ID
 * @note instance_id从0开始，取决于FSDMMC控制器的个数
 */
const FSdmmcConfig *FSdmmcLookupConfig(u32 instance_id)
{
    const FSdmmcConfig *ptr = NULL;
    u32 index;

    for (index = 0; index < (u32)FSDMMC_NUM; index++)
    {
        if (FSDMMC_CONFIG_TBL[index].instance_id == instance_id)
        {
            ptr = &FSDMMC_CONFIG_TBL[index];
            break;
        }
    }

    return (const FSdmmcConfig *)ptr;
}