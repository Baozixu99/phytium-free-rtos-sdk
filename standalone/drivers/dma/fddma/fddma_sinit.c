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
 * FilePath: fddma_sinit.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 08:24:47
 * Description:  This file is for static initialization
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/5/13    init commit
 */

/***************************** Include Files *********************************/
#include "fparameters.h"
#include "fassert.h"

#include "fddma_hw.h"
#include "fddma.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
extern const FDdmaConfig fddma_cfg_tbl[FDDMA_INSTANCE_NUM];
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
/**
 * @name: FDdmaLookupConfig
 * @msg: 获取DDMA实例默认配置
 * @return {const FDdmaConfig *} DDMA控制器默认配置
 * @param {u32} instance_id, DDMA实例号
 */
const FDdmaConfig *FDdmaLookupConfig(u32 instance_id)
{
    const FDdmaConfig *ptr = NULL;
    u32 index;

    for (index = 0; index < (u32)FDDMA_INSTANCE_NUM; index++)
    {
        if (fddma_cfg_tbl[index].id == instance_id)
        {
            ptr = &fddma_cfg_tbl[index];
            break;
        }
    }

    return (const FDdmaConfig *)ptr;
}