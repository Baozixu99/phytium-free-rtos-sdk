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
 * FilePath: fgmac_sinit.c
 * Date: 2022-04-06 14:46:52
 * LastEditTime: 2022-04-06 14:46:58
 * Description:  This file contains lookup method by device ID when success, it returns
 * pointer to config table to be used to initialize the device.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe    2021/07/13    first release
 */


/* - This file contains the implementation of driver's static initialization functionality.
- 驱动静态初始化  */

/***************************** Include Files *********************************/

#include "ftypes.h"
#include "fparameters.h"
#include "fgmac.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

extern const FGmacConfig FGMAC_CONFIG_TBL[FGMAC_NUM];

/************************** Function Prototypes ******************************/
/**
 * @name: FGmacLookupConfig
 * @msg: 获取FGMAC驱动的默认配置参数
 * @return {const FGmacConfig *}, 驱动默认配置
 * @param {u32} instance_id, 驱动控制器号
 * @note 返回FGMAC的默认配置，复制后修改配置
 *       需要确认当前平台支持输入的instance_id
 */
const FGmacConfig *FGmacLookupConfig(u32 instance_id)
{
    const FGmacConfig *ptr = NULL;
    u32 index;

    for (index = 0; index < (u32)FGMAC_NUM; index++)
    {
        if (FGMAC_CONFIG_TBL[index].instance_id == instance_id)
        {
            ptr = &FGMAC_CONFIG_TBL[index];
            break;
        }
    }

    return (const FGmacConfig *)ptr;
}